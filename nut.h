/*
    BoltGenerator - Nut component
    Copyright (C) 2025
*/

#ifndef NUT_H
#define NUT_H

#include <TopoDS_Solid.hxx>

class Nut
{
    public:
        Nut(double boltMajorD, double pitch, double nutHeight, double acrossFlats, double tolerance);
        TopoDS_Solid Solid();
    
    private:
        double boltMajorD;    // Bolt major diameter
        double pitch;         // Thread pitch
        double nutHeight;     // Nut thickness
        double acrossFlats;   // Hex across flats
        double tolerance;     // Thread tolerance clearance
        TopoDS_Solid body;
};

#endif // NUT_H
