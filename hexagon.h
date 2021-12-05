/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#ifndef HEXAGON_H
#define HEXAGON_H

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepSweep_Revol.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>

#include "cut.h"

TopoDS_Solid Hexagon(double aflats, double thickness);

#endif
