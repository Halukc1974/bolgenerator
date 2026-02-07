/*
    BoltGenerator is an automated CAD assistant which produces standard-size 3D
    bolts per ISO and ASME specifications.
    Copyright (C) 2021  Scimulate LLC <solvers@scimulate.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "chamfer.h"

TopoDS_Solid Chamfer(std::vector<gp_Pnt> points)
{
    BRepBuilderAPI_MakeWire wire;
    for(std::size_t ct = 0; ct < points.size(); ct++)
        wire.Add(BRepBuilderAPI_MakeEdge(points.at(ct), points.at((ct+1)%points.size())));
    
    TopoDS_Face sketch = BRepBuilderAPI_MakeFace(wire.Wire()).Face();

    TopoDS_Solid chamfer = TopoDS::Solid(BRepSweep_Revol(sketch,
                                         gp_Ax1(gp::Origin(),
                                         gp::DZ())).Shape());

    return chamfer;
}
