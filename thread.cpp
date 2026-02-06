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

#include "thread.h"

TopoDS_Solid Thread(double diameter, // Minor Diameter
                    double pitch, double length) {
  // ISO-style 60 degree thread profile
  // We make it slightly deeper to ensure it always cuts the shank
  const double depth = 0.614 * pitch; // Standard ISO depth is 0.614p
  const double h_clearance =
      0.05 * pitch; // Add small radial clearance for robustness

  std::vector<gp_Pnt> vertex;
  // (x, y, z)
  // x is radial distance from center
  // We start slightly inside the major diameter and go out
  vertex = {gp_Pnt(0.5 * diameter - h_clearance, 0.0, -0.125 * pitch),
            gp_Pnt(0.5 * diameter - h_clearance, 0.0, 0.125 * pitch),
            gp_Pnt(0.5 * diameter + depth + h_clearance, 0.0, 0.5 * pitch),
            gp_Pnt(0.5 * diameter + depth + h_clearance, 0.0, -0.5 * pitch)};

  BRepBuilderAPI_MakeWire wire;
  for (int ct = 0; ct < int(vertex.size()); ct++)
    wire.Add(BRepBuilderAPI_MakeEdge(vertex.at(ct),
                                     vertex.at((ct + 1) % vertex.size()))
                 .Edge());

  // Make the helix slightly longer than requested to prevent cap-face issues
  return Helix(wire, diameter, pitch, length);
}
