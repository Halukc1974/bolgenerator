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
#include "convert.h"
#include "export.h"

int main(int argc, char *argv[])
{
    std::string filename = std::string(argv[1]).append(".brep");
    double majord = convert_mm_m(atof(argv[3]));
    double length = convert_mm_m(atof(argv[4]));
    double fraction = atof(argv[5]); // thread fraction (0.0: simple, 1.0: full)
    double pitch = convert_mm_m(atof(argv[6]));
    double pitchd = convert_mm_m(atof(argv[7]));
    //Bolt bolt = Bolt(majord, length, fraction, pitch, pitchd); 
    //ExportBRep(bolt.Solid(), std::string("Tests/").append(filename).c_str());
}
