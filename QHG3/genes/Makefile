

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
  CFLAGS=-g $(WARNINGS)
endif

ifdef OPT
  CFLAGS=-O3 $(WARNINGS)
endif

ifdef OPT
  CFLAGS2=-O3 -g $(WARNINGS)
endif

ifdef PROF
  CFLAGS=-pg $(WARNINGS)
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
#GEOINFO=../geoinfo
#TILING=../tiling
#QMAPS=../qmaps
#AGENTS=../agents
#ICOSA=../icosa
IO=../io
GENES=../genes
#KERNEL=../kernel
#MODULAR=../modular

VTK_I=/opt/visit/2.7.3/linux-x86_64/include/vtk/vtk-6.0
VTK_L=/opt/visit/2.7.3/linux-x86_64/lib

COMP=-fopenmp
COPT=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS) $(DOMP) $(COMP) $(OMPPFLAGS)
COPT2=-c $< -I $(COMMON) -o $@ $(CADD) $(CFLAGS2) $(DOMP) $(COMP) $(OMPPFLAGS)

#TARGETS=showAG  idsearch ancsortX DynGraphSamplerX QDF2Plink
#TARGETS=QDF2Plink GeneDist GridSampleTest
#TARGETS=QDFSampler GeneDist CalcPCO BinGeneMerge2 GenomeVarCheck ArrivalCheck AlleleCount PBTest QDFBin2Num PlinkToBin
#TARGETS=QDFSampler QDFGenomeSampler2 QDFPhenomeSampler2 GeneDist PheneDist CalcPCO BinGeneMerge2 BinPheneMerge2 GenomeVarCheck ArrivalCheck AlleleCount QDFBin2Num PlinkToBin
TARGETS=QDFGenomeSampler2 QDFPhenomeSampler2 GeneDist2 PheneDist2 CalcPCO BinGeneMerge2 BinPheneMerge2 GenomeVarCheck ArrivalCheck AlleleCount QDFBin2Num PlinkToBin

lib: libGenes.a

all: lib app


app: $(TARGETS)

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

ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib


libGenes.a:GeneUtils.o BitGeneUtils.o GenomeCreator.o \
	ProteinBuilder.o ProteomeComparator.o
	ar $(ARFL) $@ $?

LIBTARG=lib

all: $(TARGETS)

clean:
	rm -f *.o libGenes.a $(TARGETS)

new: clean lib app

GeneUtils.o: GeneUtils.cpp GeneUtils.h
	$(CC) $(COPT) $(DEFS) 

BitGeneUtils.o: BitGeneUtils.cpp BitGeneUtils.h
	$(CC) $(COPT) $(DEFS) 

GenomeCreator.o: GenomeCreator.cpp GenomeCreator.h
	$(CC) $(COPT) $(DEFS) 


IDSampler.o: IDSampler.cpp IDSampler.h AncGraphBase.h
	$(CC) $(COPT) -g $(DEFS) -I $(IO)  

IDSampler2.o: IDSampler2.cpp IDSampler2.h IDSample.h  AnalysisUtils.h
	$(CC) $(COPT) -g $(DEFS) -I $(IO)  


IDSample.o: IDSample.cpp IDSample.h  AnalysisUtils.h 
	$(CC) $(COPT) -g $(DEFS) -I $(IO)  

AnalysisUtils.o: AnalysisUtils.cpp AnalysisUtils.h
	$(CC) $(COPT) -g $(DEFS) -I $(IO)  




BinGeneFile.o:  BinGeneFile.cpp BinGeneFile.h  AnalysisUtils.h 
	$(CC) $(COPT) $(DEFS) -I${IO}

BinPheneFile.o:  BinPheneFile.cpp BinPheneFile.h  AnalysisUtils.h
	$(CC) $(COPT) $(DEFS) -I${IO}


DistMat.o:  DistMat.cpp DistMat.h
	$(CC) $(COPT) $(DEFS) -I${IO}  -fopenmp

GeneDist.o:  GeneDist.cpp DistMat.h BinGeneFile.h
	$(CC) $(COPT) $(DEFS) -I${IO} -fopenmp

GeneDist: GeneDist.o DistMat.o BinGeneFile.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o GeneDist \
	GeneDist.o DistMat.o BinGeneFile.o \
	-L $(IO)       -lIO \
	-L $(GENES)    -lGenes \
	-L $(COMMON)   -lCommon \
        -lz  -lhdf5


PheneDist.o:  PheneDist.cpp DistMat.h BinPheneFile.h
	$(CC) $(COPT) $(DEFS) -I${IO} -fopenmp

PheneDist: PheneDist.o DistMat.o BinPheneFile.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o PheneDist \
	PheneDist.o DistMat.o BinPheneFile.o \
	-L $(IO)       -lIO \
	-L $(GENES)    -lGenes \
	-L $(COMMON)   -lCommon \
        -lz  -lhdf5

SequenceDist.o:  SequenceDist.cpp SequenceDist.h DistMat.h
	$(CC) $(COPT) $(DEFS) -I${IO} -fopenmp


PheneDist2.o:  PheneDist2.cpp SequenceDist.h DistMat.h BinPheneFile.h
	$(CC) $(COPT) $(DEFS) -I${IO} -fopenmp

PheneDist2: PheneDist2.o DistMat.o BinPheneFile.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o PheneDist2 \
	PheneDist2.o SequenceDist.o DistMat.o BinPheneFile.o \
	-L $(IO)       -lIO \
	-L $(GENES)    -lGenes \
	-L $(COMMON)   -lCommon \
        -lz  -lhdf5

GeneDist2.o:  GeneDist2.cpp SequenceDist.h DistMat.h BinGeneFile.h GeneUtils.h BitGeneUtils.h
	$(CC) $(COPT) $(DEFS) -I${IO} -fopenmp

GeneDist2: GeneDist2.o SequenceDist.o DistMat.o BinGeneFile.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o GeneDist2 \
	GeneDist2.o SequenceDist.o DistMat.o BinGeneFile.o \
	-L $(IO)       -lIO \
	-L $(GENES)    -lGenes \
	-L $(COMMON)   -lCommon \
        -lz  -lhdf5


CrossDistMat.o: CrossDistMat.cpp CrossDistMat.h
	$(CC) $(COPT) $(DEFS) -I${IO} -fopenmp

CrossComp.o: CrossComp.cpp CrossDistMat.h BinGeneFile.h
	$(CC) $(COPT) $(DEFS) -I${IO} -fopenmp

CrossComp: CrossComp.o CrossDistMat.o BinGeneFile.o 
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o CrossComp \
	CrossComp.o CrossDistMat.o BinGeneFile.o \
	-L $(IO)       -lIO \
	-L $(COMMON)   -lCommon \
	-lhdf5



GeneWriter.o: GeneWriter.cpp GeneWriter.h IDSampler2.h
	$(CC) $(COPT) $(DEFS)

PheneWriter.o: PheneWriter.cpp PheneWriter.h IDSampler2.h
	$(CC) $(COPT) $(DEFS)


merge_sort: merge_sort.cpp
	$(CC) $(CFLAGS) merge_sort.cpp -o merge_sort

merge_files: merge_files.cpp
	$(CC) $(CFLAGS) merge_files.cpp -o merge_files


QDFGenomeExtractor.o: QDFGenomeExtractor.cpp QDFGenomeExtractor.h GenomeProvider.h IDSampler2.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFSampler.o: QDFSampler.cpp QDFGenomeExtractor.cpp GenomeProvider.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFSampler: QDFSampler.o QDFGenomeExtractor.o GeneWriter.o IDSampler2.o IDSample.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o QDFSampler \
	QDFSampler.o QDFGenomeExtractor.o GeneWriter.o IDSampler2.o IDSample.o \
	-L $(IO) -lIO \
	-L ${GENES} -lGenes \
	-L $(COMMON) -lCommon \
	-lhdf5 -lz -lcrypto 


QDFSequenceExtractor.o: QDFSequenceExtractor.cpp QDFSequenceExtractor.h SequenceProvider.h IDSampler2.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp


GeneWriter2.o: GeneWriter2.cpp GeneWriter2.h IDSampler2.h
	$(CC) $(COPT) $(DEFS)

QDFGenomeExtractor2.o: QDFGenomeExtractor2.cpp QDFGenomeExtractor2.h SequenceProvider.h IDSampler2.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFGenomeSampler2.o: QDFGenomeSampler2.cpp QDFGenomeExtractor2.h SequenceProvider.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFGenomeSampler2: QDFGenomeSampler2.o QDFSequenceExtractor.o QDFGenomeExtractor2.o GeneWriter2.o IDSampler2.o IDSample.o AnalysisUtils.o 
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o QDFGenomeSampler2 \
	QDFGenomeSampler2.o QDFSequenceExtractor.o QDFGenomeExtractor2.o GeneWriter2.o IDSampler2.o IDSample.o AnalysisUtils.o \
	-L $(IO) -lIO \
	-L ${GENES} -lGenes \
	-L $(COMMON) -lCommon \
	-lhdf5 -lz -lcrypto


PheneWriter2.o: PheneWriter2.cpp PheneWriter2.h IDSampler2.h
	$(CC) $(COPT) $(DEFS)

QDFPhenomeExtractor2.o: QDFPhenomeExtractor2.cpp QDFPhenomeExtractor2.h SequenceProvider.h IDSampler2.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFPhenomeSampler2.o: QDFPhenomeSampler2.cpp QDFPhenomeExtractor2.h SequenceProvider.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFPhenomeSampler2: QDFPhenomeSampler2.o QDFSequenceExtractor.o QDFPhenomeExtractor2.o PheneWriter2.o IDSampler2.o IDSample.o AnalysisUtils.o 
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o QDFPhenomeSampler2 \
	QDFPhenomeSampler2.o QDFSequenceExtractor.o QDFPhenomeExtractor2.o PheneWriter2.o IDSampler2.o IDSample.o AnalysisUtils.o \
	-L $(IO) -lIO \
	-L ${GENES} -lGenes \
	-L $(COMMON) -lCommon \
	-lhdf5 -lz -lcrypto



QDFPhenomeExtractor.o: QDFPhenomeExtractor.cpp QDFPhenomeExtractor.h PhenomeProvider.h IDSampler2.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFPhenSampler.o: QDFPhenSampler.cpp QDFPhenomeExtractor.cpp PhenomeProvider.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -fopenmp

QDFPhenSampler: QDFPhenSampler.o QDFPhenomeExtractor.o PheneWriter.o IDSampler2.o IDSample.o 
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o QDFPheneSampler \
	QDFPhenSampler.o QDFPhenomeExtractor.o PheneWriter.o IDSampler2.o IDSample.o \
	-L $(IO) -lIO \
	-L $(COMMON) -lCommon \
	-lhdf5 -lz


CalcPCO.o: CalcPCO.cpp
	$(CC) $(COPT) $(DEFS)

CalcPCO: CalcPCO.o 
	$(CC) $(COMP) $(CFLAGS) -o CalcPCO \
        CalcPCO.o \
	-lgsl -lgslcblas

BinGeneMerge2.o: BinGeneMerge2.cpp BinGeneFile.h
	$(CC) $(COPT) $(DEFS)

BinGeneMerge2: BinGeneMerge2.o BinGeneFile.o
	$(CC) $(COMP) $(CFLAGS) -o BinGeneMerge2 \
	BinGeneMerge2.o BinGeneFile.o \
	-L $(GENES) -lGenes \
	-L $(COMMON) -lCommon

BinPheneMerge2.o: BinPheneMerge2.cpp BinPheneFile.h
	$(CC) $(COPT) $(DEFS)

BinPheneMerge2: BinPheneMerge2.o BinPheneFile.o
	$(CC) $(COMP) $(CFLAGS) -o BinPheneMerge2 \
	BinPheneMerge2.o BinPheneFile.o \
	-L $(GENES) -lGenes \
	-L $(COMMON) -lCommon

GridSampler.o: GridSampler.cpp GridSampler.h
	$(CC) $(COPT) -g $(DEFS) -I ${IO}

GridSampleTest.o: GridSampleTest.cpp GridSampler.h
	$(CC) $(COPT) -g $(DEFS) -I ${IO}

GridSampleTest: GridSampleTest.o GridSampler.o
	$(CC) $(COMP) $(CFLAGS) -o GridSampleTest \
	GridSampleTest.o GridSampler.o \
	-L $(IO) -lIO \
	-L $(COMMON) -lCommon \
	-lhdf5 -lz

GenomeVarChecker.o: GenomeVarChecker.cpp GenomeVarChecker.h 
	$(CC) $(COPT) -g $(DEFS) -I $(IO) -fopenmp

GenomeVarCheck.o: GenomeVarCheck.cpp GenomeVarChecker.h 
	$(CC) $(COPT) -g $(DEFS) -I $(IO) 

GenomeVarCheck: GenomeVarCheck.o GenomeVarChecker.o BinGeneFile.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o GenomeVarCheck \
        GenomeVarCheck.o GenomeVarChecker.o BinGeneFile.o \
	-L ${IO}     -lIO \
	-L ${GENES}  -lGenes \
        -L ${COMMON} -lCommon \
	-lhdf5


AlleleCounter.o: AlleleCounter.cpp AlleleCounter.h 
	$(CC) $(COPT) -g $(DEFS) -I $(IO) -fopenmp

AlleleCount.o: AlleleCount.cpp AlleleCounter.h 
	$(CC) $(COPT) -g $(DEFS) -I $(IO) 

AlleleCount: AlleleCount.o AlleleCounter.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o AlleleCount \
         AlleleCount.o AlleleCounter.o \
	-L ${IO}     -lIO \
	-L ${GENES}  -lGenes \
        -L ${COMMON} -lCommon \
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



ArrivalChecker.o: ArrivalChecker.cpp ArrivalChecker.h  AnalysisUtils.h
	$(CC) $(COPT) -g $(DEFS) -I$(IO) -fopenmp

ArrivalCheck.o: ArrivalCheck.cpp ArrivalChecker.h 
	$(CC) $(COPT) -g $(DEFS) -fopenmp

ArrivalCheck: ArrivalCheck.o ArrivalChecker.o  AnalysisUtils.o
	$(CC) $(COMP) $(CFLAGS) -fopenmp -o ArrivalCheck \
	ArrivalCheck.o ArrivalChecker.o  AnalysisUtils.o \
	-L $(IO) -lIO \
	-L $(COMMON) -lCommon \
	-lhdf5 -lz



ProteinBuilder.o: ProteinBuilder.cpp ProteinBuilder.h
	$(CC) $(COPT) $(DEFS) 

ProteomeComparator.o: ProteomeComparator.cpp ProteomeComparator.h ProteinBuilder.h
	$(CC) $(COPT) $(DEFS) 

PBTest.o: PBTest.cpp ProteinBuilder.h
	$(CC) $(COPT) $(DEFS) -I$(IO)


PBTest: PBTest.o QDFGenomeExtractor.o IDSample.o IDSampler2.o 
	$(CC) $(COMP) -o PBTest \
	PBTest.o QDFGenomeExtractor.o IDSample.o IDSampler2.o \
	-L$(IO) -lIO \
	-L ${GENES}  -lGenes \
	-L$(COMMON) -lCommon \
	-lhdf5 -lz -lcrypto


QDFBin2Num.o: QDFBin2Num.cpp BinGeneFile.h GeneUtils.h BitGeneUtils.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

QDFBin2Num:  QDFBin2Num.o BinGeneFile.o 
	$(CC) $(COMP) -o QDFBin2Num \
	QDFBin2Num.o BinGeneFile.o \
        -L ./ -lGenes \
	-L../common -lCommon


PlinkReader.o: PlinkReader.cpp PlinkReader.h GeneUtils.h GeneWriter2.h IDSample.h IDSampler2.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

PlinkToBin.o: PlinkToBin.cpp PlinkReader.h GeneUtils.h 
	$(CC) $(COPT) $(DEFS) -I$(IO)

PlinkToBin:  PlinkToBin.o PlinkReader.o GeneWriter2.o IDSampler2.o IDSample.o  AnalysisUtils.o
	$(CC) $(COMP) -o PlinkToBin \
	PlinkToBin.o PlinkReader.o  GeneWriter2.o IDSampler2.o IDSample.o  AnalysisUtils.o \
        -L ./ -lGenes \
	-L $(IO) -lIO \
	-L $(COMMON) -lCommon \
        -lhdf5 -lz
