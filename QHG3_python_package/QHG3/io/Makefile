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

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
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
EXTERNAL=../external

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP)
COMP=-fopenmp




ARFL=rv

all: lib
app: lib
lib: libIO.a




libIO.a:  QDFUtils.o QDFArray.o \
          PopWriter.o PopReader.o GridWriter.o GridReader.o \
          GeoWriter.o GeoReader.o ClimateWriter.o ClimateReader.o \
	  MoveStatWriter.o MoveStatReader.o VegReader.o VegWriter.o StatusWriter.o
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libIO.a *.ti 


QDFUtils.o: QDFUtils.cpp QDFUtils.h
	$(CC) $(COPT) $(DEFS)

QDFArray.o: QDFArray.cpp QDFArray.h QDFUtils.h
	$(CC) $(COPT) $(DEFS)



PopWriter.o: PopWriter.cpp PopWriter.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

PopReader.o: PopReader.cpp PopReader.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GridWriter.o: GridWriter.cpp GridWriter.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GridReader.o: GridReader.cpp GridReader.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GeoWriter.o: GeoWriter.cpp GeoWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GeoReader.o: GeoReader.cpp GeoReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

ClimateWriter.o: ClimateWriter.cpp ClimateWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

ClimateReader.o: ClimateReader.cpp ClimateReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

VegWriter.o: VegWriter.cpp VegWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

VegReader.o: VegReader.cpp VegReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

MoveStatWriter.o: MoveStatWriter.cpp MoveStatWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

MoveStatReader.o: MoveStatReader.cpp MoveStatReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

StatusWriter.o: StatusWriter.cpp StatusWriter.h PopWriter.h GridWriter.h GeoWriter.h MoveStatWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)


RectSurf.o: RectSurf.cpp RectSurf.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA)

