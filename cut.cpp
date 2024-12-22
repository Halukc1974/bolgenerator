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

#include "cut.h"

TopoDS_Solid Cut(TopoDS_Shape body, TopoDS_Shape tool)
{
    /*
        BRepAlgoAPI_Cut() works well, but it returns type TopoDS_Compound. This
        function accepts the same two arguments, but maps TopoDS_Compound for
        TopoDS_Solid components, which (by design) only contains one solid, and
        returns it.
    */

    TopExp_Explorer map(BRepAlgoAPI_Cut(body, tool), TopAbs_SOLID);
    return TopoDS::Solid(map.Current());
}
