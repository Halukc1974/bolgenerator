#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum class HeadType { HEX = 0, SOCKET_CAP = 1, FLAT = 2, COUNTERSUNK = 3 };

struct HeadParameters {
  HeadType type;
  double widthAcrossFlats;      // s
  double widthAcrossCorners;    // e (optional, can be calculated)
  double height;                // k
  double washerFaceDiameter;    // dw
  double washerFaceThickness;   // c
  double underheadFilletRadius; // r
  double socketSize;            // for socket head
  double socketDepth;           // for socket head
};

struct ShankParameters {
  double nominalDiameter;  // d
  double totalLength;      // L
  double gripLength;       // ls (unthreaded part)
  double bodyTolerance;    // tolerance on diameter
  double edgeFilletRadius; // radius for smoothing all edges
};

struct ThreadParameters {
  double majorDiameter; // d
  double pitch;         // P
  double angle;         // alpha (default 60)
  double minorDiameter; // d3
  double pitchDiameter; // d2
  double rootRadius;    // R
  double runout;        // thread runout length
};

struct NutParameters {
  bool generate;
  double widthAcrossFlats;
  double height;
  double washerFaceDiameter;
  double countersinkAngle;
  double tolerance; // clearance between bolt and nut
};

struct MaterialParameters {
  std::string propertyClass; // e.g., "8.8"
  std::string materialType;  // e.g., "Steel"
  std::string coating;       // e.g., "Zinc"
};

struct BoltParameters {
  HeadParameters head;
  ShankParameters shank;
  ThreadParameters thread;
  NutParameters nut;
  MaterialParameters material;
};

#endif // PARAMETERS_H
