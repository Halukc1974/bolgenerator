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

TopoDS_Solid Hexagon(double aflats, double height)
{
    // aflats = across flats (distance between parallel sides)
    // acorners = across corners (circumradius)
    // For a regular hexagon: acorners = aflats / cos(30°) = aflats / (sqrt(3)/2) = aflats * 2/sqrt(3)
    double acorners = aflats * 1.1547; // aflats * 2 / sqrt(3) = aflats * 1.1547

    TopoDS_Shape mask;  // Negative space, used to remove material from head
    TopoDS_Solid hex;   // End result, modified throughout the function
    gp_Trsf trans, rot; // Used to transform a simple body to form mask

    // Create cylindrical blank with radius = acorners/2 (circumradius)
    BRepPrimAPI_MakeCylinder blank = BRepPrimAPI_MakeCylinder(0.5*acorners, height);

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
