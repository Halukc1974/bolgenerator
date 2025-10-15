#include "bolt.h"
#include <stdexcept>
#include <vector>
#include <cmath>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

Bolt::Bolt(double majord_,
           double length_,
           double pitch_,
           double headD1_,
           double headD2_,
           double headD3_,
           double headD4_,
           int headType_)
    : majord(majord_),
      length(length_),
      pitch(pitch_),
      headD1(headD1_),
      headD2(headD2_),
      headD3(headD3_),
      headD4(headD4_),
      headType(headType_)
{
    TopoDS_Solid shank = Shank();

    // Rotate shank 180 degrees around X axis to point chamfered end down
    gp_Trsf rotateShank;
    rotateShank.SetRotation(gp_Ax1(gp_Pnt(0.0, 0.0, 0.5 * length), gp_Dir(1.0, 0.0, 0.0)), M_PI);
    shank = TopoDS::Solid(BRepBuilderAPI_Transform(shank, rotateShank).Shape());

    TopoDS_Solid head = Head();

    // Place head just above the shank
    const double fuseOverlap = (length > 0.2) ? 0.1 : 0.5 * length;
    gp_Trsf headPlacement;
    headPlacement.SetTranslation(gp_Vec(0.0, 0.0, length - fuseOverlap));
    TopoDS_Solid placedHead = TopoDS::Solid(BRepBuilderAPI_Transform(head, headPlacement).Shape());

    BRepAlgoAPI_Fuse fuseOp(shank, placedHead);
    fuseOp.Build();

    std::vector<TopoDS_Solid> fusedSolids;
    for (TopExp_Explorer map(fuseOp.Shape(), TopAbs_SOLID); map.More(); map.Next()) {
        fusedSolids.push_back(TopoDS::Solid(map.Current()));
    }

    if (fusedSolids.empty()) {
        throw std::runtime_error("Fuse produced no solids");
    }

    // Select largest volume solid
    double maxVolume = -1.0;
    TopoDS_Solid selected = fusedSolids.front();
    for (const auto& solid : fusedSolids) {
        GProp_GProps props;
        BRepGProp::VolumeProperties(solid, props);
        double vol = props.Mass();
        if (vol > maxVolume) {
            maxVolume = vol;
            selected = solid;
        }
    }

    body = selected;
}


TopoDS_Solid Bolt::Solid() {
    return body;
}


TopoDS_Solid Bolt::Shank()
{
    TopoDS_Solid mask, shank, thread;
    gp_Trsf offset;

    double nthreads = std::ceil((length + 2 * pitch) / pitch);

    // Base cylinder
    shank = BRepPrimAPI_MakeCylinder(0.5 * majord, pitch * nthreads);

    // Threads
    thread = Thread(majord - 2.0 * 3.0 / 8.0 * pitch, pitch, pitch * nthreads);
    shank = Cut(shank, thread);

    // Head-side cleanup
    mask = BRepPrimAPI_MakeCylinder(majord, pitch).Solid();
    shank = Cut(shank, mask);

    // End cleanup
    offset.SetTranslation(gp_Vec(0.0, 0.0, length + pitch));
    mask = BRepPrimAPI_MakeCylinder(majord, nthreads * pitch - pitch - length).Solid();
    shank = Cut(shank, BRepBuilderAPI_Transform(mask, offset).Shape());

    // Shift down to align base at z=0
    offset.SetTranslation(gp_Vec(0.0, 0.0, -pitch));
    shank = TopoDS::Solid(BRepBuilderAPI_Transform(shank, offset));

    // Chamfer
    double x[] = {0.5 * majord - pitch, majord};
    double z[] = {length, length - 0.5 * majord - pitch};
    std::vector<gp_Pnt> points = {
        gp_Pnt(x[0], 0.0, z[0]),
        gp_Pnt(x[1], 0.0, z[0]),
        gp_Pnt(x[1], 0.0, z[1])
    };
    shank = Cut(shank, Chamfer(points));

    return shank;
}


TopoDS_Solid Bolt::Head()
{
    TopoDS_Solid head;

    if (headType == 0) { // Hex head
        if (headD1 <= 0 || headD2 <= 0) {
            throw std::runtime_error("Hex head: invalid dimensions");
        }
        head = Hexagon(headD1, headD2);
    }
    else if (headType == 1) { // Socket head (cylinder + hex socket)
        if (headD1 <= 0 || headD2 <= 0 || headD3 <= 0 || headD4 <= 0) {
            throw std::runtime_error("Socket head: invalid dimensions");
        }
        if (headD3 >= headD1 * 0.9) {
            throw std::runtime_error("Socket size (D3) must be smaller than head diameter (D1)");
        }
        if (headD4 > headD2) {
            throw std::runtime_error("Socket depth (D4) cannot exceed head height (D2)");
        }

        // Head cylinder
        head = BRepPrimAPI_MakeCylinder(0.5 * headD1, headD2);

        // Create hex socket
        const double toolHeight = headD4;
        TopoDS_Solid socket = Hexagon(headD3, toolHeight);

        // Shift socket upward (as requested)
        const double offsetZ = headD2 - headD4;
        gp_Trsf socketOffset;
        socketOffset.SetTranslation(gp_Vec(0.0, 0.0, offsetZ));
        TopoDS_Solid positionedSocket = TopoDS::Solid(BRepBuilderAPI_Transform(socket, socketOffset).Shape());

        // Cut socket from cylinder
        head = Cut(head, positionedSocket);
    }
    else if (headType == 2) { // Flat head
        if (headD1 <= 0 || headD2 <= 0) {
            throw std::runtime_error("Flat head: invalid dimensions");
        }
        head = BRepPrimAPI_MakeCylinder(0.5 * headD1, headD2);
    }
    else {
        // Default: hex
        head = Hexagon(headD1, headD2);
    }

    return head;
}
