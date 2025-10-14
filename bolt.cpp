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
#include <stdexcept>
#include <iostream>

Bolt::Bolt(double majord_, 
           double length_,
           double pitch_,
           double headD1_,
           double headD2_,
           double headD3_,
           double headD4_,
           int headType_): majord(majord_),
                            length(length_),
                            pitch(pitch_),
                            headD1(headD1_), // Head Diameter or Across Flats
                            headD2(headD2_), // Head Height
                            headD3(headD3_), // Socket Size (across flats)
                            headD4(headD4_), // Socket Depth
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
        // Validation
        if (headD1 <= 0 || headD2 <= 0) {
            throw std::runtime_error("Hex head: D1 (across flats) and D2 (height) must be positive");
        }
        // Create hexagonal prism directly
        head = Hexagon(headD1, headD2);
        
    } else if (headType == 1) { // Socket head (cylinder with hex socket)
        // headD1 = outer diameter, headD2 = height
        // headD3 = socket size (across flats), headD4 = socket depth
        
        // Validation
        if (headD1 <= 0 || headD2 <= 0) {
            throw std::runtime_error("Socket head: D1 (outer diameter) and D2 (height) must be positive");
        }
        if (headD3 <= 0) {
            throw std::runtime_error("Socket head: D3 (socket size) must be positive");
        }
        if (headD4 <= 0) {
            throw std::runtime_error("Socket head: D4 (socket depth) must be positive");
        }
        // Socket must be smaller than outer diameter
        if (headD3 >= headD1 * 0.9) {
            throw std::runtime_error("Socket head: Socket size (D3) must be smaller than outer diameter (D1)");
        }
        // Socket depth must not exceed head height
        if (headD4 > headD2) {
            throw std::runtime_error("Socket head: Socket depth (D4) cannot exceed head height (D2)");
        }
        
        // Create cylindrical head
        head = BRepPrimAPI_MakeCylinder(0.5*headD1, headD2);
        
        // Cut hex socket from top
        std::cout << "Creating hex socket: size=" << headD3 << ", depth=" << headD4 << std::endl;
        TopoDS_Solid socket = Hexagon(headD3, headD4);
        std::cout << "Socket created, positioning at z=" << (headD2 - headD4) << std::endl;
        
        gp_Trsf socketOffset;
        // Position socket at top of head, going down into it
        socketOffset.SetTranslation(gp_Vec(0.0, 0.0, headD2 - headD4));
        TopoDS_Solid positionedSocket = TopoDS::Solid(BRepBuilderAPI_Transform(socket, socketOffset));
        
        std::cout << "Cutting socket from head..." << std::endl;
        head = Cut(head, positionedSocket);
        std::cout << "Cut operation completed" << std::endl;
        
    } else if (headType == 2) { // Flat head (simple cylinder)
        // headD1 = diameter, headD2 = height
        if (headD1 <= 0 || headD2 <= 0) {
            throw std::runtime_error("Flat head: D1 (diameter) and D2 (height) must be positive");
        }
        head = BRepPrimAPI_MakeCylinder(0.5*headD1, headD2);
        
    } else {
        // Default to hex
        head = Hexagon(headD1, headD2);
    }

    offset.SetTranslation(gp_Vec(0.0, 0.0, -headD2));

    return TopoDS::Solid(BRepBuilderAPI_Transform(head, offset));
}