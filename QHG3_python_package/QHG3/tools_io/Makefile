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
TARG=SPTest

COMMON=../common
GEOINFO=../geoinfo
TILING=../tiling
HEADER=../header
QMAPS=../qmaps
AGENTS=../agents
KERNEL=../kernel
MODULAR=../modular
POPULATIONS=../populations
ICOSA=../icosa
IO=../io
EXTERNAL=../external

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP)
COMP=-fopenmp

KERNEL_LIB:
	$(MAKE) --directory=$(KERNEL) lib

COMMON_LIB:
	$(MAKE) --directory=$(COMMON) lib

QMAPS_LIB:
	$(MAKE) --directory=$(QMAPS) lib

GEOINFO_LIB:
	$(MAKE) --directory=$(GEOINFO) 

EXTERNAL_LIB:
	$(MAKE) --directory=$(EXTERNAL) 

ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

IO_LIB:
	$(MAKE) --directory=$(IO) lib

HEADER_LIB:
	$(MAKE) --directory=$(HEADER) 




all: app




app: QDFDump QDF2Snap Pop2QDF Def2QDF
	$(if $(UTILS),cp $? $(UTILS),)


clean:
	rm -f *.o *.ti 

new: clean app

QDFDump.o: QDFDump.cpp
	$(CC) $(COPT) $(DEFS) -I$(IO)


QDFDump: QDFDump.o 
	$(CC) -o QDFDump \
	QDFDump.o \
	-L $(IO) -lIO \
	-L $(COMMON) -lCommon \
	-lhdf5


QDF2SnapBase.o: QDF2SnapBase.cpp QDF2SnapBase.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -I$(IO)

QDF2GeoSnap.o: QDF2GeoSnap.cpp QDF2GeoSnap.h QDF2SnapBase.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -I$(IO)

QDF2ClimateSnap.o: QDF2ClimateSnap.cpp QDF2ClimateSnap.h QDF2SnapBase.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -I$(IO)

QDF2VegSnap.o: QDF2VegSnap.cpp QDF2VegSnap.h QDF2SnapBase.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -I$(IO)

QDF2PopSnap.o: QDF2PopSnap.cpp QDF2PopSnap.h QDF2SnapBase.h 
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -I$(IO)

QDF2Snap.o: QDF2Snap.cpp  QDF2GeoSnap.h 
	$(CC) $(COPT) $(DEFS) -I$(HEADER) -I$(IO)

QDF2MoveStatSnap.o: QDF2MoveStatSnap.cpp  QDF2MoveStatSnap.h QDF2SnapBase.h 
	$(CC) $(COPT) $(DEFS) -I$(HEADER) -I$(IO)

QDF2Snap: QDF2Snap.o QDF2SnapBase.o QDF2GeoSnap.o QDF2ClimateSnap.o QDF2VegSnap.o \
	QDF2PopSnap.o QDF2MoveStatSnap.o
	$(CC) -o QDF2Snap \
	QDF2Snap.o QDF2SnapBase.o QDF2GeoSnap.o QDF2ClimateSnap.o QDF2VegSnap.o \
	QDF2PopSnap.o QDF2MoveStatSnap.o \
	-L $(IO) -lIO \
	-L $(HEADER) -lHeader \
	-L $(COMMON) -lCommon \
	-lhdf5 -lz




Def2QDF.o: Def2QDF.cpp  
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO)



#	IO_LIB KERNEL_LIB ICOSA_LIB QMAPS_LIB HEADER_LIB COMMON_LIB EXTERNAL_LIB

Def2QDF: Def2QDF.o 
	$(CC) -o Def2QDF -fopenmp \
        Def2QDF.o \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(QMAPS)    -lQMap \
	-L$(HEADER)   -lHeader \
	-L$(COMMON)   -lCommon \
	-L$(EXTERNAL) -lNR \
	-lhdf5 -lz

Pop2QDF.o: Pop2QDF.cpp
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO) -I$(GEOINFO) -I$(EXTERNAL) -I$(POPULATIONS)

#	IO_LIB KERNEL_LIB ICOSA_LIB QMAPS_LIB HEADER_LIB COMMON_LIB EXTERNAL_LIB


Pop2QDF: Pop2QDF.o  
	$(CC) -o Pop2QDF -fopenmp \
        Pop2QDF.o  \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(MODULAR)  -lModular \
	-L$(POPULATIONS)  -lPopulations \
	-L$(MODULAR)  -lModular \
	-L$(ICOSA)    -lIcosa \
	-L$(HEADER)   -lHeader \
	-L$(GEOINFO)  -lGeoInfo \
	-L$(COMMON)   -lCommon \
	-L$(QMAPS)    -lQMap \
    -L$(EXTERNAL) -lNR \
	-lz -lhdf5 

Snap2QMap.o: Snap2QMap.cpp 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(QMAPS) -I$(HEADER) -I$(GEOINFO) -I$(IO)


#	KERNEL_LIB ICOSA_LIB QMAPS_LIB HEADER_LIB COMMON_LIB GEOINFO_LIB 

Snap2QMap:Snap2QMap.o 
	$(CC) -o Snap2QMap \
        Snap2QMap.o \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(QMAPS)    -lQMap \
	-L$(HEADER)   -lHeader \
	-L$(GEOINFO)  -lGeoInfo \
	-L$(COMMON)   -lCommon \
        -lz


