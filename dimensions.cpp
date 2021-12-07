#include "dimensions.h"

Dimensions::Dimensions()
{
    threads.append("Thread Type");
    prefix.append("");
    majors.append(-1);
    pitches.append(-1);
    hex.append(QList<double>({0, 0}));

    threads.append("1/4\"-20 UNC");
    prefix.append("UN_0250_20_");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/20.0));
    hex.append(QList<double>({inchTOmeter(11.0/64.0), inchTOmeter(7.0/16.0)}));

    threads.append("1/4\"-28 UNF");
    prefix.append("UN_0250_28_");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/28.0));
    //pitches.append(inchTOmeter(0.035714));
    hex.append(QList<double>({inchTOmeter(11.0/64.0), inchTOmeter(7.0/16.0)}));

    threads.append("1/4\"-32 UNEF");
    prefix.append("UN_0250_32_");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/32.0));
    hex.append(QList<double>({inchTOmeter(11.0/64.0), inchTOmeter(7.0/16.0)}));

    threads.append("5/16\"-18 UNC");
    prefix.append("UN_0313_18_");
    majors.append(inchTOmeter(0.3125));
    pitches.append(inchTOmeter(1.0/18.0));
    hex.append(QList<double>({inchTOmeter(14.0/64.0), inchTOmeter(1.0/2.0)}));

    threads.append("5/16\"-24 UNF");
    prefix.append("UN_0313_24_");
    majors.append(inchTOmeter(0.3125));
    pitches.append(inchTOmeter(1.0/24.0));
    hex.append(QList<double>({inchTOmeter(14.0/64.0), inchTOmeter(1.0/2.0)}));

    threads.append("5/16\"-32 UNEF");
    prefix.append("UN_0313_32_");
    majors.append(inchTOmeter(0.3125));
    pitches.append(inchTOmeter(1.0/32.0));
    hex.append(QList<double>({inchTOmeter(14.0/64.0), inchTOmeter(1.0/2.0)}));

    threads.append("M5x0.8");
    prefix.append("M_05_080_");
    majors.append(5.0e-3);
    pitches.append(0.8e-3);
    hex.append(QList<double>({3.5e-3, 8.0e-3}));

    threads.append("M5x1.0");
    prefix.append("M_05_100_");
    majors.append(5.0e-3);
    pitches.append(1.0e-3);
    hex.append(QList<double>({3.5e-3, 8.0e-3}));

    threads.append("M10x1.5");
    prefix.append("M_10_150_");
    majors.append(10.0e-3);
    pitches.append(1.5e-3);
    hex.append(QList<double>({6.4e-3, 17e-3}));

    threads.append("M50x8.0");
    prefix.append("M_50_800_");
    majors.append(5.0e-2);
    pitches.append(0.8e-2);
    hex.append(QList<double>({3.5e-2, 8.0e-2}));

    heads.append("Head Type");
    heads.append("HEX");
    //heads.append("SHCS");
}

double Dimensions::Major(int index)
{
    return majors.at(index);
}

double Dimensions::Pitch(int index)
{
    return pitches.at(index);
}

QString Dimensions::Prefix(int index)
{
    return prefix.at(index);
}

QStringList Dimensions::Threads()
{
    return threads;
}

QStringList Dimensions::Heads()
{
    return heads;
}

QList<double> Dimensions::GetHeadDims(int indexHead, int indexThread)
{
    if(indexHead == 1)
        return hex.at(indexThread);
}
