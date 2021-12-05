#include "dimensions.h"

Dimensions::Dimensions()
{
    threads.append("Thread Type");
    majors.append(-1);
    pitches.append(-1);
    hex.append(QList<double>({0, 0}));

    threads.append("1/4\"-20 UNC");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/20.0));
    hex.append(QList<double>({inchTOmeter(11.0/64.0), inchTOmeter(7.0/16.0)}));

    threads.append("1/4\"-28 UNF");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/28.0));
    hex.append(QList<double>({inchTOmeter(11.0/64.0), inchTOmeter(7.0/16.0)}));

    threads.append("1/4\"-32 UNEF");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/32.0));
    hex.append(QList<double>({inchTOmeter(11.0/64.0), inchTOmeter(7.0/16.0)}));

    threads.append("5/16\"-18 UNC");
    majors.append(inchTOmeter(0.3125));
    pitches.append(inchTOmeter(1.0/18.0));
    hex.append(QList<double>({}));

    threads.append("5/16\"-24 UNF");
    majors.append(inchTOmeter(0.3125));
    pitches.append(inchTOmeter(1.0/24.0));
    hex.append(QList<double>({}));

    threads.append("5/16\"-32 UNEF");
    majors.append(inchTOmeter(0.3125));
    pitches.append(inchTOmeter(1.0/32.0));
    hex.append(QList<double>({}));

    threads.append("M5x0.8");
    majors.append(5.0e-3);
    pitches.append(0.8e-3);
    hex.append(QList<double>({}));

    heads.append("Head Type");
    heads.append("Hex");
}

double Dimensions::Major(int index)
{
    return majors.at(index);
}

double Dimensions::Pitch(int index)
{
    return pitches.at(index);
}

QStringList Dimensions::GetThreads()
{
    return threads;
}

QStringList Dimensions::GetHeads()
{
    return heads;
}

QList<double> Dimensions::GetHeadDims(int indexHead, int indexThread)
{
    if(indexHead == 1)
        return hex.at(indexThread);
}
