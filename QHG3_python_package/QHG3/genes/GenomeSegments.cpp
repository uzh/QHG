#include <stdio.h>
#include <string.h>
#include <math.h>

#include <map>
#include "types.h"
#include "GeneUtils.h"

#include "GenomeSegments.h"


//---------- segment --------------
segment:: segment(uint iStartNuc0, uint iLengthNuc0, uint iFirstBlock0) 
        : m_iStartNuc(iStartNuc0), 
          m_iLengthNuc(iLengthNuc0), 
          m_iFirstBlock(iFirstBlock0),
          m_iMaskLength(0),
          m_pMasks(NULL) {
};

segment::~segment() {
    if (m_pMasks != NULL) {
        delete[] m_pMasks;
    }
}

//----------------------------------------------------------------------------
// createMask
//
int segment::createMask() {
    int iResult = 0;
    m_iMaskLength = GeneUtils::numNucs2Blocks(m_iStartNuc + m_iLengthNuc);
    //   printf("Have %d mask blocks\n", m_iMaskLength);
    m_pMasks = new ulong[m_iMaskLength];
    memset(m_pMasks, 0xff, m_iMaskLength*sizeof(ulong));

    if (m_iStartNuc > 0) {
        // fix mask at the beginning
        ulong um = 0xffffffffffffffff;
        //        printf("Shifting mask for first block [0] by %2d bits\n", m_iStartNuc*2);
        um <<= m_iStartNuc*2;
        m_pMasks[0] &= um;
    }
    uint iLastBit = (m_iStartNuc + m_iLengthNuc) % GeneUtils::NUCSINBLOCK;
    if (iLastBit < GeneUtils::NUCSINBLOCK) {
        // fix mask at the end
        ulong um = 0xffffffffffffffff;
        //        printf("Shifting mask for last  block [%d] by %d bits\n", m_iMaskLength-1, (GeneUtils::NUCSINBLOCK - iLastBit)*2);
        um >>= (GeneUtils::NUCSINBLOCK - iLastBit)*2;
        m_pMasks[m_iMaskLength-1] &= um;
    }
    //    showMask();
    return iResult;
}

//----------------------------------------------------------------------------
// showMasks
//
void segment::showMask() {
    printf("Start %3d (%3d); L %3d; NB %d : ", 
           m_iStartNuc,  
           m_iFirstBlock*GeneUtils::NUCSINBLOCK + m_iStartNuc, 
           m_iLengthNuc,
           m_iMaskLength);
    for (uint j = 0; j < m_iMaskLength; j++) {
        char pNuc[33];
        pNuc[32] = '\0';
        printf("%s ", GeneUtils::blockToNucStr(m_pMasks[j], pNuc));
    }
    printf("\n");
}


//---------- GenomeSegment --------------

//----------------------------------------------------------------------------
// createInstance
//
GenomeSegments *GenomeSegments::createInstance(uint iGenomeSize, uint iNumSegments) {
    GenomeSegments *pGS = NULL;
    if (iGenomeSize > 0) {
        pGS = new GenomeSegments(iGenomeSize);
        int iResult = pGS->init(iNumSegments, true);  // true: equalize sizes
        if (iResult != 0) {
            delete pGS;
            pGS = NULL;
        }
    } else {
        printf("GenomeSize\n");
    }
    return pGS;
}

//----------------------------------------------------------------------------
// constructor
//
GenomeSegments::GenomeSegments(uint iGenomeSize)
    : m_iGenomeSize(iGenomeSize),
      m_iNumBlocks(0),
      m_pWorkspace(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
GenomeSegments::~GenomeSegments() {
    if (m_pWorkspace != NULL) {
        delete[] m_pWorkspace;
    }
    for (uint i = 0; i < m_vSegments.size(); i++) {
        delete m_vSegments[i];
    }
    m_vSegments.clear();

}

//----------------------------------------------------------------------------
// init
//
int GenomeSegments::init(uint iNumSegments, bool bEqualize) {
    int iResult = 0;

    m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
    m_pWorkspace = new ulong[2*m_iNumBlocks];
    memset(m_pWorkspace, 0, 2*m_iNumBlocks*sizeof(ulong));

    if (iNumSegments > 0) {
        //regular splitting (N-1  equal sized; 1 possibly smaller)
        double d = (1.0*m_iGenomeSize)/iNumSegments;
        uint iSegSize = floor(d);
        if (bEqualize) {
            d -= iSegSize;
        } else {
            d = 0;
        }
        double dAdd = d;
        uint iCurStartNuc = 0;
        while ((iResult == 0) && (iCurStartNuc < m_iGenomeSize)) {
            //@@            printf("Segment: S %d, L %d, add %f,L' %d\n", iCurStartNuc, iSegSize, dAdd, (int)(iSegSize+dAdd));
            iResult = addSegment(iCurStartNuc, iSegSize+dAdd);
            iCurStartNuc += iSegSize+dAdd;
            if (dAdd > 1) {
                dAdd -=1;
            }
            dAdd += d;
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
// addSegment
//
int GenomeSegments::addSegment(uint iStartNuc, int iLengthNuc) {
    int iResult = 0;
    
    if (iStartNuc + iLengthNuc >=  m_iGenomeSize) {
        iLengthNuc =  m_iGenomeSize - iStartNuc;
    }

    for (uint i = 0; (iResult == 0) && (i < m_vSegments.size()); i++) {
        segment *ps = m_vSegments[i];
        uint iOffset = ps->m_iFirstBlock*GeneUtils::NUCSINBLOCK;
        if (((iOffset+ps->m_iStartNuc < iStartNuc) && 
             (iStartNuc < (iOffset+ps->m_iStartNuc+ps->m_iLengthNuc))) ||
             ((iStartNuc < iOffset+ps->m_iStartNuc) &&  
              (ps->m_iStartNuc < (iOffset+iStartNuc+iLengthNuc)))) {
            iResult = -1;
            printf("Overlap [%d,%d] with segment %d[%d+%d,%d]\n", iStartNuc, iLengthNuc, i, ps->m_iStartNuc, iOffset, ps->m_iLengthNuc);
        }
    }
    if (iResult == 0) {
        uint iFirstBlock = (iStartNuc/GeneUtils::NUCSINBLOCK);
        iStartNuc -= iFirstBlock*GeneUtils::NUCSINBLOCK;
        segment *ps = new segment(iStartNuc,iLengthNuc, iFirstBlock);
        iResult = ps->createMask();
        if (iResult == 0) {
            m_vSegments.push_back(ps);
        } else {
            delete ps;
        }
    }       
    return iResult;
}



//----------------------------------------------------------------------------
// getSegmentSize
//
int GenomeSegments::getSegmentSize(uint i) {
    int iSize = -1;
    if (i < m_vSegments.size()) {
        iSize = m_vSegments[i]->m_iLengthNuc;
    }
    return iSize;
}

//----------------------------------------------------------------------------
// getFirstBlock
//
int GenomeSegments::getFirstBlock(uint i) {
    int iPos = -1;
    if (i < m_vSegments.size()) {
        iPos = m_vSegments[i]->m_iFirstBlock;
    }
    return iPos;
}

//----------------------------------------------------------------------------
// getMaskLength
//
int GenomeSegments::getMaskLength(uint i) {
    int iSize = -1;
    if (i < m_vSegments.size()) {
        iSize = m_vSegments[i]->m_iMaskLength;
    }
    return iSize;
}



//----------------------------------------------------------------------------
// getMaskedSegment
//   
ulong *GenomeSegments::getMaskedSegment(ulong *pFullGenome, int iSegmentNumber) {
    ulong *pMaskRegion = NULL;
    if ((uint)iSegmentNumber < m_vSegments.size()) {
        segment *ps = m_vSegments[iSegmentNumber];
        uint iFirstBlock = ps->m_iFirstBlock;
        uint iNumBlocks = ps->m_iMaskLength;
        ulong *pMask = ps->m_pMasks;

        memcpy(m_pWorkspace + iFirstBlock, pFullGenome + iFirstBlock, iNumBlocks*sizeof(ulong)); 
        for (uint i = 0; i < iNumBlocks; i++) {
            m_pWorkspace[iFirstBlock+i] &= pMask[i];
        }
        pMaskRegion = m_pWorkspace+iFirstBlock;
    }
    return pMaskRegion;
}

//----------------------------------------------------------------------------
// showMasks
//
void GenomeSegments::showMasks() {

    for (uint i = 0; i < m_vSegments.size(); i++) {
        segment *ps = m_vSegments[i];
        ps->showMask();
    }
}


//----------------------------------------------------------------------------
// findSegmentForNuc
//
int GenomeSegments::findSegmentForNuc(uint iNucPos) {
    int iResult = -1;
    for (uint i = 0; (iResult < 0) && (i < m_vSegments.size()); i++) {
        segment *ps = m_vSegments[i];
        uint iOffset = ps->m_iFirstBlock*GeneUtils::NUCSINBLOCK;
        if ((iOffset+ps->m_iStartNuc < iStartNuc) && 
            (iStartNuc < (iOffset+ps->m_iStartNuc+ps->m_iLengthNuc))) {
            iResult = i;
        }
    }
}
