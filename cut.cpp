/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#include "cut.h"

TopoDS_Solid Cut(TopoDS_Shape body, TopoDS_Shape tool)
{
    /*
        BRepAlgoAPI_Cut() works well, but it returns type TopoDS_Compound. This
        function accepts the same two arguments, but maps TopoDS_Compound for
        TopoDS_Solid components, which (by design) only contains one solid, and
        returns it.
    */

    TopExp_Explorer map(BRepAlgoAPI_Cut(body, tool), TopAbs_SOLID);
    return TopoDS::Solid(map.Current());
}