/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#ifndef EXPORT_H
#define EXPORT_H

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>

//#include <Interface_Static.hxx>   // Unnecessary with current solution
//#include <Standard_Integer.hxx>   // Unnecessary with current solution

void ExportBRep(TopoDS_Shape shape,
                Standard_CString filename);

void ExportSTEP(TopoDS_Shape shape,
                Standard_CString filename);

#endif // EXPORT_H
