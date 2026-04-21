# BoltGenerator

Parametric CAD generator for ISO/ASME-compliant bolts and matching nuts.
Produces **STL**, **BRep**, and **STEP** files with a live 3D preview.

Originally from [Scimulate LLC](https://scimulate.com) (GPL-3.0); actively
extended in this fork.

---

## Architecture

```
┌────────────────────────────────────────────────────┐
│  Browser                                           │
│  • Three.js STL viewer                             │
│  • Sidebar form (4 sections: Head/Shank/Thread/Nut)│
│    public/index.html                               │
└──────────────┬─────────────────────────────────────┘
               │ POST /generate  (JSON, 23 parameters)
┌──────────────▼─────────────────────────────────────┐
│  Node.js (Express)                                 │
│  • Parameter clamping & validation                 │
│  • execFile("./scim_bolts", args)                  │
│    server.js                                       │
└──────────────┬─────────────────────────────────────┘
               │ CLI args
┌──────────────▼─────────────────────────────────────┐
│  C++ Binary  (scim_bolts)                          │
│  • OpenCASCADE (OCCT 7.5) solid modeling           │
│  • BRep + STL + STEP export                        │
│    → Tests/{filename}.{brep,stl,step}              │
└────────────────────────────────────────────────────┘
```

## C++ Modules

| File              | Responsibility                                      |
| ----------------- | --------------------------------------------------- |
| `main.cpp`        | 23-arg CLI parser, orchestrator                     |
| `parameters.h`    | `Head/Shank/Thread/Nut/Material` parameter structs  |
| `bolt.cpp`        | Head + shank assembly, underhead & edge fillets     |
| `nut.cpp`         | Hex outer + internal thread cutter                  |
| `thread.cpp`      | ISO 60° thread profile (wire sketch)                |
| `helix.cpp`       | Helical pipe sweep (`BRepOffsetAPI_MakePipeShell`)  |
| `hexagon.cpp`     | Hex prism from across-flats dimension               |
| `chamfer.cpp`     | Revolved chamfer for shank tip                      |
| `cut.cpp`         | Boolean cut with volume validation & solid pick     |
| `export.cpp`      | BRep / STEP / STL writer (FreeCAD-style mesher)     |

## Supported Parameters

### Head
- **Type**: Hex (0), Socket Cap (1), Flat (2), Countersunk (3)
- Width across flats `s`, height `k`, washer face ⌀ `dw`, WF thickness `c`,
  underhead fillet radius `r`, socket size & depth (for cap head)

### Shank
- Nominal ⌀ `d`, total length `L`, grip length `ls`, body tolerance,
  edge fillet radius (auto-clamped to 10 % of `d`)

### Thread
- Major ⌀, pitch `P`, minor ⌀ (0 → ISO auto-calc `d − 1.0825·P`)

### Nut (optional)
- Width across flats, height, washer face ⌀, thread tolerance (clearance
  on diameter, not pitch — ISO 965 H class semantics)

## Quick Start

### Docker (recommended)

```bash
docker-compose up --build
# Open http://localhost:3000
```

### Native Build (Linux with OCCT 7.5)

```bash
# Install OpenCASCADE dev packages (Ubuntu/Debian)
sudo apt install libocct-modeling-algorithms-dev libocct-data-exchange-dev \
                 libocct-foundation-dev libocct-modeling-data-dev \
                 libocct-visualization-dev occt-misc

# Install Node.js 18+ and npm
npm install
make                  # builds scim_bolts
npm start             # starts Express on :3000
```

## API

### POST `/generate`

Request (JSON):
```json
{
  "headType": 1,
  "widthAcrossFlats": 16,
  "headHeight": 10,
  "nominalDiameter": 10,
  "totalLength": 50,
  "threadPitch": 1.5,
  "generateNut": true,
  "nutAcrossFlats": 17,
  "nutHeight": 8,
  "nutTolerance": 0.15
}
```

Response:
```json
{
  "success": true,
  "filename": "bolt_1760523456789",
  "boltBrep":  "/download/bolt_1760523456789.brep",
  "boltStl":   "/preview/bolt_1760523456789.stl",
  "boltStep":  "/download/bolt_1760523456789.step",
  "nutBrep":   "/download/bolt_1760523456789_nut.brep",
  "nutStl":    "/preview/bolt_1760523456789_nut.stl",
  "nutStep":   "/download/bolt_1760523456789_nut.step",
  "stats":     { "volume_mm3": 1234.56 },
  "nutStats":  { "volume_mm3": 456.78 }
}
```

### GET `/preview/:filename` and `/download/:filename`

Filename is whitelisted to `bolt_<digits>(_nut)?.(brep|stl|step|stp)` and
path-sanitized to prevent directory traversal.

## Standards Reference

- **ISO 724**: Metric thread — basic dimensions
- **ISO 898-1**: Mechanical properties of fasteners
- **ISO 965**: Metric thread tolerances (H class for nuts, 6g for bolts)
- **ISO 4014 / 4017 / 4762**: Hex-head / fully-threaded / socket-cap bolts
- **ASME B1.1**: Unified Inch Screw Threads (UN/UNR)

## License

GPL-3.0 — see [LICENSE](./LICENSE).
