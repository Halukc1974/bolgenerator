/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#ifndef CUT_H
#define CUT_H

#include <BRepAlgoAPI_Cut.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>

TopoDS_Solid Cut(TopoDS_Shape body, TopoDS_Shape tool);

#endif
