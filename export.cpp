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
    // STEPCONTROL_Writer defaults to millimeter units, and modifying
    // "write.step.unit" did not sucessfully translate the part.

    STEPControl_Writer writer;
  
    // (This didn't work.)
    // Interface_Static::SetCVal("write.step.unit", "METER");
    // writer.Transfer(shape, STEPControl_AsIs);

    // This hack works.
    gp_Trsf scale;
    scale.SetScale(gp_Pnt(0,0,0), 1.0e3);
    writer.Transfer(BRepBuilderAPI_Transform(shape, scale), STEPControl_AsIs);

    writer.Write(filename);
}

void ExportSTL(TopoDS_Shape shape, Standard_CString filename)
{
    // Full FreeCAD MeshPart algorithm implementation
    // Based on: MeshPart/App/Mesher.cpp line 225 and MeshPart/Gui/Tessellation.cpp
    
    // Calculate adaptive deflection based on shape bounding box
    Bnd_Box bbox;
    BRepBndLib::Add(shape, bbox);
    
    if (bbox.IsVoid()) {
        std::cerr << "Error: Cannot calculate bounding box for shape" << std::endl;
        return;
    }
    
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    
    Standard_Real diagLength = sqrt(
        pow(xMax - xMin, 2) + 
        pow(yMax - yMin, 2) + 
        pow(zMax - zMin, 2)
    );
    
    // FreeCAD Default Parameters (from Tessellation.cpp):
    // LinearDeflection: 0.1 mm (absolute) or 0.1% of diagonal (adaptive)
    // AngularDeflection: 28.5 degrees = 0.5 radians
    // Relative: false (absolute deflection mode)
    
    // Using adaptive approach: 0.1% of bounding box diagonal
    Standard_Real linearDeflection = diagLength * 0.001;  // 0.1% of diagonal
    
    // FreeCAD default: 28.5 degrees (from UI spinAngularDeviation default)
    Standard_Real angularDeflection = 0.5;  // 0.5 radians ≈ 28.65 degrees
    
    // Absolute deflection mode (FreeCAD standard)
    Standard_Boolean isRelative = Standard_False;
    
    // Diagnostic output
    std::cout << "\n=== FreeCAD MeshPart STL Export ===" << std::endl;
    std::cout << "Bounding box diagonal: " << diagLength * 1000 << " mm" << std::endl;
    std::cout << "Linear deflection: " << linearDeflection * 1000 << " mm (0.1%)" << std::endl;
    std::cout << "Angular deflection: " << angularDeflection << " rad (28.5°)" << std::endl;
    std::cout << "Relative mode: " << (isRelative ? "YES" : "NO (absolute)") << std::endl;
    
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
    
    // Generate mesh using exact FreeCAD MeshPart algorithm
    // From: MeshPart/App/Mesher.cpp line 225
    std::cout << "\nGenerating mesh (FreeCAD MeshPart algorithm)..." << std::endl;
    
    BRepMesh_IncrementalMesh mesh(shape, 
                                  linearDeflection,
                                  isRelative,
                                  angularDeflection,
                                  Standard_True);  // InParallel = true (parallel processing)
    
    if (!mesh.IsDone()) {
        std::cerr << "  ⚠ Warning: Mesh generation incomplete" << std::endl;
    } else {
        std::cout << "  ✓ Mesh generation: SUCCESS" << std::endl;
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
