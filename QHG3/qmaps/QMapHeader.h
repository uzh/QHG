#ifndef __QMAPHEADER_H__
#define __QMAPHEADER_H__

#include <stdio.h>

#include "ranges.h"

const int QMAP_TYPE_NULL   = -2;
const int QMAP_TYPE_NONE   = -1;
const int QMAP_TYPE_UCHAR  =  0;
const int QMAP_TYPE_SHORT  =  1;
const int QMAP_TYPE_INT    =  2;
const int QMAP_TYPE_LONG   =  3;
const int QMAP_TYPE_FLOAT  =  4;
const int QMAP_TYPE_DOUBLE =  5;

const char DEF_X_NAME[] = "Lon";
const char DEF_Y_NAME[] = "Lat";
const char DEF_V_NAME[] = "Val";

#define QMAP_TYPE_OK(t) ((t) >= 0)

class QMapHeader {

public:
    QMapHeader();
    QMapHeader(int iType,                         
               double dDataLonMin=DEF_MAP_LON_MIN, 
               double dDataLonMax=DEF_MAP_LON_MAX, 
               double dDLon=DEF_MAP_DLON, 
               double dDataLatMin=DEF_MAP_LAT_MIN, 
               double dDataLatMax=DEF_MAP_LAT_MAX, 
               double dDLat=DEF_MAP_DLAT,
               const char  *pVName=NULL,
               const char  *pXName=NULL,
               const char  *pYName=NULL,
               bool bZeroPhaseLon = false,
               bool bZeroPhaseLat = false);

    QMapHeader(int iType, 
               double adData[6],                        
               const char  *pVName=NULL,
               const char  *pXName=NULL,
               const char  *pYName=NULL);

 
    void initialize(const char *pVName=NULL, const char *pXName=NULL, const char *pYName=NULL);
   
    bool putHeader(const char *pFileIn, const char *pFileOut=NULL);
    bool replaceHeader(const char *pFileIn, const char *pFileOut=NULL);
    
    static int getQMapType(const char *pFileIn);

    bool addHeader(FILE *fOut);

    int  readHeader(FILE *fIn);
    int  readHeader(const char *pFile);

    static int getNumTypes();
    static int getTypeID(char c);
    static char getTypeChar(unsigned int iType);
    static const char *getTypeName(unsigned int iType);
    static size_t getTypeSize(unsigned int iType);
    void forceType(unsigned int iType) { m_iType = iType;};

    const char *getErrMess(int iErr);


    void display();


    static double getPhase(double dMin, double dD);
    static double getNextGridPoint(double dPhase, double dDelta, double dM);
    static int    getHeaderSize(const char *pFile);
    static int    getVersion(const char *pFile);

    void gridstuff();
    double X2Lon(double iX) const;
    double Y2Lat(double iY) const;
    double Lon2X(double fLon) const;
    double Lat2Y(double fLat) const;

public:
    int    m_iType;
    double m_dDLon;
    double m_dDataLonMin;
    double m_dDataLonMax;
    double m_dDLat;
    double m_dDataLatMin;
    double m_dDataLatMax;

    unsigned int m_iWidth;
    unsigned int m_iHeight;
    char   m_sVName[8];
    char   m_sXName[8];
    char   m_sYName[8];

    double m_dGridPhaseLon;
    double m_dGridPhaseLat;
    double m_dGridLonMin;
    double m_dGridLatMin;  
    bool   m_bZeroPhaseLon;
    bool   m_bZeroPhaseLat;

};

#endif


