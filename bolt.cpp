#include "bolt.h"

Bolt::Bolt(int indexHead, int indexThread, double length)
{
    body = Head(indexHead, indexThread);
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
