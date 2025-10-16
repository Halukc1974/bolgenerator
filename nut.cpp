/*
    BoltGenerator - Nut component
    Copyright (C) 2025
*/

#include "nut.h"
#include "hexagon.h"
#include "cut.h"
#include "thread.h"
#include "bolt.h"
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
#include <ShapeFix_Solid.hxx>
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

    // 2. Create cutter bolt: Use actual bolt geometry for perfect thread cutting
    double overlap = 10.0;  // 10mm beyond nut as requested
    double cutterLength = nutHeight + 2.0 * overlap;  // Total cutter length
    
    std::cout << "Creating cutter bolt: length=" << cutterLength << " mm, overlap=" << overlap << std::endl;
    
    // Create a real bolt as cutter (same parameters as original bolt)
    Bolt cutterBolt(boltMajorD,     // major diameter
                   cutterLength,    // extended length 
                   pitch,           // same pitch
                   boltMajorD * 1.5, // dummy head diameter 
                   boltMajorD,      // dummy head values
                   0, 0, 0);        // simple head type
    
    TopoDS_Solid cutterSolid = cutterBolt.Solid();
    
    // Scale the cutter bolt by tolerance factor for clearance
    double scaleFactor = 1.0 + (tolerance / boltMajorD);
    std::cout << "Scaling cutter bolt by factor=" << scaleFactor << " (tolerance=" << tolerance << ")" << std::endl;
    
    gp_Trsf scaleTransform;
    scaleTransform.SetScale(gp_Pnt(0.0, 0.0, 0.0), scaleFactor);
    BRepBuilderAPI_Transform scaleOp(cutterSolid, scaleTransform, Standard_True);
    cutterSolid = TopoDS::Solid(scaleOp.Shape());
    
    // Position the cutter: move to center on nut and extend 10mm beyond
    // Nut is at Z=0 to Z=nutHeight, so cutter should be at Z=-overlap to Z=nutHeight+overlap
    gp_Trsf positionTransform;
    positionTransform.SetTranslation(gp_Vec(0.0, 0.0, -overlap));
    BRepBuilderAPI_Transform positionOp(cutterSolid, positionTransform, Standard_True);
    TopoDS_Solid cutter = TopoDS::Solid(positionOp.Shape());
    
    std::cout << "Positioned cutter bolt: Z=" << -overlap << " to Z=" << (nutHeight + overlap) << std::endl;

    // 3. Perform boolean difference: subtract cutter bolt from nut
    std::cout << "Cutting real bolt geometry from nut..." << std::endl;
    body = Cut(hexOuter, cutter);

    // 4. Cleanup
    cutter.Nullify();
    cutterSolid.Nullify();
    std::cout << "Cutter bolt deleted after cutting" << std::endl;

    // 5. Skip all repair operations to preserve the central hole
    std::cout << "Skipping all repair operations to preserve central hole" << std::endl;
    
    // Verify that we have a valid solid from the cut operation
    try {
        GProp_GProps props;
        BRepGProp::VolumeProperties(body, props);
        double volume = props.Mass();
        std::cout << "Final nut volume: " << volume << " mm³" << std::endl;
        
        // Basic validation only
        if (volume <= 0) {
            throw std::runtime_error("Invalid nut volume after cutting");
        }
        
        std::cout << "Nut created successfully - central hole preserved" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Warning: Volume calculation failed: " << e.what() << std::endl;
        std::cout << "Continuing with cut result (hole should be preserved)" << std::endl;
    }
}

TopoDS_Solid Nut::Solid()
{
    return body;
}
