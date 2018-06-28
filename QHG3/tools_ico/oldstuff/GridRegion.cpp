#include <stdio.h>
#include "strutils.h"
#include "icoutil.h"
#include "IcoNode.h"
#include "Region.h"
#include "GridRegion.h"


//-----------------------------------------------------------------------------
// constructor
//
GridRegion::GridRegion() 
    : Region(),
      m_iW(-1),
      m_iH(-1),
      m_iOffX(-1),
      m_iOffY(-1),
      m_iTW(-1),
      m_iTH(-1) {
}


//-----------------------------------------------------------------------------
// constructor
//
GridRegion::GridRegion(int iID, int iW, int iH, int iOffX, int iOffY, int iTW, int iTH)
    : Region(iID),
      m_iW(iW),
      m_iH(iH),
      m_iOffX(iOffX),
      m_iOffY(iOffY),
      m_iTW(iTW),
      m_iTH(iTH) {

}

//-----------------------------------------------------------------------------
// contains
//
bool GridRegion::contains(IcoNode *pBC) {
    // calc pos from id:
    // we assume the IcoNode was created for a rectangular grid
    // with sequential IDs given rowwise
    gridtype iID =  pBC->m_lID;
    int iX = iID % m_iW;
    int iY = iID / m_iW;
    //    printf("ID %d->iX,iY(%d,%d), Off(%d,%d), Tile(%d,%d)\n", iID, iX, iY, m_iOffX, m_iOffY, m_iTW, m_iTH);
    bool bContained = ((m_iOffX <= iX) && (iX < m_iOffX+m_iTW) &&
                       (m_iOffY <= iY) && (iY < m_iOffY+m_iTH));

    //    printf("id %d -> (%d,%d)%s\n", iID, iX, iY,bContained?" is in!":" is out!");
    return bContained;
}

//-----------------------------------------------------------------------------
// display
//
void GridRegion::display() {
    printf("Region %d: Offs %d,%d,  size %dx%d\n", m_iID, m_iOffX, m_iOffY, m_iTW, m_iTH);
}

//-----------------------------------------------------------------------------
// serialize
//
unsigned char *GridRegion::serialize() {
    unsigned char *pBuffer = new unsigned char[dataSize()];
    unsigned char *p = pBuffer;
    p = putMem(p, &m_iID, sizeof(int));
    p = putMem(p, &m_iW, sizeof(int));
    p = putMem(p, &m_iH, sizeof(int));
    p = putMem(p, &m_iOffX, sizeof(int));
    p = putMem(p, &m_iOffY, sizeof(int));
    p = putMem(p, &m_iTW, sizeof(int));
    p = putMem(p, &m_iTH, sizeof(int));
    return pBuffer;
}

//-----------------------------------------------------------------------------
// deserialize
//
int GridRegion::deserialize(unsigned char *pBuffer) {
    unsigned char *p = pBuffer;
    p = getMem(&m_iID, p, sizeof(int));
    p = getMem(&m_iW, p, sizeof(int));
    p = getMem(&m_iH, p, sizeof(int));
    p = getMem(&m_iOffX, p, sizeof(int));
    p = getMem(&m_iOffY, p, sizeof(int));
    p = getMem(&m_iTW, p, sizeof(int));
    p = getMem(&m_iTH, p, sizeof(int));
    printf("region deserialized: ");
    display();
    return 0;
}

//-----------------------------------------------------------------------------
// dataSize
//
int GridRegion::dataSize() { 
    return  (int)(Region::dataSize()+6* sizeof(int)); 
};
