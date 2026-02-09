#define _USE_MATH_DEFINES
#include "helix.h"
#include <cmath>
#include <gp_Vec.hxx>

TopoDS_Solid Helix(TopoDS_Wire sketch, double diameter, double pitch,
                   double length) {
  // Create an infinite cylinder coradial with the pitch diameter.
  Handle(Geom_CylindricalSurface) cylinder = new Geom_CylindricalSurface(
      gp_Ax2(gp::Origin(), gp::DZ()), 0.5 * diameter);

  // Map the 3D tool path as a 2D function of z-position and rotation.
  gp_Pnt2d aPnt(0.0, 0.0);
  gp_Dir2d aDir(2.0 * M_PI, pitch);
  Handle(Geom2d_Line) line = new Geom2d_Line(gp_Ax2d(aPnt, aDir));

  // Trim the tool path such that it fits the shank length.
  // Add 1/4 pitch overlap at EACH end to ensure clean boolean cuts at the faces
  const double overlap = 0.25 * pitch;
  const double uStart = -(overlap / pitch) * 2.0 * M_PI;
  const double uEnd = (length / pitch + overlap / pitch) * 2.0 * M_PI;

  Handle(Geom2d_TrimmedCurve) anArc1 =
      new Geom2d_TrimmedCurve(line, uStart, uEnd);
  TopoDS_Edge toolPath = BRepBuilderAPI_MakeEdge(anArc1, cylinder);
  BRepLib::BuildCurves3d(toolPath);

  // Build a solid trace of the 2D path.
  auto threadPipe =
      BRepOffsetAPI_MakePipeShell(BRepBuilderAPI_MakeWire(toolPath));
  threadPipe.Add(sketch);
  threadPipe.SetMode(gp_Vec(0, 0, 1));
  threadPipe.Build();

  if (!threadPipe.IsDone()) {
    throw std::runtime_error("Helix: Pipe shell construction failed");
  }

  threadPipe.MakeSolid();
  return TopoDS::Solid(threadPipe.Shape());
}
