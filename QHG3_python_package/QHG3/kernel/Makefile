#ifdef MPI
#  ifdef CSCS
#    CADD=-I /apps/zlib/include -D CSCS
#  endif
#  CC=$(MPICC)
#else
#  CC=$(GCC)
#endif

ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifdef SCALASCA
  CC=/opt/scalasca/bin/skin $(GCC)
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

#ifdef OLDBIRTHS
#  CFLAGS:=$(CFLAGS) -DOLDBIRTHS
#endif 

ifdef OLDBIRTHS
  DOLD=-DOLDBIRTHS
endif 

DOMP=

COMMON=../common
GEOINFO=../geoinfo
TILING=../tiling
HEADER=../header
QMAPS=../qmaps
AGENTS=../agents
ICOSA=../icosa
IO=../io
EXTERNAL=../external
GENES=../genes
MODULAR=../modular
KERNEL=./

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(DOLD) $(OMPPFLAGS)
COMP=-fopenmp

ARFL=rv



all: kernel

lib: libKernel.a

libompA:DOMP=-DOMP_A
libompA:libKernel.a

kernel:  libKernel.a

libKernel.a:  Geography.o Climate.o VegFactory.o NPPVeg.o \
              MoveStats.o GridFactory.o SCellGrid.o PopLooper.o  \
	      SPopulation.o Prioritizer.o \
              clsutils.o
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libKernel.a *.ti


Geography.o: Geography.cpp Geography.h
	$(CC) $(COPT) $(DEFS)  -I$(HEADER) -I$(QMAPS) -I$(ICOSA)

Climate.o: Climate.cpp Climate.h Geography.h
	$(CC) $(COPT) $(DEFS)  -I $(QMAPS) -I$(HEADER) -I$(ICOSA)

VegFactory.o: VegFactory.cpp VegFactory.h
	$(CC) $(COPT) $(DEFS) -I $(QMAPS) 

NPPVeg.o: NPPVeg.cpp NPPVeg.h Climate.h Geography.h
	$(CC) $(COPT) $(COMP) $(DEFS) -I $(QMAPS) -I$(HEADER) -I$(ICOSA) 

MoveStats.o: MoveStats.cpp MoveStats.h 
	$(CC) $(COPT) $(COMP) $(DEFS) 

GridFactory.o: GridFactory.cpp GridFactory.h 
	$(CC) $(COPT) $(DEFS)  -I $(QMAPS) -I$(ICOSA) -I$(GEOINFO)

SCellGrid.o: SCellGrid.cpp SCellGrid.h SCell.h Geography.h Climate.h
	$(CC) $(COPT) $(DEFS) $(COMP) -I$(ICOSA) -I$(HEADER) -I$(QMAPS) -I$(IO)

SPopulation.o: SPopulation.cpp SPopulation.h 
	$(CC) $(COPT) $(DEFS) $(COMP)  -I$(ICOSA) -I$(HEADER) -I$(IO)

Prioritizer.o: Prioritizer.cpp Prioritizer.h $(POPS)
	$(CC) $(COPT) $(DEFS) $(COMP)  -I$(ICOSA) -I$(HEADER) -I$(IO) -I$(MODULAR)


PopLooper.o: PopLooper.cpp PopLooper.h PopBase.h clsutils.h 
	$(CC) $(COPT) $(DEFS)  $(COMP) $(DOMP) -I$(ICOSA)

clsutils.o: clsutils.cpp clsutils.h
	$(CC) $(COPT) $(DEFS)



