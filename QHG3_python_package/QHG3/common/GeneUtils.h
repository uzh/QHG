#ifndef __GENEUTILS_H__
#define __GENEUTILS_H__

#include "types.h"
#include "WELL512.h"

static const int SHOW_NUM = 1;
static const int SHOW_NUC = 2;
static const int SHOW_ALL = 3;

class GeneUtils {
public:
    static const char *p_asNuc;
    static const uint  BITSINBLOCK;
    static const uint  NUCSINBLOCK;
    static int numNucs2Blocks(int iGenomeSize) { return (iGenomeSize+NUCSINBLOCK-1)/NUCSINBLOCK;};
    static ulong *translateGenome(int iGenomeSize, const char *pLine);
    static char *blockToNucStr(ulong lBlock, char *pNuc);
    static ulong *readGenome(int iGenomeSize, char *pLine);

    static void mutateBits(ulong *pGenome, int iNumBits, int iNMutations, WELL512* pWELL);
    static void mutateNucs(ulong *pGenome, int iNumBits, int iNMutations, WELL512* pWELL);
    static ulong *createFullRandomGenes(int iGenomeSize, WELL512* pWELL);
    static ulong *createFlatRandomGenes(int iGenomeSize, int iNumber, ulong *pGenome0=NULL);
    static ulong *copyMutatedGenes(int iGenomeSize, int iNumber, ulong *pGenome0, ulong *pGenome);
    static ulong *createRandomAlleles(int iGenomeSize, int iNumMutations, WELL512* pWELL);
    static void crossOver(ulong *pGenome1, const ulong *pGenome2, int iGenomeSize, int iNumCrossOvers, WELL512* pWELL);
    static void fullCrossOver(ulong* pGenome1, const ulong* pGenome2, int iGenomeSize, WELL512* pWELL);
    static void freeReco(ulong *pGenome1, ulong *pGenome2, int iNumBlocks, WELL512* pWELL);

    static int bitcount2(ulong x);
    static int calcDist(ulong *pG1, ulong *pG2, int iNumBlocks);
    
    static void showGenome(ulong *pGenome, int iGenomeSize, int iWhat=SHOW_ALL);

    static void plinkOrderNucleotides(ulong *pIn,  ulong *pOut, int iGenomeSize);
    static void interleaveNucleotides(ulong *pIn1, ulong *pIn2, ulong *pOut, int iGenomeSize);

    static void writePlinkHeader(FILE *fOut, const char*pLoc, int iNumber, idtype iID, idtype iMomID, idtype iDadID, int iGender);
    static void writePlinkNucleotides(FILE *fOut, const ulong *pGenomes, int iGenomeSize);
    static int  writePlinkMapFile(const char *pOut, int iGenomeSize); 
protected:
    static ulong makeMultiMask(std::vector<uint> vBreaks);
};



#endif



