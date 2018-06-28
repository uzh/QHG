
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifndef CFLAGS
  CFLAGS=-g -Wall
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif


UTILS=~/utils
COMMON=../../common
QMAPS=../../qmaps
EXTERNAL=../../external
GEOINFO=../../geoinfo
HEADER=../../header
#TILING=../../tiling
#EXTERNAL=../../external
ICOSA=../../icosa
IO=../../io
VISUAL=../../visual

COMP=-fopenmp
DOMP=-DOMP

ifdef OPT
  CFLAGS=-O3 -Wall
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

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(CPROF) -I$(COMMON) -std=c++0x

CORE=GridZones.o IcoSpiral.o \
     Region.o FullRegion.o RectRegion.o GridRegion.o  LonLatRegion.o IrregRegion.o \
     RegionSplitter.o TrivialSplitter.o RectSplitter.o LonLatSplitter.o 



all: libOldIcosa.a 



lib: libOldIcosa.a


new: clean all

clean: 
	rm -f *.o $(CORE)

libOldIcosa.a: $(CORE)
	ar $(ARFL) $@ $?




GridZones.o: GridZones.cpp GridZones.h Region.h IcoSpiral.h  
	$(CC) $(COPT) $(DEFS) -I $(GEOINFO) -I$(ICOSA)

IcoSpiral.o: IcoSpiral.cpp IcoSpiral.h 
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)


RectSplitter.o: RectSplitter.cpp RectSplitter.h RegionSplitter.h Region.h RectRegion.h FullRegion.h 
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

RegionSplitter.o: RegionSplitter.cpp RegionSplitter.h 
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

TrivialSplitter.o: TrivialSplitter.cpp TrivialSplitter.h RegionSplitter.h FullRegion.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

LonLatSplitter.o: LonLatSplitter.cpp LonLatSplitter.h RegionSplitter.h Region.h FullRegion.h LonLatRegion.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)


RectRegion.o: RectRegion.cpp RectRegion.h Region.h 
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO) -I$(ICOSA)

LonLatRegion.o: LonLatRegion.cpp LonLatRegion.h Region.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

FullRegion.o: FullRegion.cpp FullRegion.h Region.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)
 
GridRegion.o: GridRegion.cpp Region.h 
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

IrregRegion.o: IrregRegion.cpp IrregRegion.h 
	$(CC) $(COPT) $(DEFS) $(COMP) -I$(ICOSA)


#####
# some general stuff
#####
interquat.o: interquat.cpp interquat.h trackball.h
	$(CC) $(COPT) $(DEFS)  -I$(HEADER)

trackball.o: trackball.c trackball.h 
	g++ -c -c trackball.c -o trackball.o

VRColorizer.o: VRColorizer.cpp VRColorizer.h IcoColorizer.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(QMAPS) -I$(VISUAL)


SnapColorizer.o: SnapColorizer.cpp SnapColorizer.h IQColorizer.h ProjInfo.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(QMAPS) -I$(VISUAL)

NodeLister.o: NodeLister.cpp NodeLister.h 
	$(CC) $(COPT) $(DEFS)  -I$(HEADER)

IcoConverter.o: IcoConverter.cpp IcoConverter.h
	$(CC) $(COPT) $(DEFS) -I $(HEADER) -I $(GEOINFO)

#####
# IGN stuff
#####
IGNWriteTest.o:IGNWriteTest.cpp IcoGridNodes.h IcoGridCreator.h TrivialSplitter.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(HEADER) -I$(VISUAL)

IGNWriteTest: IGNWriteTest.o IcoGridCreator.o IcoGridNodes.o TrivialSplitter.o FullRegion.o RegionSplitter.o Region.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o 
	$(CC) $(CADD) $(COMP) -o IGNWriteTest IGNWriteTest.o \
	IcoGridCreator.o IcoGridNodes.o TrivialSplitter.o FullRegion.o RegionSplitter.o Region.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o \
	 $(COMMON)/Vec3D.o $(COMMON)/Quat.o $(COMMON)/BufReader.o  $(COMMON)/strutils.o $(COMMON)/geomutils.o \
         $(COMMON)/BufWriter.o 

IGNDisplay.o:IGNDisplay.cpp IcoGridNodes.h
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(HEADER) -I$(VISUAL)

IGNDisplay: IGNDisplay.o IcoGridNodes.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o 
	$(CC) $(CADD) $(COMP) -o IGNDisplay IGNDisplay.o \
	IcoGridNodes.o \
	IcoNode.o IcoHeader.o VertexLinkage.o Icosahedron.o \
	GridCreator.o GridZones.o IcoFace.o QuadFace.o IcoSpiral.o symbuf.o \
	 $(COMMON)/Vec3D.o $(COMMON)/Quat.o $(COMMON)/BufReader.o  $(COMMON)/strutils.o $(COMMON)/geomutils.o \
         $(COMMON)/BufWriter.o 

#####
# Ico2PLY
#####
Ico2PLY.o: Ico2PLY.cpp 	
	$(CC) $(COPT) $(CADD) $(DEFS)  -I$(HEADER)  -I$(EXTERNAL) -I$(VISUAL)

Ico2PLY: Ico2PLY.o Icosahedron.o IcoFace.o QuadFace.o IcoHeader.o	\
        VertexLinkage.o symbuf.o NodeLister.o 
	$(CC) $(CADD) $(COMP) -o Ico2PLY Ico2PLY.o \
	$(COMMON)/strutils.o $(COMMON)/ParamReader.o \
	$(COMMON)/LineReader.o $(COMMON)/DescReader.o \
        $(COMMON)/Vec3D.o $(COMMON)/BufWriter.o $(COMMON)/BufReader.o \
	$(HEADER)/HeaderBase.o 	$(HEADER)/SnapHeader.o	$(HEADER)/PopHeader.o \
	Icosahedron.o IcoFace.o QuadFace.o IcoHeader.o	\
        VertexLinkage.o symbuf.o NodeLister.o \
	$(VISUAL)/LookUp.o \
	$(EXTERNAL)/plyfile.o \
        -L $(VISUAL) -lVisual -lz	

#####
# IQApp UI elements
#####
SnapColorizer.o: SnapColorizer.cpp SnapColorizer.h
	$(CC) $(COPT) $(DEFS) -I$(VISUAL)

ProjInfo.o: ProjInfo.cpp ProjInfo.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO) -I$(VISUAL) -I$(ICOSA) -I$(QMAPS) -I$(HEADER) 

QDFValueProvider.o: QDFValueProvider.cpp QDFValueProvider.h ValueProvider.h
	$(CC) $(COPT) $(DEFS) -I$(IO)  -I$(ICOSA) 

SnapValueProvider.o: SnapValueProvider.cpp SnapValueProvider.h ValueProvider.h
	$(CC) $(COPT) $(DEFS) -I$(HEADER)  -I$(ICOSA) 

QDFArraySniffer.o: QDFArraySniffer.cpp 
	$(CC) $(COPT) $(DEFS) -I$(IO)

QDFNodes.o: QDFNodes.cpp QDFNodes.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

SurfaceManager.o: SurfaceManager.cpp SurfaceManager.h
	$(CC) $(COPT) $(DEFS) -I$(IO)  -I$(ICOSA) 

IQSurface_OGL.o: IQSurface_OGL.cpp IQSurface_OGL.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) 

IQIco_OGL.o: IQIco_OGL.cpp IQIco_OGL.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) 

IQFlat_OGL.o: IQFlat_OGL.cpp IQFlat_OGL.h IQSurface_OGL.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)  -I$(GEOINFO)


IQEQIco_OGL.o: IQEQIco_OGL.cpp IQEQIco_OGL.h IQSurface_OGL.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) 

IQWin.o: IQWin.cpp IQWin.h IQScene.h \
         ProjInfo.h notification_codes.h init.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO) -I$(VISUAL) -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQScene.o: IQScene.cpp IQScene.h  IQView.h IQModel.h \
         ProjInfo.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQModel.o: IQModel.cpp IQModel.h IQView.h trackball.h SurfaceManager.h NodeLister.h \
         ProjInfo.h notification_codes.h init.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS) -I$(VISUAL) -I$(HEADER)  -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQView.o: IQView.cpp IQView.h IQScene.h trackball.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQDataPanel.o: IQDataPanel.cpp IQDataPanel.h IQModel.h SurfaceManager.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(VISUAL) -I$(ICOSA)  \
        `pkg-config --cflags gtkglextmm-1.2` 

IQLookUpPanel.o: IQLookUpPanel.cpp IQLookUpPanel.h \
         ProjInfo.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(VISUAL)  -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQProjPanel.o: IQProjPanel.cpp IQProjPanel.h init.h \
         IQImageViewer.h ProjInfo.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)  -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQGradientPanel.o: IQGradientPanel.cpp IQGradientPanel.h init.h \
         IQImageViewer.h ProjInfo.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(HEADER)  -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQAniPanel.o: IQAniPanel.cpp IQAniPanel.h IQAnimator.h \
         notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQQDFPanel.o: IQQDFPanel.cpp IQQDFPanel.h 
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQEffectPanel.o: IQEffectPanel.cpp IQEffectPanel.h 
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQOverlay.o: IQOverlay.cpp IQOverlay.h 
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)  -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQAnimator.o: IQAnimator.cpp IQAnimator.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO) -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQImageViewer.o: IQImageViewer.cpp IQImageViewer.h \
         ProjInfo.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO) -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 

IQGradientFinder.o: IQGradientFinder.cpp IQGradientFinder.h \
         ProjInfo.h notification_codes.h
	$(CC) $(COPT) $(DEFS) -I$(HEADER) -I$(ICOSA)

IQApp.o: IQApp.cpp IQWin.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA) \
        `pkg-config --cflags gtkglextmm-1.2` 









IQApp: IQApp.o IQWin.o IQScene.o IQView.o IQModel.o trackball.o IQOverlay.o \
        IQDataPanel.o IQLookUpPanel.o IQProjPanel.o ProjInfo.o IQImageViewer.o \
        IQGradientPanel.o IQGradientFinder.o \
        IQAniPanel.o IQEffectPanel.o IQQDFPanel.o \
        IQAnimator.o interquat.o \
	QDFValueProvider.o SnapValueProvider.o \
        SurfaceManager.o IQSurface_OGL.o IQIco_OGL.o IQFlat_OGL.o IQEQIco_OGL.o SnapColorizer.o \
	QDFArraySniffer.o QDFNodes.o 
	$(CC) -o IQApp $(CADD) $(COMP) $(CPROF) \
        IQApp.o IQWin.o IQScene.o IQView.o IQModel.o trackball.o  IQOverlay.o \
        IQDataPanel.o IQLookUpPanel.o IQProjPanel.o ProjInfo.o IQImageViewer.o \
        IQGradientPanel.o IQGradientFinder.o \
        IQAniPanel.o IQEffectPanel.o IQQDFPanel.o \
        IQAnimator.o interquat.o \
	QDFValueProvider.o SnapValueProvider.o \
        SurfaceManager.o IQSurface_OGL.o IQIco_OGL.o IQFlat_OGL.o IQEQIco_OGL.o SnapColorizer.o  \
	QDFArraySniffer.o QDFNodes.o \
        -L $(ICOSA) -lIcosa \
        -L $(IO) -lIO \
        -L $(VISUAL) -lVisual \
        -L $(QMAPS) -lQMap \
        -L $(HEADER) -lHeader \
        -L $(GEOINFO) -lGeoInfo  \
        -L $(COMMON) -lCommon \
        -lz -lhdf5 \
        `pkg-config --libs gtkglextmm-1.2`

###
# brought over from header
###

PopHeader.o: PopHeader.cpp PopHeader.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)

PopReader.o: PopReader.cpp PopReader.h PopHeader.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)

PopLoader.o: PopLoader.cpp PopLoader.h PopHeader.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)


HeaderBase.o: HeaderBase.cpp HeaderBase.h
	$(CC) $(COPT) $(DEFS) -g  -I$(ICOSA) 

SnapHeader.o: SnapHeader.cpp SnapHeader.h HeaderBase.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) 

WorldHeader.o: WorldHeader.cpp WorldHeader.h HeaderBase.h
	$(CC) $(COPT) $(DEFS) -g  

DescReader.o: DescReader.cpp  DescReader.h
	$(CC) $(COPT) $(DEFS)





