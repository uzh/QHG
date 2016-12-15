#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strutils.h"
#include "GeneUtils.h"
#include "WELL512.h"

const uint  GeneUtils::BITSINBLOCK = 8*sizeof(ulong);
const uint  GeneUtils::NUCSINBLOCK = BITSINBLOCK/2;



static  const char s_asNuc[] = {'A', 'C', 'G', 'T'};
const char *GeneUtils::p_asNuc = s_asNuc;

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
int bitcount(ulong x) {
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (int)((x * h01)>>56);  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}


//----------------------------------------------------------------------------
// bitcount2
//   count diffences in nucleotides
//   
int GeneUtils::bitcount2(ulong x) {
    x = (x & m1 ) | ((x >>  1) & m1); //put occurrence of 1 in each 2 bits into those 2 bits 
    x = (x & m2) + ((x >> 2) & m2);   //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;          //put count of each 8 bits into those 8 bits 
    return (uint)((x * h01)>>56);  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}


//----------------------------------------------------------------------------
// translateGenome
//   translate a sequence of nucleotide chars ('A', 'C', 'G', 'T')
//   to an arry of ulongs.
//   The first nucleotide of the sequence is placed on the 2
//   least significant bits.
//
ulong *GeneUtils::translateGenome(int iGenomeSize, const char *pLine) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[2*iNumBlocks];
    memset(pGenome, 0, 2*iNumBlocks*sizeof(ulong));
    int iResult =0;
   
    const char *p = pLine;
    int iC = 0;
    int iNuc = 0;
    int iPos = 0;
    while ((iResult == 0) && (iC < 2*iNumBlocks) && (*p != '\0')) {
        char c = (char)toupper(*p++);
        if ((c == 'A') || (c == 'C') || (c == 'G') || (c == 'T')) {
            ulong iVal = 0;
            switch (c) {
            case 'A':
                iVal = 0;
                break;
            case 'C':
                iVal = 1;
                break;
            case 'G':
                iVal = 2;
                break;
            case 'T':
                iVal = 3;
                break;
            }
            pGenome[iC] += iVal<<iPos;
            iPos += 2;;
            //            printf("add %c -> %lu : %016lx\n", c, iVal, pGenome[iC]);
            iNuc++;
            if (iPos == 64) {
                iC++;
                iPos = 0;
            }
                           
        } else if ((isspace(c)) || (c == ',') || (c ==';') || (c == '|')) {
            // spaces and other separators are ok
        } else {
            iResult = -1;
        }
    }
    if  ((iResult == 0) && (iC = 2*iNumBlocks)) {
        //       printf("read %d nucleotides into %d longs\n", iNuc, iGenomeSize);
    } else {
        delete[] pGenome;
        pGenome = NULL;
    }        

    return pGenome;

}
 

//----------------------------------------------------------------------------
// blockToNucStr
//   convert the bits in the ulong to a sequence of nucleotide chars.
//   The lowest 2 bits are the first nucleotide.
//   The string pNuc must have a size greater or equal to NUCSINBLOCK+1
//
char *GeneUtils::blockToNucStr(ulong lBlock, char *pNuc) {
    pNuc[NUCSINBLOCK] = '\0';
    for (uint i = 0; i < NUCSINBLOCK; i++) {
        pNuc[i] = p_asNuc[lBlock & 0x3];
        lBlock >>= 2;
    }
    return pNuc;
}


//----------------------------------------------------------------------------
// readGenome
//   read numeric genome (list of ulongs) from string 
//
ulong *GeneUtils::readGenome(int iGenomeSize, char *pLine) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[2*iNumBlocks];
    int iResult =0;
    char *p = strtok(pLine, " \t,;|");
    int iIndex = 0;
    while ((iResult == 0) && (iIndex < 2*iNumBlocks) && (p != NULL)) {
        long ul=0;
        char *pEnd;
        ul = strtol(p, &pEnd, 16);
        if (*pEnd == '\0') {
            pGenome[iIndex++] = ul;
        } else {
            printf("Bad number in genome:[%s]\n", p);
        }
        p = strtok(NULL, " \t,;|");
    }

    if ((iResult == 0) && (iIndex == 2*iNumBlocks)) {
        //        printf("Read %d lons\n", iIndex);
    } else {
        delete[] pGenome;
        pGenome = NULL;
    }

    return pGenome;
}


//----------------------------------------------------------------------------
// mutateBits
//    flip a bit at a random location per mutation
//    not all changes are possible:
//    A(0,0)-C(0,1)
//      |      |
//    G(1,0)-T(1,1)
//
void GeneUtils::mutateBits(ulong *pGenome, int iNumBits, int iNMutations, WELL512* pWELL = NULL) {
    for (int i = 0; i < iNMutations; i++) {
        
        int iPos = -1;
        if (pWELL == NULL) {
            iPos =  (int) ((2.0*iNumBits*rand())/(RAND_MAX+1.0));
        } else {
            iPos =  (int) (2.0*iNumBits*pWELL->wrandd());
        }
        
        uint iBit   = iPos%BITSINBLOCK;
        iPos /= BITSINBLOCK;
        
        ulong iNew= 1L << iBit;
        pGenome[iPos] ^= iNew;
        
    }
}


//----------------------------------------------------------------------------
// mutateNucs
//    make nucleotide-mutations in the array of nucleotides of the given size
//    - choose a nucleotide position (i.e. at an even number of bits)
//    - create an XOR mask randomly choswen from { 01, 10, 11}
//    - shift the mask to nucleotide position
//    - XOR nucleotide with mask
//   XOR magic: any nucleotide can change to any of the other three
//   01 : 00->01, 01->00, 10->11, 11->10
//   10 : 00->10, 01->11, 10->00, 11->01
//   11 : 00->11, 01->10, 10->01, 11, 00
// 
void GeneUtils::mutateNucs(ulong *pGenome, int iGenomeSize, int iNMutations, WELL512* pWELL = NULL) {
    int iNumBits = 2*iGenomeSize;
    for (int i = 0; i < iNMutations; i++) {
        int iPos = -1;
        int iMask = -1;

        if (pWELL == NULL) {
            // need an even starting position
            iPos =  2 * (int) ((0.5*iNumBits*rand())/(RAND_MAX+1.0));
            //        printf("Mutate @%d/%d[%d]\n", iPos, iNumBits, iA);
            // xor a nucleotide with (0,1), (1,0) or (1,1)
            iMask = 1+(int)((3.0*rand())/(RAND_MAX+1.0));
        } else {
            // need an even starting position
            iPos =  2 * (int) (0.5*iNumBits*pWELL->wrandd());
            //        printf("Mutate @%d/%d[%d]\n", iPos, iNumBits, iA);
            // xor a nucleotide with (0,1), (1,0) or (1,1)
            iMask = 1+(int)(3.0*pWELL->wrandd());

        }
        uint iBit   = iPos%BITSINBLOCK;
        iPos /= BITSINBLOCK;

        // the longs have an even number of bits, so we won't
        // cross a boundary
        ulong iNew= iMask << iBit;
        pGenome[iPos] ^= iNew;
    }
}


//----------------------------------------------------------------------------
// createFullRandomGenes
//   
ulong *GeneUtils::createFullRandomGenes(int iGenomeSize, WELL512* pWELL) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[2*iNumBlocks];
    if (pWELL == NULL) {
        for (int a = 0; a < 2; ++a) {
            for (int i = 0; i < iNumBlocks; ++i) {
                ulong l = rand();
                pGenome[a*iNumBlocks+i] = (l << 32)+rand();
            }
        }
    } else {
        for (int a = 0; a < 2; ++a) {
            for (int i = 0; i < iNumBlocks; ++i) {
                ulong l = pWELL->wrand();
                pGenome[a*iNumBlocks+i] = (l << 32) + pWELL->wrand();
            }
        }
    }

    return pGenome;
}

//----------------------------------------------------------------------------
// createRandomAlleles
//   
ulong *GeneUtils::createRandomAlleles(int iGenomeSize, int iNumMutations, WELL512* pWELL) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[2*iNumBlocks];
    // fill the first chromosome...
    if (pWELL == NULL) {
        for (int i = 0; i < iNumBlocks; ++i) {
            ulong l = rand();
            pGenome[i] = (l << 32)+rand();
        }
    } else {
        for (int i = 0; i < iNumBlocks; ++i) {
            ulong l = pWELL->wrand();
            pGenome[i] = (l << 32) + pWELL->wrand();
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
ulong *GeneUtils::createFlatRandomGenes(int iGenomeSize, int iNumber, ulong *pGenome0) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    ulong *pGenome = new ulong[2*iNumBlocks];
    if (pGenome0 != NULL) {
        memcpy(pGenome, pGenome0,  2*iNumBlocks*sizeof(ulong));
    } else {
        memset(pGenome, 0, 2*iNumBlocks*sizeof(ulong));
    }

    mutateNucs(pGenome, 2*iGenomeSize, iNumber);
    
    return pGenome;
}
    
//----------------------------------------------------------------------------
// copyMutatedGenes
//   
ulong *GeneUtils::copyMutatedGenes(int iGenomeSize, int iNumber, ulong *pGenome0, ulong *pGenome) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    if (pGenome0 != NULL) {
        memcpy(pGenome, pGenome0,  2*iNumBlocks*sizeof(ulong));
    } else {
        memset(pGenome, 0, 2*iNumBlocks*sizeof(ulong));
    }

    mutateNucs(pGenome, 2*iGenomeSize, iNumber);
    
    return pGenome;
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
ulong GeneUtils::makeMultiMask(std::vector<uint> vBreaks) {
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
void GeneUtils::crossOver(ulong *pGenome1, const ulong *pGenome2, int iGenomeSize, int iNumCrossOvers, WELL512* pWELL = NULL) {
    uint iNumBlocks = numNucs2Blocks(iGenomeSize);
    uint iNumBits = iNumBlocks*BITSINBLOCK;
    
    // determine crossover points (which bit)
    std::set<uint> vBreaks;
    if (pWELL == NULL) {
        for (int i = 0; i < iNumCrossOvers; i++) {
            vBreaks.insert(2*(int) ((0.5*iNumBits*rand())/(RAND_MAX+1.0)));
        }
    } else {
        for (int i = 0; i < iNumCrossOvers; i++) {
            vBreaks.insert(2*(int) (0.5*iNumBits*pWELL->wrandd()));
        }
    }

    // determine crossover points (which long (Block), which bit inside long)
    std::map<uint, std::vector<uint> > vBlockBreaks;
    std::set<uint>::const_iterator it;
    for (it = vBreaks.begin(); it != vBreaks.end(); it++) {
        uint iBlock = (*it)/BITSINBLOCK;
        vBlockBreaks[iBlock].push_back((*it)%BITSINBLOCK);
    }
    
    
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
//   creates a mask for random mixing (2-bit aligned pattern)
//
ulong makeFreeMask(WELL512* pWELL = NULL) {
    ulong lV = 0;
    ulong lM = 0x80000000;
    ulong lT = 0;
    if (pWELL != NULL) {
        lT = pWELL->wrand() & 0xffffffff;
    } else {
        lT = rand() & 0xffffffff;
    }
    while (lM > 0) {
        // a nucleotide (2 bits) must not be cut (i.e. either 00 or 11; but never 01 or 10)
        //lV = (lV<<1) + (lM & lT);
        //lV = (lV<<1) + (lM & lT);
        lV = (lM & lT) ? (lV<<2 | 0x00000003) : (lV<<2);
        lM >>= 1;
    }
    return lV;
}


//----------------------------------------------------------------------------
// freeReco
//   completely random mixture of the two chromosomes of pGenome2 into the 
//   two chromosomes of pGenome1
//
void GeneUtils::freeReco(ulong *pGenome1, ulong *pGenome2, int iNumBlocks, WELL512* pWELL = NULL) {
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
int GeneUtils::calcDist(ulong *pG1, ulong *pG2, int iGenomeSize) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    int iC = 2*iNumBlocks*GeneUtils::BITSINBLOCK;
    
    // first of G1 with first of G2
    int iC1 = 0;
    for (int k = 0; k < iNumBlocks; k++) {
        iC1 += bitcount2(pG1[k] ^ pG2[k]);
    }
    if (iC1 < iC) {
        iC = iC1;
    }
 
    // second of G1 with second of G2
    iC1 = 0;
    for (int k = iNumBlocks; k < 2*iNumBlocks; k++) {
        iC1 += bitcount2(pG1[k] ^ pG2[k]);
    }
    if (iC1 < iC) {
        iC = iC1;
    }
 
    //            memcpy(m_ulG1, m_ulGenome1[i]+iNumBlocks, iNumBlocks*sizeof(long));
    //            memcpy(m_ulG1+iNumBlocks, m_ulGenome1[i], iNumBlocks*sizeof(long));
    
    // second of G1 with frst of G2
    iC1 = 0;
    for (int k = 0; k < iNumBlocks; k++) {
        iC1 += bitcount2(pG1[k+iNumBlocks] ^ pG2[k]);
    }
    if (iC1 < iC) {
        iC = iC1;
    }
 
    // first of G1 with second of G2
    iC1 = 0;
    for (int k = 0; k < iNumBlocks; k++) {
        iC1 += bitcount2(pG1[k] ^ pG2[k+iNumBlocks]);
    }
    if (iC1 < iC) {
        iC = iC1;
    }
 
    return iC;   
}


//----------------------------------------------------------------------------
// showGenome
//
void GeneUtils::showGenome(ulong *pGenome, int iGenomeSize, int iWhat) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);

    static char NUC[] = {'A', 'C', 'G', 'T'};
    char s[NUCSINBLOCK+1];
    s[NUCSINBLOCK]='\0';

    if ((iWhat & SHOW_NUM) != 0) {
        for (int k = 0; k < 2; k++) {
            printf("    ");
            for (int j = 0; j < iNumBlocks; j++) {
                printf("%016lx ", pGenome[k*iNumBlocks+j]);
            }
            printf("\n");
        }
    }

    if ((iWhat & SHOW_NUC) != 0) {
        int iNumBits = 2*iGenomeSize;
        for (int k = 0; k < 2; k++) {
            printf("   |");
            int iC = 0;
            for (int j = 0; j < iNumBlocks; j++) {
                ulong ltemp = pGenome[k*iNumBlocks+j];
                uint i = 0;
                while ((iC < iNumBits) && (i < NUCSINBLOCK)) {
                    if (iC < iGenomeSize) {
                        int N = ltemp & 3;
                        ltemp >>= 2;
                        s[i] = NUC[N];
                    } else {
                        s[i] = '.';
                    }
                    i++;
                    iC++;
                }
                printf("%s|", s);
            }
            printf("\n");
        }
    }

}

//----------------------------------------------------------------------------
// plinkOrderNucleotides
//
void GeneUtils::plinkOrderNucleotides(ulong *pIn, ulong *pOut, int iGenomeSize) {
    int iNumBlocks = numNucs2Blocks(iGenomeSize);
    interleaveNucleotides(pIn, pIn+iNumBlocks, pOut, iGenomeSize);
}

//----------------------------------------------------------------------------
// interleaveNucleotides
//
void GeneUtils::interleaveNucleotides(ulong *pIn1, ulong *pIn2, ulong *pOut, int iGenomeSize) {
    ulong *p0 = pIn1;
    ulong *p1 = pIn2;
    int iNum = numNucs2Blocks(iGenomeSize);
    for (int i = 0; i < iNum; ++i) {
        int s = 0;
        ulong l0 = *p0;
        ulong l1 = *p1;
        ulong l  = 0;
    
        for (int j = 0; j < 32; j++) {
    
            l += (l0 & 0x3) << s;
            l0 >>= 2;
            s += 2;
    

    
            l += (l1 & 0x3) << s;
            l1 >>= 2;
            s += 2;
    
            
            if ((j %16) == 15) {
                *pOut = l;
                pOut++;
                l = 0;
            }
        }
        p0++;
        p1++;
    }
}


//----------------------------------------------------------------------------
// writePlinkHeader
//    we have F=0, M=1, plink has M=1, F=2; i.e. plink_gender = 2 - QHG_Gender
//
void GeneUtils::writePlinkHeader(FILE *fOut, const char*pLoc, int iNumber, idtype iID, idtype iMomID, idtype iDadID, int iGender) {
    fprintf(fOut, "%s_%03d % 10ld % 10ld % 10ld %d 1 ", pLoc, iNumber, iID, iDadID, iMomID, 2 - iGender);
}


//----------------------------------------------------------------------------
// writePlinkNucleotides
//
//
void GeneUtils::writePlinkNucleotides(FILE *fOut, const ulong *pGenome, int iGenomeSize) {
    int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
    int iNucPairsWritten = 0;
    const ulong *p1 = pGenome;
    const ulong *p2 = pGenome+iNumBlocks;
    for (int iB = 0; (iB < iNumBlocks) && (iNucPairsWritten < iGenomeSize); iB++) {
        char s1[GeneUtils::NUCSINBLOCK+1];
        s1[GeneUtils::NUCSINBLOCK] = '\0';
        GeneUtils::blockToNucStr(*p1, s1);
        char s2[GeneUtils::NUCSINBLOCK+1];
        s2[GeneUtils::NUCSINBLOCK] = '\0';
        GeneUtils::blockToNucStr(*p2, s2);
        char sr[4*GeneUtils::NUCSINBLOCK+1];
        
        int k = 0;
        for (uint j = 0; (j < GeneUtils::NUCSINBLOCK) && (iNucPairsWritten < iGenomeSize); j++) {
            sr[k++] = s1[j];
            sr[k++] = ' ';
            sr[k++] = s2[j];
            sr[k++] = ' ';
            iNucPairsWritten++;
        }
        sr[k] = '\0';
        fprintf(fOut, "%s", sr);
        p1++;
        p2++;
    }
    fprintf(fOut, "\n");
}


//----------------------------------------------------------------------------
// writePlinkMapFile
//
int GeneUtils::writePlinkMapFile(const char *pOut, int iGenomeSize) {
    int iResult = -1;
    FILE *fOut = fopen(pOut, "wt");
    if (fOut != NULL) {
        for (int i = 0; i < iGenomeSize; i++) {
            fprintf(fOut, "1 abc-%04d 0 %04d\n", i, i);
        }
        iResult = 0;
        fclose(fOut);
    } else {
        printf("Couldn't open [%s] for writing\n", pOut);
        iResult = -1;
    }
    return iResult;
}
