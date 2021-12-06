#ifndef BOLT_H
#define BOLT_H

#include <TopoDS_Solid.hxx>

#include "chamfer.h"
#include "dimensions.h"
#include "export.h"
#include "hexagon.h"
#include "thread.h"

class Bolt : Dimensions
{
public:
    Bolt(int, int, double, bool=false);
    TopoDS_Solid Solid();

private:
    TopoDS_Solid Head(int, int);
    TopoDS_Solid Shank(int, double, bool);
    bool simple;
    Dimensions dimensions;
    TopoDS_Solid body;
};

#endif // BOLT_H
