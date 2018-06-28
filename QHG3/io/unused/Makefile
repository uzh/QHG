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

COMMON=../../common
KERNEL=../../kernel
ICOSA=../../icosa

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP)
COMP=-fopenmp




ARFL=rv

all: lib
app: lib
lib: libIOreader.a



#          PopWriter.o PopReader.o GridWriter.o GridReader.o \
#          GeoWriter.o GeoReader.o ClimateWriter.o ClimateReader.o \
#          NavReader.o NavWriter.o StatusWriter.o

libIOreader.a:  GridReader.o \
	GeoReader.o ClimateReader.o \
	MoveStatReader.o VegReader.o \
	NavReader.o 
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libIO.a *.ti 




PopReader.o: PopReader.cpp PopReader.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GridReader.o: GridReader.cpp GridReader.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GeoReader.o: GeoReader.cpp GeoReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

ClimateReader.o: ClimateReader.cpp ClimateReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

VegReader.o: VegReader.cpp VegReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

MoveStatReader.o: MoveStatReader.cpp MoveStatReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

NavReader.o: NavReader.cpp NavReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 



