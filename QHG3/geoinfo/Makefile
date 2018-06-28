#ifdef MPI
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

COMMON=../common
VISUAL=../visual
GEOINFO=../geoinfo
TILING=../tiling

COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) -fPIC


all: geoinfo

geoinfo: libGeoInfo.a

libGeoInfo.a: Projector.o EQRProjector.o OrthoProjector.o \
	      AEProjector.o LAEAProjector.o TCEAProjector.o \
              CEAProjector.o LCCProjector.o LINProjector.o \
              GeoInfo.o \
              DEM.o StatDEM.o DynDEM.o DynSeqDEM.o DynXYZDEM.o \
              GeoProvider.o TPGeoProvider.o PlaneGeoProvider.o \
              SimpleGeoProvider.o PrGeoProvider.o \
              GridProjection.o
	ar $(ARFL) $@ $?

clean:
	rm -f *.o libGeoInfo.a *.ti

test: gptest

Projector.o: Projector.cpp  Projector.h \
             $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

EQRProjector.o: EQRProjector.cpp EQRProjector.h Projector.h  
	$(CC) $(COPT) $(DEFS)

OrthoProjector.o: OrthoProjector.cpp OrthoProjector.h Projector.h  
	$(CC) $(COPT) $(DEFS)

AEProjector.o: AEProjector.cpp AEProjector.h Projector.h   \
               $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

LAEAProjector.o: LAEAProjector.cpp LAEAProjector.h Projector.h  
	$(CC) $(COPT) $(DEFS)

TCEAProjector.o: TCEAProjector.cpp TCEAProjector.h Projector.h 
	$(CC) $(COPT) $(DEFS)

CEAProjector.o: CEAProjector.cpp CEAProjector.h Projector.h 
	$(CC) $(COPT) $(DEFS)

LCCProjector.o: LCCProjector.cpp LCCProjector.h Projector.h   \
             $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

LINProjector.o: LINProjector.cpp LINProjector.h Projector.h   \
             $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

GeoInfo.o: GeoInfo.cpp GeoInfo.h  \
               Projector.h EQRProjector.h OrthoProjector.h \
               AEProjector.h TCEAProjector.h LAEAProjector.h LCCProjector.h  \
	       LINProjector.h \
             $(COMMON)/utils.h 
	$(CC) $(COPT) $(DEFS) 

DEM.o: DEM.cpp DEM.h 
	$(CC) $(COPT) $(DEFS)

StatDEM.o: StatDEM.cpp StatDEM.h DEM.h  
	$(CC) $(COPT) $(DEFS)

DynDEM.o: DynDEM.cpp DynDEM.h DEM.h  
	$(CC) $(COPT) $(DEFS)

DynSeqDEM.o: DynSeqDEM.cpp DynSeqDEM.h DynDEM.h DEM.h  
	$(CC) $(COPT) $(DEFS)

DynXYZDEM.o: DynXYZDEM.cpp DynXYZDEM.h DynDEM.h DEM.h  
	$(CC) $(COPT) $(DEFS)

GeoProvider.o: GeoProvider.cpp GeoProvider.h DEM.h  \
                 $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

TPGeoProvider.o: TPGeoProvider.cpp TPGeoProvider.h GeoProvider.h DEM.h  \
                 $(COMMON)/utils.h 
	$(CC) $(COPT) $(DEFS)

PrGeoProvider.o: PrGeoProvider.cpp PrGeoProvider.h GeoProvider.h DEM.h  GridProjection.h \
                 $(COMMON)/utils.h 
	$(CC) $(COPT) $(DEFS)

SimpleGeoProvider.o: SimpleGeoProvider.cpp SimpleGeoProvider.h GeoProvider.h \
                     $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

PlaneGeoProvider.o: PlaneGeoProvider.cpp PlaneGeoProvider.h GeoProvider.h DEM.h GridProjection.h \
                     $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

GridProjection.o: GridProjection.cpp GridProjection.h Projector.h GeoInfo.h \
                     $(COMMON)/utils.h
	$(CC) $(COPT) $(DEFS)

GeoMapper.o: GeoMapper.cpp GeoMapper.h GridProjection.h Projector.h GeoInfo.h \
                     $(COMMON)/utils.h $(COMMON)/Interpolator.h
	$(CC) $(COPT) $(DEFS)

gt.o:gt.cpp GeoInfo.h
	$(CC) $(COPT) $(DEFS)

gt:gt.o libGeoInfo.a
	$(CC) -o gt $(CADD) \
	gt.o \
	-L $(GEOINFO) -lGeoInfo 

gptest.o:gptest.cpp GridProjection.h GeoInfo.h
	$(CC) $(COPT) $(DEFS)

gptest:gptest.o libGeoInfo.a
	$(CC) -o gptest $(CADD) \
	gptest.o \
        $(COMMON)/strutils.o \
        -L $(GEOINFO) -lGeoInfo

ProjTest.o:ProjTest.cpp GeoInfo.h
	$(CC) $(COPT) $(DEFS) 

ProjTest:ProjTest.o libGeoInfo.a
	$(CC) -o ProjTest $(CADD) \
	ProjTest.o \
	$(COMMON)/ParamReader.o $(COMMON)/strutils.o $(COMMON)/Vec3D.o $(COMMON)/MessLogger.o \
        -L $(GEOINFO) -lGeoInfo -lcurses

ProjList.o:ProjList.cpp GeoInfo.h
	$(CC) $(COPT) $(DEFS) 

ProjList:ProjList.o libGeoInfo.a
	$(CC) -o ProjList $(CADD) \
	ProjList.o \
	$(COMMON)/ParamReader.o $(COMMON)/strutils.o $(COMMON)/LineReader.o \
	$(COMMON)/Vec3D.o $(COMMON)/MessLogger.o \
        -L $(GEOINFO) -lGeoInfo -lz

