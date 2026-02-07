# Product Guidelines

## User Experience (UX)
- **Technical Focus:** The interface should cater to technical users, presenting detailed parameter controls and model data clearly.
- **Design:** Mimic the layout and density of traditional CAD software to provide a familiar environment for engineers.
- **Visual Priority:** Ensure the 3D viewer is central, but surrounded by precise controls and readouts.

## Communication & Tone
- **Professional & Precise:** All user-facing text, error messages, and documentation should be written in a formal, engineering-standard tone.
- **Accuracy:** Prioritize technical accuracy over simplification.

## Technical Specifications & Parametric Integrity
- **Comprehensive Parameters:** The system must support a full range of mechanical and geometric parameters for bolts and nuts, including advanced features like edge softening (fillets/chamfers).
- **Parameter Completeness:** Actively validate for missing standard mechanical parameters and ensure all UI inputs are directly connected to the mesh generation logic.
- **Geometric Fidelity:** The generation engine must strictly adhere to these parameters to produce physically accurate models suitable for engineering use.

## Performance & Quality
- **High Fidelity:** The generation engine must prioritize geometric precision and mesh quality over raw speed.
- **Validation:** Ensure all generated models are manifold and valid for downstream CAD or CAM processes.

## Branding
- **Identity:** Maintain a clean, industrial aesthetic consistent with engineering tools.
