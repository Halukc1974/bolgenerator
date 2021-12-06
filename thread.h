/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#ifndef THREAD_H
#define THREAD_H

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS_Solid.hxx>
#include <gp_Pnt.hxx>
#include <vector>

#include "helix.h"

TopoDS_Solid Thread(double diameter,
                    double pitch,
                    double length,
                    bool external);

#endif // THREAD_H
