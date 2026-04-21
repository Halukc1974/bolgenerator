#include "bolt.h"
#include "export.h"
#include "nut.h"
#include "parameters.h"
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static double SolidVolumeMm3(const TopoDS_Shape &shape) {
  GProp_GProps props;
  BRepGProp::VolumeProperties(shape, props);
  return props.Mass(); // volume in mm^3 (OCCT "Mass" = integral of 1 over shape)
}

int main(int argc, char *argv[]) {
  // 23 arguments + 1 for program name
  if (argc < 24) {
    std::cerr << "Usage: " << argv[0]
              << " <name> <headType> <s> <k> <dw> <c> <r> <socketS> <socketD> "
                 "<d> <L> <ls> <bodyTol> <threadD> <P> <minorD> <genNut> "
                 "<nutS> <nutH> <nutDw> <nutTol> <boltFillet> <nutFillet>"
              << std::endl;
    return 1;
  }

  try {
    BoltParameters p;
    int i = 1;
    std::string name = argv[i++];

    // Head
    p.head.type = static_cast<HeadType>(atoi(argv[i++]));
    p.head.widthAcrossFlats = atof(argv[i++]);
    p.head.height = atof(argv[i++]);
    p.head.washerFaceDiameter = atof(argv[i++]);
    p.head.washerFaceThickness = atof(argv[i++]);
    p.head.underheadFilletRadius = atof(argv[i++]);
    p.head.socketSize = atof(argv[i++]);
    p.head.socketDepth = atof(argv[i++]);

    // Shank
    p.shank.nominalDiameter = atof(argv[i++]);
    p.shank.totalLength = atof(argv[i++]);
    p.shank.gripLength = atof(argv[i++]);
    p.shank.bodyTolerance = atof(argv[i++]);

    // Thread
    p.thread.majorDiameter = atof(argv[i++]);
    p.thread.pitch = atof(argv[i++]);
    p.thread.minorDiameter = atof(argv[i++]);

    // Nut
    p.nut.generate = (atoi(argv[i++]) == 1);
    p.nut.widthAcrossFlats = atof(argv[i++]);
    p.nut.height = atof(argv[i++]);
    p.nut.washerFaceDiameter = atof(argv[i++]);
    p.nut.tolerance = atof(argv[i++]);

    // Edge smoothing
    p.shank.edgeFilletRadius = atof(argv[i++]);
    p.nut.edgeFilletRadius = atof(argv[i++]);

    std::cout << "Starting generation for " << name << "..." << std::endl;

    // ── Generate Bolt ───────────────────────────────────────────────────
    Bolt bolt(p);
    const std::string brepPath = "Tests/" + name + ".brep";
    const std::string stlPath = "Tests/" + name + ".stl";
    const std::string stepPath = "Tests/" + name + ".step";

    ExportBRep(bolt.Solid(), brepPath.c_str());
    ExportSTL(bolt.Solid(), stlPath.c_str());
    ExportSTEP(bolt.Solid(), stepPath.c_str());

    const double boltVolume = SolidVolumeMm3(bolt.Solid());
    // This line is parsed by server.js → response.stats.volume_mm3
    std::cout << "Bolt Volume: " << boltVolume << " mm^3" << std::endl;
    std::cout << "Bolt exported: " << brepPath << std::endl;

    // ── Generate Nut (if requested) ─────────────────────────────────────
    if (p.nut.generate) {
      Nut nut(p);
      const std::string nutBrepPath = "Tests/" + name + "_nut.brep";
      const std::string nutStlPath = "Tests/" + name + "_nut.stl";
      const std::string nutStepPath = "Tests/" + name + "_nut.step";

      ExportBRep(nut.Solid(), nutBrepPath.c_str());
      ExportSTL(nut.Solid(), nutStlPath.c_str());
      ExportSTEP(nut.Solid(), nutStepPath.c_str());

      const double nutVolume = SolidVolumeMm3(nut.Solid());
      std::cout << "Nut Volume: " << nutVolume << " mm^3" << std::endl;
      std::cout << "Nut exported: " << nutBrepPath << std::endl;
    }

  } catch (const std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
