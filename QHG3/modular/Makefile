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

ifdef MUTSTAT
  CFLAGS:=$(CFLAGS) -DMUTSTAT
endif

COMMON=../common
ICOSA=../icosa
KERNEL=../kernel
IO=../io
GENES=../genes

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(OMPPFLAGS) $(DYN)
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

