# Track Specification: Comprehensive Bolt & Nut Parameters

## Overview
This track aims to implement a comprehensive set of mechanical and geometric parameters for the `BoltGenerator` project. The goal is to allow full customization of bolts and nuts, satisfying high-precision engineering requirements. This includes granular control over edge treatments (fillets and chamfers), mechanical tolerances, and material properties.

## Scope
1.  **Parameter Expansion:** Extend the existing `BoltParameters` structures in `parameters.h` to include missing mechanical and geometric attributes.
2.  **Input Handling:** Update `main.cpp` to parse these new parameters from CLI arguments.
3.  **Core Logic Update:** Modify `bolt.cpp` and `nut.cpp` to apply these parameters during mesh generation, specifically focusing on:
    -   Granular fillet control (head top, head bottom, thread root).
    -   Chamfer customization (angle and distance).
    -   Mechanical tolerances (thread fit, clearance).

## Detailed Requirements

### 1. Parameter Structures (`parameters.h`)

#### Head Parameters
-   `topFilletRadius`: Radius of the fillet on the top edge of the head.
-   `verticalChamfer`: Boolean or dimension for chamfering vertical hex edges (optional but good for realism).

#### Shank Parameters
-   `transitionFilletRadius`: Specific radius for the transition between shank and head (underhead fillet). This separates it from a generic "global" fillet.

#### Thread Parameters
-   `rootRadius`: Radius of the thread root (for realistic threads vs simplified).
-   `crestRadius`: Radius of the thread crest.
-   `threadAngle`: Angle of the thread profile (standard is 60 degrees, but should be customizable).

#### Nut Parameters
-   `chamferAngle`: Angle of the chamfer on the nut faces.
-   `threadClearance`: Specific clearance value for the internal thread to ensure fit.

#### Material/Mechanical Parameters
-   `toleranceClass`: (e.g., "6g", "6H") - could map to specific clearance values.
-   `strengthGrade`: (e.g., "8.8") - Metadata for the model.

### 2. Implementation Logic

-   **Fillets:** Use `BRepFilletAPI_MakeFillet` more selectively. Instead of a global fillet, apply fillets to specific edges identified by their geometric location (e.g., edges with max Z for head top).
-   **Threads:** If `rootRadius` is specified, the `Thread` generation logic (likely in `thread.cpp` or `helix.cpp` - to be verified) needs to support rounded profiles or post-process filleting.
-   **Tolerances:** Ensure `nut.cpp` uses the `threadClearance` parameter to correctly size the cutting tool (the subtraction bolt).

## Success Criteria
-   All new parameters can be set via CLI arguments.
-   Generated models reflect the visual changes (e.g., distinct top vs. underhead fillets).
-   Nut threads have adjustable clearance.
-   The code compiles without errors.
