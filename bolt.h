#ifndef BOLT_H
#define BOLT_H

#include <TopoDS_Solid.hxx>

#include "dimensions.h"
#include "hexagon.h"

class Bolt : Dimensions
{
public:
    Bolt(int, int, double);
    TopoDS_Solid Solid();

private:
    TopoDS_Solid Head(int, int);
    TopoDS_Solid body;
    Dimensions dimensions;
};

#endif // BOLT_H
