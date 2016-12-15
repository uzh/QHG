#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "PolyLine.h"

PolyLine::PolyLine(unsigned int iNumSegments) 
:   m_iNumSegments(iNumSegments),
    m_afX(NULL),
    m_afV(NULL),
    m_afA(NULL) {
    
    if (m_iNumSegments > 0) {
        m_afX = new double[(m_iNumSegments+1)*sizeof(double)];
        memset(m_afX, 0, (m_iNumSegments+1)*sizeof(double));
        m_afV = new double[(m_iNumSegments+1)*sizeof(double)];
        memset(m_afV, 0, (m_iNumSegments+1)*sizeof(double));
        m_afA = new double[m_iNumSegments*sizeof(double)];
        memset(m_afA, 0, m_iNumSegments*sizeof(double));
    }
}

PolyLine::PolyLine(PolyLine *pPL) 
:   m_iNumSegments(pPL->m_iNumSegments),
    m_afX(NULL),
    m_afV(NULL),
    m_afA(NULL) {

    if (m_iNumSegments > 0) {
        m_afX = new double[(m_iNumSegments+1)*sizeof(double)];
        memcpy(m_afX, pPL->m_afX, (m_iNumSegments+1)*sizeof(double));
        m_afV = new double[(m_iNumSegments+1)*sizeof(double)];
        memcpy(m_afV, pPL->m_afV, (m_iNumSegments+1)*sizeof(double));
        m_afA = new double[m_iNumSegments*sizeof(double)];
        memcpy(m_afA, pPL->m_afA, m_iNumSegments*sizeof(double));
    }
}

PolyLine::~PolyLine() {
    if (m_afX != NULL) {
        delete[] m_afX;
    }
    if (m_afV != NULL) {
        delete[] m_afV;
    }
    if (m_afA != NULL) {
        delete[] m_afA;
    }
}

double PolyLine::getVal(double fX) {
    double fV = 0;
    unsigned int i = 0;
    double *pf = m_afX;

    if (m_iNumSegments > 0) {
        // if fX is too big, set highest value
        if (fX >= m_afX[m_iNumSegments]) {
            fV = m_afV[m_iNumSegments];
        } else {
            // find  starting point for segment
            while ((i <= m_iNumSegments) && (fX > *pf)) {
                ++i; ++pf;
            }
        
            // num values = num segs +1
            if (i == 0) {
                fV = m_afV[0];
            } else if (i <= m_iNumSegments) {
                fV = m_afV[i-1] + m_afA[i-1]*(fX - m_afX[i-1]);
            } else {
                // this should not happen
                fV = m_afV[m_iNumSegments];
            }
        }
    } else {
        fV = fX;
    }
    return fV;
}       


PolyLine * PolyLine::readFromString(const char *pData) {
    PolyLine *pPL = NULL;
    char *pBuf; // for strtok_r       
    char *pEnd; // for strtod
    std::vector<double> vecData;
    bool bOK = true;
    char *p2 = strdup(pData);  
    char *p = strtok_r(p2, " \t", &pBuf);     
 
    while (bOK && (p != NULL)) {
        bOK = false;
        double fX = strtod(p, &pEnd);
        if (*pEnd == '\0') {
            p = strtok_r(NULL, " \t", &pBuf);
            if (p != NULL) {
                double fV = strtod(p, &pEnd);
                if (*pEnd == '\0') {
                    vecData.push_back(fX);
                    vecData.push_back(fV);

                    bOK = true;
                } 
            }
        } 
        p = strtok_r(NULL, " \t", &pBuf);
    }
    //    printf("\n");
    if (vecData.size() > 0) {
        if (bOK && (vecData.size() > 2)) {
            pPL = new PolyLine((unsigned int)vecData.size()/2-1);
            for (unsigned int i = 0; i < vecData.size()/2; i++) {
                pPL->addPoint(i, vecData[2*i], vecData[2*i+1]); 
            }
        } else {
            printf("Bad Function def (number format) : [%s] or not enough data points (%zd)\n", pData, vecData.size());
        } 
    }
    free(p2);
    return pPL;
}



void PolyLine::write(FILE *fOut) {
    if (m_iNumSegments > 0) {
        for (unsigned int j = 0; j < m_iNumSegments+1; j++) {
            fprintf(fOut, "%f %f ", m_afX[j], m_afV[j]);
        }
    }
    fprintf(fOut, "\n");
}



void PolyLine::display(const char *pIndent, const char *pCaption) {
    printf("%s%s [%d]:\n%s  ", pIndent, pCaption, m_iNumSegments, pIndent);
    if (m_iNumSegments > 0) {
        for (unsigned int j = 0; j < m_iNumSegments+1; j++) {
            printf("%f %f ", m_afX[j], m_afV[j]);
        }
    }
    printf("\n");
}
