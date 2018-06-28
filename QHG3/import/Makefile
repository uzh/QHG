#
# determine compiler and flags
#
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

ifdef OPT
  CFLAGS=-O3 $(WARNINGS) 
endif


# different flags needed on different computers
# if HOSTNAME is not set, try to set it with the 'hostname' command
ifeq "$(HOSTNAME)" ""
#  $(warning trying hostname command)
  HOSTNAME:=$(shell hostname)
#  $(warning result: '$(HOSTNAME)')
endif 

ifeq "$(HOSTNAME)" ""
  $(error HOSTNAME is not set, and hostname does not work)
else
# set the name of the special case
  sc=enki
#  if its the special case use MagickWand-config
  ifeq "$(HOSTNAME)" "$(sc)"
    COMP=
    DOMP=
  else
    COMP=-fopenmp
    DOMP=-DOMP	
  endif

endif


ifndef ARFL
	ARFL=-rv
endif

KERNEL=../kernel
COMMON=../common
GEOINFO=../geoinfo
IO=../io
KERNEL=../kernel
ICOSA=../icosa
QMAPS=../qmaps


UTIL_DIR=~/utils

COPT=-c -g $< -I$(COMMON) -o $@ $(CADD) $(CFLAGS)

APPS=Coastal VectorImport SeaWays

app: $(APPS)

all: $(APPS)

libImport.a: TabReader.o TabDataReader.o UDelReader.o FAOReader.o
	ar $(ARFL) $@ $?

new: clean app

clean:
	rm -f *.o *.a *.ti $(APPS)

dbfReader.o: dbfReader.cpp dbfReader.h 
	$(CC) $(COPT) $(DEFS)  

shpUtils.o: shpUtils.cpp shpUtils.h 
	$(CC) $(COPT) $(DEFS)

shpRecord.o: shpRecord.cpp shpRecord.h shpUtils.h 
	$(CC) $(COPT) $(DEFS) 

shpHeader.o: shpHeader.cpp shpHeader.h shpUtils.h 
	$(CC) $(COPT) $(DEFS) 

VectorImport.o: VectorImport.cpp shpHeader.h shpUtils.h shpRecord.h dbfReader.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

VectorImport:VectorImport.o shpHeader.o shpUtils.o shpRecord.o dbfReader.o
	g++  -g -Wall -o VectorImport VectorImport.o shpHeader.o shpUtils.o shpRecord.o dbfReader.o \
	-L $(KERNEL)  -lKernel \
	-L $(ICOSA)   -lIcosa \
	-L $(GEOINFO) -lGeoInfo \
	-L $(IO)      -lIO \
	-L $(COMMON)  -lCommon \
	-lz -lhdf5 -fopenmp


Coastal.o: Coastal.cpp 
	$(CC) $(COPT) $(COMP) $(DEFS)  -I$(KERNEL)  -I$(ICOSA) -I$(IO) -I$(GEOINFO)

Coastal: Coastal.o
	g++  -g -Wall -o Coastal Coastal.o \
	-L $(KERNEL)  -lKernel \
	-L $(ICOSA)   -lIcosa \
	-L $(IO)      -lIO \
	-L $(COMMON)  -lCommon \
	-lz -lhdf5 -fopenmp



TabReader.o:TabReader.cpp TabReader.h
	$(CC) $(COPT) $(DEFS)

TabDataReader.o:TabDataReader.cpp TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS)

UDelReader.o:UDelReader.cpp UDelReader.h TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS) -I $(QMAPS)

FAOReader.o:FAOReader.cpp FAOReader.h TabDataReader.h TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS)

UDel2QMap.o:UDel2QMap.cpp
	$(CC) $(COPT) $(DEFS) -I $(QMAPS)

UDel2QMap:UDel2QMap.o \
		UDelReader.o TabDataReader.o TabReader.o \
		$(QMAPS)/QMapHeader.o $(QMAPS)/QMapReader.o $(QMAPS)/QMapUtils.o
	$(CC) -o UDel2QMap \
        UDel2QMap.o \
        UDelReader.o \
        TabDataReader.o \
        TabReader.o \
	$(QMAPS)/QMapHeader.o \
        $(QMAPS)/QMapReader.o \
        $(QMAPS)/QMapUtils.o \
        $(COMMON)/strutils.o $(COMMON)/BinSearch.o $(COMMON)/Interpolator_gsl.o


ComponentBuilder.o: ComponentBuilder.cpp ComponentBuilder.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -fopenmp 

CoastalDistances.o: CoastalDistances.cpp CoastalDistances.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -fopenmp 

SubComponent.o: SubComponent.cpp SubComponent.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -fopenmp 

SubComponentManager.o: SubComponentManager.cpp SubComponentManager.h SubComponent.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(ICOSA) -fopenmp 

SeaWays.o: SeaWays.cpp ComponentBuilder.h CoastalDistances.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(IO) -I$(ICOSA) -fopenmp 


SeaWays: SeaWays.o ComponentBuilder.o CoastalDistances.o SubComponentManager.o SubComponent.o
	$(CC) -o SeaWays -fopenmp \
	SeaWays.o ComponentBuilder.o CoastalDistances.o SubComponentManager.o SubComponent.o \
	-L$(ICOSA)  -lIcosa \
	-L$(IO)     -lIO \
	-L$(KERNEL) -lKernel \
	-L$(COMMON) -lCommon \
	-lhdf5 -lz



