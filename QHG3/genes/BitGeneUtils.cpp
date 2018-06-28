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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strutils.h"
#include "BitGeneUtils.h"
#include "WELL512.h"

const uint  BitGeneUtils::BITSINBLOCK = 8*sizeof(ulong);
const uint  BitGeneUtils::NUCSINBLOCK = BITSINBLOCK;
const uint  BitGeneUtils::BITSINNUC   = 1;
const uint  BitGeneUtils::BITSININT   = 8*sizeof(int);
const uint  BitGeneUtils::NUMPARENTS  = 2;

static  const char s_asNuc[] = {'A', 'C'};
const char *BitGeneUtils::p_asNuc = s_asNuc;

//   (from http://en.wikipedia.org/wiki/Hamming_weight)
const ulong m1  = 0x5555555555555555; //binary: 0101...
const ulong m2  = 0x3333333333333333; //binary: 00110011..
const ulong m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const ulong m8  = 0x00ff00ff00ff00ff; //binary:  8 zeros,  8 ones ...
const ulong m16 = 0x0000ffff0000ffff; //binary: 16 zeros, 16 ones ...
const ulong m32 = 0x00000000ffffffff; //binary: 32 zeros, 32 ones
const ulong hff = 0xffffffffffffffff; //binary: all ones
const ulong h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...



//----------------------------------------------------------------------------
// bitcount
//   (from http://en.wikipedia.org/wiki/Hamming_weight)
//
uint BitGeneUtils:: bitcount(ulong x) {
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (int)((x * h01)>>56);  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}


//----------------------------------------------------------------------------
// mutateNucs
//    set a bit at a random location per mutation
//
void BitGeneUtils::mutateNucs(ulong *pGenome, int iNumBits, int iNMutations, WELL512* pWELL = NULL) {
    for (int i = 0; i < iNMutations; i++) {
        
        uint iPos = 0;
        if (pWELL == NULL) {
            iPos =  (uint) ((1.0*iNumBits*rand())/(RAND_MAX+1.0));
        } else {
            iPos =  pWELL->wrandi(0, iNumBits);
        }
	
        uint iBit   = iPos%BITSINBLOCK;
        iPos /= BITSINBLOCK;
        
        ulong iNew= 1L << iBit;
        //pGenome[iPos] ^= iNew;  // flip, not set
        pGenome[iPos] |= iNew;  // set, not flip
        
    }
}



//-----------------------------------------------------------------------------
// makeMultiMask
//   create a bit mask with 
//      1:   use current chromosome
//      0:   use other chromosome
//
//   The mask always starts with 1 (i.e. LSB is 1) except if
//   the first break is at position 0.
//   At each break the bit value is flipped.
//   The value of the last bit (MSB) is 1 - numbreaks%2
//
ulong BitGeneUtils::makeMultiMask(std::vector<uint> vBreaks) {
    // assume vBreaks sorted
  
    // find bit value for last bit (highest bit)
    uint k = 1 - (vBreaks.size()%2);
    ulong lOut = 0;
    // traverse the breaks backwards
    int   i = (int)vBreaks.size()-1;
    for (uint j = 0; j < BITSINBLOCK; j++) {
        if ((i >= 0) &&(j == BITSINBLOCK-vBreaks[i])) {
            // switch bit value
            i--;
            k = 1-k;
        }
        lOut = (lOut << 1) + k;
    }


    return lOut;
}


//----------------------------------------------------------------------------
// crossOver
//   mixes the two chromosomes of pGenome2 into the two chromosomes of pGenome1
//
void BitGeneUtils::crossOver(ulong *pGenome1, const ulong *pGenome2, int iGenomeSize, int iNumCrossOvers, WELL512* pWELL = NULL) {
    uint iNumBlocks = numNucs2Blocks(iGenomeSize);
    uint iNumBits = iNumBlocks*BITSINBLOCK;
    
    // determine crossover points (which bit)
    // vBlockBreaks: blocknum  -> bitpos
    std::map<uint, std::vector<uint> > vBlockBreaks;
    //    std::set<uint> vBreaks;
    if (pWELL == NULL) {
        for (int i = 0; i < iNumCrossOvers; i++) {
            uint iBreakPos = BitGeneUtils::BITSINNUC*(int) ((1.0*iNumBits*rand())/(BitGeneUtils::BITSINNUC*(RAND_MAX+1.0)));
            uint iBlock    = iBreakPos/BitGeneUtils::BITSINBLOCK;
            uint iBitPos   = iBreakPos%BitGeneUtils::BITSINBLOCK;
            vBlockBreaks[iBlock].push_back(iBitPos);
        }
    } else {
        for (int i = 0; i < iNumCrossOvers; i++) {
            uint iBreakPos = pWELL->wrandi(0, iNumBits, BitGeneUtils::BITSINNUC);
            uint iBlock    = iBreakPos/BitGeneUtils::BITSINBLOCK;
            uint iBitPos   = iBreakPos%BitGeneUtils::BITSINBLOCK;
            vBlockBreaks[iBlock].push_back(iBitPos);
        }
    }

    /*
    // determine crossover points (which long (Block), which bit inside long)
    std::set<uint>::const_iterator it;
    for (it = vBreaks.begin(); it != vBreaks.end(); it++) {
        uint iBlock = (*it)/BitGeneUtils::BITSINBLOCK;
        vBlockBreaks[iBlock].push_back((*it)%BitGeneUtils::BITSINBLOCK);
    }
    */
    
    // begins with chromosome 0
    int iCurChromosome = 0;
    uint iLast = 0;
    uint iS = 0;
    std::map<uint, std::vector<uint> >::const_iterator itt;
    for (itt = vBlockBreaks.begin(); itt != vBlockBreaks.end(); itt++) {
        uint iBlock = itt->first;
        iCurChromosome =iS%2;

        // copy ulongs with no crossing points
        if (iBlock > iLast) {
            memcpy(pGenome1+iLast, pGenome2+(iCurChromosome*iNumBlocks+iLast), (iBlock-iLast)*sizeof(ulong));
            memcpy(pGenome1+iNumBlocks+iLast, pGenome2+((1-iCurChromosome)*iNumBlocks+iLast), (iBlock-iLast)*sizeof(ulong));
        }

        // create bit masks defining which bits to keep
        // and which to copy 
        ulong lLeft = makeMultiMask(itt->second);
        ulong lRight = ~lLeft;


        pGenome1[iBlock]            = (lLeft  & pGenome2[(iCurChromosome)*iNumBlocks+iBlock]) | 
                                      (lRight & pGenome2[(1-iCurChromosome)*iNumBlocks+iBlock]);
        pGenome1[iBlock+iNumBlocks] = (lLeft  & pGenome2[(1-iCurChromosome)*iNumBlocks+iBlock]) | 
                                      (lRight & pGenome2[(iCurChromosome)*iNumBlocks+iBlock]);

        // set start chromosome for next iteration (take into account multiple crossovers inside one long)
        iS += (uint)itt->second.size();
        iLast = iBlock+1;
    }
    // copy the rest if necessary
    if (iNumBlocks > iLast) {
        iCurChromosome = (iS)%2;
        // copy trailing 
        memcpy(pGenome1+iLast, pGenome2+(iCurChromosome*iNumBlocks+iLast), (iNumBlocks-iLast)*sizeof(ulong));
        memcpy(pGenome1+iNumBlocks+iLast, pGenome2+((1-iCurChromosome)*iNumBlocks+iLast), (iNumBlocks-iLast)*sizeof(ulong));

    }
   
}


//----------------------------------------------------------------------------
// makeFreeMask
//   creates a mask for random mixing (1-bit aligned pattern)
//   "concatenating" two uint rands
//
ulong BitGeneUtils::makeFreeMask(WELL512* pWELL) {
    ulong lV = 0;
    if (pWELL != NULL) {
        lV = ((ulong)pWELL->wrand() << BITSININT) + pWELL->wrand();
    } else {
        lV = ((ulong)rand() << BITSININT) + rand();
    }
    return lV;
}


//----------------------------------------------------------------------------
// freeReco
//   completely random mixture of the two chromosomes of pGenome2 into the 
//   two chromosomes of pGenome1
//
void BitGeneUtils::freeReco(ulong *pGenome1, ulong *pGenome2, int iNumBlocks, WELL512* pWELL = NULL) {
    //    int iNumBlocks = numNucs2Blocks(iGenomeSize);

    for (int iBlock = 0; iBlock < iNumBlocks; iBlock++) {

        ulong lLeft  = makeFreeMask(pWELL);
        ulong lRight = ~lLeft;

        pGenome1[iBlock] = (lLeft  & pGenome2[iBlock]) | 
                           (lRight & pGenome2[iNumBlocks+iBlock]);

        pGenome1[iBlock+iNumBlocks] = (lLeft  & pGenome2[iNumBlocks+iBlock]) | 
                                      (lRight & pGenome2[iBlock]);

    }
}


//----------------------------------------------------------------------------
// calcDist
// 
int BitGeneUtils::calcDist(ulong *pG1, ulong *pG2, int iGenomeSize) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    
    // first of G1 with first of G2
    int iC1 = 0;
    for (int k = 0; k < iNumBlocks; k++) {
        iC1 += bitcount(pG1[k] ^ pG2[k]);
    }

    // second of G1 with second of G2
    for (uint k = iNumBlocks; k < NUMPARENTS*iNumBlocks; k++) {
        iC1 += bitcount(pG1[k] ^ pG2[k]);
    }

    //            memcpy(m_ulG1, m_ulGenome1[i]+iNumBlocks, iNumBlocks*sizeof(long));
    //            memcpy(m_ulG1+iNumBlocks, m_ulGenome1[i], iNumBlocks*sizeof(long));
    
    // second of G1 with frst of G2
    int iC2 = 0;
    for (int k = 0; k < iNumBlocks; k++) {
        iC2 += bitcount(pG1[k+iNumBlocks] ^ pG2[k]);
    }
 
    // first of G1 with second of G2
    for (int k = 0; k < iNumBlocks; k++) {
        iC2 += bitcount(pG1[k] ^ pG2[k+iNumBlocks]);
    }
    
    return (iC1 < iC2)?iC1:iC2;   
}


//----------------------------------------------------------------------------
// calcDistFloat
// 
float BitGeneUtils::calcDistFloat(ulong *pG1, ulong *pG2, int iGenomeSize) {
    return (float)(calcDist(pG1, pG2, iGenomeSize));
}


//----------------------------------------------------------------------------
// blockToNucStr
//   convert the bits in the ulong to a sequence of nucleotide chars.
//   The lowest bit is the first nucleotide.
//   The string pNuc must have a size greater or equal to NUCSINBLOCK+1
//
char *BitGeneUtils::blockToNucStr(ulong lBlock, char *pNuc) {
    pNuc[NUCSINBLOCK] = '\0';
    for (uint i = 0; i < NUCSINBLOCK; i++) {
        pNuc[i] = p_asNuc[lBlock & 0x1];
        lBlock >>= BITSINNUC;
    }
    return pNuc;
}


//----------------------------------------------------------------------------
// blockToNumStr
//   convert the bits in the ulong to a sequence of numbers (0=A,1=C,2=G,3=T)
//   The lowest bit is the first nucleotide.
//   The string pNuc must have a size greater or equal to NUCSINBLOCK+1
//
char *BitGeneUtils::blockToNumStr(ulong lBlock, char *pNuc) {
    pNuc[2*NUCSINBLOCK] = '\0';
    for (uint i = 0; i < NUCSINBLOCK; i++) {
        pNuc[2*i]   = '0'+ (lBlock & 0x1);
        pNuc[2*i+1] = ' ';
        lBlock >>= BITSINNUC;
    }
    return pNuc;
}


//----------------------------------------------------------------------------
// createFullRandomGenes
//   
ulong *BitGeneUtils::createFullRandomGenes(uint iNumParents, int iGenomeSize, WELL512* pWELL) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[NUMPARENTS*iNumBlocks];
    if (pWELL == NULL) {
        for (uint a = 0; a < NUMPARENTS; ++a) {
            for (int i = 0; i < iNumBlocks; ++i) {
                ulong l = rand();
                pGenome[a*iNumBlocks+i] = (l << BITSININT)+rand();
            }
        }
    } else {
        for (uint a = 0; a < NUMPARENTS; ++a) {
            for (int i = 0; i < iNumBlocks; ++i) {
                ulong l = pWELL->wrand();
                pGenome[a*iNumBlocks+i] = (l << BITSININT) + pWELL->wrand();
            }
        }
    }

    return pGenome;
}


//----------------------------------------------------------------------------
// createRandomAlleles
//   
ulong *BitGeneUtils::createRandomAlleles(uint iNumParents, int iGenomeSize, int iNumMutations, WELL512* pWELL) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[NUMPARENTS*iNumBlocks];
    // fill the first chromosome...
    if (pWELL == NULL) {
        for (int i = 0; i < iNumBlocks; ++i) {
            ulong l = rand();
            pGenome[i] = (l << BITSININT)+rand();
        }
    } else {
        for (int i = 0; i < iNumBlocks; ++i) {
            ulong l = pWELL->wrand();
            pGenome[i] = (l << BITSININT) + pWELL->wrand();
        }
    }
    // ... copy it to the second chromosome ...
    memcpy(pGenome+iNumBlocks, pGenome, iNumBlocks*sizeof(ulong));
    // ... and mutate it
    mutateNucs(pGenome+iNumBlocks, iGenomeSize, iNumMutations);
    return pGenome;
}


//----------------------------------------------------------------------------
// createFlatRandomGenes
//   
ulong *BitGeneUtils::createFlatRandomGenes(uint iNumParents, int iGenomeSize, int iNumber, ulong *pGenome0) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[NUMPARENTS*iNumBlocks];
    if (pGenome0 != NULL) {
        memcpy(pGenome, pGenome0,  NUMPARENTS*iNumBlocks*sizeof(ulong));
    } else {
        memset(pGenome, 0, NUMPARENTS*iNumBlocks*sizeof(ulong));
    }

    mutateNucs(pGenome, NUMPARENTS*iGenomeSize, iNumber);
    
    return pGenome;
}
    


//----------------------------------------------------------------------------
// copyMutatedGenes
//   
ulong *BitGeneUtils::copyAndMutateGenes(uint iNumParents, int iGenomeSize, int iNumber, ulong *pGenome0, ulong *pGenome) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    if (pGenome0 != NULL) {
        memcpy(pGenome, pGenome0,  NUMPARENTS*iNumBlocks*sizeof(ulong));
    } else {
        memset(pGenome, 0, NUMPARENTS*iNumBlocks*sizeof(ulong));
    }

    mutateNucs(pGenome, NUMPARENTS*iGenomeSize, iNumber);
    
    return pGenome;
}

