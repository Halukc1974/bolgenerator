OBJECTS = main.o bolt.o convert.o export.o thread.o helix.o cut.o chamfer.o
CFLAGS = -I/usr/include/opencascade/ -Wall
LDLIBS = -lTKernel -lTKBRep -lTKBO -lTKG2d -lTKG3d -lTKGeomBase -lTKMath -lTKOffset -lTKPrim -lTKSTEP -lTKTopAlgo -lTKXSBase

scim_bolts : $(OBJECTS)
	g++ -o scim_bolts $(OBJECTS) $(LDLIBS)

main.o :
	g++ -c $(CFLAGS) main.cpp

bolt.o :
	g++ -c $(CFLAGS) bolt.cpp

convert.o :
	g++ -c $(CFLAGS) convert.cpp

export.o :
	g++ -c $(CFLAGS) export.cpp

thread.o :
	g++ -c $(CFLAGS) thread.cpp

helix.o :
	g++ -c $(CFLAGS) helix.cpp

cut.o :
	g++ -c $(CFLAGS) cut.cpp

chamfer.o :
	g++ -c $(CFLAGS) chamfer.cpp

.PHONY : clean
clean :
	rm scim_bolts $(OBJECTS) *.brep