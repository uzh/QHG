
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifndef CFLAGS
  CFLAGS=-g -Wall
endif

UTILS=~/utils
COMMON=../common
QMAPS=../qmaps
VISUAL=../visual
EXTERNAL=../external
GEOINFO=../geoinfo
HEADER=../header
TILING=../tiling
EXTERNAL=../external
KERNEL=../kernel
ICOSA=../icosa
IO=../io

COMP=-fopenmp
DOMP=-DOMP

ifdef OPT
  CFLAGS=-O3 -Wall
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

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(CPROF) -I$(COMMON) -I$(ICOSA)


TARGS=EQTileTool lifetest mpitest


app: $(TARGS)
	$(if $(UTILS),cp $? $(UTILS),)


new: clean app

clean: 
	rm -f *.o $(TARGS)


#####
# some general stuff
#####
interquat.o: interquat.cpp interquat.h trackball.h
	$(CC) $(COPT) $(DEFS)  -I$(HEADER)

trackball.o: trackball.c trackball.h 
	g++ -c -c trackball.c -o trackball.o

VRColorizer.o: VRColorizer.cpp VRColorizer.h IcoColorizer.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(QMAPS) -I$(VISUAL)


SnapColorizer.o: SnapColorizer.cpp SnapColorizer.h IQColorizer.h ProjInfo.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(QMAPS) -I$(VISUAL)

NodeLister.o: NodeLister.cpp NodeLister.h 
	$(CC) $(COPT) $(DEFS)  -I$(HEADER)

IcoConverter.o: IcoConverter.cpp IcoConverter.h
	$(CC) $(COPT) $(DEFS) -I $(HEADER) -I $(GEOINFO) -I $(TILING)




#####
# IGN stuff
#####
IGNWriteTest.o:IGNWriteTest.cpp IcoGridNodes.h IcoGridCreator.h TrivialSplitter.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(HEADER)  -I$(EXTERNAL) -I$(VISUAL)

IGNWriteTest: IGNWriteTest.o IcoGridCreator.o IcoGridNodes.o TrivialSplitter.o FullRegion.o RegionSplitter.o Region.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o 
	$(CC) $(CADD) $(COMP) -o IGNWriteTest IGNWriteTest.o \
	IcoGridCreator.o IcoGridNodes.o TrivialSplitter.o FullRegion.o RegionSplitter.o Region.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o \
	 $(COMMON)/Vec3D.o $(COMMON)/Quat.o $(COMMON)/BufReader.o  $(COMMON)/strutils.o $(COMMON)/geomutils.o \
         $(COMMON)/BufWriter.o 

IGNDisplay.o:IGNDisplay.cpp IcoGridNodes.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(HEADER)  -I$(EXTERNAL) -I$(VISUAL)

IGNDisplay: IGNDisplay.o IcoGridNodes.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o 
	$(CC) $(CADD) $(COMP) -o IGNDisplay IGNDisplay.o \
	IcoGridNodes.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o \
	 $(COMMON)/Vec3D.o $(COMMON)/Quat.o $(COMMON)/BufReader.o  $(COMMON)/strutils.o $(COMMON)/geomutils.o \
         $(COMMON)/BufWriter.o 



#####
# Ico2PLY
#####
Ico2PLY.o: Ico2PLY.cpp 	
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(HEADER)  -I$(EXTERNAL) -I$(VISUAL)

Ico2PLY: Ico2PLY.o Icosahedron.o IcoFace.o QuadFace.o IcoHeader.o	\
        VertexLinkage.o symbuf.o NodeLister.o 
	$(CC) $(CADD) $(COMP) -o Ico2PLY Ico2PLY.o \
	$(COMMON)/strutils.o $(COMMON)/ParamReader.o \
	$(COMMON)/LineReader.o $(COMMON)/DescReader.o \
        $(COMMON)/Vec3D.o $(COMMON)/BufWriter.o $(COMMON)/BufReader.o \
	$(HEADER)/HeaderBase.o 	$(HEADER)/SnapHeader.o	$(HEADER)/PopHeader.o \
	Icosahedron.o IcoFace.o QuadFace.o IcoHeader.o	\
        VertexLinkage.o symbuf.o NodeLister.o \
	$(VISUAL)/LookUp.o \
	$(EXTERNAL)/plyfile.o \
        -L $(VISUAL) -lVisual -lz	

#####
# EQTileTool
#####

EQNodeClassificator.o: EQNodeClassificator.cpp EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS)

BasicTile.o: BasicTile.cpp BasicTile.h 
	$(CC) $(COPT) $(DEFS)

BasicSplitter.o: BasicSplitter.cpp BasicSplitter.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQTile.o: EQTile.cpp EQTile.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQSplitter.o: EQSplitter.cpp EQSplitter.h BasicSplitter.h EQTile.h EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS)

LonLatTile.o: LonLatTile.cpp LonLatTile.h BasicTile.h 
	$(CC) $(COPT) $(DEFS)

LonLatSplitter.o: LonLatSplitter.cpp LonLatSplitter.h BasicSplitter.h LonLatTile.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQZones.o: EQZones.cpp EQZones.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQGridCreator.o: EQGridCreator.cpp EQGridCreator.h EQZones.h BasicTile.h BasicSplitter.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(KERNEL) -I$(ICOSA) -I$(IO)

EQTiling.o: EQTiling.cpp EQTiling.h BasicTiling.h EQSplitter.h EQGridCreator.h EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

LonLatTiling.o: LonLatTiling.cpp LonLatTiling.h BasicTiling.h LonLatSplitter.h EQGridCreator.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

BasicTiling.o: BasicTiling.cpp BasicTiling.h BasicSplitter.h EQGridCreator.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

EQTileLinks.o: EQTileLinks.cpp EQTileLinks.h 
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

EQTileTool.o: EQTileTool.cpp EQGridCreator.h 
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(KERNEL) -I$(ICOSA) -I$(IO)

EQTileTool: EQTileTool.o EQGridCreator.o EQZones.o \
	BasicTile.o BasicSplitter.o BasicTiling.o \
	EQTile.o EQSplitter.o EQTiling.o \
        EQNodeClassificator.o EQTileLinks.o \
	LonLatTile.o LonLatSplitter.o LonLatTiling.o 
	$(CC) -o EQTileTool $(CADD) $(COMP) \
	EQTileTool.o  EQGridCreator.o EQZones.o \
	BasicTile.o BasicSplitter.o BasicTiling.o \
	EQTile.o EQSplitter.o EQTiling.o \
        EQNodeClassificator.o EQTileLinks.o \
	LonLatTile.o LonLatSplitter.o LonLatTiling.o \
	-L$(ICOSA) -lIcosa \
	-L$(KERNEL) -lKernel \
	-L$(QMAPS) -lQMap \
	-L$(HEADER) -lHeader \
	-L$(ICOSA) -lIcosa \
	-L$(IO) -lIO \
	-L$(COMMON) -lCommon \
	-lhdf5 -lz


#####
# MPIMulti & friendsIco2PLY
#####

MPIMulti.o: MPIMulti.cpp MPIMulti.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON)

LifeTile.o: LifeTile.cpp LifeTile.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON)

LifeBoard.o: LifeBoard.cpp LifeBoard.h LifeTile.h MPIMulti.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON)

lifetest.o: lifetest.cpp LifeBoard.h LifeBoard.cpp
	$(CC) $(COPT) $(DEFS) -I$(COMMON)


lifetest: lifetest.o LifeBoard.o LifeTile.o MPIMulti.o
	mpic++ -o lifetest $(CADD) $(COMP) \
	lifetest.o LifeBoard.o LifeTile.o MPIMulti.o \
	-L$(COMMON) -lCommon \
	-lz

mpitest.o: mpitest.cpp MPIMulti.h EQTileLinks.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON)


mpitest: mpitest.o MPIMulti.o EQTileLinks.o
	mpic++ -o mpitest $(CADD) $(COMP) \
	 mpitest.o MPIMulti.o EQTileLinks.o \
	-L$(ICOSA) -lIcosa \
	-L$(COMMON) -lCommon \
	-lz
