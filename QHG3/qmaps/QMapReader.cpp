#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>

#include "utils.h"
#include "strutils.h"
#include "types.h"
#include "QMapHeader.h"
#include "QMapReader.h"
//#include "Interpolator_gsl.h"
#include "Interpolator.h"

//template<class T>
//typedef T *TP;


//------------------------------------------------------------------------------
// constructor
//
template<class T>
QMapReader<T>::QMapReader(const char *pFileName, int iHeaderSize,
                        double dDLon, double dDataLonMin, double dDataLonMax, 
                        double dDLat, double dDataLatMin, double dDataLatMax, 
                        bool bInterpolate) 
    :    m_aatData(NULL),
         m_aadData(NULL),
         m_bInterpolate(bInterpolate),
         m_fIn(NULL),
         m_dDLon(dDLon),
         m_dDLat(dDLat),
         m_dDataLonMin(dDataLonMin),
         m_dDataLonMax(dDataLonMax),
         m_dDataLatMin(dDataLatMin),
         m_dDataLatMax(dDataLatMax),
         m_dExtrLonMin(0),
         m_dExtrLonMax(0),
         m_dExtrLatMin(0),
         m_dExtrLatMax(0),
         m_pdLonGrids(NULL),
         m_pdLatGrids(NULL),
         m_pIP(NULL),
         m_bReverseLat(false),
         m_bNanOutside(false),
         m_iHeaderSize(iHeaderSize),
         m_pFileName(NULL),
         m_dMin(dPosInf),
         m_dMax(dNegInf),
         m_bSubRegion(false) {

    strcpy(m_sVName, "Val");
    strcpy(m_sXName, "Lon");
    strcpy(m_sYName, "Lat");
    initialize(pFileName, true, bInterpolate);
}

//------------------------------------------------------------------------------
// constructor
//
template<class T>
QMapReader<T>::QMapReader(const char *pFileName,
                          double dExtrLonMin, double dExtrLonMax, 
                          double dExtrLatMin, double dExtrLatMax, 
                          bool bPrepareArrays, 
                          bool bInterpolate) 
    :    m_aatData(NULL),
         m_aadData(NULL),
         m_bInterpolate(bInterpolate),
         m_fIn(NULL),
         m_dDLon(0),
         m_dDLat(0),
         m_dDataLonMin(0),
         m_dDataLonMax(0),
         m_dDataLatMin(0),
         m_dDataLatMax(0),
         m_dExtrLonMin(dExtrLonMax),
         m_dExtrLonMax(dExtrLonMax),
         m_dExtrLatMin(dExtrLatMin),
         m_dExtrLatMax(dExtrLatMax),
         m_pdLonGrids(NULL),
         m_pdLatGrids(NULL),
         m_pIP(NULL),
         m_bReverseLat(false),
         m_bNanOutside(false),
         m_iHeaderSize(0),
         m_pFileName(NULL),
         m_dMin(dPosInf),
         m_dMax(dNegInf),
         m_bSubRegion(true) {

    QMapHeader *pQMH = new QMapHeader();
    int iResult = pQMH->readHeader(pFileName);
    if (iResult == 0) {
        m_dDLon       = pQMH->m_dDLon;
        m_dDataLonMin = pQMH->m_dDataLonMin;
        m_dDataLonMax = pQMH->m_dDataLonMax;
        m_dDLat       = pQMH->m_dDLat;
        m_dDataLatMin = pQMH->m_dDataLatMin;
        m_dDataLatMax = pQMH->m_dDataLatMax;
        m_iHeaderSize = pQMH->getHeaderSize(pFileName);
        strcpy(m_sVName, pQMH->m_sVName);
        strcpy(m_sXName, pQMH->m_sXName);
        strcpy(m_sYName, pQMH->m_sYName);
        initialize(pFileName, bPrepareArrays, bInterpolate);
    }
    delete pQMH;    
}

//------------------------------------------------------------------------------
// constructor
//
template<class T>
QMapReader<T>::QMapReader(const char *pFileName,
                          const char *pRange,
                          bool bPrepareArrays,
                          bool bInterpolate) 
    :    m_aatData(NULL),
         m_aadData(NULL),
         m_bInterpolate(bInterpolate),
         m_fIn(NULL),
         m_dDLon(0),
         m_dDLat(0),
         m_dDataLonMin(0),
         m_dDataLonMax(0),
         m_dDataLatMin(0),
         m_dDataLatMax(0),
         m_dExtrLonMin(0),
         m_dExtrLonMax(0),
         m_dExtrLatMin(0),
         m_dExtrLatMax(0),
         m_pdLonGrids(NULL),
         m_pdLatGrids(NULL),
         m_pIP(NULL),
         m_bReverseLat(false),
         m_bNanOutside(false),
         m_iHeaderSize(0),
         m_pFileName(NULL),
         m_dMin(dPosInf),
         m_dMax(dNegInf),
         m_bSubRegion(true) {

    if (pRange != NULL) {
        int iResult = splitRange(pRange);
        if (iResult == 0) {
            QMapHeader *pQMH = new QMapHeader();
            iResult = pQMH->readHeader(pFileName);
            if (iResult == 0) {
                m_dDLon       = pQMH->m_dDLon;
                m_dDataLonMin = pQMH->m_dDataLonMin;
                m_dDataLonMax = pQMH->m_dDataLonMax;
                m_dDLat       = pQMH->m_dDLat;
                m_dDataLatMin = pQMH->m_dDataLatMin;
                m_dDataLatMax = pQMH->m_dDataLatMax;
                m_iHeaderSize = pQMH->getHeaderSize(pFileName);
                strcpy(m_sVName, pQMH->m_sVName);
                strcpy(m_sXName, pQMH->m_sXName);
                strcpy(m_sYName, pQMH->m_sYName);
                initialize(pFileName, bPrepareArrays, bInterpolate);
            }
            delete pQMH;    
        } else {
            printf("Couldn't split range\n");
        }
    }
}


//------------------------------------------------------------------------------
// constructor
//
template<class T>
QMapReader<T>::QMapReader(const char *pFileName, 
                        bool bInterpolate) 
    :    m_aatData(NULL),
         m_aadData(NULL),
         m_bInterpolate(bInterpolate),
         m_fIn(NULL),
         m_dDLon(0),
         m_dDLat(0),
         m_dDataLonMin(0),
         m_dDataLonMax(0),
         m_dDataLatMin(0),
         m_dDataLatMax(0),
         m_dExtrLonMin(0),
         m_dExtrLonMax(0),
         m_dExtrLatMin(0),
         m_dExtrLatMax(0),
         m_pdLonGrids(NULL),
         m_pdLatGrids(NULL),
         m_pIP(NULL),
         m_bReverseLat(false),
         m_bNanOutside(false),
         m_iHeaderSize(0),
         m_pFileName(NULL),
         m_dMin(dPosInf),
         m_dMax(dNegInf),
         m_bSubRegion(false)  {

    QMapHeader *pQMH = new QMapHeader();
    int iResult = pQMH->readHeader(pFileName);
    if (iResult == 0) {
        m_dDLon       = pQMH->m_dDLon;
        m_dDataLonMin = pQMH->m_dDataLonMin;
        m_dDataLonMax = pQMH->m_dDataLonMax;
        m_dDLat       = pQMH->m_dDLat;
        m_dDataLatMin = pQMH->m_dDataLatMin;
        m_dDataLatMax = pQMH->m_dDataLatMax;
        m_iHeaderSize = pQMH->getHeaderSize(pFileName);

        strcpy(m_sVName, pQMH->m_sVName);
        strcpy(m_sXName, pQMH->m_sXName);
        strcpy(m_sYName, pQMH->m_sYName);
        initialize(pFileName, true, bInterpolate);
    }
    delete pQMH;    
}

//------------------------------------------------------------------------------
// splitRange
//
template<class T>
int QMapReader<T>::splitRange(const char *pRange) {
    int iResult = -1;
    printf("splitting range [%s]\n", pRange);
    char *sRange = strdup(pRange);
    char *p =strtok(sRange, ":");
    if (p != NULL) {
        if (strToNum(p, &m_dExtrLonMin)) {
            p =strtok(NULL, ":");
            if (p != NULL) {
                if (strToNum(p, &m_dExtrLonMax)) {
                    p =strtok(NULL, ":");
                    if (p != NULL) {
                        if (strToNum(p, &m_dExtrLatMin)) {
                            p = strtok(NULL, ":");
                            if (p != NULL) {
                                if (strToNum(p, &m_dExtrLatMax)) {
                                    iResult = 0;
                                    printf("Range: lon [%f, %f], lat [%f, %f]\n", m_dExtrLonMin, m_dExtrLonMax, m_dExtrLatMin, m_dExtrLatMax);
                                } else {
                                    printf("Not a number:[%s]\n", p);
                                }
                            } else {
                                printf("not enough params for range (LatMax)\n");
                            }
                        } else {
                            printf("Not a number:[%s]\n", p);
                        }
                    } else {
                        printf("not enough params for range (LatMin)\n");
                    }
                } else {
                    printf("Not a number:[%s]\n", p);
                }
            } else {
                printf("not enough params for range (LonMax)\n");
            }
        } else {
            printf("Not a number:[%s]\n", p);
        }
    } else {
        printf("not enough params for range (LonMin)\n");
    }
    return iResult;
}

//------------------------------------------------------------------------------
// initialize
//
template<class T>
void QMapReader<T>::initialize(const char *pFileName, bool bPrepareArrays, bool bInterpolate) {
    bool bVerbose = false;
    bool bOK = true;
    int iL = strlen(pFileName);
    m_pFileName = new char[iL+1];
    strcpy(m_pFileName, pFileName);

    struct stat buf;
    unsigned int iSize = -1;
    int isRes = stat(m_pFileName, &buf);
    if (isRes == 0) {
        iSize = buf.st_size;
    }


    if (bVerbose) {
        printf("Grid  Lon: %f - %f,%f\n", m_dDataLonMin, m_dDataLonMax, m_dDLon);
        printf("Grid  Lat: %f - %f,%f\n", m_dDataLatMin, m_dDataLatMax, m_dDLat);
    }

    m_dGridPhaseLon = QMapHeader::getPhase(m_dDataLonMin, m_dDLon);
    m_dGridPhaseLat = QMapHeader::getPhase(m_dDataLatMin, m_dDLat);


    m_dGridLonMin = getNextGridPointLon(m_dDataLonMin);
    double dMaxLon = getNextGridPointLon(m_dDataLonMax) - m_dDLon;
    m_dGridLatMin = getNextGridPointLat(m_dDataLatMin);
    double dMaxLat = getNextGridPointLat(m_dDataLatMax) - m_dDLat;
    
    if (bVerbose) {
        printf("GLon: %f - %f\n", m_dGridLonMin, dMaxLon);
        printf("GLat: %f - %f\n", m_dGridLatMin, dMaxLat);
    }

    // file extents
    /* used to be:
    m_iNLon = (int)floor(round(1+(dMaxLon-m_dDataLonMin)/m_dDLon));
    m_iNLat = (int)floor(round(1+(dMaxLat-m_dDataLatMin)/m_dDLat));
    */
    m_iNLon = (int)floor(round(1+(dMaxLon-m_dGridLonMin)/m_dDLon));
    m_iNLat = (int)floor(round(1+(dMaxLat-m_dGridLatMin)/m_dDLat));
    if (bVerbose) {
        //        printf("MaxLon: %.20lf, LonMin: %.20lf, dLon:%.20lf\n", dMaxLon, m_dDataLonMin, m_dDLon);
        //        printf("diff %.20lf, quot %.20le, quot+1: %.20le, floor %.20le\n", (dMaxLon-m_dDataLonMin), (dMaxLon-m_dDataLonMin)/m_dDLon, 1+(dMaxLon-m_dDataLonMin)/m_dDLon, floor((1+(dMaxLon-m_dDataLonMin)/m_dDLon))); 
        printf("Entire: %dx%d\n", m_iNLat, m_iNLon);
    }

    if (iSize == m_iHeaderSize+m_iNLon*m_iNLat*sizeof(T)) {
        bOK = true;
    } else {
        bOK = false;
        printf("File size %d doesn't match %zd=%d+%d*%d*%zd\n", iSize, m_iHeaderSize+m_iNLon*m_iNLat*sizeof(T),m_iHeaderSize,m_iNLon,m_iNLat,sizeof(T));
    }

    if (bVerbose) {
        printf("PhaseLon: %e\n", m_dGridPhaseLon);
        printf("PhaseLat: %e\n", m_dGridPhaseLat);
    }

    if (m_bSubRegion) {
        printf("finding enclosing grid for range [%f,%f]x[%f,%f] with dlon %f, dlat %f\n", m_dExtrLonMin, m_dExtrLonMax, m_dExtrLatMin, m_dExtrLatMax, m_dDLon, m_dDLat);
        double dMinLonS  = getNextGridPointLon(m_dExtrLonMin) - m_dDLon;
        double dMaxLonS  = getNextGridPointLon(m_dExtrLonMax);
        double dMinLatS  = getNextGridPointLat(m_dExtrLatMin) - m_dDLat;
        double dMaxLatS  = getNextGridPointLat(m_dExtrLatMax);

        printf("--> [%f,%f]x[%f,%f]\n", dMinLonS, dMaxLonS, dMinLatS, dMaxLatS);

        m_iNRLon = (int)floor(round(1+(dMaxLonS-dMinLonS)/m_dDLon));
        m_iNRLat = (int)floor(round(1+(dMaxLatS-dMinLatS)/m_dDLat));
   
        m_iNumIgnoreItems = (int)floor(round(1+(dMinLatS-m_dGridLatMin)/m_dDLat))*m_iNLon + (int)floor(round(1+(dMinLonS-m_dGridLonMin)/m_dDLat));

        // now replace relevant stuff
        m_dGridLonMin = dMinLonS;
        m_dGridLatMin = dMinLatS;
        
        m_dDataLonMin = dMinLonS;
        m_dDataLonMax = dMaxLonS;
        m_dDataLatMin = dMinLatS;
        m_dDataLatMax = dMaxLatS;
    } else {
        m_iNRLon = m_iNLon;
        m_iNRLat = m_iNLat;
    }

    if (bVerbose) {
        printf("m_iNRLat: %d, m_iNRLon: %d\n", m_iNRLat, m_iNRLon);
    }
    


    if (bOK && bPrepareArrays) {




        // initialize data array
        typedef T* PT;
        m_aatData = new PT[m_iNRLat];
        for (unsigned int i = 0; i < m_iNRLat; ++i) {
            m_aatData[i] = new T[m_iNRLon];
            for (unsigned int j = 0; j < m_iNRLon; ++j) {
                m_aatData[i][j] = 0;
            }
        }

        if (m_bInterpolate) {
            if (bVerbose) {
                printf("Doing interpolatioon stuff\n");
            }
            
            // normal
            m_aadData = new double *[m_iNRLat];
            //        printf("++++++++++++++++++++++ m_aaadData: is %p\n", m_aadData);
            for (unsigned int i = 0; i < m_iNRLat; ++i) {
                m_aadData[i] = new double[m_iNRLon];
                for (unsigned int j = 0; j < m_iNRLon; ++j) {
                    m_aadData[i][j] = 0;
                }
            }
    
            //        printf("LonGrids: ");
            m_pdLonGrids = new double[m_iNRLon];
            for (unsigned int i = 0; i < m_iNRLon; ++i) {
                m_pdLonGrids[i] = m_dDataLonMin + i*m_dDLon;
                //              printf("%f ", m_pdLonGrids[i]);
            }
            //        printf("\n");
        
            //        printf("LatGrids: ");
            m_pdLatGrids = new double[m_iNRLat];
            for (unsigned int i = 0; i < m_iNRLat; ++i) {
                m_pdLatGrids[i] = m_dDataLatMin + i*m_dDLat;
            }
            //      printf("\n");
            if (bVerbose) {
                printf("finished interpolatioon stuff\n");
            }

            //      printf("\n");
            if (bVerbose) {
                printf("finished interpolatioon stuff\n");
            }
            
        }
    }
    if (bOK) {
        m_fIn = fopen(pFileName, "rb");
        
        if (m_fIn == NULL) {
            printf("Couldn't open file %s\n", pFileName);
            bOK = false;
        } else if (m_iHeaderSize > 0) {
            fseek(m_fIn, 0, SEEK_END);
            if (bVerbose) printf("end at %ld\n", ftell(m_fIn));
            
            if (bVerbose) printf("seek file to %d\n", m_iHeaderSize);
            fseek(m_fIn, m_iHeaderSize, SEEK_SET);
        }
    }

    if (bVerbose) {
        printf("finished initialise\n");
    }

}

//------------------------------------------------------------------------------
// destructor
//
template<class T>
QMapReader<T>::~QMapReader() {

    if (m_aatData != NULL) {
        for (unsigned int i = 0; i < m_iNRLat; ++i) {
            if (m_aatData[i] != NULL) {
                delete[] m_aatData[i];
            }
        }
        delete[] m_aatData;
    }
    
    if (m_aadData != NULL) {
        for (unsigned int i = 0; i < m_iNRLat; ++i) {
            if (m_aadData[i] != NULL) {
                delete[] m_aadData[i];
            }
        }
        delete[] m_aadData;
    }
    
    if (m_pdLonGrids != NULL) {
        delete[] m_pdLonGrids;
    }

    if (m_pdLatGrids != NULL) {
        delete[] m_pdLatGrids;
    }

    if (m_pIP != NULL) {
        delete m_pIP;
    }

    if (m_fIn != NULL) {
        fclose(m_fIn);
    }
  
    if (m_pFileName != NULL) {
        delete[] m_pFileName;
    }
}
/*
const double EPS =  1E-8;

//------------------------------------------------------------------------------
// getPhase
//
template<class T>
double QMapReader<T>::getPhase(double dMin, double dD) {
    double dPhase = dMin - floorf(dMin/dD)*dD;
    
    if (fabs(dPhase) < EPS) {
        dPhase  = 0;
    }
    return dMin - floorf(dMin/dD)*dD;
}
*/

//------------------------------------------------------------------------------
// getNextGridPointLon
//
template<class T>
double QMapReader<T>::getNextGridPointLon(double dM) {
    /*
    printf("\ngetNextGridPointLon\n");
    printf("dM: %.12e\n", dM);
    printf("dPhase: %.12e\n", m_dGridPhaseLon);
    printf("dlon: %.12e\n", m_dDLon);
    printf("dM-dPhase: %.12e\n", dM-m_dGridPhaseLon);
    printf("(dM-dPhase)/dLon: %.12e\n", (dM-m_dGridPhaseLon)/m_dDLon);
    printf("ceil((dM-dPhase)/dLon): %.12e\n", ceilf((dM-m_dGridPhaseLon)/m_dDLon));
    printf("ceil((dM-dPhase)/dLon)*dLon: %.12e\n", ceilf((dM-m_dGridPhaseLon)/m_dDLon)*m_dDLon);
    printf("dPhase+ceil((dM-dPhase)/dLon)*dLon: %.12e\n", m_dGridPhaseLon +ceilf((dM-m_dGridPhaseLon)/m_dDLon)*m_dDLon);
    */
    //    return m_dGridPhaseLon + ceilf((dM-m_dGridPhaseLon)/m_dDLon)*m_dDLon;
    return QMapHeader::getNextGridPoint(m_dGridPhaseLon, m_dDLon, dM);
}
		
//------------------------------------------------------------------------------
// getNextGridPointLat
//
template<class T>
double QMapReader<T>::getNextGridPointLat(double dM) {
    /*
    printf("\ngetNextGridPointLat\n");
    printf("dM: %.12e\n", dM);
    printf("dPhase: %.12e\n", m_dGridPhaseLat);
    printf("dLat: %.12e\n", m_dDLat);
    printf("dM-dPhase: %.12e\n", dM-m_dGridPhaseLat);
    printf("(dM-dPhase)/dLat: %.12e\n", (dM-m_dGridPhaseLat)/m_dDLat);
    printf("ceil((dM-dPhase)/dLat): %.12e\n", ceilf((dM-m_dGridPhaseLat)/m_dDLat));
    printf("ceil((dM-dPhase)/dLat)*dLat: %.12e\n", ceilf((dM-m_dGridPhaseLat)/m_dDLat)*m_dDLat);
    printf("dPhase+ceil((dM-dPhase)/dLat)*dLat: %.12e\n", m_dGridPhaseLat +ceilf((dM-m_dGridPhaseLat)/m_dDLat)*m_dDLat);
    */
    //    return m_dGridPhaseLat + ceilf((dM-m_dGridPhaseLat)/m_dDLat)*m_dDLat;
    return QMapHeader::getNextGridPoint(m_dGridPhaseLat, m_dDLat, dM);
}

//------------------------------------------------------------------------------
// X2Lon
//
template<class T>
double QMapReader<T>::X2Lon(double iX) const {
    return m_dGridLonMin+m_dDLon*iX;
}

//------------------------------------------------------------------------------
// Lon2X
//
template<class T>
double QMapReader<T>::Lon2X(double dLon) const {
    return (dLon - m_dGridLonMin)/m_dDLon;
}

//------------------------------------------------------------------------------
// Y2Lat
//
template<class T>
double QMapReader<T>::Y2Lat(double iY) const {
    return m_dGridLatMin+m_dDLat*iY;
}

//------------------------------------------------------------------------------
// Lat2Y
//
template<class T>
double QMapReader<T>::Lat2Y(double dLat) const {
    return (dLat - m_dGridLatMin)/m_dDLat;
}

//

//------------------------------------------------------------------------------
// extractData
//
template<class T>
bool QMapReader<T>::extractData() {
    T *pRow;
    bool bOK = (m_fIn != NULL);
    if (bOK) {
        if (m_bSubRegion) {
            // fastforward to row in which subregion starts
            fseek(m_fIn, m_iNumIgnoreItems*sizeof(T), SEEK_CUR);
            for (unsigned int i = 0; bOK && (i < m_iNRLat); ++i) {
                pRow = m_aatData[i];
                unsigned int iRead = fread(pRow, sizeof(T), m_iNRLon, m_fIn);
                if (iRead == m_iNRLon) {
                    if (m_bInterpolate) {
                        for (unsigned int k = 0; k < m_iNLon; ++k) {
                            //                        m_aadData[i][k] = (double) pRow[k];
                            //                        printf("%f ", m_aadData[i][k]);
                            double dCur = (double) pRow[k];
                            
                            // originally, this was active, don't know why...
                            /*
                              if (isnan(dCur)) {
                              // dCur = 0; 
                              } 
                            */
                            m_aadData[i][k] = dCur;
                        }
                    } 
                    
                } else {
                    bOK = false;
                    printf("Bad read at %d: %d instead of %d\n", i, iRead, m_iNLon);
                }
                // fastforward to next start
                fseek(m_fIn, (m_iNLon-m_iNRLon)*sizeof(T), SEEK_CUR);
            }
        } else {
            for (unsigned int i = 0; bOK && (i < m_iNLat); ++i) {
                pRow = m_aatData[i];
                unsigned int iRead = fread(pRow, sizeof(T), m_iNLon, m_fIn);
                if (iRead == m_iNLon) {
                    if (m_bInterpolate) {
                        for (unsigned int k = 0; k < m_iNLon; ++k) {
                            //                        m_aadData[i][k] = (double) pRow[k];
                            //                        printf("%f ", m_aadData[i][k]);
                            double dCur = (double) pRow[k];
                            
                            // originally, this was active, don't know why...
                            /*
                              if (isnan(dCur)) {
                              // dCur = 0; 
                              } 
                            */
                            m_aadData[i][k] = dCur;
                        } 
                        
                        
                        
                        //printf("\n");
                    }
                    
                } else {
                    bOK = false;
                    printf("Bad read at %d: %d instead of %d\n", i, iRead, m_iNLon);
                }
            }
            
        }
    } else {
        printf("File not open\n");
    }
    return bOK;
}



template<class T>
double QMapReader<T>::getDValue(double dLon, double dLat) {
    double dRes = dNaN;


    // make sure location is within bounds
        
    if (m_bInterpolate) {
        if (dLon < m_dGridLonMin) {
            dLon = m_dGridLonMin;
        } else if (dLon > m_dDataLonMax) {
            dLon = m_dDataLonMax;
        }
        
        if (dLat < m_dGridLatMin) {
            dLat = m_dGridLatMin;
        } else if (dLat > m_dDataLatMax) {
            dLat = m_dDataLatMax;
        }
        
        if (m_pIP == NULL) {
            m_pIP = new Interpolator(m_pdLonGrids, m_iNRLon, m_pdLatGrids, m_iNRLat, m_aadData);
            //            m_pIP = new Interpolator_gsl(INTERP_BILIN, m_pdLonGrids, m_iNRLon, m_pdLatGrids, m_iNRLat, m_aadData);
        }
        
        //orig        dRes =  m_pIP->bicub(dLon, dLat);
        dRes =  m_pIP->bilin(dLon, dLat);
        //dRes =  m_pIP->getValue(dLon, dLat);
        
    } else if ((dLon >= m_dGridLonMin) && (dLon < m_dDataLonMax) &&
               (dLat >= m_dGridLatMin) && (dLat < m_dDataLatMax)) {
        
        int iX = (int) floor(Lon2X(dLon));
        int iY = (int) floor(Lat2Y(dLat));
        if (iX < 0) {
            iX = 0;
        }
        if (iX >= (int)m_iNRLon) {
            iX = m_iNRLon-1;
        }
        if (iY < 0) {
            iY = 0;
        }
        if (iY >= (int) m_iNRLat) {
            iY = m_iNRLat-1;
            printf(" NLat:%d, dLat %f, gridlatmin %f DeltaLat %e\n", m_iNRLat, dLat, m_dGridLatMin, m_dDLat);
            printf(" Lat2Y: %9e \n", Lat2Y(dLat));
            printf(" diff:%9f, quot %9f\n", dLat - m_dGridLatMin, (dLat - m_dGridLatMin)/m_dDLat);
            printf("Lon,lat %f,%f -> XY: %d,%d\n", dLon, dLat, iX, iY);
        }
        
        /*
          printf(" NLat:%d, dLat %f, gridlatmin %f \n", m_iNRLat, dLat, m_dGridLatMin);
            printf(" diff:%f, quot %f\n", dLat - m_dGridLatMin, (dLat - m_dGridLatMin)/m_dDLat);
            printf("Lon,lat %f,%f -> XY: %d,%d\n", dLon, dLat, iX, iY);
        */
            dRes = m_aatData[iY][iX];
    
    }
    return dRes;
}

template<class T>
double QMapReader<T>::getDValue(unsigned int iX, unsigned int iY) {
    if (iX >= m_iNRLon) {
        iX = m_iNRLon-1;
    }
    if (iY >= m_iNRLat) {
        iY = m_iNRLat-1;
    }
    return m_aatData[iY][iX];
}


template<class T>
T QMapReader<T>::getValue(double dLon, double dLat) {
    T dRes=0;
  
    // make sure location is within bounds
    if ((dLon >= m_dGridLonMin) && (dLon < m_dDataLonMax) &&
        (dLat >= m_dGridLatMin) && (dLat < m_dDataLatMax)) {
        
        int iX = (int) floor(Lon2X(dLon));
        int iY = (int) floor(Lat2Y(dLat));
            /*
            printf(" NLat:%d, dLat %f, gridlatmin %f \n", m_iNRLat, dLat, m_dGridLatMin);
            printf(" diff:%f, quot %f\n", dLat - m_dGridLatMin, (dLat - m_dGridLatMin)/m_dDLat);
            printf("Lon,lat %f,%f -> XY: %d,%d\n", dLon, dLat, iX, iY);
            */
        dRes = m_aatData[iY][iX];
        
    }
    return dRes;
}


template<class T>
void QMapReader<T>::scanValues(bool bNormal)  {
    for (unsigned int i = 0; i < m_iNRLat; ++i) {
        for (unsigned int k = 0; k < m_iNRLon; ++k) {
            T dCur = m_aatData[i][k];
            if (!(bNormal && std::isinf(1.0*dCur))) {
                if (dCur < m_dMin) {
                    m_dMin = dCur;
                }
                if (dCur > m_dMax) {
                    m_dMax = dCur;
                }
            }
        }
    }
}

template<class T>
bool QMapReader<T>::sameFormat(ValReader *pVR, bool bStrict) {
    bool bEqualFormat =
        (m_iNRLon == pVR->getNRLon()) && 
        (m_iNRLat == pVR->getNRLat()) &&
        (m_dDataLonMin == pVR->getLonMin()) &&
        (m_dDataLatMin == pVR->getLatMin()) &&
        (m_dDLon == pVR->getDLon()) &&
        (m_dDLat == pVR->getDLat());

    if (bStrict && bEqualFormat) {
        bEqualFormat = bEqualFormat &&
            (m_dDataLonMax == pVR->getLonMax()) &&
            (m_dDataLatMax == pVR->getLatMax());
    }      
    return bEqualFormat;
}
