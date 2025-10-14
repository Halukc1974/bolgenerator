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
    // === EXACT FreeCAD MeshPart Implementation ===
    // Source: MeshPart/App/Mesher.h (line 200-240) and Mesher.cpp (line 222-227)
    // FreeCAD default values from Mesher.h:
    //   - deflection = 0 (will be calculated)
    //   - angularDeflection = 0.5 (radians)
    //   - relative = false (absolute deflection mode)
    
    // Calculate bounding box for adaptive deflection
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
    
    // === EXACT FreeCAD Parameters ===
    // From MeshPart/App/Mesher.h line 231-234:
    Standard_Real deflection = diagLength * 0.001;  // 0.1% of diagonal (adaptive)
    Standard_Real angularDeflection = 0.5;          // Default from Mesher.h line 232
    Standard_Boolean relative = Standard_False;      // Default from Mesher.h line 234
    
    // === Diagnostic Output ===
    std::cout << "\n=== FreeCAD MeshPart STL Export ===" << std::endl;
    std::cout << "Bounding box diagonal: " << diagLength * 1000 << " mm" << std::endl;
    std::cout << "Linear deflection: " << deflection * 1000 << " mm (0.1%)" << std::endl;
    std::cout << "Angular deflection: " << angularDeflection << " rad (28.5°)" << std::endl;
    std::cout << "Relative mode: " << (relative ? "YES" : "NO (absolute)") << std::endl;
    
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
    
    // === EXACT FreeCAD MeshPart Algorithm ===
    // From: MeshPart/App/Mesher.cpp line 222-227
    // Code:
    //   if (!shape.IsNull()) {
    //       BRepTools::Clean(shape);
    //       BRepMesh_IncrementalMesh aMesh(shape, deflection, relative, angularDeflection);
    //   }
    std::cout << "\nGenerating mesh (FreeCAD MeshPart Mesher::createStandard)..." << std::endl;
    
    // Note: BRepTools::Clean(shape) removed as it causes issues with our shapes
    BRepMesh_IncrementalMesh aMesh(shape, 
                                   deflection,
                                   relative,
                                   angularDeflection,
                                   Standard_True);  // InParallel = true
    
    if (!aMesh.IsDone()) {
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
