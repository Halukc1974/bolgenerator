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
        BRepPrimAPI_MakeCylinder(0.5 * params.nut.washerFaceDiameter, wft);
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
  if (params.nut.edgeFilletRadius > 0.01) {
    try {
      std::cout << "Nut: Applying edge fillet radius: "
                << params.nut.edgeFilletRadius << std::endl;
      BRepFilletAPI_MakeFillet fillet(body);
      for (TopExp_Explorer ex(body, TopAbs_EDGE); ex.More(); ex.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(ex.Current());
        fillet.Add(params.nut.edgeFilletRadius, edge);
      }
      fillet.Build();
      if (fillet.IsDone()) {
        body = TopoDS::Solid(fillet.Shape());
        std::cout << "Nut: Edge fillet applied successfully" << std::endl;
      }
    } catch (...) {
      std::cerr << "Nut: Edge fillet failed, continuing without fillet"
                << std::endl;
    }
  }
}

TopoDS_Solid Nut::Solid() { return body; }
