#include <stdio.h>
#ifdef OMP
#include <omp.h>
#endif
#include "utils.h"
#include "strutils.h"
#include "QMapHeader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"

#include "TempRainCap.h"


//----------------------------------------------------------------------------
// constructor
//
TempRainCap::TempRainCap() 
    : m_iW(0),
      m_iH(0),
      m_pVRTemp(NULL),
      m_pVRPrec(NULL),
      m_pVRCap(NULL),
      m_aaiCount(NULL),
      m_aadAvg(NULL),
      m_aadStd(NULL),
      m_aadAvg0(NULL),
      m_aadStd0(NULL), 
      m_iEmpty(0),
      m_dTempDiff(0),
      m_dPrecDiff(0) {


}

//----------------------------------------------------------------------------
// destructor
//
TempRainCap::~TempRainCap() {
    if (m_pVRTemp != NULL) {
        delete m_pVRTemp;
    }
    if (m_pVRPrec != NULL) {
        delete m_pVRPrec;
    }
    if (m_pVRCap != NULL) {
        delete m_pVRCap;
    }

    if (m_aaiCount != NULL) {
        QMapUtils::deleteArray(m_iMaxTemp, m_iMaxPrec, m_aaiCount[0]);
        QMapUtils::deleteArray(m_iMaxTemp, m_iMaxPrec, m_aaiCount[1]);
        delete[] m_aaiCount;
    }
    if (m_aadAvg != NULL) {
        QMapUtils::deleteArray(m_iMaxTemp, m_iMaxPrec, m_aadAvg);
    }
    if (m_aadStd != NULL) {
        QMapUtils::deleteArray(m_iMaxTemp, m_iMaxPrec, m_aadStd);
    }
    if (m_aadAvg0 != NULL) {
        QMapUtils::deleteArray(m_iMaxTemp, m_iMaxPrec, m_aadAvg0);
    }
    if (m_aadStd0 != NULL) {
        QMapUtils::deleteArray(m_iMaxTemp, m_iMaxPrec, m_aadStd0);
    }
}



//----------------------------------------------------------------------------
// setFiles
//
int TempRainCap::setFiles(char *pTempFile, char *pPrecFile, char *pCapacityFile) {
    int iResult = checkFiles(pTempFile, pPrecFile, pCapacityFile);
    if (iResult == 0) {
        m_pVRTemp = QMapUtils::createValReader(pTempFile, false);
        m_pVRPrec = QMapUtils::createValReader(pPrecFile, false);
        if (pCapacityFile != NULL) {
            m_pVRCap  = QMapUtils::createValReader(pCapacityFile, false);
        }

    }
    return iResult;
}

//----------------------------------------------------------------------------
// setDiffs
//
void TempRainCap::setDiffs(double dTempDiff, double dPrecDiff) {
    m_dTempDiff = dTempDiff;
    m_dPrecDiff = dPrecDiff;
}


//----------------------------------------------------------------------------
// calculate
//
int TempRainCap::calculate(double dTempBin, double dPrecBin) {
    
    int iResult = -1;
    if (m_pVRCap != NULL) {
        iResult = 0;

        m_dTempBin = dTempBin;
        m_dPrecBin = dPrecBin;

        m_pVRTemp->scanValues();
        double dMinTemp = m_pVRTemp->getMin();
        double dMaxTemp = m_pVRTemp->getMax();


        m_pVRPrec->scanValues();
        double dMinPrec = m_pVRPrec->getMin();
        double dMaxPrec = m_pVRPrec->getMax();

        m_dMinTemp =  floor(dMinTemp/m_dTempBin)*m_dTempBin+m_dTempDiff;
        m_dMaxTemp =  floor(dMaxTemp/m_dTempBin)*m_dTempBin+m_dTempDiff;
        m_dMinPrec =  floor(dMinPrec/m_dPrecBin)*m_dPrecBin+m_dPrecDiff;
        m_dMaxPrec =  floor(dMaxPrec/m_dPrecBin)*m_dPrecBin+m_dPrecDiff;


        m_iMaxTemp = (int)(1+ceil((m_dMaxTemp - m_dMinTemp)/m_dTempBin));
        m_iMaxPrec = (int)(1+ceil((m_dMaxPrec - m_dMinPrec)/m_dPrecBin));

        printf("Array %dx%d\n", m_iMaxTemp, m_iMaxPrec);

        m_aaiCount = new int**[2];
        m_aaiCount[0] = QMapUtils::createArray(m_iMaxTemp,m_iMaxPrec, 0);
        m_aaiCount[1] = QMapUtils::createArray(m_iMaxTemp,m_iMaxPrec, 0);
        
        m_aadAvg = QMapUtils::createArray(m_iMaxTemp,m_iMaxPrec, 0.0);
        m_aadStd = QMapUtils::createArray(m_iMaxTemp,m_iMaxPrec, 0.0);
    
        m_iPrev = 0;
        m_iCur  = 1;

#ifdef OMP
        int chunk=m_pVRTemp->getNRLat()/(4*omp_get_max_threads());
#pragma omp parallel for schedule(dynamic, chunk)
#endif
        for (unsigned i = 0; i < m_pVRTemp->getNRLat(); i++) {
            for (unsigned j = 0; j < m_pVRTemp->getNRLon(); j++) {
                double dTemp = m_pVRTemp->getDValue(j, i)+m_dTempDiff;
                double dPrec = m_pVRPrec->getDValue(j, i)+m_dPrecDiff;
                double dCap  = m_pVRCap->getDValue(j, i);
                
                if (!(isnan(dTemp) || isnan(dPrec) || isnan(dCap))) {
                    int iT = (int)((dTemp - m_dMinTemp)/m_dTempBin);
                    int iP = (int)((dPrec - m_dMinPrec)/m_dPrecBin);
                    
                    if (iP >= m_iMaxPrec) {
                        iP = m_iMaxPrec-1;
                    }
                
                    if (iT >= m_iMaxTemp) {
                        iT = m_iMaxTemp-1;
                    }
                    m_aadAvg[iP][iT] += dCap;
                    m_aadStd[iP][iT] += dCap*dCap;
               

                    m_aaiCount[m_iPrev][iP][iT]++;
                }
            }
        }
    
        m_iEmpty = 0;
#ifdef OMP
        chunk=m_iMaxPrec/(4*omp_get_max_threads());
#pragma omp parallel for schedule(dynamic, chunk)
#endif
        for (int i = 0; i < m_iMaxPrec; i++) {
            for (int j = 0; j < m_iMaxTemp; j++) {
                int iC = m_aaiCount[m_iPrev][i][j];
                if (iC != 0) {
                    double dAvg = m_aadAvg[i][j]/iC;
                    double dStd = sqrt(jmax(m_aadStd[i][j]/iC - dAvg*dAvg, 0)); 
                    
                    
                    m_aadAvg[i][j] = dAvg;
                    m_aadStd[i][j] = dStd;
                } else {
                    m_iEmpty++;
                }
            }
        } 
        
        m_aadAvg0 = QMapUtils::copyArray(m_iMaxTemp, m_iMaxPrec, m_aadAvg);
        m_aadStd0 = QMapUtils::copyArray(m_iMaxTemp, m_iMaxPrec, m_aadStd);
        
        printf("Need to fill %d empty cells\n", m_iEmpty);
    } else {
        printf("Must specify -c to calculate\n");
    }

    return iResult;
}

//----------------------------------------------------------------------------
// fill
//
int TempRainCap::fill(double dRadius) {
    int iResult = -1;
    if (m_aaiCount != NULL) {
        iResult = 0;
        double dR2 = dRadius*dRadius;
        int iLim = (int) (jmin(jmin(m_iMaxTemp, m_iMaxPrec), dRadius));
        int iEmpty = 0;
#ifdef OMP
        int chunk = m_iMaxPrec/(4*omp_get_max_threads());
#pragma omp parallel for schedule(dynamic,chunk) reduction(+:iEmpty)
#endif
        for (int i = 0; i < m_iMaxPrec; i++) {
#ifdef OMP
            if (omp_get_thread_num() == 0) {
                if (i%20==0){printf("\rY: %05d", i);fflush(stdout);}
            }
#else
            if (i%20==0){printf("\rY: %05d", i);fflush(stdout);}
#endif
            
            int iResultS = 0;
            
            for (int j = 0; j < m_iMaxTemp; j++) {
                if (m_aaiCount[m_iPrev][i][j] == 0) {
                    double dAvg=0;
                    double dStd=0;
                    //      printf("i,j:%d,%d\n", i, j);
                    iResultS = getAverages(j, i, dR2, iLim, &dAvg, &dStd);
                    if (iResultS == 0) {
                        m_aadAvg[i][j] = dAvg;
                        m_aadStd[i][j] = dStd;
                        m_aaiCount[m_iCur][i][j] = 1;
                    } else {
                        iEmpty += 1;
                    }
                } else {
                    m_aaiCount[m_iCur][i][j] = 1;
                }
            }
        } 

        // switch count arrays
        m_iCur  = 1 - m_iCur;
        m_iPrev = 1 - m_iCur;
        
        m_iEmpty = iEmpty;
        printf("\nafter fill: %d empty\n", m_iEmpty);
    } else {
        printf("must call calculate() before fill()\n");
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getAverages
//
int TempRainCap::getAverages(int iX, int iY, double dR2, int iLim, double *pdAvg, double *pdStd) {
    int iResult = -1;
    int iC = 0;

  
    for (int k = 1; k < iLim; k++) {
        for (int v = -k; v <= k; v++) {
            double v2 = v*v;
            int iYv = iY+v;
            if ((v2 > 0) && (v2 < dR2)) {
                iC += sumUp(iX, iYv, pdAvg, pdStd);
            }
            // partly unroll inner loop
            for (int u = 1; u <= k; u++) {
                double d = v2+u*u;
                if ((d > 0) && (d < dR2)) {
                    iC += sumUp(iX+u, iYv, pdAvg, pdStd);
                    iC += sumUp(iX-u, iYv, pdAvg, pdStd);
                }
            }
        }
    }
    if (iC > 0) {
        *pdAvg /= iC;
        *pdStd /= iC;
        iResult = 0;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// sumUp
//
int TempRainCap::sumUp(int iU, int iV, double *pdAvg, double *pdStd) {
    int iResult = 0;

    if ((iU >= 0) && (iU < m_iMaxTemp) &&
        (iV >= 0) && (iV < m_iMaxPrec)) {
        
        if (m_aaiCount[m_iPrev][iV][iU] > 0) {
            *pdAvg += m_aadAvg[iV][iU];
            *pdStd += m_aadStd[iV][iU];
            iResult = 1;
        }
    }
    return iResult;
}



//----------------------------------------------------------------------------
// writeFiles
//
int TempRainCap::writeFiles(const char *pOutBody) {
    int iResult = -1;
    iResult = writeFile(pOutBody, "avgc", m_aadAvg);
    if (iResult == 0) {
        iResult = writeFile(pOutBody, "stdc", m_aadStd);
        
        if (iResult == 0) {
            iResult = writeFile(pOutBody, "avgc0", m_aadAvg0);
            if (iResult == 0) {
                iResult = writeFile(pOutBody, "stdc0", m_aadStd0);
            }
        }
        
    }

    QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_INT,
                                      m_dMinTemp, m_dMaxTemp+m_dTempBin/2, m_dTempBin,
                                      m_dMinPrec, m_dMaxPrec+m_dPrecBin/2, m_dPrecBin,
                                      "Count", "Temp", "Rain", true, true);
    char sOut[LONG_INPUT];
    sprintf(sOut, "%s_count.qmap", pOutBody);
    FILE *fOut = fopen(sOut, "wb");
    if (fOut != NULL) {
        bool bOK = pQMH->addHeader(fOut);
        if (bOK) {
            bOK = QMapUtils::writeArray(fOut, m_iMaxTemp, m_iMaxPrec, m_aaiCount[m_iPrev]); 
            if (bOK) {
                iResult = 0;
            } else {
                iResult = -3;
                printf("Couldn't write data to %s\n", sOut);
            }
        } else {
            iResult = -2;
            printf("Couldn't add header to %s\n", sOut);
        }
        fclose(fOut);
    } else {
        printf("Couldn't open %s for writing\n", sOut);
    }
    
    delete pQMH;
    return iResult;
}

//----------------------------------------------------------------------------
// writeFile
//
int TempRainCap::writeFile(const char *pOutBody, const char *pID, double **aadData) {
    int iResult = -1;

    QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_DOUBLE,
                                      m_dMinTemp, m_dMaxTemp+m_dTempBin/2, m_dTempBin,
                                      m_dMinPrec, m_dMaxPrec+m_dPrecBin/2, m_dPrecBin,
                                      "Mass", "Temp", "Rain", true, true);
    char sOut[LONG_INPUT];
    sprintf(sOut, "%s_%s.qmap", pOutBody, pID);
    FILE *fOut = fopen(sOut, "wb");
    if (fOut != NULL) {
        bool bOK = pQMH->addHeader(fOut);
        if (bOK) {
            bOK = QMapUtils::writeArray(fOut, m_iMaxTemp, m_iMaxPrec, aadData); 
            if (bOK) {
                iResult = 0;
            } else {
                iResult = -3;
                printf("Couldn't write data to %s\n", sOut);
            }
        } else {
            iResult = -2;
            printf("Couldn't add header to %s\n", sOut);
        }
        fclose(fOut);
    } else {
        printf("Couldn't open %s for writing\n", sOut);
    }
    delete pQMH;



    return iResult;
}

//----------------------------------------------------------------------------
// checkFiles
//
int TempRainCap::checkFiles(char *pTempFile, char *pPrecFile, char *pCapacityFile) {
    int iResult = -1;

    unsigned int iW0=0;
    unsigned int iH0=0;
    unsigned int iW1=0;
    unsigned int iH1=0;
    QMapHeader *pQMH = new QMapHeader();
    
    iResult = pQMH->readHeader(pTempFile);
    if (iResult == 0) {
        iW0 = pQMH->m_iWidth;
        iH0 = pQMH->m_iHeight;
        iResult = pQMH->readHeader(pPrecFile);
        if (iResult == 0) {
            iW1 = pQMH->m_iWidth;
            iH1 = pQMH->m_iHeight;
            if ((iW0 == iW1) && (iH0 == iH1)) {
                if (pCapacityFile != NULL) {
                    iResult = pQMH->readHeader(pCapacityFile);
                    if (iResult == 0) {
                        iW1 = pQMH->m_iWidth;
                        iH1 = pQMH->m_iHeight;
                        if ((iW0 == iW1) && (iH0 == iH1)) {
                            m_iW = iW0;
                            m_iH = iH0;
                            iResult = 0;
                        }
                    }
                } else {
                    iResult = 0;
                }
            }
        } else {
            printf("Couldn't open [%s]\n", pPrecFile);
        }
    } else {
        printf("Couldn't open [%s]\n", pTempFile);
    }

    delete pQMH;
    return iResult;
}


//----------------------------------------------------------------------------
// checkResult
//
int TempRainCap::checkResult(char *pTable, char *pOut, bool bCalcMass) {
    int iResult =-1;
    // reconstruct the density map from given temp & rain
    
    ValReader *pTP  = QMapUtils::createValReader(pTable, true);
    double **aadData = QMapUtils::createArray(m_pVRTemp->getNRLon(), m_pVRTemp->getNRLat(), 0.0);
    double dDLon = DEG2RAD(m_pVRTemp->getDLon());
    for (unsigned int i = 0; i < m_pVRTemp->getNRLat(); i++) {
        double dPhi1 = DEG2RAD(m_pVRTemp->Y2Lat(i)-m_pVRTemp->getDLat()/2);
        double dPhi2 = DEG2RAD(m_pVRTemp->Y2Lat(i)+m_pVRTemp->getDLat()/2);
        double dAF = sin(dPhi2) -sin(dPhi1);

        for (unsigned int j = 0; j < m_pVRTemp->getNRLon(); j++) {
            double dTemp = m_pVRTemp->getDValue(j, i)+m_dTempDiff;
            double dPrec = m_pVRPrec->getDValue(j, i)+m_dPrecDiff;
            double dVal = pTP->getDValue(dTemp, dPrec);
            if ((j==810) && (i == 750)) {
                printf("Temp %f, rain %f, val %f\n", dTemp, dPrec, dVal);
            }
            if (bCalcMass) {
                dVal *= dAF*dDLon*RADIUS_EARTH2;
            }
            aadData[i][j] = dVal;
        }
    }

    delete pTP;

    printf("Making header [%f,%f]:%f, [%f,%f]:%f\n", 
           m_pVRTemp->getLonMin(), m_pVRTemp->getLonMax(), m_pVRTemp->getDLon(),
           m_pVRTemp->getLatMin(), m_pVRTemp->getLatMax(), m_pVRTemp->getDLat());

    QMapHeader *pQMH = new QMapHeader(QMAP_TYPE_DOUBLE,
                                      m_pVRTemp->getLonMin(), m_pVRTemp->getLonMax(), m_pVRTemp->getDLon(),
                                      m_pVRTemp->getLatMin(), m_pVRTemp->getLatMax(), m_pVRTemp->getDLat(),
                                      "Dens", "Temp", "Rain", true, true);
    
    // open file
    FILE *fOut = fopen(pOut, "wb");
    if (fOut != NULL) {
        // write heeader
        bool bOK = pQMH->addHeader(fOut);
        if (bOK) {
            bOK = QMapUtils::writeArray(fOut, m_pVRTemp->getNRLon(), m_pVRTemp->getNRLat(), aadData); 
            if (bOK) {
                iResult = 0;
            } else {
                iResult = -3;
                printf("Couldn't write data to %s\n", pOut);
            }
        } else {
            iResult = -2;
            printf("Couldn't add header to %s\n", pOut);
        }
        fclose(fOut);
    } else {
        printf("Couldn't open %s for writing\n", pOut);
    }
    delete pQMH;
    QMapUtils::deleteArray(m_pVRTemp->getNRLon(), m_pVRTemp->getNRLat(), aadData);
    return iResult;
}
