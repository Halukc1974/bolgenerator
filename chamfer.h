/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#ifndef CHAMFER_H
#define CHAMFER_H

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepSweep_Revol.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Solid.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pnt.hxx>
#include <vector>

TopoDS_Solid Chamfer(std::vector<gp_Pnt> points);

#endif
