ifdef GCC
	CC=$(GCC)
endif
# determine compiler and flags
#
ifdef MPI
  ifdef CSCS
    CC=CC
    CADD=-I /apps/zlib/include -D CSCS
  else
    ifdef PGI_VER
      CC=/opt/64/mpich-gm-pgi/bin/mpiCC 
    else
      CC=mpic++
#      CC=/home/jody/utils/cmpicc
    endif
  endif
else
#  CC=g++
#  CC=/home/jody/utils/cgcc
#  CC=mpiCC
  ifndef CFLAGS
    CFLAGS=-Wall 
  endif
  ifndef CADD
    CADD=-g 
  endif
endif

ifdef OPT
  CFLAGS=-O3 -Wall
  CADD=
endif

# different flags needed on different computers
# if HOSTNAME is not set, try to set it with the 'hostname' command
ifeq "$(HOSTNAME)" ""
  $(warning trying hostname command)
  HOSTNAME:=$(shell hostname)
  $(warning result: '$(HOSTNAME)')
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

KERNEL=../kernel
COMMON=../common
GEOINFO=../geoinfo
QMAPS=../qmaps
EXTERNAL=../external

UTIL_DIR=~/utils


APPS= BiomassDistribution TempRainBiomizer TempRainCapacity QGrowthTest  UDel2QMap TopoTemper

COPT=-c $< -I$(COMMON) -o $@ $(CADD) $(CFLAGS)

app: $(APPS)

clean:
	rm -f *.o $(APPS)

new: clean app

BiomassDist.o: BiomassDist.cpp BiomassDist.h TrinaryFunc.h PercentageReader.h
	$(CC) $(COPT) $(DEFS) $(COMP) $(DOMP) -I$(QMAPS) 

BiomassDistribution.o:BiomassDistribution.cpp BiomassDist.h\
        TrinaryFunc.h Altitudinal.h Latitudinal.h 
	$(CC) $(COPT) $(DEFS) -I$(QMAPS)

PercentageReader.o:PercentageReader.cpp PercentageReader.h 
	$(CC) $(COPT) $(DEFS) -I$(COMMON)

Const.o: Const.cpp Const.h TrinaryFunc.h
	$(CC) $(COPT) $(DEFS) 

Latitudinal.o: Latitudinal.cpp Latitudinal.h TrinaryFunc.h
	$(CC) $(COPT) $(DEFS) 

Altitudinal.o: Altitudinal.cpp Altitudinal.h TrinaryFunc.h
	$(CC) $(COPT) $(DEFS)  -I$(QMAPS)

LatAltitudinal.o: LatAltitudinal.cpp LatAltitudinal.h TrinaryFunc.h
	$(CC) $(COPT) $(DEFS)  -I$(QMAPS)



BiomassDistribution:BiomassDistribution.o BiomassDist.o \
	Const.o Altitudinal.o Latitudinal.o LatAltitudinal.o \
        PercentageReader.o		
	$(CC) $(CADD) $(COMP) -o BiomassDistribution \
	BiomassDistribution.o BiomassDist.o \
        Const.o Altitudinal.o Latitudinal.o LatAltitudinal.o \
	PercentageReader.o \
        -L $(QMAPS) -lQMap \
        -L $(COMMON) -lCommon \
        -L $(EXTERNAL) -lNR \
        -lz



TempRainBiomizer.o:TempRainBiomizer.cpp TempRainBiome.h
	$(CC) $(COPT) $(DEFS) \
	-I$(COMMON)  -I$(QMAPS) 

TempRainBiome.o: TempRainBiome.cpp TempRainBiome.h BStateVector.h
	$(CC) $(COPT) $(DEFS) -Wall  -I$(QMAPS)


BStateVector.o:BStateVector.cpp BStateVector.h 
	$(CC) $(COPT) $(DEFS)


TempRainBiomizer: TempRainBiomizer.o BStateVector.o TempRainBiome.o
	$(CC) -o TempRainBiomizer $(CADD) \
        TempRainBiomizer.o \
        BStateVector.o \
        TempRainBiome.o \
        -L $(QMAPS) -lQMap \
        -L $(COMMON) -lCommon \
        -L $(EXTERNAL) -lNR \
        -lz


TempRainCap.o:TempRainCap.cpp TempRainCap.h
	$(CC) $(COPT) $(DEFS) $(COMP) $(DOMP) -I$(QMAPS)


TempRainCapacity.o:TempRainCapacity.cpp TempRainCap.h
	$(CC) $(COPT) $(DEFS)


TempRainCapacity: TempRainCapacity.o TempRainCap.o
	$(CC) $(COMP) -o TempRainCapacity $(CADD) \
        TempRainCapacity.o \
        TempRainCap.o \
        -L $(QMAPS) -lQMap \
        -L $(COMMON) -lCommon \
        -L $(EXTERNAL) -lNR


QGrowthTest.o: QGrowthTest.cpp 
	$(CC) $(COPT) $(DEFS) $(COMP) -I $(QMAPS)

QGrowthTest:QGrowthTest.o 
	$(CC) $(CADD)  -o QGrowthTest QGrowthTest.o \
        -L $(QMAPS) -lQMap \
        -L $(COMMON) -lCommon \
        -L $(EXTERNAL) -lNR     


TopoTemp.o:TopoTemp.cpp TopoTemp.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(QMAPS)

AltiTemp.o:AltiTemp.cpp AltiTemp.h TopoTemp.h
	$(CC) $(COPT) $(DEFS) $(COMP) -I$(COMMON) -I$(QMAPS)

GradiTemp.o:GradiTemp.cpp GradiTemp.h TopoTemp.h
	$(CC) $(COPT) $(DEFS) $(COMP) -I$(COMMON) -I$(QMAPS)

TopoTemper.o:TopoTemper.cpp TopoTemp.h AltiTemp.h GradiTemp.h
	$(CC) $(COPT) $(DEFS) -I$(COMMON) -I$(QMAPS)


TopoTemper: TopoTemper.o TopoTemp.o AltiTemp.o GradiTemp.o
	$(CC) $(CADD)  -o TopoTemper \
	TopoTemper.o TopoTemp.o AltiTemp.o GradiTemp.o \
        -L $(QMAPS) -lQMap \
        -L $(COMMON) -lCommon \
        -L $(EXTERNAL) -lNR \
        -lz


TabReader.o:TabReader.cpp TabReader.h
	$(CC) $(COPT) $(DEFS)

TabDataReader.o:TabDataReader.cpp TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS)

UDelReader.o:UDelReader.cpp UDelReader.h TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS)



UDel2QMap.o: UDel2QMap.cpp UDelReader.h
	$(CC) $(COPT) $(DEFS)  -I$(COMMON) -I$(QMAPS)

UDel2QMap: UDel2QMap.o UDelReader.o TabDataReader.o TabReader.o
	$(CC) $(CADD) -o UDel2QMap \
        UDel2QMap.o UDelReader.o TabDataReader.o TabReader.o \
        -L $(QMAPS) -lQMap \
        -L $(COMMON) -lCommon \
        -L $(EXTERNAL) -lNR 

