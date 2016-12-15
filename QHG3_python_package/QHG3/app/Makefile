

ifdef GCC
  CC=$(GCC)
else
  CC=g++
endif

ifdef OMPP
  CC=kinst-ompp $(GCC)
  OMPPFLAGS=-I/home/simone/src/ompp-0.8.3/include/ -L/home/simone/src/ompp-0.8.3/lib/
endif

ifdef SCALASCA
  CC=/opt/scalasca/bin/scalasca -instrument $(GCC)
endif

ifndef ARFL
  ARFL=-rv
endif

ifndef CFLAGS
  CFLAGS=-g -Wall
endif

ifdef OPT
  CFLAGS=-O3 -Wall
endif

ifdef PROF
  CFLAGS=-pg -Wall
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif



DOMP=
FORCECLEAN=

COMMON=../common
GEOINFO=../geoinfo
TILING=../tiling
HEADER=../header
QMAPS=../qmaps
AGENTS=../agents
ICOSA=../icosa
IO=../io
KERNEL=../kernel
MODULAR=../modular
POPULATIONS=../populations
EXTERNAL=../external

COMP=-fopenmp
COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(COMP) $(OMPPFLAGS)

KERNEL_LIB:
	$(MAKE) --directory=$(KERNEL) $(FORCECLEAN) $(LIBTARG)

MODULAR_LIB:
	$(MAKE) --directory=$(MODULAR) $(FORCECLEAN) $(LIBTARG)

POPULATION_LIB:
	$(MAKE) --directory=$(POPULATIONS) $(FORCECLEAN) $(LIBTARG)

COMMON_LIB:
	$(MAKE) --directory=$(COMMON) lib 

QMAPS_LIB:
	$(MAKE) --directory=$(QMAPS) lib

IO_LIB:
	$(MAKE) --directory=$(IO) lib

EXTERNAL_LIB:
	$(MAKE) --directory=$(EXTERNAL) 

ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

HEADER_LIB:
	$(MAKE) --directory=$(HEADER) 

IDS_AND_POPFACTORY:
	./configure.sh


TARGETS=SimTest
LIBTARG=lib
SIMTARG=SimTest

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS) AGENT_CLASS_SUMMARY.txt

SimTestOMPA:DOMP=-DOMP_A
SimTestOMPA:SIMTARG=SimTestOMPA
SimTestOMPA:LIBTARG=libompA
SimTestOMPA:FORCECLEAN=clean
SimTestOMPA:SimTest

SimParams.o: SimParams.cpp SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

Simulator.o: Simulator.cpp Simulator.h SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

SimTest.o: SimTest.cpp Simulator.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(MODULAR) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(GEOINFO) -I$(POPULATIONS)

SimTest: SimTest.o Simulator.o SimParams.o \
    IDS_AND_POPFACTORY \
	EXTERNAL_LIB COMMON_LIB QMAPS_LIB MODULAR_LIB POPULATION_LIB KERNEL_LIB IO_LIB
	$(CC) $(COMP) $(CFLAGS)  -o $(SIMTARG) \
	SimTest.o Simulator.o SimParams.o \
	-L $(POPULATIONS) -lPopulations \
	-L $(KERNEL) -lKernel \
	-L $(MODULAR) -lModular \
	-L $(QMAPS) -lQMap \
	-L $(ICOSA) -lIcosa \
	-L $(HEADER) -lHeader \
	-L $(GEOINFO) -lGeoInfo \
	-L $(COMMON) -lCommon \
	-L $(EXTERNAL) -lNR \
	-L $(IO) -lIO \
        -lz -lhdf5 

ag:	$(QMAPS)/QMapUtils.o $(QMAPS)/QMapReader.o $(QMAPS)/QMapHeader.o \
	$(HEADER)/HeaderBase.o $(HEADER)/SnapHeader.o \
	$(ICOSA)/IcoGridCreator.o $(ICOSA)/Icosahedron.o $(ICOSA)/IcoNode.o $(ICOSA)/IcoGridNodes.o \
        $(ICOSA)/LonLatRegion.o $(ICOSA)/VertexLinkage.o $(ICOSA)/GridZones.o $(ICOSA)/IcoFace.o \
	$(ICOSA)/IcoHeader.o $(ICOSA)/IcoSpiral.o $(ICOSA)/GridCreator.o $(ICOSA)/symbuf.o \
	$(ICOSA)/Region.o  






