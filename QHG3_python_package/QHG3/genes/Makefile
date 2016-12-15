

ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifdef OMPP
  CC=kinst-ompp $(GCC)
  OMPPFLAGS=-I/home/simone/src/ompp-0.8.3/include/ -L/home/simone/src/ompp-0.8.3/lib/
endif

ifdef SCALASCA
  CC=/opt/scalasca/bin/scalasca -instrument $(GCC)
endif

ifndef ARFL
  ARFL=-rv
endif

ifndef CFLAGS
  CFLAGS=-g -Wall
endif

ifdef OPT
  CFLAGS=-O3 -Wall
endif

ifdef OPT
  CFLAGS2=-O3 -g -Wall
endif

ifdef PROF
  CFLAGS=-pg -Wall
endif

ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif

ifdef ANC4
  ANC=-DANC4
else
  ANC=
endif


DOMP=
FORCECLEAN=

COMMON=../common
GEOINFO=../geoinfo
TILING=../tiling
HEADER=../header
QMAPS=../qmaps
AGENTS=../agents
ICOSA=../icosa
IO=../io
KERNEL=../kernel
MODULAR=../modular
EXTERNAL=../external

VTK_I=/opt/visit/2.7.3/linux-x86_64/include/vtk/vtk-6.0
VTK_L=/opt/visit/2.7.3/linux-x86_64/lib

COMP=-fopenmp
COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(COMP) $(OMPPFLAGS)
COPT2=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS2) $(DOMP) $(COMP) $(OMPPFLAGS)

TARGETS=showAG  idsearch ancsortX DynGraphSamplerX QDF2Plink

KERNEL_LIB:
	$(MAKE) --directory=$(KERNEL) $(FORCECLEAN) $(LIBTARG)

MODULAR_LIB:
	$(MAKE) --directory=$(MODULAR) $(FORCECLEAN) $(LIBTARG)

COMMON_LIB:
	$(MAKE) --directory=$(COMMON) lib 

QMAPS_LIB:
	$(MAKE) --directory=$(QMAPS) lib

IO_LIB:
	$(MAKE) --directory=$(IO) lib

EXTERNAL_LIB:
	$(MAKE) --directory=$(EXTERNAL) 

ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

HEADER_LIB:
	$(MAKE) --directory=$(HEADER) 


LIBTARG=lib

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)


RWAncGraph.o: RWAncGraph.cpp RWAncGraph.h AncGraphBase.h AncestorNode.h
	$(CC) $(COPT) -g $(DEFS) 

AncAnalyzer.o: AncAnalyzer.cpp AncAnalyzer.h
	$(CC) $(COPT) $(DEFS)

FTTest.o: FTTest.cpp RWAncGraph.h
	$(CC) $(COPT) $(DEFS)  -I$(KERNEL) -I$(QMAPS) -I$(ICOSA) -I$(IO)

FTTest: FTTest.o RWAncGraph.o \
	KERNEL_LIB IO_LIB
	$(CC) $(COMP) $(CFLAGS)  -o FTTest \
	FTTest.o RWAncGraph.o \
	-L $(KERNEL) -lKernel \
	-L $(COMMON) -lCommon \
	-L $(IO) -lIO \
        -lz -lhdf5


AncGraphBase.o: AncGraphBase.cpp AncestorNode.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO)

AGFileMerger.o: AGFileMerger.cpp AGFileMerger.h AncestorNode.h RWAncGraph.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO)

AGOracle.o: AGOracle.cpp AGOracle.h AncestorNode.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO)

AGOTest.o:AGOTest.cpp AGOracle.h  AncestorNode.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO)

AGOTest: AGOTest.o AGOracle.o AncGraphBase.o RWAncGraph.o
	$(CC) $(COMP) -o AGOTest \
	AGOTest.o AGOracle.o AncGraphBase.o RWAncGraph.o \
	-L$(COMMON) -lCommon

AGWindow.o: AGWindow.cpp AGWindow.h AGOracle.h AncestorNode.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO)


GraphEvolverBase.o: GraphEvolverBase.cpp GraphEvolverBase.h AncGraphBase.h
	$(CC) $(COPT) $(DEFS) -I $(EXTERNAL)


GraphEvolverF.o: GraphEvolverF.cpp GraphEvolverF.h GraphEvolverBase.h  AncestorNode.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS)

IDSampler.o: IDSampler.cpp IDSampler.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS) -I $(IO)  

IDSampler2.o: IDSampler2.cpp IDSampler2.h IDSample.h
	$(CC) $(COPT) -g $(DEFS) -I $(IO)  


IDSample.o: IDSample.cpp IDSample.h
	$(CC) $(COPT) -g $(DEFS) -I $(IO)  


showAG.o: showAG.cpp RWAncGraph.h AncGraphBase.h AncestorNode.h
	$(CC) $(COPT) $(DEFS)


showAG: showAG.o RWAncGraph.o AncGraphBase.o
	$(CC) $(COMP) $(CFLAGS) -o showAG \
	showAG.o RWAncGraph.o AncGraphBase.o \
	-L $(COMMON)   -lCommon \
        -lz 

dynAG.o: dynAG.cpp RWAncGraph.h AncGraphBase.h AncestorNode.h AGOracle.h AGWindow.h
	$(CC) $(COPT) $(DEFS)


dynAG: dynAG.o RWAncGraph.o AncGraphBase.o AGOracle.o AGWindow.o
	$(CC) $(COMP) $(CFLAGS) -o dynAG \
	dynAG.o RWAncGraph.o AncGraphBase.o AGOracle.o AGWindow.o \
	-L $(COMMON)   -lCommon \
        -lz 

idsearch.o: idsearch.cpp 
	$(CC) $(COPT) $(DEFS)

idsearch: idsearch.o
	$(CC) $(COMP) $(CFLAGS) -o idsearch \
	idsearch.o \
	-L $(COMMON)   -lCommon \
 

DistMat.o:  DistMat.cpp DistMat.h
	$(CC) $(COPT) $(DEFS) -I${IO}

GeneDist.o:  GeneDist.cpp DistMat.h 
	$(CC) $(COPT) $(DEFS) -I${IO}

GeneDist: GeneDist.o DistMat.o
	$(CC) $(COMP) $(CFLAGS) -o GeneDist \
	GeneDist.o DistMat.o \
	-L $(COMMON)   -lCommon \
	-L $(IO)       -lIO \
        -lz  -lhdf5


AncFileBuffers.o: AncFileBuffers.cpp AncFileBuffers.h
	$(CC) $(COPT) $(DEFS) -fopenmp

AncMergeSorter.o: AncMergeSorter.cpp AncMergeSorter.h 
	$(CC) $(COPT) $(DEFS) -fopenmp

ancsort.o:ancsort.cpp AncFileBuffers.h
	$(CC) $(COPT) $(DEFS)

ancsort: ancsort.o AncFileBuffers.o
	$(CC) $(COMP) $(CFLAGS) -o ancsort \
	ancsort.o AncFileBuffers.o \
	-L $(COMMON)   -lCommon 

anc4sort.o:anc4sort.cpp AncFileBuffers.h
	$(CC) $(COPT) $(DEFS) -fopenmp

anc4sort: anc4sort.o AncFileBuffers.o
	$(CC) $(COMP) $(CFLAGS) -o anc4sort \
	anc4sort.o AncFileBuffers.o \
	-L $(COMMON)   -lCommon  -fopenmp

ancsortX.o:ancsortX.cpp AncFileBuffers.h
	$(CC) $(COPT) $(DEFS) -fopenmp

ancsortX: ancsortX.o AncFileBuffers.o AncMergeSorter.o
	$(CC) $(COMP) $(CFLAGS) -o ancsortX \
	ancsortX.o AncFileBuffers.o AncMergeSorter.o \
	-L $(COMMON)   -lCommon  -fopenmp

GeneWriter.o: GeneWriter.cpp GeneWriter.h IDSampler2.h
	$(CC) $(COPT) $(DEFS)

RGeneration.o: RGeneration.cpp RGeneration.h
	$(CC) $(COPT) $(DEFS)

DynAncGraph.o: DynAncGraph.cpp DynAncGraph.h AncGraphBase.h AncestorNode.h RGeneration.h
	$(CC) $(COPT) -g $(DEFS) -fopenmp

DynAnc4Graph.o: DynAnc4Graph.cpp DynAnc4Graph.h AncGraphBase.h AncestorNode.h RGeneration.h
	$(CC) $(COPT) -g $(DEFS) -fopenmp

DynAncGraphX.o: DynAncGraphX.cpp DynAncGraphX.h AncGraphBase.h AncestorNode.h RGeneration.h
	$(CC) $(COPT) -g $(DEFS) -fopenmp

DynGraphSampler4.o: DynGraphSampler4.cpp DynAnc4Graph.h AncGraphBase.h AncestorNode.h \
	GraphEvolverF.h IDSampler.h GeneWriter.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO) -fopenmp $(ANC)

DynGraphSampler3.o: DynGraphSampler3.cpp DynAncGraph.h AncGraphBase.h AncestorNode.h \
	GraphEvolverF.h IDSampler.h GeneWriter.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO) -fopenmp $(ANC)

DynGraphSamplerX.o: DynGraphSamplerX.cpp DynAncGraphX.h AncGraphBase.h AncestorNode.h \
	GraphEvolverF.h IDSampler2.h IDSample.h GeneWriter.h GenomeProvider.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO) -fopenmp $(ANC)

DynGraphSampler3: DynGraphSampler3.o AncGraphBase.o  DynAncGraph.o RWAncGraph.o RGeneration.o \
	GraphEvolverBase.o GraphEvolverF.o AGOracle.o IDSampler.o AGFileMerger.o GeneWriter.o
	$(CC) $(COMP) $(CFLAGS) -o DynGraphSampler3 \
	DynGraphSampler3.o AncGraphBase.o DynAncGraph.o RWAncGraph.o RGeneration.o \
	GraphEvolverBase.o GraphEvolverF.o AGOracle.o IDSampler.o AGFileMerger.o GeneWriter.o \
	-L $(COMMON) -lCommon \
	-L $(IO) -lIO \
	-L $(EXTERNAL) -lNR \
	-lhdf5 -lz -lcrypto

DynGraphSampler4: DynGraphSampler4.o AncGraphBase.o DynAnc4Graph.o RWAncGraph.o RGeneration.o \
	GraphEvolverBase.o GraphEvolverF.o AGOracle.o IDSampler.o AGFileMerger.o GeneWriter.o
	$(CC) $(COMP) $(CFLAGS) -o DynGraphSampler4 \
	DynGraphSampler.o AncGraphBase.o DynAnc4Graph.o RWAncGraph.o RGeneration.o \
	GraphEvolverBase.o GraphEvolverF.o AGOracle.o IDSampler.o AGFileMerger.o GeneWriter.o \
	-L $(COMMON) -lCommon \
	-L $(IO) -lIO \
	-L $(EXTERNAL) -lNR \
	-lhdf5 -lz -lcrypto

DynGraphSamplerX: DynGraphSamplerX.o AncGraphBase.o DynAncGraphX.o RWAncGraph.o RGeneration.o \
	GraphEvolverBase.o GraphEvolverF.o AGOracle.o IDSampler2.o IDSample.o AGFileMerger.o GeneWriter.o
	$(CC) $(COMP) $(CFLAGS) -o DynGraphSamplerX \
	DynGraphSamplerX.o AncGraphBase.o DynAncGraphX.o RWAncGraph.o RGeneration.o \
	GraphEvolverBase.o GraphEvolverF.o AGOracle.o IDSampler2.o IDSample.o AGFileMerger.o GeneWriter.o \
	-L $(COMMON) -lCommon \
	-L $(IO) -lIO \
	-L $(EXTERNAL) -lNR \
	-lhdf5 -lz -lcrypto


merge_sort: merge_sort.cpp
	$(CC) $(CFLAGS) merge_sort.cpp -o merge_sort

merge_files: merge_files.cpp
	$(CC) $(CFLAGS) merge_files.cpp -o merge_files


QDFGenomeExtractor.o: QDFGenomeExtractor.cpp QDFGenomeExtractor.h GenomeProvider.h IDSampler2.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

QDF2Plink.o: QDF2Plink.cpp QDFGenomeExtractor.cpp GenomeProvider.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

QDF2Plink: QDF2Plink.o QDFGenomeExtractor.o GeneWriter.o IDSampler2.o IDSample.o
	$(CC) $(COMP) $(CFLAGS) -o QDF2Plink \
	QDF2Plink.o QDFGenomeExtractor.o GeneWriter.o IDSampler2.o IDSample.o \
	-L $(COMMON) -lCommon \
	-L $(IO) -lIO \
	-lhdf5 -lz


GenomeSegments.o: GenomeSegments.cpp GenomeSegments.h
	$(CC) $(COPT) -g $(DEFS)


SegmentCompare.o: SegmentCompare.cpp GenomeSegments.h IDSampler.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS) -I $(IO) \
        -I $(VTK_I) 

SegmentCompare: SegmentCompare.o GenomeSegments.o IDSampler.o AncGraphBase.o
	$(CC) $(COMP) $(CFLAGS) -o SegmentCompare \
	SegmentCompare.o GenomeSegments.o IDSampler.o AncGraphBase.o \
	-L $(COMMON) -lCommon \
	-L $(IO) -lIO \
	-L $(VTK_L) \
	-l vtkCommonCore \
	-l vtkCommonComputationalGeometry \
	-l vtkCommonDataModel \
	-l vtkIOCore \
	-l vtkIOLegacy \
	-lhdf5 -lz


