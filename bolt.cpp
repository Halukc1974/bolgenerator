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

#include "bolt.h"

Bolt::Bolt(double majord_, 
           double length_,
           double pitch_,
           double headD1_,
           double headD2_,
           double headD3_,
           int headType_): majord(majord_),
                            length(length_),
                            pitch(pitch_),
                            headD1(headD1_), // Head Diameter
                            headD2(headD2_), // Head Height
                            headD3(headD3_), // Drive Size
                            headType(headType_)
{
    TopExp_Explorer map(BRepAlgoAPI_Fuse(Shank(),
                                         Head()),
                        TopAbs_SOLID);
    body = TopoDS::Solid(map.Current());
}

TopoDS_Solid Bolt::Shank()
{
    TopoDS_Solid mask, shank, thread;
    gp_Trsf offset;

    // The geometry kernel has issues building threads unevenly. The applied
    // work-around creates an integer number of complete threads, later trimmed.
    // Add extra length (2*pitch) to clean up partial threads.
    // Round up to nearest full thread.
    double nthreads = std::ceil((length + 2*pitch)/pitch);
    
    // Start with a cylinder
    shank = BRepPrimAPI_MakeCylinder(0.5*majord, pitch*nthreads);

    // Create a 3D body of threads, cut them from the cylinder.
    thread = Thread(majord-2.0*3.0/8.0*pitch, pitch, pitch*nthreads);
    shank = Cut(shank, thread);

    // Remove head-side partial thread
    mask = BRepPrimAPI_MakeCylinder(majord, pitch).Solid();
    shank = Cut(shank, mask);

    // Remove end-side partial thread
    offset.SetTranslation(gp_Vec(0.0, 0.0, length+pitch));
    mask = BRepPrimAPI_MakeCylinder(majord, nthreads*pitch-pitch-length).Solid();
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    // Currently the shank is floating off of the origin by dz=pitch. Shift down.
    offset.SetTranslation(gp_Vec(0.0, 0.0, -pitch));
    shank = TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset));

    // Chamfer lead-in thread
    double x[] = {0.5*majord-pitch,
                  majord};
    double z[] = {length,
                  length-0.5*majord-pitch};
    std::vector<gp_Pnt> points = {gp_Pnt(x[0], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[1])};
    shank = Cut(shank, Chamfer(points));

    return shank;
}

TopoDS_Solid Bolt::Solid()
{
    return body;
}

TopoDS_Solid Bolt::Head()
{
    TopoDS_Solid head;
    gp_Trsf offset;

    if (headType == 0) { // Hex head
        // headD1 = across flats, headD2 = height
        // Create hexagonal prism directly
        head = Hexagon(headD1, headD2);
        
    } else if (headType == 1) { // Socket head (cylinder with hex socket)
        // headD1 = outer diameter, headD2 = height, headD3 = socket size
        head = BRepPrimAPI_MakeCylinder(0.5*headD1, headD2);
        // Cut hex socket from top
        TopoDS_Solid socket = Hexagon(headD3, headD2 * 0.5); // Socket depth = half height
        gp_Trsf socketOffset;
        socketOffset.SetTranslation(gp_Vec(0.0, 0.0, headD2 * 0.5));
        TopoDS_Solid positionedSocket = TopoDS::Solid(BRepBuilderAPI_Transform(socket, socketOffset));
        head = Cut(head, positionedSocket);
        
    } else if (headType == 2) { // Flat head (simple cylinder)
        head = BRepPrimAPI_MakeCylinder(0.5*headD1, headD2);
        
    } else {
        // Default to hex
        head = Hexagon(headD1, headD2);
    }

    offset.SetTranslation(gp_Vec(0.0, 0.0, -headD2));

    return TopoDS::Solid(BRepBuilderAPI_Transform(head, offset));
}