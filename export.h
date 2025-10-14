/*
    BoltGenerator is an automated CAD assistant which produces standard-size 3D
    bolts per ISO and ASME specifications.
    Copyright (C) 2021  Scimulate LLC <solvers@scimulate.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EXPORT_H
#define EXPORT_H

// OpenCASCADE includes for CAD operations
#include <TopoDS_Shape.hxx>
#include <BRepTools.hxx>
#include <STEPControl_Writer.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Trsf.hxx>
#include <Standard_CString.hxx>
#include <TopAbs_ShapeEnum.hxx>

// Bounding box calculation
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

// STL export includes
#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

// Math for adaptive mesh
#include <cmath>

void ExportBRep(TopoDS_Shape shape,
                Standard_CString filename);

void ExportSTEP(TopoDS_Shape shape,
                Standard_CString filename);

void ExportSTL(TopoDS_Shape shape,
               Standard_CString filename);

#endif // EXPORT_H
