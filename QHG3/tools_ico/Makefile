
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifndef CFLAGS
  CFLAGS=-g $(WARNINGS)
endif

UTILS=~/utils
COMMON=../common
KERNEL=../kernel
ICOSA=../icosa
IO=../io

COMP=-fopenmp
DOMP=-DOMP

ifdef OPT
  CFLAGS=-O3 $(WARNINGS)
endif
ifdef PROF
  CPROF=-pg
  DOMP=
else
  CPROF=
endif

ifndef ARFL
  ARFL=-rv
endif

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(CPROF) -I$(COMMON) -I$(ICOSA)


TARGS=EQTileTool 


app: $(TARGS)
	$(if $(UTILS),cp $? $(UTILS),)


new: clean app

clean: 
	rm -f *.o $(TARGS)



#####
# EQTileTool
#####

EQNodeClassificator.o: EQNodeClassificator.cpp EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS)

BasicTile.o: BasicTile.cpp BasicTile.h 
	$(CC) $(COPT) $(DEFS)

BasicSplitter.o: BasicSplitter.cpp BasicSplitter.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQTile.o: EQTile.cpp EQTile.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQSplitter.o: EQSplitter.cpp EQSplitter.h BasicSplitter.h EQTile.h EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS)

LonLatTile.o: LonLatTile.cpp LonLatTile.h BasicTile.h 
	$(CC) $(COPT) $(DEFS)

LonLatSplitter.o: LonLatSplitter.cpp LonLatSplitter.h BasicSplitter.h LonLatTile.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQZones.o: EQZones.cpp EQZones.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQGridCreator.o: EQGridCreator.cpp EQGridCreator.h EQZones.h BasicTile.h BasicSplitter.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(KERNEL) -I$(ICOSA) -I$(IO)

EQTiling.o: EQTiling.cpp EQTiling.h BasicTiling.h EQSplitter.h EQGridCreator.h EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

LonLatTiling.o: LonLatTiling.cpp LonLatTiling.h BasicTiling.h LonLatSplitter.h EQGridCreator.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

BasicTiling.o: BasicTiling.cpp BasicTiling.h BasicSplitter.h EQGridCreator.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

EQTileLinks.o: EQTileLinks.cpp EQTileLinks.h 
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(ICOSA)

EQTileTool.o: EQTileTool.cpp EQGridCreator.h 
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(KERNEL) -I$(ICOSA) -I$(IO)

EQTileTool: EQTileTool.o EQGridCreator.o EQZones.o \
	BasicTile.o BasicSplitter.o BasicTiling.o \
	EQTile.o EQSplitter.o EQTiling.o \
        EQNodeClassificator.o EQTileLinks.o \
	LonLatTile.o LonLatSplitter.o LonLatTiling.o 
	$(CC) -o EQTileTool $(CADD) $(COMP) \
	EQTileTool.o  EQGridCreator.o EQZones.o \
	BasicTile.o BasicSplitter.o BasicTiling.o \
	EQTile.o EQSplitter.o EQTiling.o \
        EQNodeClassificator.o EQTileLinks.o \
	LonLatTile.o LonLatSplitter.o LonLatTiling.o \
	-L$(ICOSA) -lIcosa \
	-L$(KERNEL) -lKernel \
	-L$(ICOSA) -lIcosa \
	-L$(IO) -lIO \
	-L$(COMMON) -lCommon \
	-lhdf5 -lz


