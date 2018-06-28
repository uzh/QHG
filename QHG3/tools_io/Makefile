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
  CFLAGS=-g $(WARNINGS)
endif

ifdef OPT
  CFLAGS=-O3 $(WARNINGS)
endif

ifdef PROF
  CFLAGS=-pg $(WARNINGS)
endif

UTILS=~/utils

DOMP=
TARG=SPTest

COMMON=../common
VISUAL=../visual
KERNEL=../kernel
ICOSA=../icosa
IO=../io
QMAPS=../qmaps

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP)
COMP=-fopenmp

KERNEL_LIB:
	$(MAKE) --directory=$(KERNEL) lib

COMMON_LIB:
	$(MAKE) --directory=$(COMMON) lib

QMAPS_LIB:
	$(MAKE) --directory=$(QMAPS) lib


ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

IO_LIB:
	$(MAKE) --directory=$(IO) lib

HEADER_LIB:
	$(MAKE) --directory=$(HEADER) 




all: app




app: QDFDump Def2QDF QDF2PNG QDF2PNGNew
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

Def2QDF.o: Def2QDF.cpp  
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO)


Def2QDF: Def2QDF.o 
	$(CC) -o Def2QDF -fopenmp \
        Def2QDF.o \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(QMAPS)    -lQMap \
	-L$(COMMON)   -lCommon \
	-lhdf5 -lz -lgsl

	
CreateEmptyQDF.o: CreateEmptyQDF.cpp  
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO)


CreateEmptyQDF: CreateEmptyQDF.o 
	$(CC) -o CreateEmptyQDF -fopenmp \
        CreateEmptyQDF.o \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(QMAPS)    -lQMap \
	-L$(COMMON)   -lCommon \
	-lhdf5 -lz


SurfaceGrid.o: SurfaceGrid.cpp SurfaceGrid.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO)

QDF2PNG.o: QDF2PNG.cpp SurfaceGrid.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(VISUAL) -I$(IO)  `pkg-config --cflags gdk-2.0 pangocairo` -I /usr/include/gtk-2.0/gdk
QDF2PNG: QDF2PNG.o SurfaceGrid.o
	$(CC) -o QDF2PNG \
	QDF2PNG.o SurfaceGrid.o \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(COMMON)   -lCommon \
	-L$(VISUAL)   -lVisual \
        -lz -lhdf5 -lpng -fopenmp \
        `pkg-config --libs gdk-2.0 pangocairo`

QDFImageExtractor.o: QDFImageExtractor.cpp QDFImageExtractor.h SurfaceGrid.h  
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(VISUAL) -I$(IO)  `pkg-config --cflags gdk-2.0 pangocairo` -I /usr/include/gtk-2.0/gdk

QDF2PNGNew.o: QDF2PNGNew.cpp QDFImageExtractor.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA)  -I$(IO)  `pkg-config --cflags gdk-2.0 pangocairo` -I /usr/include/gtk-2.0/gdk
QDF2PNGNew: QDF2PNGNew.o QDFImageExtractor.o SurfaceGrid.o
	$(CC) -o QDF2PNGNew \
	QDF2PNGNew.o QDFImageExtractor.o SurfaceGrid.o \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(COMMON)   -lCommon \
	-L$(VISUAL)   -lVisual \
        -lz -lhdf5 -lpng -fopenmp \
        `pkg-config --libs gdk-2.0 pangocairo`


Angulator.o: Angulator.cpp Angulator.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO)  

Angulator: Angulator.o
	$(CC) -o Angulator \
	Angulator.o \
	-L$(IO)       -lIO \
	-L$(KERNEL)   -lKernel \
	-L$(ICOSA)    -lIcosa \
	-L$(COMMON)   -lCommon \
        -lz -lhdf5  -fopenmp 


