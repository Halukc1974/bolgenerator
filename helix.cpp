/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#include "helix.h"

TopoDS_Solid Helix(TopoDS_Wire sketch,
                   double diameter,
                   double pitch,
                   double length)
{
    // Create an infinite cylinder coradial with the pitch diameter.
    Handle(Geom_CylindricalSurface) cylinder
        = new Geom_CylindricalSurface(gp_Ax2(gp::Origin(), gp::DZ()),
                                      0.5*diameter);

    // Map the 3D tool path as a 2D function of z-position and rotation.
    gp_Pnt2d aPnt(0.0, 0.0);
    gp_Dir2d aDir(2.0*M_PI, pitch);
    Handle(Geom2d_Line) line = new Geom2d_Line(gp_Ax2d(aPnt, aDir));

    // Trim the tool path such that it fits the shank length.
    Handle(Geom2d_TrimmedCurve) anArc1
        = new Geom2d_TrimmedCurve(line, 0, (length/pitch)*2*M_PI);
    TopoDS_Edge toolPath = BRepBuilderAPI_MakeEdge(anArc1, cylinder);
    BRepLib::BuildCurves3d(toolPath);

    // Build a solid trace of the 2D path.
    auto thread = BRepOffsetAPI_MakePipeShell(BRepBuilderAPI_MakeWire(toolPath));
    thread.Add(sketch);
    thread.SetMode(gp_Vec(0, 0, 1));
    thread.Build();
    thread.MakeSolid();

    return TopoDS::Solid(thread);
}