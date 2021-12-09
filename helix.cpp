/*
    This file is part of BoltGenerator.

    BoltGenerator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BoltGenerator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BoltGenerator.  If not, see <https://www.gnu.org/licenses/>.
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
