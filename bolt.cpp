#include "bolt.h"

Bolt::Bolt(int indexHead,
           int indexThread,
           double length,
           bool simple)
{
    //body = Head(indexHead, indexThread);
    TopExp_Explorer map(BRepAlgoAPI_Fuse(Shank(indexThread, length, simple),
                                         Head(indexHead, indexThread)),
                                         TopAbs_SOLID);
    body = TopoDS::Solid(map.Current());
}

TopoDS_Solid Bolt::Solid()
{
    return body;
}

#include <iostream>

TopoDS_Solid Bolt::Head(int indexHead, int indexThread)
{
    TopoDS_Solid output;
    double thickness = dimensions.GetHeadDims(indexHead, indexThread).at(0);
    gp_Trsf offset;

    if(indexHead == 1) // Hex Bolt
    {
        double aflats = dimensions.GetHeadDims(indexHead, indexThread).at(1);
        output = Hexagon(aflats, thickness);
    }
    /*
    else if(ct == 1) // SHCS
    {
        double diam = dimSHCS.at(IndexThread()).at(0);
        thickness = dimSHCS.at(IndexThread()).at(1);
        double aflats = dimSHCS.at(IndexThread()).at(2);
        TopoDS_Solid hexPrism = Hexagon(aflats, aflats);
        output = BRepPrimAPI_MakeCylinder(0.5*diam, thickness);
        output = Cut(output, hexPrism);
        //output = Hexagon(aflats, thickness);
    }
    */

    offset.SetTranslation(gp_Vec(0.0, 0.0, -thickness));
    return TopoDS::Solid(BRepBuilderAPI_Transform(output, offset));
}

#include <BRepAlgoAPI_Fuse.hxx>

TopoDS_Solid Bolt::Shank(int indexThread, double length, bool simple)
{
    TopoDS_Solid mask, shank, thread;
    gp_Trsf offset;

    double major = dimensions.Major(indexThread);
    double pitch = dimensions.Pitch(indexThread);

    // Add +2*pitch so that the geometry can be cleaned at both ends
    shank = BRepPrimAPI_MakeCylinder(0.5*major, pitch);//length+2*pitch);

    if(!simple)
    {
        thread = Thread(major-2.0*3.0/8.0*pitch, pitch, 2*pitch, true);//length+2*pitch, true);

        ExportBRep(shank, "shank.brep");
        ExportBRep(thread, "thread.brep");
        //shank = TopoDS::Solid(BRepAlgoAPI_Fuse(shank, thread));
        shank = Cut(shank, thread);
    }
    ExportBRep(shank, "cut.brep");

    // Remove head-side partial thread
    mask = BRepPrimAPI_MakeCylinder(major, 2*pitch).Solid();
    offset.SetTranslation(gp_Vec(0.0, 0.0, -1.0*pitch));
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    // Remove end-side partial thread
    offset.SetTranslation(gp_Vec(0.0, 0.0, length+pitch));
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    // Currently the shank is floating off of the origin by dz=pitch.
    offset.SetTranslation(gp_Vec(0.0, 0.0, -pitch));
    shank = TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset));

    double x[] = {0.5*major-pitch,
                  major};
    double z[] = {length,
                  length-major};

    std::vector<gp_Pnt> points = {gp_Pnt(x[0], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[1])};

    shank = Cut(shank, Chamfer(points));

    return shank;
}
