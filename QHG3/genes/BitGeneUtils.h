/*============================================================================
| BitGeneUtils
| 
|  Methods to handle and manipulate genomes consisting of 2 nucleotides
|  represented as 1-bit items packed in arrays of unsiged longs:
|  - creation and conversion to/from string
|  - mutation amd recombination
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __BITGENEUTILS_H__
#define __BITGENEUTILS_H__

#include "types.h"
#include "WELL512.h"

#include <vector>


class BitGeneUtils {
public:
    static const char *p_asNuc;
    static const uint  BITSINBLOCK;
    static const uint  BITSINNUC;
    static const uint  BITSININT;
    static const uint  NUCSINBLOCK;
    static const uint  NUMPARENTS;

    static int numNucs2Blocks(int iGenomeSize) { return (iGenomeSize+BITSINBLOCK-1)/BITSINBLOCK;};
  
    static char *blockToNucStr(ulong lBlock, char *pNuc);
    static char *blockToNumStr(ulong lBlock, char *pNuc);

    static void mutateNucs(ulong *pGenome, int iNumBits, int iNMutations, WELL512* pWELL);
  
    static void crossOver(ulong *pGenome1, const ulong *pGenome2, int iGenomeSize, int iNumCrossOvers, WELL512* pWELL);
    static void fullCrossOver(ulong* pGenome1, const ulong* pGenome2, int iGenomeSize, WELL512* pWELL);
    static void freeReco(ulong *pGenome1, ulong *pGenome2, int iNumBlocks, WELL512* pWELL);

    static uint bitcount(ulong x);
    static int calcDist(ulong *pG1, ulong *pG2, int iNumBlocks);
    static float calcDistFloat(ulong *pG1, ulong *pG2, int iNumBlocks);

    // genome creation
    static ulong *createFullRandomGenes(uint iNumParents, int iGenomeSize, WELL512* pWELL);
    static ulong *createFlatRandomGenes(uint iNumParents, int iGenomeSize, int iNumber, ulong *pGenome0=NULL);
    static ulong *copyAndMutateGenes(uint iNumParents, int iGenomeSize, int iNumber, ulong *pGenome0, ulong *pGenome);
    static ulong *createRandomAlleles(uint iNumParents, int iGenomeSize, int iNumMutations, WELL512* pWELL);
  
protected:
    static ulong makeMultiMask(std::vector<uint> vBreaks);
    static ulong makeFreeMask(WELL512* pWELL = NULL);
};



#endif



