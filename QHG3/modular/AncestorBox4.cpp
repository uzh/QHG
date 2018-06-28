#include <string.h>
#include <vector>
#include <omp.h>

#include "QDFUtils.h"
#include "LayerArrBuf.h"
#include "LayerArrBuf.cpp"
#include "LBController.h"

#include "AncestorBox4.h"

static const uint  BUFSIZE_READ_ADDITIONAL=2048;

#define ANCESTOR_DATASET_NAME   "Ancestors"
#define ANCESTOR_LINE_LAYERSIZE "LAYERSIZE"

//----------------------------------------------------------------------------
// createInstance
//   Creates an instance
//
AncestorBox4 *AncestorBox4::createInstance(uint iLayerSize, const char *pOutputFile) {
    AncestorBox4 *pAB = new AncestorBox4();
    int iResult = pAB->init(iLayerSize, pOutputFile);
    if (iResult != 0) {
        delete pAB;
        pAB = NULL;
    }
    return pAB;
}


//----------------------------------------------------------------------------
// constructor
//
AncestorBox4::AncestorBox4() 
    : m_iLayerSize(0),
      m_pLBC(NULL),
      m_aAncestors(NULL),
      m_fOut(NULL) {
}


//----------------------------------------------------------------------------
// init
//  sets layersize and creates layered buffer and controller
//  opens file
//
int AncestorBox4::init(uint iLayerSize, const char *pOutputFile) {
    int iResult = 0;

    m_iLayerSize = iLayerSize;

#ifdef OMP_A
#pragma omp parallel 
#endif
{
    m_iNumThreads = omp_get_max_threads();
}
    m_pLBC = new LBController*[m_iNumThreads];
    m_aAncestors = new LayerArrBuf<idtype>*[m_iNumThreads];

#ifdef OMP_A
#pragma omp parallel 
#endif
{
    int iT = omp_get_thread_num();
    m_pLBC[iT] = new LBController();
    m_aAncestors[iT] = new LayerArrBuf<idtype>();
   
    m_aAncestors[iT]->init(iLayerSize, ANCESTOR4_BLOCK_SIZE);
    m_pLBC[iT]->init(iLayerSize);
    m_pLBC[iT]->addBuffer(static_cast<LBBase *>(m_aAncestors[iT]));
}

    if (pOutputFile != NULL) {
        m_fOut = fopen(pOutputFile, "ab");
        if (m_fOut != NULL) {
            printf("Opened output file [%s] OK: %p\n", pOutputFile, m_fOut);
        } else {
            printf("Couldn't open output file [%s]\n", pOutputFile);
            iResult = -1;
        }
    }

#ifdef OMP_A
#pragma omp parallel 
#endif
{
    int iT = omp_get_thread_num();
    printf("Listcheck in init (%d)\n", iT);
    m_pLBC[iT]->checkLists();
}

    return iResult;
}

//----------------------------------------------------------------------------
// setOutputFile
//  set name of output file
//
int AncestorBox4::setOutputFile(char *pOutputFile, bool bAppend) {
    int iResult = 0;
    if (m_fOut != NULL) {
        fclose(m_fOut);
    }
    if (bAppend) {
        m_fOut = fopen(pOutputFile, "ab");
    } else {
        m_fOut = fopen(pOutputFile, "wb");
    }
    if (m_fOut != NULL) {
        printf("[AncestorBox4::setOutputFile] Opened output file [%s] OK: %p\n", pOutputFile, m_fOut);
        iResult = 0;
    } else {
        printf("[AncestorBox4::setOutputFile] Couldn't open output file [%s]\n", pOutputFile);
        iResult = -1;
    }   
    return iResult;
}



//----------------------------------------------------------------------------
// destructor
//
AncestorBox4::~AncestorBox4() {
    if (m_pLBC != NULL) {
        for (int i = 0; i < m_iNumThreads; ++i) {
            delete m_pLBC[i];
        }
        delete[] m_pLBC;
    }
    if (m_aAncestors != NULL) {
        for (int i = 0; i < m_iNumThreads; ++i) {
            delete m_aAncestors[i];
        }
        delete[] m_aAncestors;
    }

    if (m_fOut != NULL) {
        fclose(m_fOut);
    }

    
}


//----------------------------------------------------------------------------
// addBaby
//   adds Baby ID with parent IDs to m_aAncestors
//   time and iCellID are actually 32bit values
//   which we place into a single long
//   we assume that fTime has a value in the int range
//
void AncestorBox4::addBaby(idtype iBabyID, idtype iMotherID, idtype iFatherID, float fTime, gridtype iCellID) {
    int iT = omp_get_thread_num();
    // get free index
    int iFreeIndex = m_pLBC[iT]->getFreeIndex();

//    printf("thread %d got free index %d for baby %d %d %d\n",iT,iFreeIndex,iBabyID,iMotherID,iFatherID);

    idtype *pBabyBuf =  &((*(m_aAncestors[iT]))[iFreeIndex]);

    //    printf("have time %08x and cell %08x\n", (int)fTime, iCellID);
    idtype spacetime = ((long)fTime << 32) + iCellID;

    *pBabyBuf++ = iBabyID;
    *pBabyBuf++ = iMotherID;
    *pBabyBuf++ = iFatherID;
    *pBabyBuf++ = spacetime;
    

}


//----------------------------------------------------------------------------
// writeDataQDF
//  write ancestor array to the group
//  Here: create a new Dataset for the ancestors.
//  Write genomes sequentially from all cells.
// 
//  Go through all cells, and use the BufLayers of m_aGenome as slabs 
//
int AncestorBox4::writeData() {
    printf("AncestorBox4 writing data\n");
    int iResult = 0;
    for (int i = 0; (iResult == 0) && (i < m_iNumThreads); i++) {
        m_pLBC[i]->compactData();
        for (uint j = 0; (iResult == 0) && (j < m_aAncestors[i]->getNumUsedLayers()); j++) {
            const idtype* pSlab = m_aAncestors[i]->getLayer(j);
            ulong iNum = m_pLBC[i]->getNumUsed(j)*ANCESTOR4_BLOCK_SIZE;
            ulong iWritten = fwrite(pSlab, sizeof(idtype), iNum, m_fOut);
            if (iWritten != iNum) {
                printf("Only wrote %ld instead of %ld\n", iWritten, iNum);
                iResult = -1;
            }
        }
        m_pLBC[i]->clear();
    }
    fflush(m_fOut);
    return iResult;
}



//----------------------------------------------------------------------------
// copyBlock
//   copy ancestors for iCount agents from pData to position iIndex
//   
int AncestorBox4::copyBlock(int iIndex, idtype *pData, int iCount) {
    int iT = 0;
    return m_aAncestors[iT]->copyBlock(iIndex, pData, iCount);
}


