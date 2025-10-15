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
#include <vector>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

TopoDS_Solid Cut(TopoDS_Shape body, TopoDS_Shape tool)
{
    /*
        BRepAlgoAPI_Cut() works well, but it returns type TopoDS_Compound. This
        function accepts the same two arguments, but maps TopoDS_Compound for
        TopoDS_Solid components, which (by design) only contains one solid, and
        returns it.
    */

    std::cout << "Cut: Performing boolean cut operation..." << std::endl;
    
    // Calculate volume before cut
    GProp_GProps propsBody;
    BRepGProp::VolumeProperties(body, propsBody);
    double volumeBefore = propsBody.Mass();
    std::cout << "Cut: Body volume before cut: " << volumeBefore << " mm³" << std::endl;
    
    BRepAlgoAPI_Cut cutOp(body, tool);
    cutOp.SetFuzzyValue(1.0e-6);
    cutOp.Build();
    
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
    
    // Collect all solids from result
    std::vector<TopoDS_Solid> solids;
    TopExp_Explorer map(result, TopAbs_SOLID);
    int solidCount = 0;
    while (map.More()) {
        solids.push_back(TopoDS::Solid(map.Current()));
        solidCount++;
        map.Next();
    }
    
    std::cout << "Cut: Found " << solidCount << " solid(s) in result" << std::endl;
    
    if (solids.empty()) {
        std::cerr << "ERROR: No solid found in cut result!" << std::endl;
        // Try to return the original body instead of failing
        if (body.ShapeType() == TopAbs_SOLID) {
            std::cerr << "WARNING: Returning original body unchanged (no solid in result)" << std::endl;
            return TopoDS::Solid(body);
        }
        throw std::runtime_error("No solid found in cut result");
    }
    
    // If multiple solids, pick the one with largest volume (should be the body after cut)
    TopoDS_Solid resultSolid = solids[0];
    if (solids.size() > 1) {
        std::cout << "Cut: Multiple solids found, selecting largest volume..." << std::endl;
        double maxVolume = 0;
        for (const auto& solid : solids) {
            GProp_GProps props;
            BRepGProp::VolumeProperties(solid, props);
            double vol = props.Mass();
            std::cout << "  Solid volume: " << vol << " mm³" << std::endl;
            if (vol > maxVolume) {
                maxVolume = vol;
                resultSolid = solid;
            }
        }
        std::cout << "Cut: Selected solid with volume: " << maxVolume << " mm³" << std::endl;
    }
    
    // Calculate volume after cut
    GProp_GProps propsResult;
    BRepGProp::VolumeProperties(resultSolid, propsResult);
    double volumeAfter = propsResult.Mass();
    double volumeRemoved = volumeBefore - volumeAfter;
    std::cout << "Cut: Result volume after cut: " << volumeAfter << " mm³" << std::endl;
    std::cout << "Cut: Volume removed: " << volumeRemoved << " mm³ (" << (volumeRemoved/volumeBefore*100) << "%)" << std::endl;
    
    std::cout << "Cut: Successfully extracted solid from result" << std::endl;
    return resultSolid;
}
