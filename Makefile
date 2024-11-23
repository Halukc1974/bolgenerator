OBJECTS = main.o export.o
CFLAGS = -I/usr/include/opencascade/ -Wall
LDLIBS = -lTKernel -lTKBRep -lTKBO -lTKG2d -lTKG3d -lTKGeomBase -lTKMath -lTKOffset -lTKPrim -lTKSTEP -lTKTopAlgo -lTKXSBase

scim_bolts : $(OBJECTS)
	g++ -o scim_bolts $(OBJECTS) $(LDLIBS)

main.o :
	g++ -c $(CFLAGS) main.cpp

export.o :
	g++ -c $(CFLAGS) export.cpp

.PHONY : clean
clean :
	rm scim_bolts $(OBJECTS) *.brep