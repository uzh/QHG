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
ifndef WARNINGS
  WARNINGS =-Wall -Wshadow
endif

ifndef CFLAGS
  CFLAGS=-g $(WARNINGS)
endif

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


COMMON=../common
EXTERNAL=../external
GEOINFO=../geoinfo
VISUAL=../visual

APPS= QMapHead qmh \
      QScale QCombine QCanvas QConvert QSmooth QStat QRoll \
      QExtract QText QMask QDump QGrad QProj QPlot QGen QKillNan \
      QTranslate flipper div QMapNPP NC2QMap 

UTIL_DIR=~/utils

COPT=-c -g $< -I$(COMMON) -o $@ $(CADD) $(CFLAGS) -std=c++11 -fPIC

#all: lib app
all: lib

lib: COMMON_LIB libQMap.a

libQMap.a: QMapReader.o QMapHeader.o QMapUtils.o QMapUtils_T.o QMapDEM.o Interpolator.o BinSearch.o
	ar $(ARFL) $@ $?

clean:
	rm -f *.o *.a *.ti $(APPS) 

app: $(APPS) COMMON_LIB
	$(if $(UTIL_DIR),cp $(APPS) $(UTIL_DIR),)


COMMON_LIB:
	$(MAKE) --directory=$(COMMON) lib 
 
test: BMRTest SFTest QMapRT ARTest UDelRT TrinTest 


QMapHeader.o:QMapHeader.cpp QMapHeader.h 
	$(CC) $(COPT) $(DEFS)

QMapReader.o:QMapReader.cpp QMapReader.h ValReader.h QMapHeader.h Interpolator.h
	$(CC) $(COPT) $(DEFS)



VRTest.o: VRTest.cpp ValReader.h QMapReader.h QMapReader.cpp
	$(CC) $(COPT) $(DEFS)

VRTest: VRTest.o QMapReader.o QMapHeader.o 
	$(CC) $(CADD) VRTest.o QMapReader.o QMapHeader.o -o VRTest \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR


QMapUtils.o: QMapUtils.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QMapUtils_T.o: QMapUtils_T.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

BinConvolver.o: BinConvolver.cpp BinConvolver.h $(COMMON)/ConvKernel.h
	$(CC) $(COPT) $(DEFS)

ConvKernel.o: ConvKernel.cpp ConvKernel.h
	$(CC) $(COPT) $(DEFS)

QSmooth.o: QSmooth.cpp BinConvolver.h $(COMMON)/ConvKernel.h
	$(CC) $(COPT) $(DEFS) 

QSmooth: QSmooth.o \
        COMMON_LIB \
	BinConvolver.o ConvKernel.o QMapReader.o QMapHeader.o $(COMMON)/ParamReader.o
	$(CC) $(CADD) QSmooth.o BinConvolver.o ConvKernel.o QMapReader.o QMapHeader.o \
	-L $(COMMON) -lCommon \
	-L $(EXTERNAL) -lNR 
#	 -o QSmooth



QMapHead.o: QMapHead.cpp QMapHeader.h 
	$(CC) $(COPT) $(DEFS)

QMapHead: QMapHead.o QMapHeader.o
	$(CC) $(CADD) -o QMapHead \
	QMapHead.o \
	QMapHeader.o \
	-L $(COMMON) -lCommon ; cp QMapHead qmh 

qmh: QMapHead
	cp QMapHead qmh


BinSearch.o: BinSearch.cpp BinSearch.h 
	$(CC) $(COPT) $(DEFS)

Interpolator.o: Interpolator.cpp Interpolator.h 
	$(CC) $(COPT) $(DEFS) 

Interpolator_gsl.o: Interpolator_gsl.cpp Interpolator_gsl.h 
	$(CC) $(COPT) $(DEFS) 



flipper.o:flipper.cpp QMapHeader.h \
	$(COMMON)/ParamReader.h
	$(CC) $(COPT) $(DEFS)

flipper:flipper.o QMapHeader.o
	$(CC) $(CADD) -g flipper.o QMapHeader.o \
	-L $(COMMON) -lCommon \
	-o flipper


QCanvas.o: QCanvas.cpp QMapHeader.h QMapReader.h QMapReader.cpp QMapUtils.h QMapUtils_T.cpp
	$(CC) $(COPT) $(DEFS)

QCanvas:QCanvas.o QMapReader.o QMapHeader.o QMapUtils.o QMapUtils_T.o
	$(CC) $(CADD) QCanvas.o QMapReader.o QMapHeader.o QMapUtils.o QMapUtils_T.o -o QCanvas \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR	

QExtract.o: QExtract.cpp QMapHeader.h QMapReader.h QMapReader.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QExtract:QExtract.o QMapReader.o QMapHeader.o QMapUtils.o 
	$(CC) $(CADD) QExtract.o QMapReader.o QMapHeader.o QMapUtils.o -o QExtract \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR

QText.o: QText.cpp QMapUtils.h  QMapReader.h ValReader.h
	$(CC) $(COPT) $(DEFS)

QText:QText.o  QMapUtils.o  QMapReader.o QMapHeader.o
	$(CC) $(CADD) QText.o  QMapUtils.o  QMapReader.o QMapHeader.o  -o QText \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR


QCombine.o: QCombine.cpp QConverter.h QMapHeader.h QMapReader.h QMapReader.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QCombine:QCombine.o QConverter.o QMapReader.o QMapHeader.o QMapUtils.o 
	$(CC) $(CADD) QCombine.o QConverter.o QMapReader.o QMapHeader.o QMapUtils.o -o QCombine \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR	

QDump.o: QDump.cpp  QMapHeader.h QMapReader.h QMapReader.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QDump: QDump.o QMapReader.o QMapHeader.o QMapUtils.o 
	$(CC) $(CADD) QDump.o QMapReader.o QMapHeader.o QMapUtils.o -o QDump \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR	

QKillNan.o: QKillNan.cpp  QMapHeader.h QMapReader.h QMapReader.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QKillNan: QKillNan.o QMapReader.o QMapHeader.o QMapUtils.o 
	$(CC) $(CADD) QKillNan.o QMapReader.o QMapHeader.o QMapUtils.o -o QKillNan \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR	


QStat.o: QStat.cpp QMapHeader.h ValReader.h QMapReader.h QMapReader.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QStat:QStat.o QMapReader.o QMapHeader.o QMapUtils.o 
	$(CC) $(CADD) QStat.o QMapReader.o QMapHeader.o QMapUtils.o -o QStat \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR	

QGrowthTest.o: QGrowthTest.cpp QMapHeader.h ValReader.h QMapReader.h QMapReader.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QGrowthTest:QGrowthTest.o QMapReader.o QMapHeader.o QMapUtils.o 
	$(CC) $(CADD) QGrowthTest.o QMapReader.o QMapHeader.o QMapUtils.o -o QGrowthTest \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR	


QConverter.o: QConverter.cpp QConverter.h QMapReader.h QMapReader.cpp QMapUtils.h QMapUtils_T.cpp QMapHeader.h
	$(CC) $(COPT) $(DEFS)


QConvert.o: QConvert.cpp QConverter.h
	$(CC) $(COPT) $(DEFS)

QConvert:QConvert.o QConverter.o QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o 
	$(CC) $(CADD) QConvert.o QConverter.o QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o -o QConvert \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR 


QScale.o: QScale.cpp QMapReader.h QMapReader.cpp QMapUtils.h QMapUtils_T.cpp QMapHeader.h
	$(CC) $(COPT) $(DEFS)

QScale: QScale.o  QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o 
	$(CC)  $(CADD) -o QScale \
        QScale.o QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR 

QRaw.o: QRaw.cpp QMapReader.h QMapReader.cpp QMapUtils.h QMapUtils_T.cpp QMapHeader.h
	$(CC) $(COPT) $(DEFS)

QRaw: QRaw.o  QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o 
	$(CC)  $(CADD) -o QRaw \
        QRaw.o QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR 

QMapDEM.o: QMapDEM.cpp QMapDEM.h ValReader.h QMapUtils.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)

QMask.o: QMask.cpp QMapHeader.h QMapReader.h QMapReader.cpp QMapUtils.h
	$(CC) $(COPT) $(DEFS)

QMask:QMask.o QMapReader.o QMapHeader.o QMapUtils.o 
	$(CC) $(CADD) QMask.o QMapReader.o QMapHeader.o QMapUtils.o -o QMask \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR	

QGradientFinder.o: QGradientFinder.cpp QGradientFinder.h
	$(CC) $(COPT) $(DEFS)

QGrad.o: QGrad.cpp QGradientFinder.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)

QGrad: QGrad.o QGradientFinder.o \
	QMapUtils.o QMapReader.o QMapHeader.o 
	$(CC)  $(CADD) -o QGrad \
	QGrad.o QGradientFinder.o \
	QMapUtils.o QMapReader.o QMapHeader.o \
        -L $(GEOINFO) -lGeoInfo \
	-L $(COMMON) -lCommon \
        -lz 


#	-L $(EXTERNAL) -lNR	\


QPlot.o: QPlot.cpp QConverter.h QMapUtils.h ValReader.h QMapReader.h QMapHeader.h
	$(CC) $(COPT) $(DEFS)

QPlot: QPlot.o QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o QConverter.o
	$(CC) $(CADD) -o QPlot QPlot.o QMapHeader.o QMapReader.o \
	QMapUtils.o QMapUtils_T.o QConverter.o \
	-L $(COMMON) -lCommon \
	-lz

#	-L $(EXTERNAL) -lNR \

pathlen.o: pathlen.cpp
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)

pathlen: pathlen.o 
	$(CC)  $(CADD) -o pathlen pathlen.o \
        -L $(GEOINFO) -lGeoInfo \
	-L $(COMMON) -lCommon \
	-lz 

QProj.o: QProj.cpp QMapUtils.h ValReader.h QMapReader.h QMapHeader.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)

QProj: QProj.o QMapHeader.o QMapReader.o QMapUtils.o QMapUtils_T.o 
	$(CC) $(CADD) -o QProj QProj.o QMapHeader.o QMapReader.o \
	QMapUtils.o QMapUtils_T.o \
        -L $(GEOINFO) -lGeoInfo \
	-L $(COMMON) -lCommon \
	-lz

#	-L $(EXTERNAL) -lNR \

QGen.o: QGen.cpp QMapUtils.h QMapReader.h QMapHeader.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)

QGen: QGen.o QMapHeader.o QMapUtils.o QMapUtils_T.o 
	$(CC) $(CADD) -o QGen QGen.o QMapHeader.o QMapReader.o \
	QMapUtils.o QMapUtils_T.o \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR 

QRoll.o: QRoll.cpp QMapUtils.h QMapReader.h QMapHeader.h
	$(CC) $(COPT) $(DEFS) 

QRoll: QRoll.o QMapHeader.o QMapUtils.o QMapReader.o 
	$(CC) $(CADD) -o QRoll QRoll.o QMapHeader.o QMapReader.o \
	QMapUtils.o  \
	-L $(COMMON) -lCommon 
#	-L $(EXTERNAL) -lNR 

QTranslate.o: QTranslate.cpp QMapUtils.h QMapReader.h QMapHeader.h
	$(CC) $(COPT) $(DEFS) 

QTranslate: QTranslate.o QMapHeader.o QMapUtils.o QMapReader.o 
	$(CC) $(CADD) -o QTranslate QTranslate.o QMapHeader.o QMapReader.o \
	QMapUtils.o  \
	-L $(COMMON) -lCommon \
        -lz

#	-L $(EXTERNAL) -lNR \

QMapNPP.o: QMapNPP.cpp QMapUtils.h QMapReader.h QMapHeader.h
	$(CC) $(COPT) $(DEFS) 

QMapNPP: QMapNPP.o QMapHeader.o QMapUtils.o QMapReader.o 
	$(CC) $(CADD) -o QMapNPP QMapNPP.o QMapHeader.o QMapReader.o \
	QMapUtils.o  \
	-L $(COMMON) -lCommon \
        -lz

#	-L $(EXTERNAL) -lNR \

div: div.cpp
	$(CC) $(CADD) -o div div.cpp

translations.o: translations.cpp translations.h
	$(CC) $(COPT) $(DEFS)

PNG2QMap.o: PNG2QMap.cpp translations.h QMapUtils.h QMapHeader.h
	$(CC) $(COPT) $(DEFS) -I$(VISUAL)

PNG2QMap: PNG2QMap.o QMapHeader.o QMapUtils.o translations.o 
	$(CC) $(CADD) -o PNG2QMap \
         PNG2QMap.o QMapUtils.o QMapHeader.o translations.o \
        $(COMMON)/Interpolator.o $(COMMON)/BinSearch.o \
        $(COMMON)/LineReader.o $(COMMON)/strutils.o \
        $(VISUAL)/PNGImage.o \
        -lpng -lz

#        -L $(EXTERNAL) -lNR \

NC2QMap.o: NC2QMap.cpp QMapUtils.h QMapHeader.h
	$(CC) $(COPT) $(DEFS) -I$(VISUAL)

NC2QMap: NC2QMap.o QMapHeader.o QMapUtils.o
	$(CC) $(CADD) -o NC2QMap \
	NC2QMap.o QMapUtils.o QMapHeader.o QMapReader.o \
        -L $(COMMON) -lCommon \
	-lz 

#        -L $(EXTERNAL) -lNR \

