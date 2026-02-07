# Implementation Plan - Track: parameters_20260207

## Phase 1: Parameter Definition and Parsing
- [x] Task: Update `parameters.h` to include new fields in `HeadParameters`, `ShankParameters`, `ThreadParameters`, and `NutParameters` as defined in the spec. [commit: f15f83c]
- [~] Task: Update `main.cpp` to parse the new CLI arguments and populate the extended parameter structs.
- [ ] Task: Verify that the project compiles with the new struct definitions (even if logic isn't used yet).
- [ ] Task: Conductor - User Manual Verification 'Phase 1' (Protocol in workflow.md)

## Phase 2: Core Logic Implementation - Bolt
- [ ] Task: Update `bolt.cpp` constructor to apply `topFilletRadius` specifically to the top edges of the head.
- [ ] Task: Update `bolt.cpp` constructor to apply `transitionFilletRadius` specifically to the head-shank intersection.
- [ ] Task: Update `bolt.cpp` (or `chamfer.cpp` integration) to support customizable chamfer angles if applicable.
- [ ] Task: Write Tests: Create a test case in `tests.csv` or a script to generate a bolt with distinct top and underhead fillets.
- [ ] Task: Implement Feature: Verify visually that the fillets are applied correctly.
- [ ] Task: Conductor - User Manual Verification 'Phase 2' (Protocol in workflow.md)

## Phase 3: Core Logic Implementation - Nut & Threads
- [ ] Task: Update `nut.cpp` to use `threadClearance` and `chamferAngle` from parameters.
- [ ] Task: Refine thread generation (if accessible in `thread.cpp` or via `bolt.cpp` logic) to respect `threadAngle` if possible, or at least document limitations.
- [ ] Task: Write Tests: Create a test case for a nut with custom clearance.
- [ ] Task: Implement Feature: Generate a nut and bolt pair and verify the clearance logic (conceptually or via boolean checks if possible).
- [ ] Task: Conductor - User Manual Verification 'Phase 3' (Protocol in workflow.md)

## Phase 4: Final Validation
- [ ] Task: Run full regression tests to ensure existing default behaviors are preserved.
- [ ] Task: Generate a comprehensive "showcase" set of models demonstrating the new parameters.
- [ ] Task: Conductor - User Manual Verification 'Phase 4' (Protocol in workflow.md)
