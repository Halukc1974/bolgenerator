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

#ifndef CUT_H
#define CUT_H

#include <BRepAlgoAPI_Cut.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>

TopoDS_Solid Cut(TopoDS_Shape body, TopoDS_Shape tool);

#endif
