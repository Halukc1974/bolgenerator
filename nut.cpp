#include "nut.h"
#include "bolt.h"
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

  if (d <= 0 || p_pitch <= 0 || h <= 0 || s <= 0) {
    throw std::runtime_error("Nut: All dimensions must be positive");
  }

  // 1. Create hex outer shape
  TopoDS_Solid hexOuter = Hexagon(s, h);

  // Add Washer Face if specified
  if (params.nut.washerFaceDiameter > 0) {
    // Assume standard washer face thickness of 10% of nut height or 0.5mm
    double wft = 0.5;
    TopoDS_Solid washer =
        BRepPrimAPI_MakeCylinder(0.5 * params.nut.washerFaceDiameter, wft).Solid();
    BRepAlgoAPI_Fuse washerFuse(hexOuter, washer);
    washerFuse.Build();
    hexOuter = TopoDS::Solid(washerFuse.Shape());
  }

  // 2. Create cutter bolt
  // We use a simplified bolt as a cutter.
  // It MUST be significantly longer than the nut to ensure a hole is
  // through-cut.
  double overlap = 5.0;
  double cutterLength = h + 2.0 * overlap;

  BoltParameters cutterParams = params;
  cutterParams.shank.totalLength = cutterLength;
  cutterParams.nut.generate = false;

  Bolt cutterBolt(cutterParams);
  TopoDS_Solid cutterSolid = cutterBolt.Solid();

  // Scale the cutter bolt by tolerance factor for clearance
  // The user reported missing holes; scaling ensures the core is larger
  double scaleFactor = 1.0 + (tol / d);
  gp_Trsf scaleTransform;
  scaleTransform.SetScale(gp_Pnt(0.0, 0.0, h / 2.0),
                          scaleFactor); // Scale around center
  BRepBuilderAPI_Transform scaleOp(cutterSolid, scaleTransform, Standard_True);
  cutterSolid = TopoDS::Solid(scaleOp.Shape());

  // Position the cutter so it passes completely through the nut
  gp_Trsf positionTransform;
  positionTransform.SetTranslation(gp_Vec(0.0, 0.0, -overlap));
  BRepBuilderAPI_Transform positionOp(cutterSolid, positionTransform,
                                      Standard_True);

  // 3. Perform boolean difference
  std::cout << "Nut: Cutting internal threads..." << std::endl;
  try {
    body = Cut(hexOuter, positionOp.Shape());
  } catch (const std::exception &e) {
    std::cerr << "Nut: Boolean cut failed: " << e.what() << std::endl;
    // If cut fails, create a simple hole so at least the nut isn't solid hex
    TopoDS_Solid hole =
        BRepPrimAPI_MakeCylinder(0.5 * d * scaleFactor, h).Solid();
    body = Cut(hexOuter, hole);
  }

  // 4. Apply edge fillet for smooth edges
  // SAFE FILLET ALGORITHM: Validate radius against geometry to prevent
  // corruption
  double filletRadius = params.nut.edgeFilletRadius;
  if (filletRadius > 0.01) {
    // Clamp fillet radius to safe maximum (10% of nut width across flats)
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
}

TopoDS_Solid Nut::Solid() { return body; }
