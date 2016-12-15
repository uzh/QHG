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
KERNEL=../kernel

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(OMPPFLAGS)
COMP=-fopenmp

ARFL=rv

POPSRC=$(wildcard *Pop.cpp)
POPS=$(POPSRC:.cpp=.o)

all: populations

CONFIGURE:
	../app/configure.sh

lib: CONFIGURE libPopulations.a

libompA:DOMP=-DOMP_A
libompA:libPopulations.a

populations: CONFIGURE libPopulations.a

PopulationFactory.o: PopulationFactory.cpp PopulationFactory.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA) -I$(IO) -I$(MODULAR) -I$(KERNEL) $(COMP) 


libPopulations.a:  PopulationFactory.o \
	              $(POPS)
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libPopulations.a *.ti






# populations

%Pop.o: %Pop.cpp %Pop.h  
	 $(CC) $(COPT) $(DEFS) $(COMP) -I$(MODULAR) -I$(ICOSA) -I$(IO) -I$(KERNEL) -I$(GENES)



