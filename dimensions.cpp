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

#include "dimensions.h"

Dimensions::Dimensions()
{
    threads.append("Thread Type");
    prefix.append("");
    majors.append(-1);
    pitches.append(-1);
    hex.append(QList<double>({0, 0}));

    threads.append("#08-32 UNC");
    prefix.append("UN_No08_32_");
    majors.append(inchTOmeter(0.164));
    pitches.append(inchTOmeter(1.0/32.0));
    hex.append(QList<double>({inchTOmeter(7.0/64.0), inchTOmeter(1.0/4.0)}));

    threads.append("#10-24 UNC");
    prefix.append("UN_No10_24_");
    majors.append(inchTOmeter(0.190));
    pitches.append(inchTOmeter(1.0/24.0));
    hex.append(QList<double>({inchTOmeter(1.0/8.0), inchTOmeter(5.0/16.0)}));

    threads.append("#10-32 UNF");
    prefix.append("UN_No10_32_");
    majors.append(inchTOmeter(0.190));
    pitches.append(inchTOmeter(1.0/32.0));
    hex.append(QList<double>({inchTOmeter(1.0/8.0), inchTOmeter(5.0/16.0)}));

    threads.append("1/4\"-20 UNC");
    prefix.append("UN_0250_20_");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/20.0));
    hex.append(QList<double>({inchTOmeter(11.0/64.0), inchTOmeter(7.0/16.0)}));

    threads.append("1/4\"-28 UNF");
    prefix.append("UN_0250_28_");
    majors.append(inchTOmeter(0.250));
    pitches.append(inchTOmeter(1.0/28.0));
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

    threads.append("3/8\"-16 UNC");
    prefix.append("UN_0375_16_");
    majors.append(inchTOmeter(3.0/8.0));
    pitches.append(inchTOmeter(1.0/16.0));
    hex.append(QList<double>({inchTOmeter(1.0/4.0), inchTOmeter(9.0/16.0)}));

    threads.append("3/8\"-24 UNF");
    prefix.append("UN_0375_24_");
    majors.append(inchTOmeter(0.375));
    pitches.append(inchTOmeter(1.0/24.0));
    hex.append(QList<double>({inchTOmeter(1.0/4.0), inchTOmeter(9.0/16.0)}));

    threads.append("3/8\"-32 UNEF");
    prefix.append("UN_0375_32_");
    majors.append(inchTOmeter(0.375));
    pitches.append(inchTOmeter(1.0/32.0));
    hex.append(QList<double>({inchTOmeter(1.0/4.0), inchTOmeter(9.0/16.0)}));

    threads.append("7/16\"-14 UNC");
    prefix.append("UN_0438_32_");
    majors.append(inchTOmeter(0.4375));
    pitches.append(inchTOmeter(1.0/14.0));
    hex.append(QList<double>({inchTOmeter(19.0/64.0), inchTOmeter(5.0/8.0)}));

    threads.append("7/16\"-20 UNF");
    prefix.append("UN_0438_20_");
    majors.append(inchTOmeter(0.4375));
    pitches.append(inchTOmeter(1.0/20.0));
    hex.append(QList<double>({inchTOmeter(19.0/64.0), inchTOmeter(5.0/8.0)}));

    /* Omitted for now. Small bolts break the program.
    threads.append("M3x0.5");
    prefix.append("M_04_050e-2_");
    majors.append(3.0e-3);
    pitches.append(0.5e-3);
    hex.append(QList<double>({2.1e-3, 5.5e-3}));
    */

    threads.append("M4x0.7");
    prefix.append("M_04_070e-2_");
    majors.append(4.0e-3);
    pitches.append(0.7e-3);
    hex.append(QList<double>({2.9e-3, 7.0e-3}));

    threads.append("M5x0.8");
    prefix.append("M_05_080e-2_");
    majors.append(5.0e-3);
    pitches.append(0.8e-3);
    hex.append(QList<double>({3.5e-3, 8.0e-3}));

    threads.append("M5x1.0");
    prefix.append("M_05_100e-2_");
    majors.append(5.0e-3);
    pitches.append(1.0e-3);
    hex.append(QList<double>({3.5e-3, 8.0e-3}));

    threads.append("M6x0.75");
    prefix.append("M_06_075e-2_");
    majors.append(6.0e-3);
    pitches.append(0.75e-3);
    hex.append(QList<double>({4.0e-3, 10.0e-3}));

    threads.append("M6x1.0");
    prefix.append("M_06_100e-2_");
    majors.append(6.0e-3);
    pitches.append(1.0e-3);
    hex.append(QList<double>({4.0e-3, 10.0e-3}));

    threads.append("M8x1.0");
    prefix.append("M_08_100e-2_");
    majors.append(8.0e-3);
    pitches.append(1.0e-3);
    hex.append(QList<double>({5.3e-3, 13.0e-3}));

    threads.append("M8x1.25");
    prefix.append("M_08_125e-2_");
    majors.append(8.0e-3);
    pitches.append(1.25e-3);
    hex.append(QList<double>({5.3e-3, 13.0e-3}));

    threads.append("M10x1.25");
    prefix.append("M_10_125e-2_");
    majors.append(10.0e-3);
    pitches.append(1.25e-3);
    hex.append(QList<double>({6.4e-3, 17e-3}));

    threads.append("M10x1.5");
    prefix.append("M_10_150_");
    majors.append(10.0e-3);
    pitches.append(1.5e-3);
    hex.append(QList<double>({6.4e-3, 17e-3}));

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
