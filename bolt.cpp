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
    TopoDS_Solid mask, shank, thread, fullshank;
    gp_Trsf offset;

    double major = dimensions.Major(indexThread);
    double pitch = dimensions.Pitch(indexThread);

    //    double coeff = 10.0;
    //    while(coeff*pitch <= length+2*pitch)
    //        coeff += 10;

    //double modlength = 10*ceil(length/pitch/10)*pitch;
    double modlength = 10*pitch;

    // Add +2*pitch so that the geometry can be cleaned at both ends
    shank = BRepPrimAPI_MakeCylinder(0.5*major, modlength+2*pitch);

    if(!simple)
    {
        thread = Thread(major-2.0*3.0/8.0*pitch, pitch, modlength+2*pitch, true);
        shank = Cut(shank, thread);
    }

    // Remove head-side partial thread
    mask = BRepPrimAPI_MakeCylinder(major, 2*pitch).Solid();
    offset.SetTranslation(gp_Vec(0.0, 0.0, -1.0*pitch));
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    // Remove end-side partial thread
    offset.SetTranslation(gp_Vec(0.0, 0.0, modlength+pitch));
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    fullshank = shank;
    int ct = 1;
    while(ct*modlength <= length)
    {
        offset.SetTranslation(gp_Vec(0.0, 0.0, ct*modlength));
        fullshank = TopoDS::Solid(TopExp_Explorer(BRepAlgoAPI_Fuse(fullshank, TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset).Shape())), TopAbs_SOLID).Current());
        ct++;
    }
    /*
    offset.SetTranslation(gp_Vec(0.0, 0.0, modlength));
    TopExp_Explorer map(BRepAlgoAPI_Fuse(fullshank,
                                         TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset).Shape())),
                        TopAbs_SOLID);
    fullshank = TopoDS::Solid(map.Current());

    offset.SetTranslation(gp_Vec(0.0, 0.0, 2*modlength));
    //TopExp_Explorer map(BRepAlgoAPI_Fuse(fullshank,
    //                                     TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset).Shape())),
    //                    TopAbs_SOLID);
    //fullshank = TopoDS::Solid(map.Current());
    */


    /*
    fullshank = shank;
    int coeff;
    for(coeff = 1; coeff*pitch < length; coeff+=5)
    {
        offset.SetTranslation(gp_Vec(0.0, 0.0, pitch*coeff));
        TopExp_Explorer map(BRepAlgoAPI_Fuse(fullshank,
                                             TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset).Shape())),
                            TopAbs_SOLID);
        fullshank = TopoDS::Solid(map.Current());
    }
    */

    ExportBRep(fullshank, "/home/samjacobs/Documents/CAD/shank.brep");

    // Currently the shank is floating off of the origin by dz=pitch.
    offset.SetTranslation(gp_Vec(0.0, 0.0, -pitch));
    fullshank = TopoDS::Solid(BRepBuilderAPI_Transform(fullshank, offset));

    double x[] = {0.5*major-pitch,
                  major};
    double z[] = {length,
                  length-major};

    /*
    std::vector<gp_Pnt> points = {gp_Pnt(x[0], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[1])};
    */


    std::vector<gp_Pnt> points = {gp_Pnt(0.0, 0.0, z[0]),
                                  gp_Pnt(x[0], 0.0, z[0]),
                                  gp_Pnt(x[1], 0.0, z[1]),
                                  gp_Pnt(x[1], 0.0, z[0]+modlength),
                                  gp_Pnt(0.0, 0.0, z[0]+modlength)};

    fullshank = Cut(fullshank, Chamfer(points));

    return fullshank;
}
