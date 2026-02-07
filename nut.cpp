#include "nut.h"
#include "cut.h"
#include "hexagon.h"
#include "thread.h"
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Boolean.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <iostream>
#include <stdexcept>

Nut::Nut(const BoltParameters &p) : params(p) {
  double d = params.thread.majorDiameter;
  double p_pitch = params.thread.pitch;
  double h = params.nut.height;
  double s = params.nut.widthAcrossFlats;
  double tol = params.nut.tolerance;
  double threadClearance = params.nut.threadClearance;

  if (d <= 0 || p_pitch <= 0 || h <= 0 || s <= 0) {
    throw std::runtime_error("Nut: All dimensions must be positive");
  }

  std::cout << "Nut: Creating with d=" << d << " pitch=" << p_pitch
            << " height=" << h << " width=" << s << std::endl;

  // 1. Create hex outer shape
  TopoDS_Solid hexOuter = Hexagon(s, h);

  // Add Washer Face if specified
  if (params.nut.washerFaceDiameter > 0) {
    double wft = 0.5;
    TopoDS_Solid washer =
        BRepPrimAPI_MakeCylinder(0.5 * params.nut.washerFaceDiameter, wft)
            .Solid();
    BRepAlgoAPI_Fuse washerFuse(hexOuter, washer);
    washerFuse.Build();
    if (washerFuse.IsDone()) {
      hexOuter = TopoDS::Solid(washerFuse.Shape());
    }
  }

  // 2. Create the central hole (unthreaded bore)
  // The bore diameter should be slightly larger than the minor diameter
  // to allow the bolt threads to fit
  double minorD = (params.thread.minorDiameter > 0)
                      ? params.thread.minorDiameter
                      : (d - 1.0825 * p_pitch);

  // Add tolerance/clearance to the bore
  double boreRadius = 0.5 * minorD + tol + threadClearance;

  std::cout << "Nut: minorD=" << minorD << " boreRadius=" << boreRadius
            << std::endl;

  // Create bore hole that goes through the entire nut
  double overlap = 1.0;
  TopoDS_Solid bore =
      BRepPrimAPI_MakeCylinder(boreRadius, h + 2.0 * overlap).Solid();

  // Position bore to go through nut
  gp_Trsf boreTransform;
  boreTransform.SetTranslation(gp_Vec(0.0, 0.0, -overlap));
  BRepBuilderAPI_Transform borePos(bore, boreTransform, Standard_True);

  // Cut the bore from hex
  std::cout << "Nut: Cutting central bore..." << std::endl;
  TopoDS_Solid nutWithHole;
  try {
    nutWithHole = Cut(hexOuter, borePos.Shape());
  } catch (const std::exception &e) {
    std::cerr << "Nut: Bore cut failed: " << e.what() << std::endl;
    nutWithHole = hexOuter;
  }

  // 3. Create internal threads
  // For internal threads, we create a thread profile that will be added
  // (fused) to the inner surface of the nut hole
  std::cout << "Nut: Creating internal thread profile..." << std::endl;

  // Thread profile for internal threads
  // The thread needs to protrude inward from the bore wall
  double threadBuildLength = h + 2.0 * p_pitch;

  // Create the helical thread cutter/adder
  // For internal threads, we use the major diameter and cut inward
  TopoDS_Solid threadSolid = Thread(minorD, p_pitch, threadBuildLength);

  // Position the thread to align with the nut
  gp_Trsf threadTransform;
  threadTransform.SetTranslation(gp_Vec(0.0, 0.0, -p_pitch));
  BRepBuilderAPI_Transform threadPos(threadSolid, threadTransform,
                                     Standard_True);

  // Fuse the thread profile to create internal threads
  std::cout << "Nut: Adding internal thread ridges..." << std::endl;
  try {
    BRepAlgoAPI_Fuse threadFuse(nutWithHole, threadPos.Shape());
    threadFuse.Build();
    if (threadFuse.IsDone()) {
      // Extract the solid from fuse result
      for (TopExp_Explorer ex(threadFuse.Shape(), TopAbs_SOLID); ex.More();
           ex.Next()) {
        body = TopoDS::Solid(ex.Current());
        break;
      }
      std::cout << "Nut: Internal threads created successfully" << std::endl;
    } else {
      std::cerr << "Nut: Thread fuse failed, using nut without threads"
                << std::endl;
      body = nutWithHole;
    }
  } catch (const std::exception &e) {
    std::cerr << "Nut: Thread creation failed: " << e.what() << std::endl;
    body = nutWithHole;
  }

  // 4. Apply edge fillet for smooth edges
  double filletRadius = params.nut.edgeFilletRadius;
  if (filletRadius > 0.01) {
    double maxSafeRadius = s * 0.1;
    if (filletRadius > maxSafeRadius) {
      std::cout << "Nut Fillet: Clamping radius from " << filletRadius
                << " to safe max " << maxSafeRadius << std::endl;
      filletRadius = maxSafeRadius;
    }

    try {
      std::cout << "Nut: Applying safe edge fillet radius: " << filletRadius
                << std::endl;
      BRepFilletAPI_MakeFillet fillet(body);
      int edgesAdded = 0;

      for (TopExp_Explorer ex(body, TopAbs_EDGE); ex.More(); ex.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(ex.Current());

        GProp_GProps edgeProps;
        BRepGProp::LinearProperties(edge, edgeProps);
        double edgeLength = edgeProps.Mass();

        if (edgeLength > filletRadius * 4.0) {
          fillet.Add(filletRadius, edge);
          edgesAdded++;
        }
      }

      std::cout << "Nut Fillet: Added to " << edgesAdded << " edges"
                << std::endl;

      if (edgesAdded > 0) {
        fillet.Build();
        if (fillet.IsDone()) {
          body = TopoDS::Solid(fillet.Shape());
          std::cout << "Nut: Edge fillet applied successfully" << std::endl;
        } else {
          std::cerr << "Nut Fillet: Build incomplete, keeping original geometry"
                    << std::endl;
        }
      } else {
        std::cout << "Nut Fillet: No suitable edges found, skipping"
                  << std::endl;
      }
    } catch (...) {
      std::cerr << "Nut Fillet failed, keeping original geometry" << std::endl;
    }
  }

  std::cout << "Nut: Generation complete" << std::endl;
}

TopoDS_Solid Nut::Solid() { return body; }
