/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#include "hexagon.h"

TopoDS_Solid Hexagon(double aflats, double height)
{
    double acorners = 1.1*aflats; // Approximate

    TopoDS_Shape mask;  // Negative space, used to remove material from head
    TopoDS_Solid hex;   // End result, modified throughout the function
    gp_Trsf trans, rot; // Used to transform a simple body to form mask

    BRepPrimAPI_MakeCylinder blank
        = BRepPrimAPI_MakeCylinder(0.5*acorners, height);
    
    // Build the simple body used to construct mask
    TopoDS_Solid tool = BRepPrimAPI_MakeBox(acorners, acorners, height);
    
    // Rotate it about to make a hexagonal flower petal shape.
    trans.SetTranslation(gp_Vec(-0.5*acorners, 0.5*aflats, 0.0));
    mask = BRepBuilderAPI_Transform(tool, trans).Shape();
    for(int step = 1; step < 6; step++)
    {
        rot.SetRotation(gp::OZ(), step*M_PI/3.0);
        mask = BRepAlgoAPI_Fuse(mask, BRepBuilderAPI_Transform(BRepBuilderAPI_Transform(tool, trans).Shape(), rot)).Shape();
    }

    hex = Cut(blank, mask);
    //trans.SetTranslation(gp_Vec(0.0, 0.0, -height));
    
    //return TopoDS::Solid(BRepBuilderAPI_Transform(hex, trans));
    return hex;
}