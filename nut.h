/*
    BoltGenerator - Nut component
    Copyright (C) 2025
*/

#ifndef NUT_H
#define NUT_H

#include <TopoDS_Solid.hxx>

#include "parameters.h"

class Nut {
public:
  Nut(const BoltParameters &);
  TopoDS_Solid Solid();

private:
  BoltParameters params;
  TopoDS_Solid body;
};

#endif // NUT_H
