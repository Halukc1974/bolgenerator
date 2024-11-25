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

#include <iostream>


//#include "bolt.h"
#include "convert.h"
#include "export.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
//#include <BRepTools.hxx>

int main(int argc, char *argv[])
{
    double majorDiam = convert_mm_m(atof(argv[3]));
    double length = convert_mm_m(atof(argv[4]));
    bool fraction = atof(argv[5]);                  // thread fraction (0.0: simple, 1.0: full)

    if(fraction > 0.0)
    {
        double pitch = convert_mm_m(atof(argv[6]));
        double pitchDiam = convert_mm_m(atof(argv[7]));
    }

    std::string x;
    std::strcat(x, argv[1]);
    std::cout << x << std::endl;
    
    //std::str(argv[1]);

    //ExportBRep(BRepPrimAPI_MakeCylinder(1.0, 1.0),  "Tests/" + std::string(argv[1]) + ".brep");

    //BRepPrimAPI_MakeCylinder shank = BRepPrimAPI_MakeCylinder(majorDiam, length);

    //std::cout << "test" << std::endl;
    //BRepPrimAPI_MakeCylinder volume = BRepPrimAPI_MakeCylinder(atof(argv[1]), atof(argv[2]));
    //ExportBRep(shank, "new.brep");


    //BRepTools::Write(volume.Shape(), "test.brep");
    
    /*
    for(int ct = 0; ct < argc; ct++)
        std::cout << argv[ct] << "\n";
    std::cout << argc << "\n" << argv[0] << std::endl;
    */
}
