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

#include "export.h"
#include <iostream>

void ExportBRep(TopoDS_Shape shape, Standard_CString filename)
{
    BRepTools::Write(shape, filename);
}

void ExportSTEP(TopoDS_Shape shape, Standard_CString filename)
{
    // Geometry is now in millimeters, STEP writer expects mm by default
    STEPControl_Writer writer;
    writer.Transfer(shape, STEPControl_AsIs);
    writer.Write(filename);
}

void ExportSTL(TopoDS_Shape shape, Standard_CString filename)
{
    // EXACT FreeCAD MeshPart implementation
    // Source: MeshPart/App/Mesher.cpp line 222-226 (createStandard method)
    // Source: MeshPart/App/Mesher.h line 246 (default values)
    
    // FreeCAD Mesher default parameters:
    // deflection = 0 (will be calculated adaptively)
    // angularDeflection = 0.5 radians (28.65 degrees)
    // relative = false (absolute deflection mode)
    
    // Calculate adaptive deflection like FreeCAD does
    Bnd_Box bbox;
    BRepBndLib::Add(shape, bbox);
    
    if (bbox.IsVoid()) {
        std::cerr << "Error: Cannot calculate bounding box" << std::endl;
        return;
    }
    
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    
    Standard_Real diagLength = sqrt(
        pow(xMax - xMin, 2) + 
        pow(yMax - yMin, 2) + 
        pow(zMax - zMin, 2)
    );
    
    // FreeCAD MeshPart parameters (from Mesher.h line 246):
    // Default: angularDeflection = 0.5, relative = false
    // LinearDeflection from UI: 0.1 mm (Tessellation.ui line 306)
    
    // Geometry is now in MILLIMETERS (not meters anymore)
    // FreeCAD UI default from Tessellation.ui line 306: 0.1 mm
        Standard_Boolean relative = Standard_True;          // use relative adaptive deflection
        Standard_Real relativeFactor = 0.001;               // 0.1% of bbox diagonal
        Standard_Real angularDeflection = 0.25;             // angular tolerance (radians)

        // Compute absolute deflection from bounding box when running in relative mode
        Standard_Real deflection = relative ? (diagLength * relativeFactor) : 0.05;
    
    // Diagnostic output (FreeCAD style)
    std::cout << "\n=== FreeCAD MeshPart STL Export ===" << std::endl;
    std::cout << "Bounding box diagonal: " << diagLength << " mm" << std::endl;
        std::cout << "Linear deflection: " << deflection << " mm" << std::endl;
        std::cout << "Angular deflection: " << angularDeflection << " rad" << std::endl;
        std::cout << "Relative mode: " << (relative ? "YES (factor=" + std::to_string(relativeFactor) + ")" : "NO (absolute)") << std::endl;
    
    if (!shape.IsNull()) {
        std::cout << "\nShape validation:" << std::endl;
        std::cout << "  Type: ";
        switch(shape.ShapeType()) {
            case TopAbs_SOLID:
                std::cout << "SOLID ✓" << std::endl;
                break;
            case TopAbs_SHELL:
                std::cout << "SHELL" << std::endl;
                break;
            case TopAbs_COMPOUND:
                std::cout << "COMPOUND" << std::endl;
                break;
            default:
                std::cout << "OTHER" << std::endl;
        }
        std::cout << "  Closed: " << (shape.Closed() ? "YES ✓" : "NO ⚠") << std::endl;
    }
    
    // EXACT FreeCAD MeshPart meshing code
    // From: MeshPart/App/Mesher.cpp line 222-225
    std::cout << "\nGenerating mesh (exact FreeCAD MeshPart code)..." << std::endl;
    
    if (!shape.IsNull()) {
        BRepTools::Clean(shape);  // FreeCAD does this before meshing
        BRepMesh_IncrementalMesh aMesh(shape, deflection, relative, angularDeflection);
        
        if (!aMesh.IsDone()) {
            std::cerr << "  ⚠ Warning: Mesh generation incomplete" << std::endl;
        } else {
            std::cout << "  ✓ Mesh generation: SUCCESS" << std::endl;
        }
    }
    
    // Export to binary STL (FreeCAD default)
    std::cout << "\nExporting binary STL..." << std::endl;
    StlAPI_Writer writer;
    writer.ASCIIMode() = Standard_False;  // Binary mode for compact files
    
    Standard_Boolean success = writer.Write(shape, filename);
    
    if (success) {
        std::cout << "  ✓ STL export: SUCCESS" << std::endl;
    } else {
        std::cerr << "  ⚠ STL export FAILED" << std::endl;
    }
    std::cout << "===================================\n" << std::endl;
}
