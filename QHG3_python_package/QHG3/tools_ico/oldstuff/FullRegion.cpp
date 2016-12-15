#include <stdio.h>

#include "strutils.h"
#include "Region.h"
#include "FullRegion.h"


//----------------------------------------------------------------------------
//  constructor
//
FullRegion::FullRegion(int iID)
    : Region(iID) {
    printf("CCREATEDDDDDDDDD FFFFUUULLLLLLRRRRREEEGGIIOONNN\n");
}


//----------------------------------------------------------------------------
//  display
//
void FullRegion::display() {
    printf("Id %d Full Sphere\n", m_iID);
}

//----------------------------------------------------------------------------
//  serialize
//
unsigned char *FullRegion::serialize() {
    unsigned char *pS = new unsigned char[dataSize()];
    unsigned char *p = pS;
    p = putMem(p, &m_iID, sizeof(int));
    return p;           
}

//----------------------------------------------------------------------------
//  deserialize
//
int FullRegion::deserialize(unsigned char *pBuffer) {
    unsigned char *p = pBuffer;
    p = getMem(&m_iID, p, sizeof(int));
    return 0;
}


