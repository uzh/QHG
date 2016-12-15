/******************************************************************************
| PolyLine
|   representation for a piecewise linear function
|
\*****************************************************************************/
#ifndef __POLYLINE_H__
#define __POLYLINE_H__

#include <vector>
#include <stdio.h>
#include "zlib.h"


class PolyLine {
public:
    PolyLine(unsigned int iNumSegments);
    PolyLine(PolyLine *pPL);
   ~PolyLine();
    
    void addPoint(unsigned int i, double fX, double fV) {
        m_afX[i] = fX;    
        m_afV[i] = fV;
        if (i > 0) {
            m_afA[i-1] = (m_afV[i] - m_afV[i-1])/(m_afX[i] - m_afX[i-1]);
        }
    }

    double getVal(double fX);
   
    void write(FILE *fOut);
    void display(const char *pIndent, const char *pCaption);

    static PolyLine *readFromString(const char *pData);

    unsigned int m_iNumSegments;
    double *m_afX;
    double *m_afV;
    double *m_afA;
};

#endif
