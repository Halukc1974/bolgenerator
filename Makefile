# Define variables
OBJECTS = main.o bolt.o convert.o export.o thread.o helix.o cut.o chamfer.o hexagon.o nut.o
CFLAGS = -I/usr/include/opencascade -Wall
LDLIBS = -lTKernel -lTKBRep -lTKBO -lTKG2d -lTKG3d -lTKGeomBase -lTKMath -lTKOffset -lTKPrim -lTKSTEP -lTKTopAlgo -lTKXSBase -lTKSTL -lTKMesh -lTKShHealing -lTKFillet -lTKGeomAlgo -lTKService -lTKV3d 

CC = g++

# Target to build the executable
scim_bolts: $(OBJECTS)
	$(CC) -o $@ $^ $(LDLIBS)

# Pattern rule for object files
%.o: %.cpp
	$(CC) -c $(CFLAGS) $<

# Phony target for cleaning up
.PHONY: clean
clean:
	rm -f scim_bolts $(OBJECTS) *.brep