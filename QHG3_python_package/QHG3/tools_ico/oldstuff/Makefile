
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifndef CFLAGS
  CFLAGS=-g -Wall
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif


UTILS=~/utils
COMMON=../../common
QMAPS=../../qmaps
EXTERNAL=../../external
GEOINFO=../../geoinfo
HEADER=../../header
TILING=../../tiling
EXTERNAL=../../external
ICOSA=../../icosa

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

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(CPROF) -I$(COMMON)

CORE=GridZones.o IcoSpiral.o \
     Region.o FullRegion.o RectRegion.o GridRegion.o  LonLatRegion.o IrregRegion.o \
     RegionSplitter.o TrivialSplitter.o RectSplitter.o LonLatSplitter.o \
     Lattice.o



all: libOldIcosa.a 



lib: libOldIcosa.a


new: clean all

clean: 
	rm -f *.o $(CORE)

libOldIcosa.a: $(CORE)
	ar $(ARFL) $@ $?




GridZones.o: GridZones.cpp GridZones.h Region.h IcoSpiral.h  
	$(CC) $(COPT) $(DEFS) -I $(GEOINFO) -I$(ICOSA)

IcoSpiral.o: IcoSpiral.cpp IcoSpiral.h 
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)


RectSplitter.o: RectSplitter.cpp RectSplitter.h RegionSplitter.h Region.h RectRegion.h FullRegion.h 
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

RegionSplitter.o: RegionSplitter.cpp RegionSplitter.h 
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

TrivialSplitter.o: TrivialSplitter.cpp TrivialSplitter.h RegionSplitter.h FullRegion.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

LonLatSplitter.o: LonLatSplitter.cpp LonLatSplitter.h RegionSplitter.h Region.h FullRegion.h LonLatRegion.h
	$(CC) $(COPT) $(DEFS) -I$(TILING) -I$(ICOSA)


RectRegion.o: RectRegion.cpp RectRegion.h Region.h 
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO) -I$(ICOSA)

LonLatRegion.o: LonLatRegion.cpp LonLatRegion.h Region.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

FullRegion.o: FullRegion.cpp FullRegion.h Region.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)
 
GridRegion.o: GridRegion.cpp Region.h 
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

IrregRegion.o: IrregRegion.cpp IrregRegion.h 
	$(CC) $(COPT) $(DEFS) $(COMP) -I$(ICOSA)


















