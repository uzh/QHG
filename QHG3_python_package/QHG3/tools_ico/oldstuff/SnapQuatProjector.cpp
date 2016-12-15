#include <gtkmm.h>

#include "GeoInfo.h"
#include "LookUpFactory.h"
#include "ProjInfo.h"
#include "Icosahedron.h"
#include "SnapQuatProjector.h"

//-----------------------------------------------------------------------------
//  constructor
//
SnapQuatProjector::SnapQuatProjector(Icosahedron *pIco, int iW, int iH, double dMaxVal)
    : m_pPI(NULL),
      m_iW(iW), 
      m_iH(iH),
      m_pImage(NULL) {

    *m_sLastSnap = '\0';

    m_pPI = new ProjInfo();
    
    m_pPI->setIco(pIco);
    
    // orthographich projection to northpole
    // (that way images will correspond with IQApp views)
    ProjGrid *pPG = new ProjGrid(m_iW, m_iH, 2.0, 2.0, -m_iW/2, -m_iH/2, 1.0);
    ProjType *pPT = new ProjType(PR_ORTHOGRAPHIC, 0.0, M_PI/2, 0, NULL);
    m_pPI->setGP(pPT, pPG);

    // rainbow lookup
    double adLUParams[2];
    adLUParams[0] = 0;
    adLUParams[1] = dMaxVal;
    setLU(LOOKUP_RAINBOW, 2, adLUParams);

    // prepare the image buffer
    m_pImage = new unsigned char[m_iW*m_iH*3]; 

}

//-----------------------------------------------------------------------------
//  destructor
//
SnapQuatProjector::~SnapQuatProjector() {
    if (m_pPI != NULL) {
        delete m_pPI;
    }

    if (m_pImage != NULL) {
        delete[] m_pImage;
    }
}

//-----------------------------------------------------------------------------
//  setSnap
//   set the snap file
//
int SnapQuatProjector::setSnap(char *pSnapFile) {
    int iResult = m_pPI->loadData(pSnapFile);
    if (iResult == 0) {
        strcpy(m_sLastSnap, pSnapFile);
    } else {
        *m_sLastSnap = '\0';
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  drawProjection
//    draw a projection with current quat for last loaded snap
//
int SnapQuatProjector::drawProjection(char *pOutputFile) {
    int iResult = 0;
    if (*m_sLastSnap != '\0') {
        
        m_pPI->fillRGBInterpolatedQ(m_iW, m_iH, m_pImage);

        Glib::RefPtr< Gdk::Pixbuf > pixbuf;
        pixbuf = Gdk::Pixbuf::create_from_data(m_pImage, Gdk::COLORSPACE_RGB, false, 8,  m_iW, m_iH, 3*m_iW);
        
        // And finally save the image
        pixbuf->save(pOutputFile, "png");

    } else {
        iResult = -1;
    }
    return iResult;
}
    
//-----------------------------------------------------------------------------
//  drawProjection
//    draw a projected image with specified quat for specified snap
//
int SnapQuatProjector::drawProjection(char *pSnapFile, float *pQ, char *pOutputFile) {
    if (pQ != NULL) {
        m_pPI->setQuat(pQ);
    } 

    int iResult = setSnap(pSnapFile);
    if (iResult == 0) {
        iResult = drawProjection(pOutputFile);
    } else {
        printf("Couldn't load [%s]\n", pSnapFile);
    }
    return iResult;
}

