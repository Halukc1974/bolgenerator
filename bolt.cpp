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

  body = selected;
}

TopoDS_Solid Bolt::Solid() { return body; }

TopoDS_Solid Bolt::Shank() {
  TopoDS_Solid mask, shank, thread;
  gp_Trsf offset;

  double d = params.thread.majorDiameter;
  double p = params.thread.pitch;
  double L = params.shank.totalLength;

  // Ensure body tolerance is applied if specified
  double shankCap = d - params.shank.bodyTolerance;

  double nthreads = std::ceil((L + 2 * p) / p);

  // Base cylinder (shank)
  shank = BRepPrimAPI_MakeCylinder(0.5 * shankCap, p * nthreads);

  // Threads
  // Minor diameter is roughly major - 1.082 * pitch for 60 deg threads
  double minorD = (params.thread.minorDiameter > 0)
                      ? params.thread.minorDiameter
                      : (d - 1.08 * p);
  thread = Thread(minorD, p, p * nthreads);
  shank = Cut(shank, thread);

  // Head-side cleanup (create unthreaded grip length if requested)
  if (params.shank.gripLength > 0) {
    // Cut threads from the grip section and replace with solid cylinder
    gp_Trsf gripOffset;
    gripOffset.SetTranslation(
        gp_Vec(0.0, 0.0, p * nthreads - params.shank.gripLength));
    TopoDS_Solid gripMask =
        BRepPrimAPI_MakeCylinder(d, params.shank.gripLength).Solid();
    shank = Cut(shank, BRepBuilderAPI_Transform(gripMask, gripOffset).Shape());

    TopoDS_Solid gripCylinder =
        BRepPrimAPI_MakeCylinder(0.5 * shankCap, params.shank.gripLength)
            .Solid();
    BRepAlgoAPI_Fuse gripFuse(
        shank, BRepBuilderAPI_Transform(gripCylinder, gripOffset).Shape());
    gripFuse.Build();
    shank = TopoDS::Solid(gripFuse.Shape());
  }

  // Top and bottom cleanup
  mask = BRepPrimAPI_MakeCylinder(d, p).Solid();
  shank = Cut(shank, mask);

  offset.SetTranslation(gp_Vec(0.0, 0.0, L + p));
  mask = BRepPrimAPI_MakeCylinder(d, nthreads * p - p - L).Solid();
  shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

  // Shift down to align base at z=0
  offset.SetTranslation(gp_Vec(0.0, 0.0, -p));
  shank = TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset));

  // End Chamfer
  double x[] = {0.5 * d - p, d};
  double z[] = {L, L - 0.5 * d - p};
  std::vector<gp_Pnt> points = {gp_Pnt(x[0], 0.0, z[0]),
                                gp_Pnt(x[1], 0.0, z[0]),
                                gp_Pnt(x[1], 0.0, z[1])};
  shank = Cut(shank, Chamfer(points));

  return shank;
}

TopoDS_Solid Bolt::Head() {
  TopoDS_Solid head;
  double s = params.head.widthAcrossFlats;
  double k = params.head.height;

  if (params.head.type == HeadType::HEX) {
    head = Hexagon(s, k);
  } else if (params.head.type == HeadType::SOCKET_CAP) {
    head = BRepPrimAPI_MakeCylinder(0.5 * params.head.widthAcrossFlats, k);
    TopoDS_Solid socket =
        Hexagon(params.head.socketSize, params.head.socketDepth);
    gp_Trsf socketOffset;
    socketOffset.SetTranslation(gp_Vec(0.0, 0.0, k - params.head.socketDepth));
    head = Cut(head, BRepBuilderAPI_Transform(socket, socketOffset).Shape());
  } else if (params.head.type == HeadType::FLAT ||
             params.head.type == HeadType::COUNTERSUNK) {
    head = BRepPrimAPI_MakeCylinder(0.5 * s, k);
  } else {
    head = Hexagon(s, k);
  }

  // Add Washer Face if specified
  if (params.head.washerFaceDiameter > 0 &&
      params.head.washerFaceThickness > 0) {
    TopoDS_Solid washer = BRepPrimAPI_MakeCylinder(
        0.5 * params.head.washerFaceDiameter, params.head.washerFaceThickness);
    // Washer face is usually at the bottom of the head
    BRepAlgoAPI_Fuse washerFuse(head, washer);
    washerFuse.Build();
    head = TopoDS::Solid(washerFuse.Shape());
  }

  return head;
}
