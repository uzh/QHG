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
VISUAL=../visual
GEOINFO=../geoinfo
IO=../io
KERNEL=../kernel
ICOSA=../icosa
HEADER=../header
QMAPS=../qmaps
EXTERNAL=../external


UTIL_DIR=~/utils

COPT=-c -g $< -I$(COMMON) -o $@ $(CADD) $(CFLAGS)

APPS: UDel2QMap

all: libImport.a $(APPS)

libImport.a: TabReader.o TabDataReader.o UDelReader.o FAOReader.o
	ar $(ARFL) $@ $?


clean:
	rm -f *.o *.a *.ti $(APPS)

dbfReader.o: dbfReader.cpp dbfReader.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

shpUtils.o: shpUtils.cpp shpUtils.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

shpRecord.o: shpRecord.cpp shpRecord.h shpUtils.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

shpHeader.o: shpHeader.cpp shpHeader.h shpUtils.h 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

VectorImport.o: VectorImport.cpp shpHeader.h shpUtils.h shpRecord.h dbfReader.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

VectorImport:VectorImport.o shpHeader.o shpUtils.o shpRecord.o dbfReader.o
	g++  -g -Wall -o VectorImport VectorImport.o shpHeader.o shpUtils.o shpRecord.o dbfReader.o \
	-L $(KERNEL)  -lKernel \
	-L $(QMAPS)   -lQMap \
	-L $(ICOSA)   -lIcosa \
	-L $(HEADER)  -lHeader \
	-L $(GEOINFO) -lGeoInfo \
	-L $(IO)      -lIO \
	-L $(COMMON)  -lCommon \
	-lz -lhdf5 -fopenmp


Coastal.o: Coastal.cpp 
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

Coastal: Coastal.o
	g++  -g -Wall -o Coastal Coastal.o \
	-L $(KERNEL)  -lKernel \
	-L $(QMAPS)   -lQMap \
	-L $(ICOSA)   -lIcosa \
	-L $(HEADER)  -lHeader \
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
        $(COMMON)/strutils.o $(COMMON)/BinSearch.o $(COMMON)/Interpolator.o \
        -L $(EXTERNAL) -lNR 

