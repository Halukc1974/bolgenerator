# Product Definition

## Vision
BoltGenerator is a comprehensive solution designed to generate high-precision 3D models of bolts and nuts. It functions both as a standalone library and a web-based application, bridging the gap between low-level mesh generation and accessible user interfaces.

## Target Audience
- **Primary:** Mechanical Engineers and Designers requiring accurate CAD models.

## Core Features
- **Parametric Customization:** Fine-tune bolt and nut parameters including size, thread specifications, and head types.
- **Multi-Format Export:** Generate and download models in industry-standard formats like STL and BREP.
- **Real-Time Visualization:** Interactive 3D preview of the models directly in the web browser.
- **Dual-Mode Operation:** Accessible via a user-friendly Web Interface or integrated as a C++ Library.

## Architecture & Design Guidelines
- **Core Engine:** Modular C++ architecture for efficient and accurate mesh generation.
- **Web Backend:** Node.js with Express providing a RESTful API to interface with the C++ core.
- **Frontend:** Client-side rendering for responsive 3D visualization.
- **Deployment:** Optimized for web deployment to ensure accessibility.
