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

#ifndef BOLT_H
#define BOLT_H

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Trsf.hxx>
#include <TopoDS_Solid.hxx>

#include "chamfer.h"
#include "cut.h"
#include "hexagon.h"
#include "thread.h"

class Bolt
{
    public:
        Bolt(double, double, double, double, double, double, int);
        TopoDS_Solid Solid();
    
    private:
        double majord, length, pitch, headD1, headD2, headD3;
        int headType;
        TopoDS_Solid Shank();
        TopoDS_Solid Head();
        TopoDS_Solid body;
};

#endif // BOLT_H
