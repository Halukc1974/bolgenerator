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

  // 2. Create a threaded shaft (like a bolt shank) to subtract from the nut
  // This creates the internal female threads by boolean subtraction
  double overlap = 2.0;
  double cutterLength = h + 2.0 * overlap;

  // Calculate minor diameter for thread profile
  double minorD = (params.thread.minorDiameter > 0)
                      ? params.thread.minorDiameter
                      : (d - 1.0825 * p_pitch);

  std::cout << "Nut: majorD=" << d << " minorD=" << minorD
            << " cutterLength=" << cutterLength << std::endl;

  // Create a plain cylinder with the major diameter (+ clearance for the bolt
  // to fit)
  double shaftRadius = 0.5 * d + tol + threadClearance;
  TopoDS_Solid shaftCylinder =
      BRepPrimAPI_MakeCylinder(shaftRadius, cutterLength).Solid();

  // Create the helical thread profile to subtract from the shaft
  // This cuts the thread grooves into our cutter cylinder
  TopoDS_Solid threadCutter = Thread(minorD, p_pitch, cutterLength + p_pitch);

  // Position thread to start before the shaft
  gp_Trsf threadOffset;
  threadOffset.SetTranslation(gp_Vec(0.0, 0.0, -0.5 * p_pitch));
  BRepBuilderAPI_Transform threadPos(threadCutter, threadOffset, Standard_True);

  // Create the threaded shaft by cutting thread grooves from the cylinder
  std::cout << "Nut: Creating threaded shaft cutter..." << std::endl;
  TopoDS_Solid threadedShaft;
  try {
    threadedShaft = Cut(shaftCylinder, threadPos.Shape());
  } catch (...) {
    std::cerr << "Nut: Thread cut failed, using plain cylinder" << std::endl;
    threadedShaft = shaftCylinder;
  }

  // Position the threaded shaft to pass through the nut
  gp_Trsf shaftTransform;
  shaftTransform.SetTranslation(gp_Vec(0.0, 0.0, -overlap));
  BRepBuilderAPI_Transform shaftPos(threadedShaft, shaftTransform,
                                    Standard_True);

  // 3. Boolean subtract the threaded shaft from the hex to create internal
  // threads
  std::cout << "Nut: Cutting internal threads from hex body..." << std::endl;
  try {
    body = Cut(hexOuter, shaftPos.Shape());
    std::cout << "Nut: Internal threads created successfully" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Nut: Boolean cut failed: " << e.what() << std::endl;
    // Fallback: just cut a plain hole
    TopoDS_Solid plainHole =
        BRepPrimAPI_MakeCylinder(shaftRadius, cutterLength).Solid();
    gp_Trsf holeTransform;
    holeTransform.SetTranslation(gp_Vec(0.0, 0.0, -overlap));
    BRepBuilderAPI_Transform holePos(plainHole, holeTransform, Standard_True);
    body = Cut(hexOuter, holePos.Shape());
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
        }
      }
    } catch (...) {
      std::cerr << "Nut Fillet failed, keeping original geometry" << std::endl;
    }
  }

  std::cout << "Nut: Generation complete" << std::endl;
}

TopoDS_Solid Nut::Solid() { return body; }
