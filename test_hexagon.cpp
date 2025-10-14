// Test hexagon function directly
#include "hexagon.h"
#include "export.h"
#include <iostream>

int main() {
    std::cout << "Testing Hexagon function..." << std::endl;
    
    // Create a hexagon: 12mm across flats, 7mm height
    TopoDS_Solid hex = Hexagon(12.0, 7.0);
    
    std::cout << "Hexagon created, exporting..." << std::endl;
    
    // Export to BREP and STL
    ExportBRep(hex, "Tests/test_hexagon_only.brep");
    ExportSTL(hex, "Tests/test_hexagon_only.stl");
    
    std::cout << "Done!" << std::endl;
    return 0;
}
