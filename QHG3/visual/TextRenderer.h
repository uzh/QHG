#ifndef __TEXT_RENDERER_H__
#define __TEXT_RENDERER_H__

#include <gdk/gdkcairo.h>

class TextRenderer {
public:
    static TextRenderer *createInstance(int iW, int iH);
    virtual ~TextRenderer();

    void setFontSize(double dSize);
    void setColor(double dR, double dG, double dB, double dA);
    void addText(const char *pText, double dX, double dY);
    uchar **createData();
    int writeToPNG(const char *pOut);

    void deleteArray(uchar **ppData);
protected:
    TextRenderer();
    int init(int iW, int iH);
    int m_iW;
    int m_iH;

    double m_dR;
    double m_dG;
    double m_dB;
    double m_dA;
    cairo_surface_t *m_pSurface; 
    cairo_t         *m_pCR;

};


#endif
