#include "bolt.h"
#include "export.h"
#include "nut.h"
#include "parameters.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  // New argument list (22 arguments + 1 for program name)
  if (argc < 23) {
    std::cerr << "Usage: " << argv[0]
              << " <name> <headType> <s> <k> <dw> <c> <r> <socketS> <socketD> "
                 "<d> <L> <ls> <bodyTol> <threadD> <P> <minorD> <genNut> "
                 "<nutS> <nutH> <nutDw> <nutTol> <edgeFillet>"
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

    std::cout << "Starting generation for " << name << "..." << std::endl;

    // Generate Bolt
    Bolt bolt(p);
    std::string brepPath = std::string("Tests/").append(name).append(".brep");
    std::string stlPath = std::string("Tests/").append(name).append(".stl");

    ExportBRep(bolt.Solid(), brepPath.c_str());
    ExportSTL(bolt.Solid(), stlPath.c_str());
    std::cout << "Bolt exported: " << brepPath << std::endl;

    // Generate Nut if requested
    if (p.nut.generate) {
      Nut nut(p);
      std::string nutBrepPath =
          std::string("Tests/").append(name).append("_nut.brep");
      std::string nutStlPath =
          std::string("Tests/").append(name).append("_nut.stl");

      ExportBRep(nut.Solid(), nutBrepPath.c_str());
      ExportSTL(nut.Solid(), nutStlPath.c_str());
      std::cout << "Nut exported: " << nutBrepPath << std::endl;
    }

  } catch (const std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
