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

#include "thread.h"

TopoDS_Solid Thread(double diameter, // Pitch Diameter
                    double pitch,
                    double length)
{
    std::vector<gp_Pnt> vertex;
    vertex = {gp_Pnt(0.5*diameter-0.25*pitch, 0.0, -0.125*pitch),
              gp_Pnt(0.5*diameter-0.25*pitch, 0.0, 0.125*pitch),
              gp_Pnt(0.5*diameter+0.5*pitch, 0.0, 0.5*pitch),
              gp_Pnt(0.5*diameter+0.5*pitch, 0.0, -0.5*pitch)};

    // Build a wire profile by connecting the dots.
    BRepBuilderAPI_MakeWire wire;
    for(int ct = 0; ct < int(vertex.size()); ct++)
        wire.Add(BRepBuilderAPI_MakeEdge(vertex.at(ct),
                                         vertex.at((ct+1)%vertex.size())).Edge());

    return Helix(wire, diameter, pitch, length);
}
