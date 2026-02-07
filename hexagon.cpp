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

#include "hexagon.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <cmath>

TopoDS_Solid Hexagon(double aflats, double height)
{
    constexpr double kPi = 3.14159265358979323846;
    const double circumradius = aflats / std::sqrt(3.0);

    BRepBuilderAPI_MakePolygon polygon;
    for (int i = 0; i < 6; ++i) {
    const double angle = kPi / 6.0 + static_cast<double>(i) * (kPi / 3.0);
        const double x = circumradius * std::cos(angle);
        const double y = circumradius * std::sin(angle);
        polygon.Add(gp_Pnt(x, y, 0.0));
    }
    polygon.Close();

    TopoDS_Wire wire = polygon.Wire();
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire).Face();
    TopoDS_Shape prism = BRepPrimAPI_MakePrism(face, gp_Vec(0.0, 0.0, height)).Shape();

    return TopoDS::Solid(prism);
}
