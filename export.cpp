/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#include "export.h"

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
    std::cout << filename << std::endl;
    gp_Trsf scale;
    scale.SetScale(gp_Pnt(0,0,0), 1.0e3);
    writer.Transfer(BRepBuilderAPI_Transform(shape, scale), STEPControl_AsIs);
    writer.Write(filename);
}
