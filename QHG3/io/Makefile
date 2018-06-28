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
KERNEL=../kernel
ICOSA=../icosa

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) -fPIC
COMP=-fopenmp 




ARFL=rv

all: lib
app: lib
lib: libIO.a



#          PopWriter.o PopReader.o GridWriter.o GridReader.o \
#          GeoWriter.o GeoReader.o ClimateWriter.o ClimateReader.o \
#          NavReader.o NavWriter.o StatusWriter.o

libIO.a:  QDFUtils.o QDFArray.o \
	PopReader.o PopWriter.o GroupReader.o \
	GridWriter.o GridGroupReader.o \
	GeoWriter.o GeoGroupReader.o \
	ClimateWriter.o ClimateGroupReader.o \
	MoveStatWriter.o MoveStatGroupReader.o \
	VegGroupReader.o VegWriter.o \
	NavGroupReader.o NavWriter.o StatusWriter.o
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

GroupReader.o: GroupReader.cpp GroupReader.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GridWriter.o: GridWriter.cpp GridWriter.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GridGroupReader.o: GridGroupReader.cpp GridGroupReader.h GroupReader.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GeoWriter.o: GeoWriter.cpp GeoWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

GeoGroupReader.o: GeoGroupReader.cpp GeoGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)

ClimateWriter.o: ClimateWriter.cpp ClimateWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

ClimateGroupReader.o: ClimateGroupReader.cpp ClimateGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

VegWriter.o: VegWriter.cpp VegWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

VegGroupReader.o: VegGroupReader.cpp VegGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

MoveStatWriter.o: MoveStatWriter.cpp MoveStatWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

MoveStatGroupReader.o: MoveStatGroupReader.cpp MoveStatGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

NavWriter.o: NavWriter.cpp NavWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

NavGroupReader.o: NavGroupReader.cpp NavGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

StatusWriter.o: StatusWriter.cpp StatusWriter.h PopWriter.h GridWriter.h GeoWriter.h MoveStatWriter.h NavWriter.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA)


RectSurf.o: RectSurf.cpp RectSurf.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA)



GRTest.o: GRTest.cpp GroupReader.h GridGroupReader.h ClimateGroupReader.h VegGroupReader.h MoveStatGroupReader.h NavGroupReader.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) 

GRTest: GRTest.o GroupReader.o GridGroupReader.o GeoGroupReader.o ClimateGroupReader.o VegGroupReader.o MoveStatGroupReader.o NavGroupReader.o QDFUtils.o
	$(CC) -o GRTest -fopenmp \
	GRTest.o GroupReader.o GridGroupReader.o GeoGroupReader.o ClimateGroupReader.o VegGroupReader.o MoveStatGroupReader.o NavGroupReader.o QDFUtils.o \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(COMMON)   -lCommon \
	-lhdf5 -lz
