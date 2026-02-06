#include "nut.h"
#include "bolt.h"
#include "cut.h"
#include "hexagon.h"
#include "thread.h"
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <GProp_GProps.hxx>
#include <TopoDS.hxx>
#include <gp_Trsf.hxx>
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
  double overlap = 10.0;
  double cutterLength = h + 2.0 * overlap;

  // Create a real bolt as cutter
  // Copy params for cutter but simplify some things
  BoltParameters cutterParams = params;
  cutterParams.shank.totalLength = cutterLength;
  cutterParams.nut.generate = false; // Don't recursively generate nuts

  Bolt cutterBolt(cutterParams);
  TopoDS_Solid cutterSolid = cutterBolt.Solid();

  // Scale the cutter bolt by tolerance factor for clearance
  double scaleFactor = 1.0 + (tol / d);
  gp_Trsf scaleTransform;
  scaleTransform.SetScale(gp_Pnt(0.0, 0.0, 0.0), scaleFactor);
  BRepBuilderAPI_Transform scaleOp(cutterSolid, scaleTransform, Standard_True);
  cutterSolid = TopoDS::Solid(scaleOp.Shape());

  // Position the cutter
  gp_Trsf positionTransform;
  positionTransform.SetTranslation(gp_Vec(0.0, 0.0, -overlap));
  BRepBuilderAPI_Transform positionOp(cutterSolid, positionTransform,
                                      Standard_True);
  TopoDS_Solid cutter = TopoDS::Solid(positionOp.Shape());

  // 3. Perform boolean difference
  body = Cut(hexOuter, cutter);

  // 4. Cleanup
  cutter.Nullify();
  cutterSolid.Nullify();
}

TopoDS_Solid Nut::Solid() { return body; }
