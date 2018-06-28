
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifndef WARNINGS
  WARNINGS=-Wall -Wshadow
endif

ifndef CFLAGS
  CFLAGS=-g $(WARNINGS)
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif


UTILS=~/utils
COMMON=../common
QMAPS=../qmaps
GEOINFO=../geoinfo
TILING=../tiling

COMP=-fopenmp
DOMP=-DOMP

ifdef OPT
  CFLAGS=-O3 $(WARNINGS)
endif
ifdef PROF
  CPROF=-pg
  DOMP=
else
  CPROF=
endif

ifndef ARFL
  ARFL=-rv
endif

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(CPROF) -I$(COMMON)

CORE=IcoGridNodes.o IcoNode.o \
     EQTriangle.o Tegmark.o EQsahedron.o  \
     IcoFace.o SimpleFace.o QuadFace.o TriFace.o \
     Icosahedron.o IcoHeader.o VertexLinkage.o symbuf.o FaceChecker.o Lattice.o


all: libIcosa.a 



lib: libIcosa.a


new: clean all

clean: 
	rm -f *.o $(CORE)

libIcosa.a: $(CORE)
	ar $(ARFL) $@ $?



Lattice.o: Lattice.cpp Lattice.h  
	$(CC) $(COPT) $(DEFS) $(COMP) $(DOMP) -I$(GEOINFO)

SimpleFace.o: SimpleFace.cpp SimpleFace.h
	$(CC) $(COPT) $(DEFS) 

IcoFace.o: IcoFace.cpp IcoFace.h 
	$(CC) $(COPT) $(DEFS) -I$(QMAPS)

QuadFace.o: QuadFace.cpp QuadFace.h SimpleFace.h PolyFace.h 
	$(CC) $(COPT) $(DEFS)

TriFace.o: TriFace.cpp TriFace.h SimpleFace.h PolyFace.h 
	$(CC) $(COPT) $(DEFS)

VertexLinkage.o: VertexLinkage.cpp VertexLinkage.h IcoFace.h QuadFace.h icoutil.h
	$(CC) $(COPT) $(DEFS) $(COMP) $(DOMP)

Icosahedron.o: Icosahedron.cpp Icosahedron.h IcoHeader.h PolyFace.h IcoFace.h icoutil.h \
	VertexLinkage.h 
	$(CC) $(COPT) $(DEFS) $(COMP) $(DOMP)  


IcoHeader.o: IcoHeader.cpp IcoHeader.h icoutil.h
	$(CC) $(COPT) $(DEFS)  

Tegmark.o:Tegmark.cpp Tegmark.h
	$(CC) $(COPT) $(DEFS) $(COMP) 

EQTriangle.o:EQTriangle.cpp EQTriangle.h Tegmark.h
	$(CC) $(COPT) $(DEFS) $(COMP) 

EQsahedron.o:EQsahedron.cpp EQsahedron.h EQTriangle.h Tegmark.h FaceChecker.h
	$(CC) $(COPT) $(DEFS)  $(COMP) 

FaceChecker.o:FaceChecker.cpp FaceChecker.h IcoFace.h
	$(CC) $(COPT) $(DEFS)  $(COMP) -I$(QMAPS)


IcoNode.o: IcoNode.cpp IcoNode.h icoutil.h
	$(CC) $(COPT) $(DEFS) 


symbuf.o: symbuf.cpp symbuf.h
	$(CC) $(COPT) $(DEFS) 


IcoGridNodes.o: IcoGridNodes.cpp IcoGridNodes.h IcoNode.h
	$(CC) $(COPT) $(DEFS) 


faceTest.o: faceTest.cpp IcoFace.h Face.h
	$(CC) $(COPT) $(DEFS) 


faceTest:faceTest.o Face.o IcoFace.o QuadFace.o Icosahedron.o VertexLinkage.o symbuf.o IcoHeader.o
	$(CC) -o faceTest $(CADD) $(COMP) \
	faceTest.o Face.o IcoFace.o QuadFace.o Icosahedron.o VertexLinkage.o  symbuf.o IcoHeader.o \
	$(COMMON)/Vec3D.o  $(COMMON)/Quat.o  $(COMMON)/geomutils.o $(COMMON)/strutils.o $(COMMON)/BufWriter.o $(COMMON)/BufReader.o















