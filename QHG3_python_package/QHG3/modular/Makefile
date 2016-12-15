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


COMMON=../common
GEOINFO=../geoinfo
TILING=../tiling
HEADER=../header
QMAPS=../qmaps
AGENTS=../agents
ICOSA=../icosa
KERNEL=../kernel
IO=../io
EXTERNAL=../external
GENES=../genes

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(OMPPFLAGS)
COMP=-fopenmp

ARFL=rv

ACTSRC=$(wildcard *.cpp)
ACT=$(ACTSRC:.cpp=.o)

all: modular

lib: libModular.a

libompA:DOMP=-DOMP_A
libompA:libModular.a

modular: libModular.a

libModular.a: $(ACT)
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libModular.a *.ti


%.o: %.cpp %.h 
	$(CC) $(COPT) $(COMP) $(DEFS) -I${KERNEL} -I$(ICOSA) -I$(IO) -I$(GENES) -fopenmp

