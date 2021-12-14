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

#include <TopoDS_Solid.hxx>

#include "chamfer.h"
#include "dimensions.h"
#include "export.h"
#include "hexagon.h"
#include "thread.h"

class Bolt : Dimensions
{
public:
    Bolt(int, int, double, bool=false);
    TopoDS_Solid Solid();

private:
    TopoDS_Solid Head(int, int);
    TopoDS_Solid Shank(int, double, bool);
    bool simple;
    Dimensions dimensions;
    TopoDS_Solid body;
};

#endif // BOLT_H
