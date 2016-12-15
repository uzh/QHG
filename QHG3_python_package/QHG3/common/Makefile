
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif


ifdef MUTE
  MUTEFLAG=-D MUTE
else
  MUTEFLAG=
endif


#ifndef CFLAGS
  CFLAGS=-g -Wall 
#endif

ifdef OPT
  CFLAGS=-O3 -Wall
endif

ifdef PROF
  CFLAGS=-pg -Wall
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif	

EXTERNAL=../external
VORONOI=../voronoi

COPT=-c $< -o $@ $(CADD) $(CFLAGS)

ARFL=rv

CONFIGURE:
	../app/configure.sh


lib: libCommon.a

all: common

common: libCommon.a


libCommon.a: ids.o utils.o strutils.o geomutils.o calcutils.o PolyLine.o \
      LineReader.o DescReader.o \
      Pattern.o FileList.o  \
      BinSearch.o Interpolator.o \
      Quat.o \
      Vec3D.o \
      smallprimes.o \
      MessLogger.o ParamReader.o dbgprint.o \
      BufWriter.o BufReader.o \
      Observable.o \
      LayerBuf.o LayerArrBuf.o L2List.o LBController.o \
      SeqInterval.o Triggers.o EventData.o EventManager.o \
      WELL512.o xrand.o BinomialDist.o \
      ConvKernel.o SimpleMapper.o crypto.o \
      SystemInfo.o \
      UnitConverter.o \
      GeneUtils.o \
      ArrayShare.o
	ar $(ARFL) $@ $?



clean:
	rm -f *.o libCommon.a *.ti

test: BSTest conftest FLTest FMTest GeomTest gztest IntPTest IQTest PLTest SFTest TSTest vt xrandtest MBTest

bad: FSTest

apps: bindouble gridint isaac_rand SpeciesFinder Spreader 

xrand.o: xrand.cpp  xrand.h xrandtab.h
	$(CC) $(COPT) $(DEFS)

utils.o: utils.cpp  utils.h 
	$(CC) $(COPT) $(DEFS)

strutils.o: strutils.cpp  strutils.h 
	$(CC) $(COPT) $(DEFS) -fPIC

fileutils.o: fileutils.cpp  fileutils.h utils.h
	$(CC) $(COPT) $(DEFS)

geomutils.o: geomutils.cpp  geomutils.h utils.h
	$(CC) $(COPT) $(DEFS)

calcutils.o: calcutils.cpp  calcutils.h PolyLine.h
	$(CC) $(COPT) $(DEFS)

UnitConverter.o: UnitConverter.cpp UnitConverter.h
	$(CC) $(COPT) $(DEFS)

LineReader.o: LineReader.cpp  LineReader.h  utils.h
	$(CC) $(COPT) $(DEFS)

DescReader.o: DescReader.cpp  DescReader.h
	$(CC) $(COPT) $(DEFS)

Vec3D.o: Vec3D.cpp  Vec3D.h
	$(CC) $(COPT) $(DEFS)

Quat.o: Quat.cpp  Quat.h Vec3D.h
	$(CC) $(COPT) $(DEFS)

ids.o: ids.cpp  ids.h CONFIGURE
	$(CC) $(COPT) $(DEFS)

ParamReader.o: ParamReader.cpp ParamReader.h
	$(CC) $(COPT) $(DEFS) -fPIC

Pattern.o: Pattern.cpp Pattern.h
	$(CC) $(COPT) $(DEFS)

FileList.o: FileList.cpp FileList.h Pattern.h
	$(CC) $(COPT) $(DEFS)

PolyLine.o: PolyLine.cpp  PolyLine.h 
	$(CC) $(COPT) $(DEFS)

SimpleMapper.o: SimpleMapper.cpp  SimpleMapper.h ValueMapper.h 
	$(CC) $(COPT) $(DEFS)

IdentityMapper.o: IdentityMapper.cpp IdentityMapper.h ValueMapper.h 
	$(CC) $(COPT) $(DEFS)

FPointComp.o: FPointComp.cpp FPointComp.h 
	$(CC) $(COPT) $(DEFS)

MessLogger.o: MessLogger.cpp  MessLogger.h
	$(CC) $(COPT) $(MUTEFLAG) $(DEFS) -g 

MessLogTest.o: MessLogTest.cpp  MessLogger.h
	$(CC) $(COPT) $(DEFS) -g

MessLogTest: MessLogTest.o MessLogger.o
	g++ MessLogTest.o MessLogger.o -o MessLogTest


dbgprint.o: dbgprint.cpp  dbgprint.h
	$(CC) $(COPT) $(MUTEFLAG) $(DEFS) -g 

BSTest.o: BSTest.cpp BinSearch.h 
	$(CC) $(COPT) $(DEFS)

BSTest: BSTest.o BinSearch.o 
	g++ BSTest.o BinSearch.o  -o BSTest

version.o: version.cpp version.h 
	$(CC) $(COPT) $(DEFS)

isaac_rand.o: isaac_rand.cpp isaac_rand.h isaac_standard.h
	$(CC) $(COPT) $(DEFS)

WELL512.o: WELL512.cpp WELL512.h 
	$(CC) $(COPT) $(DEFS)

LWELL512.o: LWELL512.cpp LWELL512.h 
	$(CC) $(COPT) $(DEFS)

WELLTest.o: WELLTest.cpp WELL512.h 
	$(CC) $(COPT) $(DEFS) -fopenmp

WELLTest: WELLTest.o WELL512.o 
	g++ WELLTest.o WELL512.o  \
          -o WELLTest -fopenmp

SystemInfo.o: SystemInfo.cpp SystemInfo.h
	$(CC) $(COPT) $(DEFS)

SysInfoTest.o: SysInfoTest.cpp SystemInfo.h
	$(CC) $(COPT) $(DEFS)

SysInfoTest:SystemInfo.o SysInfoTest.o
	g++ SystemInfo.o SysInfoTest.o  \
          -o SysInfoTest

BinSearch.o: BinSearch.cpp BinSearch.h 
	$(CC) $(COPT) $(DEFS)

Interpolator.o: Interpolator.cpp Interpolator.h Interpolator_nr.h BinSearch.h
	$(CC) $(COPT) $(DEFS) -I $(EXTERNAL)

MapInterpolator.o: MapInterpolator.cpp MapInterpolator.h Interpolator.h Interpolator_nr.h  BinSearch.h
	$(CC) $(COPT) $(DEFS) -I $(EXTERNAL)

IntPTest.o: IntPTest.cpp Interpolator.h
	$(CC) $(COPT) $(DEFS)


IntPTest: IntPTest.o Interpolator.o BinSearch.o \
          $(EXTERNAL)/libNR.a
	g++ IntPTest.o Interpolator.o BinSearch.o \
	  -L $(EXTERNAL) -lNR \
          -o IntPTest

ZipTest.o: ZipTest.cpp Zipper.h
	$(CC) $(COPT) $(DEFS)

ZipTest: ZipTest.o Zipper.o
	g++ ZipTest.o Zipper.o -lz -o ZipTest

gztest.o: gztest.cpp 
	$(CC) $(COPT) $(DEFS)

gztest: gztest.o 
	g++ gztest.o -lz -o gztest

GeomTest.o: GeomTest.cpp geomutils.h Vec3D.h Quat.h
	$(CC) $(COPT) $(DEFS)

GeomTest: GeomTest.o geomutils.o Vec3D.o Quat.o
	g++ GeomTest.o geomutils.o Vec3D.o Quat.o -o GeomTest

IQTest.o: IQTest.cpp IndexedQueue.h
	$(CC) $(COPT) $(DEFS)

IQTest: IQTest.o IndexedQueue.o
	g++ IQTest.o IndexedQueue.o -o IQTest

findspecies.o: findspecies.cpp FileList.h fileutils.h utils.h ids.h 
	$(CC) $(COPT) $(DEFS)

SpeciesFinder.o: SpeciesFinder.cpp findspecies.h
	$(CC) $(COPT) $(DEFS)

SpeciesFinder: SpeciesFinder.o findspecies.o FileList.o Pattern.o LineReader.o fileutils.o version.o strutils.o utils.o ids.o
	g++ SpeciesFinder.o findspecies.o FileList.o Pattern.o LineReader.o fileutils.o utils.o ids.o strutils.o version.o -lz -o SpeciesFinder

conftest.o: conftest.cpp ParamReader.h ParamReader.o
	$(CC) $(COPT) $(DEFS)

conftest: conftest.o 
	g++ conftest.o ParamReader.o -o conftest


BinomialDist.o: BinomialDist.cpp BinomialDist.h
	$(CC) $(COPT) $(DEFS) -I$(EXTERNAL)

BinoDistTest.o: BinoDistTest.cpp BinomialDist.h
	$(CC) $(COPT) $(DEFS)

BinoDistTest: BinoDistTest.o BinomialDist.o
	g++ BinoDistTest.o BinomialDist.o -L $(EXTERNAL) -lNR -o BinoDistTest

PLTest.o: PLTest.cpp PolyLine.h utils.h
	$(CC) $(COPT) $(DEFS)

PLTest: PLTest.o PolyLine.o utils.o
	g++ PLTest.o PolyLine.o utils.o -lz -o PLTest

SeasonProvider.o: SeasonProvider.h
	$(CC) $(COPT) $(DEFS)


ConvKernel.o: ConvKernel.cpp ConvKernel.h
	$(CC) $(COPT) $(DEFS)

SeqInterval.o: SeqInterval.cpp SeqInterval.h utils.h types.h
	$(CC) $(COPT) $(DEFS)

Triggers.o: Triggers.cpp Triggers.h utils.h types.h
	$(CC) $(COPT) $(DEFS)

EventData.o: EventData.cpp EventData.h strutils.h
	$(CC) $(COPT) $(DEFS)

EventManager.o: EventManager.cpp EventManager.h EventData.h Triggers.h strutils.h
	$(CC) $(COPT) $(DEFS)

smallprimes.o: smallprimes.cpp smallprimes.h 
	$(CC) $(COPT) $(DEFS)

crypto.o: crypto.cpp crypto.h 
	$(CC) $(COPT) $(DEFS)

BufWriter.o: BufWriter.cpp BufWriter.h 	
	$(CC) $(COPT) $(DEFS)

BWTest.o: BWTest.cpp BufWriter.h
	$(CC) $(COPT) $(DEFS)

BWTest: BWTest.o BufWriter.o
	g++ BWTest.o BufWriter.o -o BWTest

BufReader.o: BufReader.cpp BufReader.h 	
	$(CC) $(COPT) $(DEFS)


Observable.o: Observable.cpp Observable.h Observer.h 	
	$(CC) $(COPT) $(DEFS)


LayerBuf.o: LayerBuf.cpp LayerBuf.h LBBase.h 	
	$(CC) $(COPT) $(DEFS)

LayerArrBuf.o: LayerArrBuf.cpp LayerArrBuf.h LayerBuf.h LayerBuf.cpp LBBase.h 	
	$(CC) $(COPT) $(DEFS)

SMemBlocks.o: SMemBlocks.cpp SMemBlocks.h 	
	$(CC) $(COPT) $(DEFS)

LBController.o: LBController.cpp LBController.h LayerBuf.h LBBase.h
	$(CC) $(COPT) $(DEFS)

LBTest.o: LBTest.cpp L2List.h LayerBuf.h LayerBuf.cpp LBController.h
	$(CC) $(COPT) $(DEFS)

LBTest: LBTest.o libCommon.a 
	g++ LBTest.o libCommon.a -o LBTest


LABTest: LABTest.o libCommon.a 
	g++ LABTest.o libCommon.a -o LABTest

LABTest.o: LABTest.cpp L2List.h LayerArrBuf.h LayerBuf.h LayerBuf.cpp LBController.h
	$(CC) $(COPT) $(DEFS)



L2List.o: L2List.cpp L2List.h 
	$(CC) $(COPT) $(DEFS)

L2LTest.o: L2LTest.cpp L2List.h
	$(CC) $(COPT) $(DEFS)

L2LTest: L2LTest.o L2List.o
	g++ L2LTest.o L2List.o -o L2LTest

GeneUtils.o: GeneUtils.cpp GeneUtils.h
	$(CC) $(COPT) $(DEFS) 

GUTest.o: GUTest.cpp GeneUtils.h
	$(CC) $(COPT) $(DEFS)

GUTest: GUTest.o GeneUtils.o
	$(CC) $(COMP) -o GUTest \
	GUTest.o GeneUtils.o


ArrayShare.o: ArrayShare.cpp ArrayShare.h
	$(CC) $(COPT) $(DEFS) 

ASTest.o: ASTest.cpp ArrayShare.h
	$(CC) $(COPT) $(DEFS)

ASTest: ASTest.o ArrayShare.o
	$(CC) $(COMP) -o ASTest \
	ASTest.o ArrayShare.o


