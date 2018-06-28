#include "DEM.h"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapUtils.h"

#include "QMapDEM.h" 

QMapDEM::QMapDEM() :
    DEM(0,0),
    m_pVR(NULL) {
}

QMapDEM::~QMapDEM() {
    if (m_pVR != NULL) {
        delete m_pVR;
    }
}


bool QMapDEM::load(char *pName) {
    m_pVR = QMapUtils::createValReader(pName, true);
    printf("QMAPDEM: loaded [%s]: %p\n", pName, m_pVR);
    return (m_pVR != NULL);
}

double QMapDEM::getAltitude(double dLon, double dLat) {
    return m_pVR->getDValue(dLon, dLat);
}
