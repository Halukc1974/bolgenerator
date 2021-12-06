/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#include "chamfer.h"

TopoDS_Solid Chamfer(std::vector<gp_Pnt> points)
{
    BRepBuilderAPI_MakeWire wire;
    for(std::size_t ct = 0; ct < points.size(); ct++)
        wire.Add(BRepBuilderAPI_MakeEdge(points.at(ct), points.at((ct+1)%3)));
    
    TopoDS_Face sketch = BRepBuilderAPI_MakeFace(wire);
    TopoDS_Solid chamfer = TopoDS::Solid(BRepSweep_Revol(sketch,
                                         gp_Ax1(gp::Origin(),
                                         gp::DZ())).Shape());

    return chamfer;
}