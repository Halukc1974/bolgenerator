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

Bolt::Bolt(int indexHead,
           int indexThread,
           double length,
           bool simple)
{
    TopExp_Explorer map(BRepAlgoAPI_Fuse(Shank(indexThread, length, simple),
                                         Head(indexHead, indexThread)),
                        TopAbs_SOLID);
    body = TopoDS::Solid(map.Current());
}

TopoDS_Solid Bolt::Solid()
{
    return body;
}

TopoDS_Solid Bolt::Head(int indexHead, int indexThread)
{
    TopoDS_Solid output;
    double thickness = dimensions.GetHeadDims(indexHead, indexThread).at(0);
    gp_Trsf offset;

    if(indexHead == 1) // 1 == Hex Bolt
    {
        double aflats = dimensions.GetHeadDims(indexHead, indexThread).at(1);
        output = Hexagon(aflats, thickness);
    }
    else if(indexHead == 2) // 2 == SHCS
    {
        double aflats = dimensions.GetHeadDims(indexHead, indexThread).at(2);
        double diam = dimensions.GetHeadDims(indexHead, indexThread).at(1);
        //output = Cut(BRepPrimAPI_MakeCylinder(0.5*diam, thickness).Solid(), Hexagon(aflats, aflats));
        output = BRepPrimAPI_MakeCylinder(0.5*diam, thickness).Solid();
        //ExportBRep(output, "test.brep");
    }

    offset.SetTranslation(gp_Vec(0.0, 0.0, -thickness));
    return TopoDS::Solid(BRepBuilderAPI_Transform(output, offset));
}

TopoDS_Solid Bolt::Shank(int indexThread, double length, bool simple)
{
    TopoDS_Solid mask, shank, thread, fullshank;
    gp_Trsf offset;

    double major = dimensions.Major(indexThread);
    double pitch = dimensions.Pitch(indexThread);

    // The geometry kernel has issues building threads unevenly. The applied
    // work-around creates a static number of threads (10*pitch, for example).
    // At the Chamfer()
    double modlength = 10*pitch;

    // Add +2*pitch so that the geometry can be cleaned at both ends
    shank = BRepPrimAPI_MakeCylinder(0.5*major, modlength+2*pitch);

    if(!simple)
    {
        thread = Thread(major-2.0*3.0/8.0*pitch, pitch, modlength+2*pitch);
        shank = Cut(shank, thread);
    }

    // Remove head-side partial thread
    mask = BRepPrimAPI_MakeCylinder(major, 2*pitch).Solid();
    offset.SetTranslation(gp_Vec(0.0, 0.0, -1.0*pitch));
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    // Remove end-side partial thread
    offset.SetTranslation(gp_Vec(0.0, 0.0, modlength+pitch));
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    // Building threads seems to fail
    fullshank = shank;
    int ct = 1;
    while(ct*modlength <= length)
    {
        offset.SetTranslation(gp_Vec(0.0, 0.0, ct*modlength));
        fullshank = TopoDS::Solid(TopExp_Explorer(BRepAlgoAPI_Fuse(fullshank, TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset).Shape())), TopAbs_SOLID).Current());
        ct++;
    }

    // Currently the shank is floating off of the origin by dz=pitch.
    offset.SetTranslation(gp_Vec(0.0, 0.0, -pitch));
    fullshank = TopoDS::Solid(BRepBuilderAPI_Transform(fullshank, offset));

    // Trim excess threaded rod beyond length
    mask = BRepPrimAPI_MakeCylinder(major, ct*modlength).Solid();
    offset.SetTranslation(gp_Vec(0, 0, length));
    fullshank = Cut(fullshank, BRepBuilderAPI_Transform(mask, offset));

    // Chamfer lead-in thread
    double x[] = {0.5*major-pitch,
                  major};
    double z[] = {length,
                  length-0.5*major-pitch};
    std::vector<gp_Pnt> points = {gp_Pnt(x[0], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[1])};
    fullshank = Cut(fullshank, Chamfer(points));

    return fullshank;
}
