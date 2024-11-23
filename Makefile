OBJECTS = main.o fileout.o
CFLAGS = -I/usr/include/opencascade/
LDLIBS = -lTKernel -lTKBRep -lTKBO -lTKG2d -lTKG3d -lTKGeomBase -lTKMath -lTKOffset -lTKPrim -lTKSTEP -lTKTopAlgo -lTKXSBase

scim_bolts : $(OBJECTS)
	g++ -o scim_bolts $(OBJECTS) $(LDLIBS)

main.o :
	g++ -c $(CFLAGS) main.cpp

fileout.o :
	g++ -c $(CFLAGS) fileout.cpp

.PHONY : clean
clean :
	rm scim_bolts $(OBJECTS) *.brep