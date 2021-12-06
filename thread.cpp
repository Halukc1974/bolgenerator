/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#include "thread.h"

TopoDS_Solid Thread(double diameter,
                    double pitch,
                    double length,
                    bool external)
{
    std::vector<gp_Pnt> vertex;
    if(external)
        /*
        vertex = {gp_Pnt(0.5*(diameter+pitch), 0.0, 0.0),
                  gp_Pnt(0.5*(diameter-pitch), 0.0, 0.5*pitch),
                  gp_Pnt(0.5*(diameter-pitch), 0.0, -0.5*pitch)};
        */

        vertex = {gp_Pnt(0.5*(diameter-pitch), 0.0, 0.0),
                  gp_Pnt(0.5*(diameter+pitch), 0.0, 0.5*pitch),
                  gp_Pnt(0.5*(diameter+pitch), 0.0, -0.5*pitch)};

        /*
        vertex = {gp_Pnt(0.5*diameter-0.25*pitch, 0.0, -0.375*pitch),
                  gp_Pnt(0.5*diameter-0.25*pitch, 0.0, 0.375*pitch),
                  gp_Pnt(0.5*diameter+0.375*pitch, 0.0, 0.0625*pitch),
                  gp_Pnt(0.5*diameter+0.375*pitch, 0.0, -0.0625*pitch)};
        */
    else
        vertex = {gp_Pnt(0.5*(diameter+pitch), 0.0, 0.0),
                  gp_Pnt(0.5*(diameter-pitch), 0.0, 0.5*pitch),
                  gp_Pnt(0.5*(diameter-pitch), 0.0, -0.5*pitch)};

    // Build a wire profile by connecting the dots.
    BRepBuilderAPI_MakeWire wire;
    for(int ct = 0; ct < int(vertex.size()); ct++)
        wire.Add(BRepBuilderAPI_MakeEdge(vertex.at(ct),
                                         vertex.at((ct+1)%vertex.size())).Edge());

   return Helix(wire, diameter, pitch, length);
}
