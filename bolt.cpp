#define _USE_MATH_DEFINES
#include "bolt.h"
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GProp_GProps.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <cmath>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <iostream>
#include <stdexcept>
#include <vector>

Bolt::Bolt(const BoltParameters &p) : params(p) {
  TopoDS_Solid shank = Shank();

  // Rotate shank 180 degrees around X axis to point chamfered end down
  gp_Trsf rotateShank;
  rotateShank.SetRotation(
      gp_Ax1(gp_Pnt(0.0, 0.0, 0.5 * params.shank.totalLength),
             gp_Dir(1.0, 0.0, 0.0)),
      M_PI);
  shank = TopoDS::Solid(BRepBuilderAPI_Transform(shank, rotateShank).Shape());

  TopoDS_Solid head = Head();

  // Place head just above the shank
  // Use overlap to ensure a clean boolean fuse
  const double fuseOverlap =
      (params.shank.totalLength > 0.2) ? 0.1 : 0.5 * params.shank.totalLength;
  gp_Trsf headPlacement;
  headPlacement.SetTranslation(
      gp_Vec(0.0, 0.0, params.shank.totalLength - fuseOverlap));
  TopoDS_Solid placedHead =
      TopoDS::Solid(BRepBuilderAPI_Transform(head, headPlacement).Shape());

  BRepAlgoAPI_Fuse fuseOp(shank, placedHead);
  fuseOp.Build();

  std::vector<TopoDS_Solid> fusedSolids;
  for (TopExp_Explorer map(fuseOp.Shape(), TopAbs_SOLID); map.More();
       map.Next()) {
    fusedSolids.push_back(TopoDS::Solid(map.Current()));
  }

  if (fusedSolids.empty()) {
    throw std::runtime_error("Bolt Fuse produced no solids");
  }

  // Select largest volume solid
  double maxVolume = -1.0;
  TopoDS_Solid selected = fusedSolids.front();
  for (const auto &solid : fusedSolids) {
    GProp_GProps props;
    BRepGProp::VolumeProperties(solid, props);
    double vol = props.Mass();
    if (vol > maxVolume) {
      maxVolume = vol;
      selected = solid;
    }
  }

  // Apply underhead fillet if radius > 0
  if (params.head.underheadFilletRadius > 0) {
    try {
      BRepFilletAPI_MakeFillet filler(selected);
      // We need to find the edge at the intersection of head and shank
      // For a cylinder, it's roughly at z = length - fuseOverlap
      const double targetZ = params.shank.totalLength - fuseOverlap;
      for (TopExp_Explorer ex(selected, TopAbs_EDGE); ex.More(); ex.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(ex.Current());
        GProp_GProps edgeProps;
        BRepGProp::LinearProperties(edge, edgeProps);
        gp_Pnt center = edgeProps.CentreOfMass();
        if (std::abs(center.Z() - targetZ) < 0.2) {
          filler.Add(params.head.underheadFilletRadius, edge);
        }
      }
      if (filler.IsDone()) {
        selected = TopoDS::Solid(filler.Shape());
      }
    } catch (...) {
      // Fillet might fail if radius is too large for geometry
    }
  }

  // Apply global edge fillet if radius > 0
  // SAFE FILLET ALGORITHM: Validate radius against geometry to prevent
  // corruption
  double filletRadius = params.shank.edgeFilletRadius;
  if (filletRadius > 0.01) {
    // Clamp fillet radius to safe maximum (10% of nominal diameter)
    double maxSafeRadius = params.thread.majorDiameter * 0.1;
    if (filletRadius > maxSafeRadius) {
      std::cout << "Fillet: Clamping radius from " << filletRadius
                << " to safe max " << maxSafeRadius << std::endl;
      filletRadius = maxSafeRadius;
    }

    try {
      std::cout << "Applying safe edge fillet radius: " << filletRadius
                << std::endl;
      BRepFilletAPI_MakeFillet fillet(selected);
      int edgesAdded = 0;

      for (TopExp_Explorer ex(selected, TopAbs_EDGE); ex.More(); ex.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(ex.Current());

        // Calculate edge length to validate if fillet is safe
        GProp_GProps edgeProps;
        BRepGProp::LinearProperties(edge, edgeProps);
        double edgeLength = edgeProps.Mass(); // Length of edge

        // Only apply fillet if edge is long enough (at least 4x the radius)
        if (edgeLength > filletRadius * 4.0) {
          fillet.Add(filletRadius, edge);
          edgesAdded++;
        }
      }

      std::cout << "Fillet: Added to " << edgesAdded << " edges" << std::endl;

      if (edgesAdded > 0) {
        fillet.Build();
        if (fillet.IsDone()) {
          selected = TopoDS::Solid(fillet.Shape());
          std::cout << "Edge fillet applied successfully" << std::endl;
        } else {
          std::cerr << "Fillet: Build incomplete, keeping original geometry"
                    << std::endl;
        }
      } else {
        std::cout << "Fillet: No suitable edges found, skipping" << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "Fillet failed (" << e.what()
                << "), keeping original geometry" << std::endl;
    } catch (...) {
      std::cerr << "Fillet failed (unknown), keeping original geometry"
                << std::endl;
    }
  }

  body = selected;
}

TopoDS_Solid Bolt::Solid() { return body; }

TopoDS_Solid Bolt::Shank() {
  double d = params.thread.majorDiameter;
  double p = params.thread.pitch;
  double L = params.shank.totalLength;
  double shankCap = d - params.shank.bodyTolerance;

  // Clamp grip length safely
  double ls = std::max(0.0, std::min(params.shank.gripLength, L - 3.0 * p));
  double threadedLength = L - ls;

  std::cout << "Shank: L=" << L << " ls=" << ls
            << " threadedL=" << threadedLength << std::endl;

  TopoDS_Solid result;

  if (threadedLength < p) {
    // If threaded section is too short, just make a plain cylinder
    std::cout << "Shank: Threaded section too short, making plain cylinder"
              << std::endl;
    result = BRepPrimAPI_MakeCylinder(0.5 * shankCap, L).Solid();
    return result;
  }

  // 1. Create the unthreaded grip section (if any)
  TopoDS_Solid gripPart;
  bool hasGrip = (ls > 0.1);

  if (hasGrip) {
    std::cout << "Shank: Creating grip section of length " << ls << std::endl;
    gripPart = BRepPrimAPI_MakeCylinder(0.5 * shankCap, ls).Solid();
  }

  // 2. Create the threaded section
  std::cout << "Shank: Creating threaded section of length " << threadedLength
            << std::endl;

  // Build threaded section longer than needed, then trim
  double buildLen = threadedLength + 4.0 * p;
  TopoDS_Solid threadedPart =
      BRepPrimAPI_MakeCylinder(0.5 * shankCap, buildLen).Solid();

  // Apply thread profile
  double minorD = (params.thread.minorDiameter > 0)
                      ? params.thread.minorDiameter
                      : (d - 1.0825 * p);
  TopoDS_Solid threadCutter = Thread(minorD, p, buildLen);
  threadedPart = Cut(threadedPart, threadCutter);

  // Trim to exact threaded length
  gp_Trsf trimTrans;
  trimTrans.SetTranslation(gp_Vec(0, 0, threadedLength));
  TopoDS_Solid trimMask =
      BRepPrimAPI_MakeCylinder(d * 2.0, buildLen + 10.0).Solid();
  threadedPart =
      Cut(threadedPart, BRepBuilderAPI_Transform(trimMask, trimTrans).Shape());

  // 3. Position threaded section after grip
  if (hasGrip) {
    gp_Trsf threadOffset;
    threadOffset.SetTranslation(gp_Vec(0, 0, ls));
    threadedPart = TopoDS::Solid(
        BRepBuilderAPI_Transform(threadedPart, threadOffset).Shape());

    // Fuse grip and threaded sections
    std::cout << "Shank: Fusing grip and threaded sections" << std::endl;
    BRepAlgoAPI_Fuse fuseOp(gripPart, threadedPart);
    fuseOp.Build();
    if (!fuseOp.IsDone()) {
      std::cerr << "Shank: Fuse failed, returning threaded part only"
                << std::endl;
      result = threadedPart;
    } else {
      // Extract single solid from fuse result
      TopExp_Explorer ex(fuseOp.Shape(), TopAbs_SOLID);
      if (ex.More()) {
        result = TopoDS::Solid(ex.Current());
      } else {
        result = threadedPart;
      }
    }
  } else {
    result = threadedPart;
  }

  // 4. End chamfer at the tip
  double cx[] = {0.5 * d - p, d};
  double cz[] = {L, L - 0.5 * d - p};
  std::vector<gp_Pnt> chamferPts = {gp_Pnt(cx[0], 0.0, cz[0]),
                                    gp_Pnt(cx[1], 0.0, cz[0]),
                                    gp_Pnt(cx[1], 0.0, cz[1])};
  result = Cut(result, Chamfer(chamferPts));

  std::cout << "Shank: Generation complete" << std::endl;
  return result;
}

TopoDS_Solid Bolt::Head() {
  TopoDS_Solid head;
  double s = params.head.widthAcrossFlats;
  double k = params.head.height;

  if (params.head.type == HeadType::HEX) {
    head = Hexagon(s, k);
  } else if (params.head.type == HeadType::SOCKET_CAP) {
    head =
        BRepPrimAPI_MakeCylinder(0.5 * params.head.widthAcrossFlats, k).Solid();
    TopoDS_Solid socket =
        Hexagon(params.head.socketSize, params.head.socketDepth);
    gp_Trsf socketOffset;
    socketOffset.SetTranslation(gp_Vec(0.0, 0.0, k - params.head.socketDepth));
    head = Cut(head, BRepBuilderAPI_Transform(socket, socketOffset).Shape());
  } else if (params.head.type == HeadType::FLAT ||
             params.head.type == HeadType::COUNTERSUNK) {
    head = BRepPrimAPI_MakeCylinder(0.5 * s, k).Solid();
  } else {
    head = Hexagon(s, k);
  }

  // Add Washer Face if specified
  if (params.head.washerFaceDiameter > 0 &&
      params.head.washerFaceThickness > 0) {
    TopoDS_Solid washer =
        BRepPrimAPI_MakeCylinder(0.5 * params.head.washerFaceDiameter,
                                 params.head.washerFaceThickness)
            .Solid();
    // Washer face is usually at the bottom of the head
    BRepAlgoAPI_Fuse washerFuse(head, washer);
    washerFuse.Build();
    head = TopoDS::Solid(washerFuse.Shape());
  }

  return head;
}
