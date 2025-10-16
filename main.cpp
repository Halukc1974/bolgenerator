/*
    BoltGenerator is an automated CAD assistant which produces standard-size 3D
    bolts per ISO and ASME specifications.
    Copyright (C) 2021-2024  Scimulate LLC <solvers@scimulate.com>

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

#include "bolt.h"
#include "nut.h"
#include "export.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 4) {
        std::cerr << "Usage for bolt: " << argv[0] << " <name> <majorD> <pitch> <length> <headD1> <headD2> <headD3> <headD4> <headType>" << std::endl;
        std::cerr << "Usage for nut:  " << argv[0] << " <majorD> <pitch> nut <name>" << std::endl;
        return 1;
    }
    
    // Check if this is a nut creation command
    if (argc == 5 && std::string(argv[3]) == "nut") {
        double boltMajorD = atof(argv[1]);
        double pitch = atof(argv[2]);
        std::string name = argv[4];
        
        std::cout << "Creating nut with majorD=" << boltMajorD << ", pitch=" << pitch << std::endl;
        
        // Standard nut dimensions based on bolt diameter
        double nutHeight = boltMajorD * 0.8;  // Standard nut height
        double nutAcrossFlats = boltMajorD * 1.5;  // Standard across flats
        double tolerance = 0.1;  // Small tolerance
        
        std::string nutFilename = name + "_nut.brep";
        std::string nutStlFilename = name + "_nut.stl";
        
        try {
            Nut nut = Nut(boltMajorD, pitch, nutHeight, nutAcrossFlats, tolerance);
            ExportBRep(nut.Solid(), std::string("Tests/").append(nutFilename).c_str());
            ExportSTL(nut.Solid(), std::string("Tests/").append(nutStlFilename).c_str());
            std::cout << "Nut exported successfully as " << nutFilename << " and " << nutStlFilename << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error creating nut: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }
    
    // Original bolt creation code for backward compatibility
    if (argc < 10) {
        std::cerr << "Insufficient arguments for bolt creation" << std::endl;
        return 1;
    }
    
    std::string filename = std::string(argv[1]).append(".brep");
    std::string stlFilename = std::string(argv[1]).append(".stl");
    
    // Create bolt
    Bolt bolt = Bolt(atof(argv[2]),  // majord
                     atof(argv[4]),  // length
                     atof(argv[3]),  // pitch
                     atof(argv[5]),  // headD1
                     atof(argv[6]),  // headD2
                     atof(argv[7]),  // headD3
                     atof(argv[8]),  // headD4
                     atoi(argv[9])); // headType 
    ExportBRep(bolt.Solid(), std::string("Tests/").append(filename).c_str());
    ExportSTL(bolt.Solid(), std::string("Tests/").append(stlFilename).c_str());
    
    // Create nut if parameters provided (argc >= 14)
    if (argc >= 14) {
        double nutHeight = atof(argv[10]);
        double nutAcrossFlats = atof(argv[11]);
        double tolerance = atof(argv[12]);
        bool generateNut = atoi(argv[13]) == 1;
        
        if (generateNut && nutHeight > 0 && nutAcrossFlats > 0) {
            std::cout << "Generating nut..." << std::endl;
            std::string nutFilename = std::string(argv[1]).append("_nut.brep");
            std::string nutStlFilename = std::string(argv[1]).append("_nut.stl");
            
            Nut nut = Nut(atof(argv[2]),  // boltMajorD
                         atof(argv[3]),  // pitch
                         nutHeight,
                         nutAcrossFlats,
                         tolerance);
            
            ExportBRep(nut.Solid(), std::string("Tests/").append(nutFilename).c_str());
            ExportSTL(nut.Solid(), std::string("Tests/").append(nutStlFilename).c_str());
            std::cout << "Nut exported successfully" << std::endl;
        }
    }
    
    return 0;
}
