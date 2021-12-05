#ifndef DIMENSIONS_H
#define DIMENSIONS_H

#include <QStringList>

#include "convert.h"

class Dimensions
{
public:
    Dimensions();
    double Major(int);
    double Pitch(int);
    QStringList GetThreads();
    QStringList GetHeads();
    QList<double> GetHeadDims(int, int);

private:
    QStringList threads, heads;
    QList<double> majors, pitches;
    QList<QList<double>> hex, shcs;
};

#endif // DIMENSIONS_H
