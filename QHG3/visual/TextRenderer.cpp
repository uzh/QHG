#include <stdio.h>
#include <glib.h>
#include <cairo.h>
#include <gdkcairo.h>

#include <string.h>
#include "types.h"
#include "PNGImage.h"

#include "TextRenderer.h"

//----------------------------------------------------------------------------
// createInstance
//
TextRenderer *TextRenderer::createInstance(int iW, int iH) {
    TextRenderer *pTR = new TextRenderer();
    int iResult = pTR->init(iW, iH);
    if (iResult != 0) {
        delete pTR;
        pTR = NULL;
    }
    return pTR;
}


//----------------------------------------------------------------------------
// constructor
//
TextRenderer::TextRenderer() :
    m_iW(0),
    m_iH(0),
    m_dR(0),
    m_dG(0),
    m_dB(0),
    m_dA(0),
    m_pSurface(NULL),
    m_pCR(NULL) {
}


//----------------------------------------------------------------------------
// init
//
int TextRenderer::init(int iW, int iH) {
    m_iW = iW; 
    m_iH = iH;
    int iResult = -1;
    m_pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, iW, iH);
    if (m_pSurface != NULL) {
        m_pCR = cairo_create(m_pSurface);
        if (m_pCR != NULL) {
            iResult = 0;
        }
    }
    
    return iResult;
}




//----------------------------------------------------------------------------
// destructor
//
TextRenderer::~TextRenderer() {
    if (m_pSurface != NULL) {
       cairo_surface_destroy(m_pSurface);
    }
    if (m_pCR != NULL) {
       cairo_destroy(m_pCR);
    }
}        
  
//----------------------------------------------------------------------------
// setFontSize
//
void TextRenderer::setFontSize(double dSize) {
    cairo_set_font_size(m_pCR, dSize);
  
}


//----------------------------------------------------------------------------
// setColor
//
void TextRenderer::setColor(double dR, double dG, double dB, double dA) {
    m_dR = dR;
    m_dG = dG;
    m_dB = dB;
    m_dA = dA;
}


//----------------------------------------------------------------------------
// addText
//
void TextRenderer::addText(const char *pText, double dX, double dY) {
    cairo_text_extents_t ext;
    cairo_text_extents(m_pCR, pText, &ext);
    cairo_set_source_rgba(m_pCR, 0, 0, 0,0.5);

    cairo_rectangle (m_pCR, dX-2, dY-ext.height, ext.width+4, ext.height+4);
    cairo_fill(m_pCR);


    cairo_set_source_rgba(m_pCR, m_dR, m_dG, m_dB, m_dA);

    cairo_move_to(m_pCR, dX, dY);
    cairo_show_text(m_pCR, pText); 
}


//----------------------------------------------------------------------------
// createData
//
uchar **TextRenderer::createData() {
    unsigned char *pPixels=cairo_image_surface_get_data(m_pSurface);
    int iStride = cairo_image_surface_get_stride(m_pSurface);
 

    uchar **ppData = new uchar*[m_iH];
    for (int i = 0; i < m_iH; i++) {
        ppData[i] = new uchar[4*m_iW];
        uchar *pIn =  pPixels+i*iStride;

        for (int j = 0; j < m_iW; ++j) {
            ppData[i][4*j+2] = *pIn++;
            ppData[i][4*j+1] = *pIn++;
            ppData[i][4*j]   = *pIn++;
            ppData[i][4*j+3] = *pIn++;
        }
       
    }            

    return ppData;
}


//----------------------------------------------------------------------------
// writeToPNG
//
int TextRenderer::writeToPNG(const char *pOut) {
    int iResult = -1;
    PNGImage *pPI = new PNGImage(m_iW, m_iH);
    if (pPI != NULL) {
        uchar **ppData = createData();
        bool bOK = pPI->createPNGFromData(ppData, pOut);
        if (bOK) {
            iResult = 0;
        } else {
            iResult = -1;
        }
        
        deleteArray(ppData);
            
        delete pPI;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// deleteArray
//
void TextRenderer::deleteArray(uchar **ppData) {
    for (int i = 0; i < m_iH; ++i) {
        delete[] ppData[i];
    }
    delete[] ppData;
}
