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

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif


ifndef ARFL
  ARFL=-rv
endif


COMMON=../common
GEOINFO=../geoinfo
TILING=../tiling
KERNEL=../kernel
QMAPS=../qmaps
AGENTS=../agents
ICOSA=../icosa

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) 

ARFL=rv

all: header

header: libHeader.a



libHeader.a:  PopLoader.o HeaderBase.o PopHeader.o SnapHeader.o WorldHeader.o
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libHeader.a *.ti


PopHeader.o: PopHeader.cpp PopHeader.h
	$(CC) $(COPT) $(DEFS)  -I $(TILING) -I$(ICOSA)

PopReader.o: PopReader.cpp PopReader.h PopHeader.h
	$(CC) $(COPT) $(DEFS)  -I $(TILING) -I$(ICOSA)

PopLoader.o: PopLoader.cpp PopLoader.h PopHeader.h
	$(CC) $(COPT) $(DEFS)  -I $(TILING) -I$(ICOSA)


HeaderBase.o: HeaderBase.cpp HeaderBase.h
	$(CC) $(COPT) $(DEFS) -g  -I $(TILING) -I$(ICOSA) 

SnapHeader.o: SnapHeader.cpp SnapHeader.h HeaderBase.h
	$(CC) $(COPT) $(DEFS)  -I $(TILING) -I$(ICOSA) 

WorldHeader.o: WorldHeader.cpp WorldHeader.h HeaderBase.h
	$(CC) $(COPT) $(DEFS) -g  -I $(TILING) 

