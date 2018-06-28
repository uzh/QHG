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

UTILS=~/utils

DOMP=

COMMON=../common
GEOINFO=../geoinfo
TILING=../tiling
HEADER=../header
QMAPS=../qmaps
AGENTS=../agents
KERNEL=../kernel
ICOSA=../icosa
IO=../io
MODULAR=../modular
EXTERNAL=../external

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP)
COMP=-fopenmp

TARGETS=GRTest

all: $(TARGETS)



app: GRTest
	$(if $(UTILS),cp $? $(UTILS),)



clean:
	rm -f *.o *.ti GRTest



GRegion.o: GRegion.cpp GRegion.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GRegionizer.o: GRegionizer.cpp GRegionizer.h GRegion.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO) -I$(HEADER)

GRTest.o: GRTest.cpp GRegionizer.h GRegion.h
	$(CC) $(COPT) $(DEFS)  -I$(IO)


GRTest: GRTest.o GRegionizer.o GRegion.o 
	$(CC) $(COMP) -o GRTest \
	GRTest.o GRegionizer.o GRegion.o \
	-L $(IO)      -lIO \
	-L $(KERNEL)  -lKernel \
	-L $(QMAPS)   -lQMap \
	-L $(MODULAR) -lModular \
	-L $(HEADER)  -lHeader \
	-L $(COMMON)  -lCommon \
	-L $(EXTERNAL) -lNR \
	-lhdf5 -lz


