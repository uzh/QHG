#include <stdio.h>
#include <string.h>

#include <algorithm>

#include <openssl/md5.h>

#include "types.h"
#include "GeneUtils.h"
#include "ProteinBuilder.h"

#define UMASK 0x3
#define USHIFT 2

#define NUC_NUL         0xff
#define NUC_END_TOTAL   0xfe
#define NUC_END_ALLELE  0xfd

#define NUL_INDEX        64
#define END_TOTAL_INDEX  65
#define END_ALLELE_INDEX 66

//----------------------------------------------------------------------------
// createInstance
//
ProteinBuilder *ProteinBuilder::createInstance(const ulong *pGenome, int iGenomeSize) {
    ProteinBuilder *pPB = new ProteinBuilder();
    int iResult = pPB->init(pGenome, iGenomeSize);
    if (iResult != 0) {
        delete pPB;
        pPB = NULL;
    }
    return pPB;
}


//----------------------------------------------------------------------------
// constructor
//
ProteinBuilder::ProteinBuilder() 
    : m_iCurPos(-1),
      m_iCurBlockIndex(-1),
      m_lCurBlock(0),
      m_iCurAllele(0),
      m_iNumBlocks(-1),
      m_pGenome(NULL),
      m_iGenomeSize(0) {

}


//----------------------------------------------------------------------------
// destructor
//
ProteinBuilder::~ProteinBuilder() {
    
    for (uint i = 0; i < m_vAAHash.size(); i++) {
        if (m_vAAHash[i] != NULL) {
            delete[] m_vAAHash[i];
        }
    }
    
}


//----------------------------------------------------------------------------
// init
//
int ProteinBuilder::init(const ulong *pGenome, int iGenomeSize) {
    int iResult = -1;
    if ((pGenome != NULL) && (iGenomeSize > 3)) {
        m_pGenome        = pGenome;
        m_iGenomeSize    = iGenomeSize;
        m_iNumBlocks     = GeneUtils::numNucs2Blocks(m_iGenomeSize);

        // prepare for first allele
        m_iCurAllele     =  0;
        m_iCurPos        = -1;
        m_iCurBlockIndex = -1;
        m_lCurBlock      = pGenome[0];

        m_vProteome.clear();
        m_vAll[0].clear();
        m_vAll[1].clear();
        iResult = 0;
    } else {
        if (pGenome == NULL) {
            printf("ProteinBuilder iinitialized with NULL-genome\n");
        } else {
            printf("ProteinBuilder iinitialized with too short genome (%d)\n",  m_iGenomeSize);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// translateGenome
//
int ProteinBuilder::translateGenome() {
    int iResult = -1;

    uchar    aaNumber = AA_NUL;
    bool     bBuilding = false;
    protein  vCurProtein;

    while (aaNumber != AA_END_TOTAL) {

        // ignore non-coding parts
        while ((!bBuilding) && (aaNumber != AA_END_TOTAL)) {
            int aaIndex =  getNextAA();
            aaNumber =  aTranslationTable[aaIndex];
            // if Met is found, we start coding
            if (aaNumber == AA_STR) {
                bBuilding = true;
                vCurProtein.clear();
                //@@                printf("Start\n");
            }

            if (aaIndex < 64) {
                m_vAll[m_iCurAllele].push_back(std::pair<int,int>(aaIndex, aaNumber));
            }

        }

        while ((bBuilding) && (aaNumber != AA_END_TOTAL)) {
            int aaIndex =  getNextAA();
            aaNumber =  aTranslationTable[aaIndex];
            // check for stop, end of allele, or end of everything
            if ((aaNumber == AA_STP) || (aaNumber == AA_END_TOTAL) || (aaNumber == AA_END_ALLELE)) {
                protein vProteinCopy;
                vProteinCopy.insert(vProteinCopy.begin(), vCurProtein.begin(), vCurProtein.end());
                m_vProteome.push_back(vProteinCopy);
                bBuilding = false;
                //@@                printf("Stop\n");
            } else {
                // no stop - add amino acid
                vCurProtein.push_back(aaNumber);
            }

            //            printf("index %d\n", aaIndex);
            if (aaIndex < 64) {
                m_vAll[m_iCurAllele].push_back(std::pair<int,int>(aaIndex, aaNumber));
            }

        }
    }
    
    if (m_vProteome.size() > 0) {
        iResult = 0;
    } else {
        //printf("No proteins\n");
        iResult = 0;
    }
    return iResult;
}

bool hashcomp (ulong *lhs, ulong *rhs) {return (lhs[0] < rhs[0]) || ((lhs[0] == rhs[0]) && (lhs[1] < rhs[1]));};

//----------------------------------------------------------------------------
// translateGenomeHash
//
int ProteinBuilder::translateGenomeHash() {
    int iResult = -1;

    uchar    aaNumber = AA_NUL;
    bool     bBuilding = false;
    int iNumAA = 0;
    MD5_CTX c;
    uchar *aBufAA = new uchar[m_iGenomeSize];
    
    uchar aTempHash[MD5_SIZE];
    ulong *aCurHash = NULL;
    
    while (aaNumber != AA_END_TOTAL) {

        // ignore non-coding parts
        while ((!bBuilding) && (aaNumber != AA_END_TOTAL)) {
            int aaIndex =  getNextAA();
            aaNumber =  aTranslationTable[aaIndex];
            // if Met is found, we start coding
            if (aaNumber == AA_STR) {
                bBuilding = true; 
                MD5_Init(&c);
                iNumAA = 0;
                aCurHash = new ulong[MD5_SIZE/sizeof(ulong)];
                
            }

        }

        while ((bBuilding) && (aaNumber != AA_END_TOTAL)) {
            int aaIndex =  getNextAA();
            aaNumber =  aTranslationTable[aaIndex];
            // check for stop, end of allele, or end of everything
            if ((aaNumber == AA_STP) || (aaNumber == AA_END_TOTAL) || (aaNumber == AA_END_ALLELE)) {
                MD5_Update(&c, aBufAA, iNumAA);
                MD5_Final(aTempHash, &c);
                memcpy(aCurHash, aTempHash, MD5_SIZE*sizeof(uchar));
                m_vAAHash.push_back(aCurHash);
                bBuilding = false;
                //@@                printf("Stop\n");
            } else {
                // no stop - add amino acid
                aBufAA[iNumAA++] = aaNumber;
            }

        }
    }
    delete[] aBufAA;
    
    if (m_vAAHash.size() > 0) {
        iResult = 0;
        std::sort(m_vAAHash.begin(), m_vAAHash.end());

    } else {
        //printf("No proteins\n");
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getNextNuc
//   
uchar ProteinBuilder::getNextNuc() {
    // remember current nucleotide
    uchar uResult = NUC_NUL;

    m_iCurPos++;

    if (((m_iCurPos % GeneUtils::NUCSINBLOCK) == 0) || (m_iCurPos >= m_iGenomeSize)){
        if (m_iCurPos < m_iGenomeSize) {
            // inside allele - load new block and continue as normal
            m_iCurBlockIndex++;
            m_lCurBlock  = m_pGenome[m_iCurAllele*m_iNumBlocks+m_iCurBlockIndex];

        } else {
            if (m_iCurAllele < 1) {
                // finished first allele - prepare for next allele anc signal total end
                uResult = NUC_END_ALLELE;
                m_iCurAllele++;
                m_iCurPos        = -1;
                m_iCurBlockIndex = -1;

            } else {
                // finished second allele - signal total end
                uResult = NUC_END_TOTAL;
                //                printf("end total\n");
            }
        }
    }

    if (uResult == NUC_NUL) {
        uResult = m_lCurBlock & UMASK;
        m_lCurBlock >>= USHIFT;
    }
    return uResult;
}


//----------------------------------------------------------------------------
// getNextAA
//
uchar ProteinBuilder::getNextAA() {

    uchar uIndex = 0;
    bool bOK = true;

    for (int i = 0; bOK && (i < 3); i++) {
        uchar uNuc = getNextNuc();
        if (uNuc == NUC_END_TOTAL) {
            uIndex = END_TOTAL_INDEX;
            bOK = false;
        } else if (uNuc == NUC_END_ALLELE) {
            uIndex = END_ALLELE_INDEX;
            bOK = false;
        } else {
            //                        printf("%c", aNucleotides[uNuc]);
            uIndex = (uIndex << USHIFT) + uNuc;
        }
    }
    
    /*
    if (uIndex != END_TOTAL_INDEX) {
        printf(" %02d -> %02d %s", uIndex, aTranslationTable[uIndex], aAminoAcids[aTranslationTable[uIndex]][1]);fflush(stdout);
    }
    printf("\n");
    */
    return uIndex;
}


//----------------------------------------------------------------------------
// getAAName
//
const char *ProteinBuilder::getAAName(int iAAIndex) {
    const char *pName = "???";
    if ((iAAIndex >= 0) && (iAAIndex <= 21)) {
        pName = aAminoAcids[iAAIndex][1];
    }
    return pName;
}


//----------------------------------------------------------------------------
// getAASymbol
//
char ProteinBuilder::getAASymbol(int iAAIndex) {
    char cSym = '?';
    if ((iAAIndex >= 0) && (iAAIndex <= 21)) {
        cSym = aAminoAcids[iAAIndex][0][0];
    }
    return cSym;;
}


//----------------------------------------------------------------------------
// displaySequence
//
void ProteinBuilder::displaySequence() {
    for (int i = 0; i < 2; i++) {
        printf("    |");
        for (uint j = 0; j < m_vAll[i].size(); j++) {
            int iIndex = m_vAll[i][j].first;
            int n1 = iIndex / 16;
            iIndex -= (n1*16);
            int n2 = iIndex / 4;
            iIndex -= (n2*4);
            int n3 = iIndex;

            printf("%c%c%c|", aNucleotides[n1], aNucleotides[n2], aNucleotides[n3]);
        }
        printf("\n");

        printf("    |");
        for (uint j = 0; j < m_vAll[i].size(); j++) {
            int iNumber = m_vAll[i][j].second;

            printf("%s|", aAminoAcids[iNumber][1]);
        }
        printf("\n");
        
        if (i == 0) {
            printf("    +");
            for (uint j = 0; j < m_vAll[i].size(); j++) {
                printf("---+");
            }
            printf("\n");
        }
    }
        
}

