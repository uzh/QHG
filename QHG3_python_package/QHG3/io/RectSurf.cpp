#include <stdio.h>
#include <stdlib.h>

#include "strutils.h"
#include "IcoLoc.h"
#include "icoutil.h"

#include "RectSurf.h"

RectSurf::RectSurf(int iW, int iH)
    :m_iW(iW),
     m_iH(iH) {

}

RectSurf::~RectSurf() {

}

RectSurf *RectSurf::createRectSurf(char *pFormat) {
    RectSurf *pRL = NULL;
    int iW;
    int iH;
    if (splitSizeString(pFormat, &iW, &iH)) {
        pRL = new RectSurf(iW, iH);
    }
    return pRL;
}

gridtype RectSurf::findNode(double dLon, double dLat) {
    gridtype lNode = -1;
    int iX = (int) dLon;
    int iY = (int) dLon;
    if ((iX >= 0) && (iX < m_iW) &&
        (iY >= 0) && (iY < m_iH)) {
        lNode = iY*m_iW + iX;
    } else {
        printf("coordinates(%d,%d) out of range [%dx%d]\n", iX, iY, m_iW, m_iH);
    }
    return lNode;
}

gridtype RectSurf::findNode(Vec3D *pv) {
    return -1;
}
PolyFace *RectSurf::findFace(double dLon, double dLat) {
    return NULL;
}
Vec3D *RectSurf::getVertex(gridtype lID) {
    return NULL;
}
int RectSurf::collectNeighborIDs(gridtype iID, int iDist, std::set<gridtype> & sIds) {
    int iResult = 0;
    return iResult;
}

tbox *RectSurf::getBox() {
    return NULL;
}

void RectSurf::display() {
    printf("%dx%d\n", m_iW, m_iH);
}

int RectSurf::load(const char *pFileName) {
    return -1;
}
int RectSurf::save(const char *pFileName) {
    return -1;
}
