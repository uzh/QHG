#include <stdio.h>
#include <mpi.h>
#include <vector>
#include "LifeTile.h"
#include "LifeBoard.h"



//----------------------------------------------------------------------------
// createInstance
//   create a board for a game-of-life tile
//   iID     tile id
//   iNX     number of tiles in x direction
//   iNY     number of tiles in y direction
//   iW      width  of tiles
//   iH      height of tiles
//   iHalo   halo size
//
LifeBoard *LifeBoard::createInstance(int iID, int iNX, int iNY, int iW, int iH, int iHalo) {
    LifeBoard *pLB = new LifeBoard(iID, iNX, iNY, iW, iH, iHalo);
    int iResult = pLB->init();
    if (iResult != 0) {
        delete pLB;
        pLB = NULL;
    }
    return pLB;
}


//----------------------------------------------------------------------------
// constructor
//
LifeBoard::LifeBoard(int iID, int iNX, int iNY, int iW, int iH, int iHalo) 
    : m_iID(iID),
      m_iNX(iNX),
      m_iNY(iNY),
      m_iW(iW),
      m_iH(iH),
      m_iHalo(iHalo),
      m_pLifeTile(NULL),
      m_pMM(NULL) {
}


//----------------------------------------------------------------------------
// destructor
//
LifeBoard::~LifeBoard() {
    if (m_pLifeTile != NULL) {
        delete m_pLifeTile;
    }
    if (m_pMM != NULL) {
        delete m_pMM;
    }
}

 

//----------------------------------------------------------------------------
// init
//  creates links, obtains a MPIIMult object, and creates a LifeTile.
//
int LifeBoard::init() {
    int iResult = 0;

    iResult = createLinks();
    if (iResult == 0) {
        m_pMM = MPIMulti::createInstance(m_iID, m_vOutLinks, m_vOutLinks, m_vTags);
        if (m_pMM != NULL) {
            m_pLifeTile = LifeTile::createInstance(m_iID, m_iW, m_iH, m_iHalo);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createLinks
//   "manual" creation of link information
//   if necessary, tags are created
//
int LifeBoard::createLinks() {
    int iResult = 0;
   
    m_vOutLinks.clear();
    m_vInLinks.clear();

    int iY0 = m_iID / m_iNX;
    int iX0 = m_iID % m_iNX;
    
    int iXEast  = (iX0 + 1)%m_iNX;
    int iXWest  = (iX0 + m_iNX - 1)%m_iNX;
    int iYNorth = (iY0 + m_iNY -1)%m_iNY;
    int iYSouth = (iY0 + 1)%m_iNY;
    
    // find the outward links
    // east
    m_vOutLinks.push_back(makeID(iY0, iXEast));
    // northeast
    m_vOutLinks.push_back(makeID(iYNorth, iXEast));
    // north
    m_vOutLinks.push_back(makeID(iYNorth, iX0));
    // north west
    m_vOutLinks.push_back(makeID(iYNorth, iXWest));
    // west
    m_vOutLinks.push_back(makeID(iY0, iXWest));
    // south west
    m_vOutLinks.push_back(makeID(iYSouth, iXWest));
    // south
    m_vOutLinks.push_back(makeID(iYSouth, iX0));
    // south east
    m_vOutLinks.push_back(makeID(iYSouth, iXEast));
   
    // make some incoming links - in this case the numbers don't seem to matter
    for (uint i = 0; i < m_vOutLinks.size(); i++) {
        //m_vInLinks.push_back(m_vOutLinks[ m_vOutLinks.size()-i -1]);
        m_vInLinks.push_back((3*i+2)%NUM_DIR);
    }


    printf("OutLinks ");
    for (uint i = 0; i < m_vOutLinks.size(); i++) {
         printf("%d ", m_vOutLinks[i]);
    }
    printf("\n");
    printf("InLinks  ");
    for (uint i = 0; i < m_vInLinks.size(); i++) {
         printf("%d ", m_vInLinks[i]);
    }
    printf("\n");

    // tags are necessary if Target ids in m_vOutLinks are not unique
    // Here this happens if NX or NY is less than 3
    m_vTags.clear();
    std::set<int> s(m_vOutLinks.begin(), m_vOutLinks.end());
    if (s.size() != m_vOutLinks.size()) {
        printf("Building %zd tags\n", m_vOutLinks.size());
        for (uint i = 0; i < m_vOutLinks.size(); i++) {
            m_vTags.push_back((i+4)%8);
        }
    } else {
        printf("No tags required\n");
    }
    return iResult;
    
}


//----------------------------------------------------------------------------
// exchangeData
//  for each target get a send buffer and fill it .
//  then exchange data
//  retrieve buffer of received data and set array accordingly 
//
int LifeBoard::exchangeData() {
    int iResult = 0;
    
    // set send buffers
    for (int iTargetIndex = 0; (iResult == 0) && (iTargetIndex < NUM_DIR); iTargetIndex++) {
        // create a buffer of the required size
        int iSendBufSize = m_pLifeTile->getSendSize(iTargetIndex);
        //  printf("[%d] size for sendbuf[%d]:%d\n", m_iID, iTargetIndex, iSendBufSize);
        uchar *pSendBuf = m_pMM->createSendBuf(iTargetIndex, iSendBufSize);
        if (pSendBuf != NULL) {
            // printf("[%d] -> size[%d] %d\n", m_iID, iTargetIndex, iSendBufSize);

            // now fill it with the elements of the set
            m_pLifeTile->setSendData(iTargetIndex, pSendBuf);
        } else {
            printf("[%d] Couldn't create buffer for dir %d\n", m_iID, iTargetIndex);
            iResult = -1;
        }

    }

    if (iResult == 0) {
    // exchange data
        iResult = m_pMM->exchangeData();
    }
    
    if (iResult == 0) {
        // data to tile
        for (int iSourceIndex = 0; (iResult == 0) && (iSourceIndex < NUM_DIR); iSourceIndex++) {

            int iSize = 0;
            MPI_Barrier(MPI_COMM_WORLD);
            const uchar *pBuf = m_pMM->getRecvBuf(iSourceIndex, &iSize);
            if ((pBuf != NULL) && (iSize > 0)) {
                //                                printf("[%d] RecvBuf[%d]<-%d: \n", m_iID, iSourceIndex, m_pMM->getSource(iSourceIndex));
                iResult = m_pLifeTile->getRecvData(iSourceIndex, pBuf);
            } else {
                                printf("[%d] source #%d: size %d, o %p\n", m_iID, iSourceIndex, iSize, pBuf);fflush(stdout);
            }
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setPattern
//   mark the points specified by the (local) coordinate pairs in vCoords
//   in buffer pData
//
int LifeBoard::setPattern(std::vector<std::pair<int, int> >&vCoords) {
    int iResult = 0;

    int iNX0 = m_iID % m_iNX;
    int iNY0 = m_iID / m_iNX;
    printf("[%d] this tile (%d,%d)\n", m_iID, iNX0, iNY0);
    uchar *pData = new uchar[m_iW*m_iH];
    memset(pData, 0, m_iW*m_iH*sizeof(uchar));

    for (uint i = 0; i < vCoords.size(); i++) {
        int iX = vCoords[i].first/*-iNX0*m_iW*/;
        int iY = vCoords[i].second/*-iNY0*m_iH*/;
        
        int iPos = iY*m_iW+iX;
        printf("[%d]Set point at pos (%d,%d) -> %d\n", m_iID, iX, iY, iPos);
        pData[iPos] = 1;
    }

    iResult = m_pLifeTile->setInitialPattern(pData);
    printf("[%d] initial patttern set\n", m_iID);

    delete[] pData;
    return iResult;
}


//----------------------------------------------------------------------------
// doStep 
//   exchange data between tiles and do a single step
//
int LifeBoard::doStep() {
    int iResult = 0;

    iResult = exchangeData();
    if (iResult == 0) {
        iResult = m_pLifeTile->doStep();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeCurrent
//   write current state of arrays to file using the specified template
//   for the file name
//
int LifeBoard::writeCurrent(const char *pTemplate) {
    int iResult = 0;

    iResult = m_pLifeTile->writeCurrent(pTemplate);
    return iResult;
}


//----------------------------------------------------------------------------
// showPattern 
//   show current pattern on screen
//
void LifeBoard::showPattern(int z) {
    uchar *pData = new uchar[m_iW*m_iH];
    int iResult = m_pLifeTile->getFinalPattern(pData);
    if (iResult == 0) {
        printf("[%d] Showing pattern %d\n", m_iID, z);
        for (int i = 0; i < m_iH; i++) {
            for (int j = 0; j < m_iW; j++) {
                printf("%c ", (pData[i*m_iW+j]>0)?'X':'·');
            }
            printf("\n");
        }
    } else {
        printf("Error while retrieving field data\n");
    }

    delete[] pData;
}
