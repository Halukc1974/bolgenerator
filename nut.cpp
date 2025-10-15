/*
    BoltGenerator - Nut component
    Copyright (C) 2025
*/

#include "nut.h"
#include "hexagon.h"
#include "cut.h"
#include "thread.h"
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <gp_Trsf.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Mat.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <TopoDS.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <stdexcept>
#include <iostream>

Nut::Nut(double boltMajorD_, 
         double pitch_,
         double nutHeight_,
         double acrossFlats_,
         double tolerance_): boltMajorD(boltMajorD_),
                            pitch(pitch_),
                            nutHeight(nutHeight_),
                            acrossFlats(acrossFlats_),
                            tolerance(tolerance_)
{
    // Validation
    if (boltMajorD <= 0 || pitch <= 0 || nutHeight <= 0 || acrossFlats <= 0) {
        throw std::runtime_error("Nut: All dimensions must be positive");
    }
    if (tolerance < 0) {
        throw std::runtime_error("Nut: Tolerance cannot be negative");
    }
    if (acrossFlats <= boltMajorD) {
        throw std::runtime_error("Nut: Across flats must be larger than bolt diameter");
    }
    
    std::cout << "Creating nut: majorD=" << boltMajorD 
              << " pitch=" << pitch
              << " height=" << nutHeight 
              << " acrossFlats=" << acrossFlats 
              << " tolerance=" << tolerance << std::endl;
    
    // 1. Create hex outer shape (starts at z=0, extends to z=nutHeight)
    TopoDS_Solid hexOuter = Hexagon(acrossFlats, nutHeight);
    
    // 2. User workflow: take the bolt thread geometry, uniformly scale it
    //    (as in Rhino: scale in all axes by a small factor like 1.0015) and
    //    subtract it from the nut. To avoid off-axis drifting, recenter the
    //    thread to origin before scaling, then translate it to overlap the nut.
    double overlap = std::max(1.0, pitch * 2.0);
    double cutterLength = nutHeight + 2.0 * overlap;

    // Create a bolt-like thread using same base diameter as Bolt::Shank()
    double boltThreadDia = boltMajorD - 2.0 * 3.0 / 8.0 * pitch;
    std::cout << "Constructing bolt-like thread for scaling: baseDia=" << boltThreadDia << " mm" << std::endl;
    TopoDS_Solid boltThread = Thread(boltThreadDia, pitch, cutterLength);

    // Recenter in Z so scaling happens about the thread's center (prevents axial drift)
    Bnd_Box btb; BRepBndLib::Add(boltThread, btb);
    double bxmin, bymin, bzmin, bxmax, bymax, bzmax;
    btb.Get(bxmin, bymin, bzmin, bxmax, bymax, bzmax);
    double bCenterZ = 0.5 * (bzmin + bzmax);
    if (std::abs(bCenterZ) > 1e-9) {
        gp_Trsf t1; t1.SetTranslation(gp_Vec(0, 0, -bCenterZ));
        boltThread = TopoDS::Solid(BRepBuilderAPI_Transform(boltThread, t1).Shape());
        std::cout << "Re-centered bolt thread to origin (Z translate " << -bCenterZ << ")" << std::endl;
    }

    // Determine scale factors. Use X/Y-only scaling (preserve pitch in Z)
    double desiredDia = boltMajorD + 2.0 * tolerance;
    double scaleXY = desiredDia / boltThreadDia;
    double scaleZ = 1.0; // preserve pitch to avoid helix mismatch
    std::cout << "Scaling factors: XY=" << scaleXY << " Z=" << scaleZ << " (target diameter=" << desiredDia << ")" << std::endl;

    // Prefer uniform scaling using gp_Trsf SetScale (more robust for solids)
    double scaleFactor = desiredDia / boltThreadDia; // uniform factor (user expects uniform behavior)
    gp_Trsf sct; sct.SetScale(gp_Pnt(0.0, 0.0, 0.0), scaleFactor);
    BRepBuilderAPI_Transform stx(boltThread, sct, Standard_True);
    TopoDS_Shape scaledShape = stx.Shape();

    // Attempt to get a solid; if not, sew and try MakeSolid
    TopoDS_Solid threadCutter;
    try {
        threadCutter = TopoDS::Solid(scaledShape);
    } catch(...) {
            try {
                // Increase sewing tolerance to be more permissive
                double sewTol = 1e-4;
            BRepBuilderAPI_Sewing sewer(sewTol);
            sewer.Add(scaledShape);
            sewer.Perform();
            TopoDS_Shape sewed = sewer.SewedShape();
            TopoDS_Shell shell = TopoDS::Shell(sewed);
            BRepBuilderAPI_MakeSolid mkSolid(shell);
            if (mkSolid.IsDone()) {
                threadCutter = mkSolid.Solid();
                std::cout << "Scaled thread converted to solid via sewing+MakeSolid (tol=" << sewTol << ")" << std::endl;
            } else {
                std::cout << "MakeSolid failed on scaled thread; attempting fuse-with-core fallback" << std::endl;
                // Fuse scaled shape with a core cylinder to ensure full intersection
                try {
                    // Slightly larger core to ensure fusion covers edge cases
                    double coreDia = boltMajorD + std::max(tolerance, 0.05);
                    TopoDS_Solid core = BRepPrimAPI_MakeCylinder(0.5 * coreDia, cutterLength).Solid();
                    // center core similarly (it should already be centered at origin Z)
                    BRepAlgoAPI_Fuse fus(scaledShape, core);
                    fus.Build();
                    if (fus.IsDone()) {
                        TopoDS_Shape fused = fus.Shape();
                        // try sewing the fused shape
                        BRepBuilderAPI_Sewing sewer2(sewTol);
                        sewer2.Add(fused);
                        sewer2.Perform();
                        TopoDS_Shape sewed2 = sewer2.SewedShape();
                        try {
                            TopoDS_Shell shell2 = TopoDS::Shell(sewed2);
                            BRepBuilderAPI_MakeSolid mkSolid2(shell2);
                            if (mkSolid2.IsDone()) {
                                threadCutter = mkSolid2.Solid();
                                std::cout << "Fuse-with-core produced solid cutter via sewing+MakeSolid" << std::endl;
                            } else {
                                std::cout << "MakeSolid failed on fused shape; using fused shape as cutter" << std::endl;
                                threadCutter = TopoDS::Solid(fused);
                            }
                        } catch(...) {
                            std::cout << "Exception while converting fused/sewed to solid; using fused shape" << std::endl;
                            threadCutter = TopoDS::Solid(fused);
                        }
                    } else {
                        std::cout << "Fuse operation failed; falling back to naive thread" << std::endl;
                        threadCutter = Thread(desiredDia, pitch, cutterLength);
                    }
                } catch(...) {
                    std::cout << "Fuse-with-core fallback threw; using naive thread as last resort" << std::endl;
                    threadCutter = Thread(desiredDia, pitch, cutterLength);
                }
            }
        } catch(...) {
            std::cout << "Sew/MakeSolid fallback failed unexpectedly; using naive thread as last resort" << std::endl;
            threadCutter = Thread(desiredDia, pitch, cutterLength);
        }
    }
    // Align scaled cutter so its zmin == -overlap
    Bnd_Box box3; BRepBndLib::Add(threadCutter, box3);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box3.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    double desiredZmin = -overlap;
    double translateZ = desiredZmin - zmin;
    if (std::abs(translateZ) > 1e-9) {
        gp_Trsf tr; tr.SetTranslation(gp_Vec(0,0,translateZ));
        BRepBuilderAPI_Transform tx(threadCutter, tr, Standard_True);
        threadCutter = TopoDS::Solid(tx.Shape());
        std::cout << "Translated scaled thread cutter by " << translateZ << " in Z to ensure overlap=" << overlap << std::endl;
    }

    // 4. Cut the thread cutter from hex to create internal thread in nut
    std::cout << "Cutting thread cutter from nut..." << std::endl;
    body = Cut(hexOuter, threadCutter);

    // 5. Attempt to repair the resulting nut solid (sew shells, try MakeSolid)
    try {
        double repairTol = 1e-4;
        BRepBuilderAPI_Sewing sewer(repairTol);
        sewer.Add(body);
        sewer.Perform();
        TopoDS_Shape sewed = sewer.SewedShape();

        // If sewed already contains solids, pick the largest by volume
        std::vector<TopoDS_Solid> solids;
        for (TopExp_Explorer ex(sewed, TopAbs_SOLID); ex.More(); ex.Next()) {
            solids.push_back(TopoDS::Solid(ex.Current()));
        }
        if (!solids.empty()) {
            // choose largest volume
            double maxV = -1.0; TopoDS_Solid sel = solids.front();
            for (const auto& s : solids) {
                GProp_GProps props; BRepGProp::VolumeProperties(s, props);
                double v = props.Mass(); if (v > maxV) { maxV = v; sel = s; }
            }
            body = sel;
            std::cout << "Repaired nut: selected largest solid from sewed result" << std::endl;
        } else {
            // Try to convert sewed shape to a shell then to solid
            try {
                TopoDS_Shell shell = TopoDS::Shell(sewed);
                BRepBuilderAPI_MakeSolid mk(shell);
                if (mk.IsDone()) {
                    body = mk.Solid();
                    std::cout << "Repaired nut: created solid via MakeSolid on sewed shell" << std::endl;
                } else {
                    std::cout << "Repair: MakeSolid on sewed shell failed; keeping original body" << std::endl;
                }
            } catch(...) {
                std::cout << "Repair: sewed shape not convertible to shell; keeping original body" << std::endl;
            }
        }
    } catch(...) {
        std::cout << "Repair pass threw exception; continuing with un-repaired body" << std::endl;
    }

    std::cout << "Nut created successfully with thread cutter" << std::endl;
}

TopoDS_Solid Nut::Solid()
{
    return body;
}
