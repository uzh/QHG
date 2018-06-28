
#include "ValReader.h"
#include "IcoFace.h"
#include "icoutil.h"
#include "FaceChecker.h"

//-----------------------------------------------------------------------------
// FaceCheckerAlt
//
FaceCheckerAlt::FaceCheckerAlt(ValReader *pVR, float fMinAlt)
    : m_pVR(pVR),
      m_fMinAlt(fMinAlt) {
}

//-----------------------------------------------------------------------------
// check
//
bool FaceCheckerAlt::check(Vec3D **pVerts) {
    bool bKeep = false;
    for (int k = 0;  (k < 3); k++) {
        double dLat = asin(pVerts[k]->m_fZ)*180/M_PI;
        double dLon = atan2(pVerts[k]->m_fY, pVerts[k]->m_fX)*180/M_PI;
        bKeep = (m_pVR->getDValue(dLon, dLat) > m_fMinAlt) || bKeep;
    }
    return bKeep;
}

//-----------------------------------------------------------------------------
// FaceCheckerRect
//
FaceCheckerRect::FaceCheckerRect(box *pbox)
    : m_dLonMin(pbox->dLonMin),
      m_dLonMax(pbox->dLonMax),
      m_dLatMin(pbox->dLatMin),
      m_dLatMax(pbox->dLatMax),
      m_dOffset(0) {

    if (m_dLonMin < -180) {
        m_dLonMin += 360;
    }
    if (m_dLonMax > 180) {
        m_dLonMax -= 360;
    }
    if (m_dLonMin > m_dLonMax) {
        m_dLonMax += 360;
        m_dOffset = 360;
    }
}

//-----------------------------------------------------------------------------
// check
//
bool FaceCheckerRect::check(Vec3D **pVerts) {
    bool bKeep = false;
    for (int k = 0;  (k < 3); k++) {
        double dLat = asin(pVerts[k]->m_fZ)*180/M_PI;
        double dLon = atan2(pVerts[k]->m_fY, pVerts[k]->m_fX)*180/M_PI;
        if (dLon < m_dLonMin) {
            dLon += m_dOffset;
        }
        bKeep = bKeep || ((m_dLonMin <= dLon) &&
                          (m_dLonMax >= dLon) &&
                          (m_dLatMin <= dLat) &&
                          (m_dLatMax >= dLat));

    }
    return bKeep;
}

//-----------------------------------------------------------------------------
// FaceCheckerAltRect
//
FaceCheckerAltRect::FaceCheckerAltRect(ValReader *pVR, float fMinAlt,
                                       tbox *pbox)
    : FaceCheckerAlt(pVR, fMinAlt),
      FaceCheckerRect(pbox) {
}

//-----------------------------------------------------------------------------
// check
//
bool FaceCheckerAltRect::check(Vec3D **pVerts) {
    bool bKeep = FaceCheckerAlt::check(pVerts);
    if (bKeep) {
        bKeep = FaceCheckerRect::check(pVerts);
    }
    return bKeep;
}
