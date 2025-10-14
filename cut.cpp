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
#include <iostream>
#include <stdexcept>

TopoDS_Solid Cut(TopoDS_Shape body, TopoDS_Shape tool)
{
    /*
        BRepAlgoAPI_Cut() works well, but it returns type TopoDS_Compound. This
        function accepts the same two arguments, but maps TopoDS_Compound for
        TopoDS_Solid components, which (by design) only contains one solid, and
        returns it.
    */

    std::cout << "Cut: Performing boolean cut operation..." << std::endl;
    BRepAlgoAPI_Cut cutOp(body, tool);
    
    if (!cutOp.IsDone()) {
        std::cerr << "ERROR: Cut operation failed!" << std::endl;
        // Return original body if cut fails
        if (body.ShapeType() == TopAbs_SOLID) {
            std::cerr << "WARNING: Returning original body unchanged due to cut failure" << std::endl;
            return TopoDS::Solid(body);
        }
        throw std::runtime_error("Cut operation failed and body is not a solid");
    }
    
    TopoDS_Shape result = cutOp.Shape();
    std::cout << "Cut: Result shape type: " << result.ShapeType() << std::endl;
    
    TopExp_Explorer map(result, TopAbs_SOLID);
    if (!map.More()) {
        std::cerr << "ERROR: No solid found in cut result!" << std::endl;
        // Try to return the original body instead of failing
        if (body.ShapeType() == TopAbs_SOLID) {
            std::cerr << "WARNING: Returning original body unchanged (no solid in result)" << std::endl;
            return TopoDS::Solid(body);
        }
        throw std::runtime_error("No solid found in cut result");
    }
    
    TopoDS_Solid resultSolid = TopoDS::Solid(map.Current());
    std::cout << "Cut: Successfully extracted solid from result" << std::endl;
    return resultSolid;
}
