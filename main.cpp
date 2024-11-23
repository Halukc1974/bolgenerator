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

#include "fileout.h"
#include <iostream>
#include <opencascade/BRepPrimAPI_MakeCylinder.hxx>
#include <opencascade/BRepTools.hxx>

int main(int argc, char *argv[])
{
    std::cout << "test" << std::endl;
    BRepPrimAPI_MakeCylinder volume = BRepPrimAPI_MakeCylinder(0.25, 1.0);

    ExportBRep(volume, "new.brep");


    //BRepTools::Write(volume.Shape(), "test.brep");
    
    /*
    for(int ct = 0; ct < argc; ct++)
        std::cout << argv[ct] << "\n";
    std::cout << argc << "\n" << argv[0] << std::endl;
    */
}
