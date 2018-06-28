#include "Projector.h"
#include "GeoMapper.h"


//-----------------------------------------------------------------------------
// constructor
//
template<class T>
GeoMapper<T>::GeoMapper(const ProjGrid *ppg0, Projector *pProj0, T **pptData0,
                        const ProjGrid *ppg1, Projector *pProj1, T **pptData1)
    : m_pGP0(NULL),
      m_pGP1(NULL),
      m_pptData0(pptData0),
      m_pptData1(pptData1),
      m_ppg0(ppg0),
      m_ppg1(ppg1),
      m_bCloseX(false),
      m_bCloseY(false) {

    m_pGP0 = new GridProjection(ppg0, pProj0, false, false);
    m_pGP1 = new GridProjection(ppg1, pProj1, false, false);

}


//-----------------------------------------------------------------------------
// destructor
//
template<class T>
GeoMapper<T>::~GeoMapper() {
    if (m_pGP0 != NULL) {
        delete m_pGP0;
    }
    if (m_pGP1 != NULL) {
        delete m_pGP1;
    }
}

//-----------------------------------------------------------------------------
// map
//
template<class T>
void GeoMapper<T>::setClosure(bool bCloseX, bool bCloseY) {
    m_bCloseX = bCloseX;
    m_bCloseY = bCloseY;
}

//-----------------------------------------------------------------------------
// map
//
template<class T>
void GeoMapper<T>::map(T tUndef) {

    for (int iY1 = 0; iY1 < m_ppg1->m_iGridH; ++iY1) {
        for (int iX1 = 0; iX1 < m_ppg1->m_iGridW; ++iX1) {
            double dLon;
            double dLat;
            double dX;
            double dY;
            m_pGP1->gridToSphere(iX1, iY1, dLon, dLat);
            m_pGP0->sphereToGrid(dLon, dLat, dX, dY);
            int iX0 = (int) (dX+0.5);
            int iY0 = (int) (dY+0.5);

            if (m_bCloseX) {
                if (iX0 < 0) {
                    iX0 = -((-iX0)%m_ppg0->m_iGridW);
                } else if (iX0 > m_ppg0->m_iGridW) {
                    iX0 = iX0 % m_ppg0->m_iGridW;
                }
            }
            if (m_bCloseY) {
                if (iY0 < 0) {
                    iY0 = -((-iY0)%m_ppg0->m_iGridW);
                } else if (iY0 > m_ppg0->m_iGridW) {
                    iY0 = iY0 % m_ppg0->m_iGridW;
                }
            }
            if ((iX0 >= 0) && (iX0 < m_ppg0->m_iGridW) && 
                (iY0 >= 0) && (iY0 < m_ppg0->m_iGridH)) {
                m_pptData1[iY1][iX1] = m_pptData0[iY0][iX0];
            } else {
                //                                printf("?");
                m_pptData1[iY1][iX1] = tUndef;
            }

        }
    }

}

