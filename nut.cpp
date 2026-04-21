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
    double wft = 0.5;
    TopoDS_Solid washer =
        BRepPrimAPI_MakeCylinder(0.5 * params.nut.washerFaceDiameter, wft);
    BRepAlgoAPI_Fuse washerFuse(hexOuter, washer);
    washerFuse.Build();
    hexOuter = TopoDS::Solid(washerFuse.Shape());
  }

  // 2. Build the internal-thread cutter as a "virtual bolt" with
  //    DIAMETER-ONLY tolerance offset. Pitch is IDENTICAL to the user's bolt
  //    so they actually mate — the previous scale-based approach also scaled
  //    pitch and produced geometrically incompatible threads.
  const double overlap = 5.0;
  const double cutterLength = h + 2.0 * overlap;

  BoltParameters cutterParams = params;
  cutterParams.shank.totalLength = cutterLength;
  cutterParams.shank.gripLength = 0;          // fully threaded cutter
  cutterParams.shank.bodyTolerance = 0;       // no body-tolerance subtraction
  cutterParams.shank.edgeFilletRadius = 0;    // no edge fillet on the cutter
  cutterParams.head.underheadFilletRadius = 0;// no underhead fillet on cutter
  cutterParams.head.widthAcrossFlats = 0.1;   // minimal head so Head() is cheap
  cutterParams.head.height = 0.1;             // head will be trimmed off anyway
  cutterParams.head.type = HeadType::HEX;     // simplest head
  cutterParams.nut.generate = false;

  // Enlarge diameters by tolerance (clearance on radius) — keep pitch.
  cutterParams.thread.majorDiameter = d + tol;
  cutterParams.thread.minorDiameter =
      (params.thread.minorDiameter > 0 ? params.thread.minorDiameter
                                       : (d - 1.0825 * p_pitch)) +
      tol;
  cutterParams.shank.nominalDiameter = d + tol;

  std::cout << "Nut: Building diameter-offset cutter (d=" << (d + tol)
            << ", pitch=" << p_pitch << ", len=" << cutterLength << ")"
            << std::endl;
  Bolt cutterBolt(cutterParams);
  TopoDS_Solid cutterSolid = cutterBolt.Solid();

  // Position cutter so it passes completely through the nut
  gp_Trsf positionTransform;
  positionTransform.SetTranslation(gp_Vec(0.0, 0.0, -overlap));
  TopoDS_Shape cutterShape =
      BRepBuilderAPI_Transform(cutterSolid, positionTransform).Shape();

  // 3. Perform boolean difference
  std::cout << "Nut: Cutting internal threads..." << std::endl;
  try {
    body = Cut(hexOuter, cutterShape);
  } catch (const std::exception &e) {
    std::cerr << "Nut: Boolean cut failed: " << e.what() << std::endl;
    // Fallback: plain cylindrical hole so at least the nut isn't solid hex
    TopoDS_Solid hole =
        BRepPrimAPI_MakeCylinder(0.5 * (d + tol), h).Solid();
    body = Cut(hexOuter, hole);
  }

  // 4. Apply edge fillet for smooth edges.
  //    Safety: only fillet OUTER hex edges (avoid internal thread edges that
  //    number in the hundreds and crash OCCT's filleter).
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

      // Only consider edges whose axial extent lies on the top or bottom
      // face of the nut (|z_center - 0| < eps OR |z_center - h| < eps).
      // This avoids picking up the ~80+ helical thread edges that make
      // OCCT's filleter unstable and caused a segfault previously.
      const double zTol = 0.01;
      for (TopExp_Explorer ex(body, TopAbs_EDGE); ex.More(); ex.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(ex.Current());

        GProp_GProps edgeProps;
        BRepGProp::LinearProperties(edge, edgeProps);
        double edgeLength = edgeProps.Mass();
        double zCenter = edgeProps.CentreOfMass().Z();

        const bool onTopOrBottom =
            (std::abs(zCenter - 0.0) < zTol) || (std::abs(zCenter - h) < zTol);

        // Require long-enough edge AND top/bottom location
        if (edgeLength > filletRadius * 4.0 && onTopOrBottom) {
          fillet.Add(filletRadius, edge);
          edgesAdded++;
        }
      }

      std::cout << "Nut Fillet: Added to " << edgesAdded << " top/bottom edges"
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
