
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <map>

//#include "utils.h"
#include "types.h"
#include "QMapHeader.h"

const char *OLD_MAGIC = "QHGBMAP";
const char *MAGIC     = "QHGMAP2";
const unsigned char MAG_LEN    = 7;

const int QMH_HEADER_SIZE_1 = 64;
const int QMH_HEADER_SIZE_2 = 96;

const int QMH_HEADER_SIZE = QMH_HEADER_SIZE_2;

const char sTypes[] = "usilfd";
const char *sTypeNames[] = {
    "uchar",
    "short",
    "int",
    "long",
    "float",
    "double",
};

const int QMH_ERR_OPEN       = -1;
const int QMH_ERR_READ_DATA  = -2;
const int QMH_ERR_BAD_MAGIC  = -3;
const int QMH_ERR_BAD_TYPE   = -4;


const double EPS = 1E-8;

const char *sErrMess[] = {
    "Unknown error",
    "Couldn't open file",
    "Couldn't read header data",
    "Bad magic number",
    "Bad type",
};

//-----------------------------------------------------------------------------
// constructor
//   to be followed by readHeader
// 
QMapHeader::QMapHeader() 
    : m_iType(-1),
      m_dDLon(0),
      m_dDataLonMin(0),
      m_dDataLonMax(0),
      m_dDLat(0),
      m_dDataLatMin(0),
      m_dDataLatMax(0),
      m_iWidth(0),
      m_iHeight(0), 
      m_dGridPhaseLon(0),
      m_dGridPhaseLat(0),
      m_dGridLonMin(0),
      m_dGridLatMin(0),
      m_bZeroPhaseLon(false),
      m_bZeroPhaseLat(false) {

    strcpy(m_sVName, DEF_V_NAME);
    strcpy(m_sXName, DEF_X_NAME);
    strcpy(m_sYName, DEF_Y_NAME);   
}

//-----------------------------------------------------------------------------
// constructor
// 
QMapHeader::QMapHeader(int iType,                         
                       double dDataLonMin, double dDataLonMax, double dDLon, 
                       double dDataLatMin, double dDataLatMax, double dDLat,
                       const char *pVName, const char *pXName, const char *pYName,
                       bool bZeroPhaseLon, bool bZeroPhaseLat)
    : m_iType(iType),
      m_dDLon(dDLon),
      m_dDataLonMin(dDataLonMin),
      m_dDataLonMax(dDataLonMax),
      m_dDLat(dDLat),
      m_dDataLatMin(dDataLatMin),
      m_dDataLatMax(dDataLatMax),
      m_iWidth(0),
      m_iHeight(0), 
      m_dGridPhaseLon(0),
      m_dGridPhaseLat(0),
      m_dGridLonMin(0),
      m_dGridLatMin(0),
      m_bZeroPhaseLon(bZeroPhaseLon),
      m_bZeroPhaseLat(bZeroPhaseLat) {
    
    initialize(pVName, pXName, pYName);

    gridstuff();
 
}

//-----------------------------------------------------------------------------
// constructor
// 
QMapHeader::QMapHeader(int iType, 
                       double adData[6],                        
                       const char  *pVName,
                       const char  *pXName,
                       const char  *pYName)
    : m_iType(iType),
      m_dDLon(adData[2]),
      m_dDataLonMin(adData[0]),
      m_dDataLonMax(adData[1]),
      m_dDLat(adData[5]),
      m_dDataLatMin(adData[3]),
      m_dDataLatMax(adData[4]),
      m_iWidth(0),
      m_iHeight(0), 
      m_dGridPhaseLon(0),
      m_dGridPhaseLat(0),
      m_dGridLonMin(0),
      m_dGridLatMin(0),
      m_bZeroPhaseLon(false),
      m_bZeroPhaseLat(false) {

     
    initialize(pVName, pXName, pYName);
    gridstuff();
}

//-----------------------------------------------------------------------------
// initialize
// 
void QMapHeader::initialize(const char  *pVName, const char  *pXName, const char  *pYName) {
    bzero(m_sVName, 8);
    bzero(m_sXName, 8);
    bzero(m_sYName, 8);
    if (pVName != NULL) {
        strcpy(m_sVName, pVName);
    } else {
        strcpy(m_sVName, DEF_V_NAME);
    }

    if (pXName != NULL) {
        strcpy(m_sXName, pXName);
    } else {
        strcpy(m_sXName, DEF_X_NAME);
    }

    if (pYName != NULL) {
        strcpy(m_sYName, pYName);
    } else {
        strcpy(m_sYName, DEF_Y_NAME);
    }


    if (m_iType > 32) {
	m_iType = getTypeID(m_iType);
    }
    double dGridPhaseLon = m_bZeroPhaseLon?0:getPhase(m_dDataLonMin, m_dDLon);
    double dGridPhaseLat = m_bZeroPhaseLat?0:getPhase(m_dDataLatMin, m_dDLat);
    //    printf("phases: %f, %f\n", dGridPhaseLon, dGridPhaseLat);
    double dGridLonMin = getNextGridPoint(dGridPhaseLon, m_dDLon, m_dDataLonMin);
    double dMaxLon = getNextGridPoint(dGridPhaseLon, m_dDLon, m_dDataLonMax) - m_dDLon;
    double dGridLatMin = getNextGridPoint(dGridPhaseLat, m_dDLat, m_dDataLatMin);
    double dMaxLat = getNextGridPoint(dGridPhaseLat, m_dDLat, m_dDataLatMax) - m_dDLat;
    //    printf("gridmins: %f, %f\n", dGridLonMin,dGridLatMin);
    

    // file extents
    m_iWidth  = (int)(1+(dMaxLon-dGridLonMin+EPS)/m_dDLon);
    m_iHeight = (int)(1+(dMaxLat-dGridLatMin+EPS)/m_dDLat);
}

//-----------------------------------------------------------------------------
// gridstuff
//   calculate som gridstuff
// 
void QMapHeader::gridstuff() {
    m_dGridPhaseLon =  m_bZeroPhaseLon?0:getPhase(m_dDataLonMin, m_dDLon);
    m_dGridPhaseLat =  m_bZeroPhaseLat?0:getPhase(m_dDataLatMin, m_dDLat);


    m_dGridLonMin = getNextGridPoint(m_dGridPhaseLon, m_dDLon, m_dDataLonMin);
    m_dGridLatMin = getNextGridPoint(m_dGridPhaseLat, m_dDLat, m_dDataLatMin);

}
//------------------------------------------------------------------------------
// X2Lon
//
double QMapHeader::X2Lon(double iX) const {
    return m_dGridLonMin+m_dDLon*iX;
}

//------------------------------------------------------------------------------
// Lon2X
//
double QMapHeader::Lon2X(double dLon) const {
    return (dLon - m_dGridLonMin)/m_dDLon;
}

//------------------------------------------------------------------------------
// Y2Lat
//
double QMapHeader::Y2Lat(double iY) const {
    return m_dGridLatMin+m_dDLat*iY;
}

//------------------------------------------------------------------------------
// Lat2Y
//
double QMapHeader::Lat2Y(double dLat) const {
    return (dLat - m_dGridLatMin)/m_dDLat;
}


//-----------------------------------------------------------------------------
// putHeader
//   put header 
// 
bool QMapHeader::putHeader(const char *pFileIn, const char *pFileOut) {
    char sBuf[1024];
    char sFileOut[1024];
    bool bOverWrite = false;
    if (pFileOut == NULL) {
        srand(clock());
        sprintf(sFileOut, "QM%d", (int) ((1000*rand())/RAND_MAX));
        bOverWrite = true;
    } else {
        strcpy(sFileOut, pFileOut);
    }
    printf("Output to [%s]\n", sFileOut);
    bool bOK = false;

    FILE *fOut = fopen(sFileOut, "wb");
    if (fOut != NULL) {
        FILE *fIn = fopen(pFileIn, "rb");
        if (fIn != NULL) {
            bOK = true;
            addHeader(fOut);
            while (bOK && (!feof(fIn))) {
                int iRead = fread(sBuf, 1, 1024, fIn);
                int iWritten = fwrite(sBuf, 1, iRead, fOut);
                bOK = (iWritten == iRead);
            }
            fclose(fOut);
            if (!bOK) {
                printf("Error during write\n");
            }
        } else {
            printf("Couldn't open File [%s] for reading\n", pFileIn);
        }
        fclose(fIn);
        if (bOK && bOverWrite) {
	    printf("Renaming\n");
            int iResult = rename(sFileOut, pFileIn);
	    if (iResult != 0) {
		bOK = false;
	    }
        }
    } else {
        printf("Couldn't open File [%s] for writing\n", sFileOut);
    }
    return bOK;
}
    
//-----------------------------------------------------------------------------
// replaceHeader
//   replace header only if there is a valid QHGBMAP header already present
// 
bool QMapHeader::replaceHeader(const char *pFileIn, const char *pFileOut) {
    char sBuf[1024];
    char sFileOut[1024];
    bool bOverWrite = false;
    bool bOK = false;
    QMapHeader *pQMH = new QMapHeader();
    int iResult = pQMH->readHeader(pFileIn);
    delete pQMH;
    if (iResult == 0) {
	if (pFileOut == NULL) {
	    srand(clock());
	    sprintf(sFileOut, "QM%d", (int) ((1000*rand())/RAND_MAX));
	    bOverWrite = true;
	} else {
	    strcpy(sFileOut, pFileOut);
	}
	printf("Output to [%s]\n", sFileOut);
	
        int iHeaderSize = getHeaderSize(pFileIn);
        
	FILE *fOut = fopen(sFileOut, "wb");
	if (fOut != NULL) {
	    FILE *fIn = fopen(pFileIn, "rb");
	    if (fIn != NULL) {
		fseek(fIn, iHeaderSize, SEEK_SET);
		addHeader(fOut);
                bOK = true;
		while (bOK && (!feof(fIn))) {
		    int iRead = fread(sBuf, 1, 1024, fIn);
		    int iWritten = fwrite(sBuf, 1, iRead, fOut);
                    bOK = (iWritten == iRead);
		}
		fclose(fOut);
                if (!bOK) {
                    printf("Error during write\n");
                }
	    } else {
		printf("Couldn't open File [%s] for reading\n", pFileIn);
	    }
	    fclose(fIn);
	    if (bOK && bOverWrite) {
		printf("Renaming\n");
		iResult = rename(sFileOut, pFileIn);
		if (iResult != 0) {
		    bOK = false;
		}
	    }
	} else {
	    printf("Couldn't open File [%s] for writing\n", sFileOut);
	}
    } else {
	printf("File [%s] has no header -> no replacing\n", pFileIn);
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// addHeader
//    
bool QMapHeader::addHeader(FILE *fOut) {
    bool bOK = false;
    double dZero = 0;
    unsigned int iWritten = fwrite(MAGIC, sizeof(char), MAG_LEN, fOut);
    if (iWritten == MAG_LEN) {
        iWritten = fwrite(&(sTypes[m_iType]), sizeof(char), 1, fOut);
        if (iWritten == 1) {
            iWritten = fwrite(&m_iWidth, sizeof(int), 1, fOut);
            if (iWritten == 1) {
                iWritten = fwrite(&m_iHeight, sizeof(int), 1, fOut);
                if (iWritten == 1) {
                    iWritten = fwrite(&m_dDLon, sizeof(double), 1, fOut);
                    if (iWritten == 1) {
                        iWritten = fwrite(&m_dDataLonMin, sizeof(double), 1, fOut);
                        if (iWritten == 1) {
                            iWritten = fwrite(&m_dDataLonMax, sizeof(double), 1, fOut);
                            if (iWritten == 1) {
                                iWritten = fwrite(&m_dDLat, sizeof(double), 1, fOut);
                                if (iWritten == 1) {
                                    iWritten = fwrite(&m_dDataLatMin, sizeof(double), 1, fOut);
                                    if (iWritten == 1) {
                                        iWritten = fwrite(&m_dDataLatMax, sizeof(double), 1, fOut);
                                        if (iWritten == 1) {
                                            iWritten = fwrite(m_sVName, sizeof(char), 8, fOut);
                                            if (iWritten == 8) {
                                                iWritten = fwrite(m_sXName, sizeof(char), 8, fOut);
                                                if (iWritten == 8) {
                                                    iWritten = fwrite(m_sYName, sizeof(char), 8, fOut);
                                                    if (iWritten == 8) {
                                                        iWritten = fwrite(&dZero, sizeof(double), 1, fOut);
                                                        bOK = true;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// getBinMapType
//   returns binmap type BMAP_TYPE_XXX
//           or BMAP_TYPE_NONE if not a valid qhg binmap
//
int QMapHeader::getQMapType(const char *pFileIn) {
    char sBuf[64];
    char c;
    int iType = QMAP_TYPE_NULL;
    FILE *fIn = fopen(pFileIn, "rb");
    if (fIn != NULL) {
        iType = QMAP_TYPE_NONE;
        unsigned int iRead = fread(sBuf, 1, MAG_LEN, fIn);
        if (iRead == MAG_LEN) {
            if ((memcmp(sBuf, OLD_MAGIC, MAG_LEN) == 0) || (memcmp(sBuf, MAGIC, MAG_LEN) == 0)) {
                iRead = fread(&c, sizeof(char), 1, fIn);
                if (iRead == 1) {
                    iType = getTypeID(c);
                }
            }
        }
        fclose(fIn);
    }
    return iType;
}

//-----------------------------------------------------------------------------
// getVersion
//   returns version (1 for old, 2, for new, -1 for unknown)
//
int QMapHeader::getVersion(const char *pFileIn) {
    int iVersion = -1;
    char sBuf[QMH_HEADER_SIZE];

    FILE *fIn = fopen(pFileIn, "rb");
    if (fIn != NULL) {
        unsigned int iRead = fread(sBuf, 1, MAG_LEN, fIn);
        if (iRead == MAG_LEN) {
            if (memcmp(sBuf, OLD_MAGIC, MAG_LEN) == 0) {
                iVersion = 1;
            } else if (memcmp(sBuf, MAGIC, MAG_LEN) == 0) {
                iVersion = 2;
            }
        }
        fclose(fIn);
    }
    return iVersion;
}


//-----------------------------------------------------------------------------
// getHeaderSize
//   returns header size (64 for old, 80 for new)
//
int QMapHeader::getHeaderSize(const char *pFileIn) {
    int iSize = 0;

    int iV = getVersion(pFileIn);
    switch (iV) {
    case 1:
        iSize = QMH_HEADER_SIZE_1;
        break;
    case 2:
        iSize = QMH_HEADER_SIZE_2;
        break;
    }
    return iSize;
}

//-----------------------------------------------------------------------------
// readHeader
//
int  QMapHeader::readHeader(FILE *fIn) {
    int iResult = 0;
    int iVersion = -1;
    char sBuf[64];
    char c;
    memset(sBuf,0, 64);

    unsigned int iRead = fread(sBuf, sizeof(char), MAG_LEN, fIn);
    if (iRead == MAG_LEN) {
        
        if (memcmp(sBuf, MAGIC, MAG_LEN) == 0) {
            iVersion = 2;
        } else if (memcmp(sBuf, OLD_MAGIC, MAG_LEN) == 0) {
            iVersion = 1;
            printf("Old file -- replace header eventually!\n");
        }
        if (iVersion > 0) {
            iRead = fread(&c, sizeof(char), 1, fIn);
            if (iRead == 1) {
		m_iType = getTypeID(c);
		if (m_iType != QMAP_TYPE_NONE) {
		    iRead = fread(&m_iWidth, sizeof(int), 1, fIn);
		    if (iRead == 1) {
			iRead = fread(&m_iHeight, sizeof(int), 1, fIn);
			if (iRead == 1) {
			    iRead = fread(&m_dDLon, sizeof(double), 1, fIn);
			    if (iRead == 1) {
				iRead = fread(&m_dDataLonMin, sizeof(double), 1, fIn);
				if (iRead == 1) {
				    iRead = fread(&m_dDataLonMax, sizeof(double), 1, fIn);
				    if (iRead == 1) {
					iRead = fread(&m_dDLat, sizeof(double), 1, fIn);
					if (iRead == 1) {
					    iRead = fread(&m_dDataLatMin, sizeof(double), 1, fIn);
					    if (iRead == 1) {
						iRead = fread(&m_dDataLatMax, sizeof(double), 1, fIn);
						if (iRead == 1) {
                                                    if (iVersion == 2) {
                                                        iRead = fread(m_sVName, sizeof(char), 8, fIn);
                                                        if (iRead == 8) {
                                                            iRead = fread(m_sXName, sizeof(char), 8, fIn);
                                                            if (iRead == 8) {
                                                                iRead = fread(m_sYName, sizeof(char), 8, fIn);
                                                                if (iRead == 8) {
                                                                    iResult = 0;
                                                                }  
                                                            }
                                                        }
                                                    } else {
                                                        strcpy(m_sVName, DEF_V_NAME);
                                                        strcpy(m_sXName, DEF_X_NAME);
                                                        strcpy(m_sYName, DEF_Y_NAME);
                                                        iResult = 0;
                                                    }
						} else {
						    iResult = QMH_ERR_READ_DATA;
						}
					    } else {
						iResult = QMH_ERR_READ_DATA;
					    }
					} else {
					    iResult = QMH_ERR_READ_DATA;
					}
				    } else {
					iResult = QMH_ERR_READ_DATA;
				    }
				} else {
				    iResult = QMH_ERR_READ_DATA;
				}
			    } else {
				iResult = QMH_ERR_READ_DATA;
			    }
			} else {
			    iResult = QMH_ERR_READ_DATA;
			}
		    } else {
			iResult = QMH_ERR_READ_DATA;
		    }
		} else {
		    iResult = QMH_ERR_BAD_TYPE;
		}
	    } else {
		iResult = QMH_ERR_READ_DATA;
	    }
	} else {
	    iResult = QMH_ERR_BAD_MAGIC;
	}
    } else {
	iResult = QMH_ERR_READ_DATA;
    }
 
    return iResult;
}

//-----------------------------------------------------------------------------
// getTypeID
//
int QMapHeader::getTypeID(char c) {
    int iType = QMAP_TYPE_NONE;
    for (unsigned int iT = 0; (iType == QMAP_TYPE_NONE) && (iT < strlen(sTypes)); iT++) {
	if (sTypes[iT] == tolower(c)) {
	    iType = iT;
	}
    }
    return iType;
}

//-----------------------------------------------------------------------------
// getTypeChar
//
char QMapHeader::getTypeChar(unsigned int i) {
    char c = '\0';
    if (i < strlen(sTypes)) {
        c = sTypes[i];
    }
    return c;
}
//-----------------------------------------------------------------------------
// getTypeName
//
const char *QMapHeader::getTypeName(unsigned int i) {
    const char *p = NULL;
    if (i < strlen(sTypes)) {
        p = sTypeNames[i];
    }
    return p;
}

//-----------------------------------------------------------------------------
// getNumTypes
//
int QMapHeader::getNumTypes() {
    return strlen(sTypes);
}

//-----------------------------------------------------------------------------
// getTypeSize
//
size_t QMapHeader::getTypeSize(unsigned int iType) {
    size_t is=0;
    switch (iType) {
    case QMAP_TYPE_UCHAR:
        is = sizeof(uchar);
        break;
    case QMAP_TYPE_SHORT:
        is = sizeof(short);
        break;
    case QMAP_TYPE_INT:
        is = sizeof(int);
        break;
    case QMAP_TYPE_LONG:
        is = sizeof(long);
        break;
    case QMAP_TYPE_FLOAT:
        is = sizeof(float);
        break;
    case QMAP_TYPE_DOUBLE:
        is = sizeof(double);
        break;
    }
    return is;
}

//-----------------------------------------------------------------------------
// readHeader
//
int QMapHeader::readHeader(const char *pFile) {
    int iResult = QMH_ERR_OPEN;
    FILE *fIn = fopen(pFile, "rb");
    if (fIn != NULL) {
        iResult = readHeader(fIn);
        fclose(fIn);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getPhase
//
double QMapHeader::getPhase(double dMin, double dD) {
    double dPhase = dMin - floorf(dMin/dD)*dD;
    
    if (fabs(dPhase) < EPS) {
        dPhase  = 0;
    }
    return dMin - floorf(dMin/dD)*dD;
}
	


//-----------------------------------------------------------------------------
// getNextGridPoint
//
double QMapHeader::getNextGridPoint(double dPhase, double dDelta, double dM) {
    return dPhase + ceilf((dM-dPhase)/dDelta)*dDelta;
}
	


//-----------------------------------------------------------------------------
// display
//
void QMapHeader::display() {
    printf("Type : [%c](%d)\n", sTypes[m_iType], m_iType);
    printf("Size : %dx%d\n", m_iWidth, m_iHeight);
    printf("Lon  : [%-4.4f, %-4.4f] D:%-4.4f\n", m_dDataLonMin, m_dDataLonMax, m_dDLon);
    printf("Lat  : [%-4.4f, %-4.4f] D:%-4.4f\n", m_dDataLatMin, m_dDataLatMax, m_dDLat);
}

//-----------------------------------------------------------------------------
// getErrMess
//
const char *QMapHeader::getErrMess(int iErr) {
    const char *pErrMess = sErrMess[0];
    iErr = -iErr;
    if ((iErr > 0) && (iErr < (int)(sizeof(sErrMess)/sizeof(char *)))) {
	pErrMess = sErrMess[iErr];
    }
    return pErrMess;
}


