/*============================================================================
| GenomeCreator
| 
|  Creates a number of initial genome sequences according to various schemes:
|  - completely random               ("random")
|  - variants of a random            ("variants:<NumMutations>")
|  - all genes 0                     ("zero")
|  - hardy-wrinberg distribution     ("hw:<siterate>:<mutrate>")
|
|  GenomeCreator's template is usuallay GeneUtils or BitGeneUtils
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "strutils.h"
#include "GenomeCreator.h"

#define INIT_STATE_NONE     0
#define INIT_STATE_VARIANTS 1
#define INIT_STATE_RANDOM   2
#define INIT_STATE_ZERO     3
#define INIT_STATE_HW       4


#define INIT_NAME_NONE     "none"
#define INIT_NAME_VARIANTS "variants"
#define INIT_NAME_RANDOM   "random"
#define INIT_NAME_ZERO     "zero"
#define INIT_NAME_HW       "hw"

//-----------------------------------------------------------------------------
// constructor
//
template<class U>
GenomeCreator<U>::GenomeCreator(uint iNumParents)
    : m_iNumParents(iNumParents),
      m_iInitialMutations(0),
      m_dInitialMutRate(0),
      m_dInitialSiteRate(0),
      m_iInitialType(INIT_STATE_NONE) {
    
}


//-----------------------------------------------------------------------------
// determineInitData
//  parse the attrbut string for the initial genome.
//  Possible forms:
//    variants:<NumMutations>
//    random
//    zero
//    hw:<MutationRate>:<SiteFraction>
//  For a while, we will still "undestand" the old numeric codes
//
template<class U>
int GenomeCreator<U>::determineInitData(char *pLine) {
    int iResult = 0;
    int iIM = 0;
    // legacy - remove numeric soon
    if (strToNum(pLine, &iIM)) {
        if ((iIM > 0) || (iIM == -2)) {
            m_iInitialType      = INIT_STATE_VARIANTS;
            m_iInitialMutations = iIM;
            m_dInitialMutRate   = 0;
            m_dInitialSiteRate  = 0;
        } else {
            switch (iIM) {
            case -1:
                m_iInitialType      = INIT_STATE_RANDOM;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0;
                m_dInitialSiteRate  = 0;
                break;
            case -3:
                m_iInitialType      = INIT_STATE_ZERO;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0;
                m_dInitialSiteRate  = 0;
                break;
            case -4:
                m_iInitialType      = INIT_STATE_HW;
                m_iInitialMutations = 0;
                m_dInitialMutRate   = 0.05;
                m_dInitialSiteRate  = 0.1;
                break;
            default:
                iResult = -1;
            }
        }
    } else {
        // split
        iResult = -1;
        char *pL = strchr(pLine, '=');
        if (pL != NULL) {
            pLine = trim(pL+1);
        }
        char *p0 = strtok(pLine, ":");
	
        if (strcasecmp(p0, INIT_NAME_VARIANTS) == 0) {
            int    iN = 0;
            char *p2 = strtok(NULL, ":");
            if (p2 != NULL) {
                if (strToNum(p2, &iN)) {
                    m_iInitialType      = INIT_STATE_VARIANTS;
                    m_iInitialMutations = iN;
                    m_dInitialMutRate   = 0;
                    m_dInitialSiteRate  = 0;
                    iResult = 0;
                }
            } else {
		printf("expected [%s:<num_mutations>]\n", INIT_NAME_VARIANTS);
            }
        } else if  (strcasecmp(p0, INIT_NAME_RANDOM) == 0) {
            m_iInitialType      = INIT_STATE_RANDOM;
            m_iInitialMutations = 0;
            m_dInitialMutRate   = 0;
            m_dInitialSiteRate  = 0;
            iResult = 0;
         

        } else if  (strcasecmp(p0, INIT_NAME_ZERO) == 0) {
            m_iInitialType      = INIT_STATE_ZERO;
            m_iInitialMutations = 0;
            m_dInitialMutRate   = 0;
            m_dInitialSiteRate  = 0;
            iResult = 0;

        } else if  (strcasecmp(p0, INIT_NAME_HW) == 0) {
            double dR = 0;
            double dS = 0;
            char *p1 = strtok(NULL, ":");
            char *p2 = strtok(NULL, ":");
            if ((p1 != NULL) && (p2 != NULL)) {
                if (strToNum(p1, &dR)) {
                    if (strToNum(p2, &dS)) {
                        m_iInitialType      = INIT_STATE_HW;
                        m_iInitialMutations = 0;
                        m_dInitialMutRate   = dR;
                        m_dInitialSiteRate  = dS;
                        iResult = 0;
                    } else {
		        printf("Not a number: [%s]\n", p2);
		    }
                } else {
	            printf("Not a number: [%s]\n", p1);
                }
	    } else {
	        printf("expected [%s:<mut_rate>:<site_rate>]\n", INIT_NAME_HW);
            }
        }
    }
    if (iResult == 0) {
        buildInitString();
        //        printf("Successfully parsed and rebuilt initial genome info:[%s]\n", m_sInitString.c_str());
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// buildInitString
//   build a string for output in a QDF file
//
template<class U>
void GenomeCreator<U>::buildInitString() {
    char *p1 = NULL;

    switch (m_iInitialType) {
    case INIT_STATE_NONE:
        m_sInitString = INIT_NAME_NONE;
        break;
    
    case INIT_STATE_VARIANTS:
        p1 = new char[strlen(INIT_NAME_VARIANTS)+1+32+1];
        sprintf(p1, "%s:%d", INIT_NAME_VARIANTS, m_iInitialMutations);
        m_sInitString = p1;
        break;
    
    case INIT_STATE_RANDOM:
        m_sInitString = INIT_NAME_RANDOM;
        break;
    
    case INIT_STATE_ZERO:
        m_sInitString = INIT_NAME_ZERO;
        break;
    
    case INIT_STATE_HW:
        p1 = new char[strlen(INIT_NAME_HW)+1+10+1+10+1];
        sprintf(p1, "%s:%4.2e:%4.2e", INIT_NAME_HW, m_dInitialMutRate, m_dInitialSiteRate);
        m_sInitString = p1;
        break;

    default:
        m_sInitString = INIT_NAME_NONE;
    }

    if (p1 != NULL) {
        delete[] p1;
    }
}


//-----------------------------------------------------------------------------
// createInitialGenomes
//   only do this if the buffer has been already added to the controller
//
template<class U>
int GenomeCreator<U>::createInitialGenomes(int iGenomeSize, int iNumGenomes, LayerArrBuf<ulong> &aGenome, WELL512 **apWELL) {
    int iResult = 0;
    printf("initial type: %d\n", m_iInitialType);
            
    ulong *pG0;
    ulong *pGenome;
    int iNumBlocks  = U::numNucs2Blocks(iGenomeSize);

    switch (m_iInitialType) {
    case INIT_STATE_NONE:
        // nothing to do
        break;

    case INIT_STATE_VARIANTS: 
        if (m_iInitialMutations > 0) {
            printf("Creating %d variants with %d mutations\n", iNumGenomes, m_iInitialMutations);
            pG0 = U::createRandomAlleles(m_iNumParents, iGenomeSize, m_iInitialMutations, apWELL[omp_get_thread_num()]);
            for (int i = 0; i < iNumGenomes; i++) {
                // go to position for next genome
                pGenome = &(aGenome[i]); 
                U::copyAndMutateGenes(m_iNumParents, iGenomeSize, m_iInitialMutations, pG0, pGenome);
            }
            delete[] pG0;
        }
        break;

    case INIT_STATE_RANDOM: 
        printf("Creating %d totally random genes\n", iNumGenomes);
        for (int i = 0; i < iNumGenomes; i++) {
            pGenome = &(aGenome[i]); 
            pG0 = U::createFullRandomGenes(m_iNumParents, iGenomeSize,  apWELL[omp_get_thread_num()]);
            memcpy(pGenome, pG0, m_iNumParents*iNumBlocks*sizeof(ulong));
            delete[] pG0;
        }
        break;


    case INIT_STATE_ZERO: 
        printf("Creating %d flat zeroed genes\n", iNumGenomes);
        for (int i = 0; i < iNumGenomes; i++) {
            pGenome = &(aGenome[i]); 
            memset(pGenome, 0, m_iNumParents*iNumBlocks*sizeof(ulong));
        }
            
        break;

    case INIT_STATE_HW: 
        printf("Creating %d genes at Hardy-Weinberg equilibrium, site ratio %4.2e, mutation rate %4.2e\n", iNumGenomes, m_dInitialSiteRate, m_dInitialMutRate);
        if ((m_dInitialSiteRate > 0) && (m_dInitialMutRate > 0)) {
            for (int i = 0; i < iNumGenomes; i++) {
                ulong *pGenome1 = &(aGenome[i]); 
                memset(pGenome1, 0, m_iNumParents*iNumBlocks*sizeof(ulong));
            }
            std::vector<uint> vMutSites;
            ulong   iMask = (1<<U::BITSINNUC)-1;
            for (int i = 0; i < iGenomeSize; i++) {
                if (apWELL[omp_get_thread_num()]->wrandd() < m_dInitialSiteRate) {
                    uint iBlock = i/(U::NUCSINBLOCK);
                    uint iBit   = U::BITSINNUC*(i%U::NUCSINBLOCK);
                    for (uint j = 0; j < m_iNumParents*iNumGenomes; j++) {
                        if (apWELL[omp_get_thread_num()]->wrandd() < m_dInitialMutRate) {
                            int iGenome = j/m_iNumParents;
                            int iAllele = j%m_iNumParents;
                            ulong *pGenome2 = &(aGenome[iGenome]); 
                                    
                            pGenome2[iBlock+iAllele*iNumBlocks] |= iMask << iBit;
                        }
                    }
                }
            }
        } else {
        }
        break;
        
    default:
        printf("Bad initialType [%d]\n", m_iInitialType);
        iResult = -1;
                
    }

    return iResult;
}

