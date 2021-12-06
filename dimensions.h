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
    QString Prefix(int);
    QStringList Threads();
    QStringList Heads();
    QList<double> GetHeadDims(int, int);

private:
    QStringList threads, heads, prefix;
    QList<double> majors, pitches;
    QList<QList<double>> hex, shcs;
};

#endif // DIMENSIONS_H
