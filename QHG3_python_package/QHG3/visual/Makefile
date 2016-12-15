
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif


COMMON=../common
KERNEL=../kernel
GEOINFO=../geoinfo
QMAPS=../qmaps
IMPORT=../import
EXTERNAL=../external
PNG=/usr/include/libpng
APPS:= _xbvm _qmv 
UTIL_DIR=~/utils

COPT=-c -Wall $< -I$(COMMON) -o $@ $(CADD) $(CFLAGS)
GLUTFLAGS =	-L/usr/X11R6/lib -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm

ARFL=rv

# different flags needed on different computers
# if HOSTNAME is not set, try to set it with the 'hostname' command
ifeq "$(HOSTNAME)" ""
  #$(warning trying hostname command)
  HOSTNAME:=$(shell hostname)
  #$(warning result: '$(HOSTNAME)')
endif 

ifeq "$(HOSTNAME)" ""
  $(error HOSTNAME is not set, and hostname does not work)
else
# set the name of the special case
  sc=enki
#  if its the special case use MagickWand-config
  ifeq "$(HOSTNAME)" "$(sc)"
    MAGIC_CFLAGS=`MagickWand-config --cflags --cppflags`
    MAGIC_LFLAGS=`MagickWand-config --ldflags --libs` 
  else
    MAGIC_CFLAGS=
    MAGIC_LFLAGS=-lWand -lMagick
  endif

  all: visual
  apps: $(APPS)
endif

visual: libVisual.a

libVisual.a: PNGImage.o LookUp.o GeoLookUp.o VegLookUp.o PopLookUp.o TwoToneLookUp.o ThreshLookUp.o \
	     DensityLookUp.o DiscreteLookUp.o UCharLookUp.o BVLookUp.o SunLookUp.o RainbowLookUp.o \
	     BiomeLookUp.o DensityLookUp.o ZebraLookUp.o SegmentLookUp.o  GenLookUp.o LookUpFactory.o
	ar $(ARFL) $@ $?

clean:
	rm -f *.o libKernel.a $(APPS) *.ti

utils: $(APPS)
	cp $? $(UTIL_DIR)


PNGImage.o: PNGImage.cpp PNGImage.h
	$(CC) $(COPT) $(DEFS)

SunLookUp.o: SunLookUp.cpp SunLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

GeoLookUp.o: GeoLookUp.cpp GeoLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

VegLookUp.o: VegLookUp.cpp VegLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

PopLookUp.o: PopLookUp.cpp PopLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

RainbowLookUp.o: RainbowLookUp.cpp RainbowLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

DiscreteLookUp.o: DiscreteLookUp.cpp DiscreteLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

UCharLookUp.o: UCharLookUp.cpp UCharLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

TabLookUp.o: TabLookUp.cpp TabLookUp.h LookUp.h SegCenters.h
	$(CC) $(COPT) $(DEFS)

TwoToneLookUp.o: TwoToneLookUp.cpp TwoToneLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

ThreshLookUp.o: ThreshLookUp.cpp ThreshLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

DensityLookUp.o: DensityLookUp.cpp DensityLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

BiomeLookUp.o: BiomeLookUp.cpp BiomeLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

ZebraLookUp.o: ZebraLookUp.cpp ZebraLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

SegmentLookUp.o: SegmentLookUp.cpp SegmentLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

GenLookUp.o: GenLookUp.cpp GenLookUp.h LookUp.h
	$(CC) $(COPT) $(DEFS)

LookUpFactory.o: LookUpFactory.cpp LookUpFactory.h LookUp.h \
                 UCharLookUp.h BVLookUp.h VegLookUp.h PopLookUp.h \
	         GeoLookUp.h SunLookUp.h RainbowLookUp.h ThreshLookUp.h \
		 DensityLookUp.h BiomeLookUp.h ZebraLookUp.h SegmentLookUp.h GenLookUp.h
	$(CC) $(COPT) $(DEFS)

ImageDisplay.o: ImageDisplay.cpp ImageDisplay.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS) \
        `pkg-config gtkmm-2.4 --cflags`

xbv.o: xbv.cpp xbv.h
	$(CC) $(COPT) $(DEFS) \
	-I$(QMAPS) -I$(GEOINFO) \
         `pkg-config gtkmm-2.4 --cflags`

QMapView.o: QMapView.cpp xbv.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS) -I$(GEOINFO) \
	`pkg-config gtkmm-2.4 --cflags`

xbvd.o: xbvd.cpp xbvd.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS)  -I$(GEOINFO) \
	`pkg-config gtkmm-2.4 --cflags`

xbvod.o: xbvod.cpp xbvod.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS)  -I$(GEOINFO) \
	`pkg-config gtkmm-2.4 --cflags`

xbvsd.o: xbvsd.cpp xbvsd.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS) -I$(GEOINFO) \
	`pkg-config gtkmm-2.4 --cflags`

xbvrd.o: xbvrd.cpp xbvrd.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS) -I$(GEOINFO) \
	`pkg-config gtkmm-2.4 --cflags`

# removed snapreader.o
QMapView: QMapView.o xbv.o xbvd.o xbvod.o xbvsd.o xbvrd.o \
	  LookUp.o DiscreteLookUp.o UCharLookUp.o BVLookUp.o VegLookUp.o PopLookUp.o GeoLookUp.o \
          SunLookUp.o RainbowLookUp.o ThreshLookUp.o DensityLookUp.o BiomeLookUp.o ZebraLookUp.o SegmentLookUp.o GenLookUp.o LookUpFactory.o PNGImage.o \
	  $(QMAPS)/QMapHeader.o $(QMAPS)/QMapReader.o $(QMAPS)/QMapUtils.o
	$(CC) $(CADD) -o QMapView \
        QMapView.o xbv.o xbvd.o xbvod.o xbvsd.o xbvrd.o \
	LookUp.o DiscreteLookUp.o UCharLookUp.o BVLookUp.o VegLookUp.o PopLookUp.o GeoLookUp.o \
        SunLookUp.o RainbowLookUp.o  ThreshLookUp.o DensityLookUp.o BiomeLookUp.o ZebraLookUp.o SegmentLookUp.o GenLookUp.o LookUpFactory.o PNGImage.o  \
	$(QMAPS)/QMapHeader.o $(QMAPS)/QMapReader.o $(QMAPS)/QMapUtils.o \
	$(COMMON)/utils.o $(COMMON)/BinSearch.o $(COMMON)/Interpolator.o \
	$(COMMON)/LineReader.o $(COMMON)/Vec3D.o $(COMMON)/strutils.o \
	$(COMMON)/MessLogger.o \
	-L $(GEOINFO) -lGeoInfo -L $(EXTERNAL) -lNR  -lz -lpng \
	`pkg-config gtkmm-2.4 --libs`

_xbvm: QMapView
	cp QMapView _xbvm; cp QMapView _qmv 



# for tools

BinVisualizer.o: BinVisualizer.cpp BinVisualizer.h $(COMMON)/ValueMapper.h LookUp.h PNGImage.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL) -I$(IMPORT)

ListVisualizer.o: ListVisualizer.cpp ListVisualizer.h LookUp.h PNGImage.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL)

ImageDecorator.o: ImageDecorator.cpp ImageDecorator.h
	$(CC) $(COPT) $(DEFS) -I$(KERNEL)

