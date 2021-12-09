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

#ifndef THREAD_H
#define THREAD_H

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS_Solid.hxx>
#include <gp_Pnt.hxx>
#include <vector>

#include "helix.h"

TopoDS_Solid Thread(double diameter,
                    double pitch,
                    double length);

#endif // THREAD_H
