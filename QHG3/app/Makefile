

ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
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
  CFLAGS=-g $(WARNINGS)
endif

ifdef OPT
  CFLAGS=-O3 $(WARNINGS)
endif

ifdef PROF
  CFLAGS=-pg $(WARNINGS)
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif

ifdef DYN
  CFLAGS:=$(CFLAGS) -rdynamic
  DYN=-DDYN
endif	


DOMP=
FORCECLEAN=

COMMON=../common
QMAPS=../qmaps
ICOSA=../icosa
IO=../io
GEOINFO=../geoinfo
KERNEL=../kernel
GENES=../genes
MODULAR=../modular
POPULATIONS=../populations

COMP=-fopenmp
COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(COMP) $(OMPPFLAGS) $(DYN)

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

ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

§HEADER_LIB:
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
SimTestOMPA:FULL_LIBS=COMMON_LIB QMAPS_LIB MODULAR_LIB POPULATION_LIB KERNEL_LIB IO_LIB
SimTestOMPA:LIBTARG=libompA
SimTestOMPA:FORCECLEAN=clean
SimTestOMPA:COMMON_LIB QMAPS_LIB MODULAR_LIB POPULATION_LIB KERNEL_LIB IO_LIB
SimTestOMPA:SimTest

QHGMain:SimTest
	mv SimTestOMPA QHGMain

QHGMainDirect:DOMP=-DOMP_A
QHGMainDirect:FULL_LIBS=
QHGMainDirect:LIBTARG=
QHGMainDirect:SIMTARG=QHGMain
QHGMainDirect:SimTest


SimParams.o: SimParams.cpp SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

Simulator.o: Simulator.cpp Simulator.h SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

SimTest.o: SimTest.cpp Simulator.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(MODULAR) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

SimTest: SimTest.o Simulator.o SimParams.o \
    IDS_AND_POPFACTORY 
	$(CC) $(COMP) $(CFLAGS)  -o $(SIMTARG) \
	SimTest.o Simulator.o SimParams.o \
	-L $(POPULATIONS) -lPopulations \
	-L $(GENES) -lGenes \
	-L $(KERNEL) -lKernel \
	-L $(MODULAR) -lModular \
	-L $(QMAPS) -lQMap \
	-L $(ICOSA) -lIcosa \
	-L $(HEADER) -lHeader \
	-L $(GEOINFO) -lGeoInfo \
	-L $(COMMON) -lCommon \
	-L $(IO) -lIO \
        -lz -lhdf5 -lm -lcrypto -lgsl








