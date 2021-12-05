/*
    Copyright (c) 2021 Scimulate LLC <solvers@scimulate.com>
*/

#include "convert.h"

double degTOrad(double deg)
{
    return M_PI/180.0*deg;
}

double radTOdeg(double rad)
{
    return 180.0/M_PI*rad;
}
double inchTOmeter(double inch)
{
    return 2.54e-2*inch;
}
