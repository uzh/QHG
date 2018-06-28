/*============================================================================
| GeneUtils
| 
|  Methods to handle and manipulate genomes consisting of 4 nucleotides
|  represented as 2-bit items packed in arrays of unsiged longs:
|  - creation and conversion to/from string
|  - mutation amd recombination
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __GENEUTILS_H__
#define __GENEUTILS_H__

#include "types.h"
#include "WELL512.h"

#include <vector>

static const int SHOW_GENES_NUM = 1;
static const int SHOW_GENES_NUC = 2;
static const int SHOW_GENES_ALL = 3;

class GeneUtils {
public:
    static const char *p_asNuc;
    static const uint  BITSINBLOCK;
    static const uint  NUCSINBLOCK;
    static const uint  BITSINNUC;
    static const uint  BITSININT;
    static const uint  NUMPARENTS;

    static int numNucs2Blocks(int iGenomeSize) { return (iGenomeSize+NUCSINBLOCK-1)/NUCSINBLOCK;};
    static int insertSequence(ulong *pGenome, int iGenomeSize, uint iNucPos, const char *pTriplet);
    static ulong *translateGenome(int iGenomeSize, const char *pLine);
    static char *blockToNucStr(ulong lBlock, char *pNuc);
    static char *blockToNumStr(ulong lBlock, char *pNuc);
    static ulong *readGenome(int iGenomeSize, char *pLine);

    static void mutateBits(ulong *pGenome, int iNumBits, int iNMutations, WELL512* pWELL);
    static void mutateNucs(ulong *pGenome, int iNumBits, int iNMutations, WELL512* pWELL);
    static void crossOver(ulong *pGenome1, const ulong *pGenome2, int iGenomeSize, int iNumCrossOvers, WELL512* pWELL);
    static void fullCrossOver(ulong* pGenome1, const ulong* pGenome2, int iGenomeSize, WELL512* pWELL);
    static void freeReco(ulong *pGenome1, ulong *pGenome2, int iNumBlocks, WELL512* pWELL);

    static uint bitcount2(ulong x);
    static int calcDist(ulong *pG1, ulong *pG2, int iNumBlocks);
    static float calcDistFloat(ulong *pG1, ulong *pG2, int iNumBlocks);

    static ulong *createFullRandomGenes(uint iNumParents, int iGenomeSize, WELL512* pWELL);
    static ulong *createFlatRandomGenes(uint iNumParents, int iGenomeSize, int iNumber, ulong *pGenome0=NULL);
    static ulong *copyAndMutateGenes(uint iNumParents, int iGenomeSize, int iNumber, ulong *pGenome0, ulong *pGenome);
    static ulong *createRandomAlleles(uint iNumParents, int iGenomeSize, int iNumMutations, WELL512* pWELL);
    
    static void showGenome(ulong *pGenome, int iGenomeSize, int iWhat=SHOW_GENES_ALL);

    static void plinkOrderNucleotides(ulong *pIn,  ulong *pOut, int iGenomeSize);
    static void interleaveNucleotides(ulong *pIn1, ulong *pIn2, ulong *pOut, int iGenomeSize);

    static void writePlinkHeader(FILE *fOut, const char*pLoc, int iNumber, idtype iID, idtype iMomID, idtype iDadID, int iGender);
    static void writePlinkNucleotides(FILE *fOut, const ulong *pGenomes, int iGenomeSize);
    static int  writePlinkMapFile(const char *pOut, int iGenomeSize); 
protected:
    static ulong makeMultiMask(std::vector<uint> vBreaks);
    static ulong makeFreeMask(WELL512* pWELL = NULL);
};



#endif



