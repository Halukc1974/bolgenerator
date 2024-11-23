OBJECTS = main.o export.o
CFLAGS = -I/usr/include/opencascade/ -Wall
LDLIBS = -lTKernel -lTKBRep -lTKBO -lTKG2d -lTKG3d -lTKGeomBase -lTKMath -lTKOffset -lTKPrim -lTKSTEP -lTKTopAlgo -lTKXSBase
ARGS = shcs 5.0 25.0 0.0

scim_bolts : $(OBJECTS)
	g++ -o scim_bolts $(OBJECTS) $(LDLIBS)

main.o :
	g++ -c $(CFLAGS) main.cpp

export.o :
	g++ -c $(CFLAGS) export.cpp

run:
	./scim_bolts $(ARGS)

.PHONY : clean
clean :
	rm scim_bolts $(OBJECTS) *.brep