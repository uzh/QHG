
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


ifndef WARNINGS
  WARNINGS=-Wall -Wshadow 
endif

ifndef CFLAGS
  CFLAGS=-g $(WARNINGS) 
endif

ifdef OPT
  CFLAGS=-O3 $(WARNINGS)
endif

ifdef PROF
  FLAGS=-pg $(WARNINGS)
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif	

ifdef DYN
  CFLAGS:=$(CFLAGS) -rdynamic
  DYN=-DDYN
endif	

VORONOI=../voronoi

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(DYN) -fPIC

ARFL=rv

CONFIGURE:
	../app/configure.sh


lib: libCommon.a

all: common

common: libCommon.a


libCommon.a: ids.o utils.o strutils.o stdstrutils.o geomutils.o calcutils.o PolyLine.o \
      LineReader.o \
      Pattern.o FileList.o  \
      Quat.o \
      Vec3D.o \
      smallprimes.o \
      MessLogger.o ParamReader.o dbgprint.o \
      BufWriter.o BufReader.o \
      Observable.o \
      LayerBuf.o LayerArrBuf.o L2List.o LBController.o \
      SeqInterval.o Triggers.o EventData.o EventManager.o \
      WELL512.o WELLUtils.o bino_tools.o BinomialDist.o Permutator.o \
      SimpleMapper.o crypto.o crc.o \
      SystemInfo.o \
      UnitConverter.o \
      ArrayShare.o \
      RankTable.o
	ar $(ARFL) $@ $?



clean:
	rm -f *.o libCommon.a *.ti

test: ASTest BinoDistTest L2LTest LABTest LBTest SysInfoTest  WELLTest



utils.o: utils.cpp  utils.h 
	$(CC) $(COPT) $(DEFS)

strutils.o: strutils.cpp  strutils.h 
	$(CC) $(COPT) $(DEFS) -fPIC

stdstrutils.o: stdstrutils.cpp  stdstrutils.h 
	$(CC) $(COPT) $(DEFS) -fPIC	


geomutils.o: geomutils.cpp  geomutils.h utils.h
	$(CC) $(COPT) $(DEFS)

calcutils.o: calcutils.cpp  calcutils.h PolyLine.h
	$(CC) $(COPT) $(DEFS)

UnitConverter.o: UnitConverter.cpp UnitConverter.h
	$(CC) $(COPT) $(DEFS)

LineReader.o: LineReader.cpp  LineReader.h  utils.h
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

MessLogger.o: MessLogger.cpp  MessLogger.h
	$(CC) $(COPT) $(MUTEFLAG) $(DEFS) -g 


dbgprint.o: dbgprint.cpp  dbgprint.h
	$(CC) $(COPT) $(MUTEFLAG) $(DEFS) -g 

WELL512.o: WELL512.cpp WELL512.h 
	$(CC) $(COPT) $(DEFS) -O3

WELLUtils.o: WELLUtils.cpp WELLUtils.h WELL512.h 
	$(CC) $(COPT) $(DEFS) -O3 

WELLTest.o: WELLTest.cpp WELL512.h 
	$(CC) $(COPT) $(DEFS) -O3 -fopenmp

WELLTest: WELLTest.o WELL512.o 
	g++ WELLTest.o WELL512.o  \
          -o WELLTest -fopenmp

SystemInfo.o: SystemInfo.cpp SystemInfo.h
	$(CC) $(COPT) $(DEFS)

SysInfoTest.o: SysInfoTest.cpp SystemInfo.h
	$(CC) $(COPT) $(DEFS)

SysInfoTest:SystemInfo.o SysInfoTest.o
	g++ SystemInfo.o SysInfoTest.o strutils.o \
          -o SysInfoTest

BinomialDist.o: BinomialDist.cpp BinomialDist.h bino_tools.h
	$(CC) $(COPT) $(DEFS)

BinoDistTest.o: BinoDistTest.cpp BinomialDist.h
	$(CC) $(COPT) $(DEFS)

bino_tools.o: bino_tools.cpp bino_tools.h
	$(CC) $(COPT) $(DEFS)

BinoDistTest: BinoDistTest.o BinomialDist.o
	g++ BinoDistTest.o BinomialDist.o bino_tools.o -o BinoDistTest

Permutator.o: Permutator.cpp Permutator.h
	$(CC) $(COPT) $(DEFS)

SeasonProvider.o: SeasonProvider.h
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

crc.o: crc.cpp crc.h 
	$(CC) $(COPT) $(DEFS)

BufWriter.o: BufWriter.cpp BufWriter.h 	
	$(CC) $(COPT) $(DEFS)


BufReader.o: BufReader.cpp BufReader.h 	
	$(CC) $(COPT) $(DEFS)


Observable.o: Observable.cpp Observable.h Observer.h 	
	$(CC) $(COPT) $(DEFS)


LayerBuf.o: LayerBuf.cpp LayerBuf.h LBBase.h 	
	$(CC) $(COPT) $(DEFS)

LayerArrBuf.o: LayerArrBuf.cpp LayerArrBuf.h LayerBuf.h LayerBuf.cpp LBBase.h 	
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

L2LTest: L2LTest.o L2List.o strutils.o
	g++ L2LTest.o L2List.o strutils.o -o L2LTest

ArrayShare.o: ArrayShare.cpp ArrayShare.h
	$(CC) $(COPT) $(DEFS) 

ASTest.o: ASTest.cpp ArrayShare.h
	$(CC) $(COPT) $(DEFS)

ASTest: ASTest.o ArrayShare.o
	$(CC) $(COMP) -o ASTest \
	ASTest.o ArrayShare.o Pattern.o


RankTable.o: RankTable.cpp RankTable.h
	$(CC) $(COPT) $(DEFS) 

RTTest.o: RTTest.cpp RankTable.h
	$(CC) $(COPT) $(DEFS) 

RTTest: RTTest.o RankTable.o
	$(CC) $(COMP) -o RTTest \
	RTTest.o RankTable.o \
	-L$(COMMON) -lCommon 

