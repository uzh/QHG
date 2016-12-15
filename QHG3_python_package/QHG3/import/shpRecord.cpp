#include <stdio.h>
#include <string.h>

#include <vector>
#include <map>

#include "shpUtils.h"
#include "shpRecord.h"

#define REC_HEADER_SIZE 8

//----------------------------------------------------------------------------
// constructor
//
shpRecord::shpRecord(FILE *fIn)
    : m_fIn(fIn),
      m_iRecNumber(0),
      m_iRecLenWords(0),
      m_iRecLen(0)/*,
                    m_pShape(NULL)*/ {

}
     


//----------------------------------------------------------------------------
// read
//
int shpRecord::read(vecdoubledouble &vCoords) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = readHeader();
    }

    if (iResult == 0) {
        iResult = readShape(vCoords);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// readHeader
//
int shpRecord::readHeader() {
    int iResult = -1;
    uchar pHeader[REC_HEADER_SIZE];
    m_lFilePos = ftell(m_fIn);
    int iRead = fread(pHeader, 1, REC_HEADER_SIZE, m_fIn);
    if (iRead == REC_HEADER_SIZE) {
        uchar *p = pHeader;
        p = shpUtils::getNum(p, &m_iRecNumber, BIGENDIAN);
        p = shpUtils::getNum(p, &m_iRecLenWords, BIGENDIAN);
        m_iRecLen = 2*m_iRecLenWords;
        
        iResult = 0;
    } else {
        if (feof(m_fIn)) {
            iResult = 1;
        } else {
            printf("Only read [%d] bytes instead of [%d] from header\n", iRead, REC_HEADER_SIZE);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// display
//
void shpRecord::display(const char*pCaption) {
    printf("%s\n", pCaption);
    printf("Record #%d:\n", m_iRecNumber);
    printf("  size: %d bytes\n", m_iRecLen);
    printf("  shape:%d [%s]\n", m_iShape, shpUtils::getShapeName(m_iShape)); 
    /*
    if (m_pShape != NULL) {
        m_pShape->display("");
    }
    */
}


//----------------------------------------------------------------------------
// readShape
//
int shpRecord::readShape(vecdoubledouble &vCoords) {
    int iResult = -1;
    // shape is little endian - can read directly
    int iRead = fread(&m_iShape, sizeof(int), 1, m_fIn);
    if (iRead == 1) {
        iResult = 2;
        switch (m_iShape) {
        case SHP_NULL:
            // nothing to do
            break;
        case SHP_POINT:
            // X,Y
            break;
        case SHP_POLYLINE:
            iResult = readPolyLine(vCoords);
            // MBR, num parts,num points, parts, points
            // parts:index to first point of polyline-part
            break;
        case SHP_POLYGON:
            // MBR, num parts,num points, parts, points
            // parts:index to first point of polyline-part
            break;
        case SHP_MULTIPOINT:
            // MBR, num opints, points
            break;
        case SHP_POINTZ:
            // X, Y, Z [optional: M]
            break;
        case SHP_POLYLINEZ:
            // MBR, num parts,num points, parts, points Z range, Z array
            // [optional: M range, M array]
            break;
        case SHP_POLYGONZ:
            // MBR, num parts,num points, parts, points Z range, Z array
            // [optional: M range, M array]
            break;
        case SHP_MULTIPOINTZ:
            // MBR, num points, points, Z range, Z array
            // [optional M range, M array]
            break;
        case SHP_POINTM:
            // X, Y, M 
            break;
        case SHP_POLYLINEM:
            // MBR, num parts,num points, parts 
            // [optional: M range, M array]
            break;
        case SHP_POLYGONM:
            // MBR, num parts,num points, parts
            // [optional: M range, M array]
            break;
        case SHP_MULTIPOINTM:
            // MBR, num points, points
            // [optional M range, M array]
            break;
        case SHP_MULTIPATCH:
            // MBR, Number of parts, Number of points, 
            // Parts, Part types, Points, Z range, Z array
            // [optional M range, M array]
            break;
        default:
            printf("Unknown type [%d]\n", m_iShape); 
            iResult = -1;
        }
        if (iResult == 2) {
            printf("shape [%s] not yet supported\n", shpUtils::getShapeName(m_iShape));
        }

        
        // here we would read the record.
        // for now: skip it
        //  fseek(m_fIn, m_lFilePos + m_iRecLen, SEEK_SET);
    } else {
        printf("Couldn't read shape id\n");
    }
    return iResult;
}


int shpRecord::readPolyLine(vecdoubledouble &vCoords) {
    int iResult = -1;
    int iBufSize = m_iRecLen-sizeof(int);
    uchar *pBuf = new uchar[iBufSize];
    int iRead = fread(pBuf, 1, iBufSize, m_fIn);
    if (iRead == iBufSize) {
        mbr MBR;
        int iNumParts;
        int iNumPoints;
        uchar *p = pBuf;
        p = shpUtils::getMBR(p, MBR);
        p = shpUtils::getNum(p, &iNumParts, LITTLEENDIAN); 
        p = shpUtils::getNum(p, &iNumPoints, LITTLEENDIAN); 

        /*
        printf("bounds: [%f,%f] - [%f,%f]\n", MBR.dXmin, MBR.dXmax, MBR.dYmin, MBR.dYmax);
        printf("Num parts %d, num points %d\n", iNumParts, iNumPoints);
        */
        int *pParts = new int[iNumParts];
        // we can copy because the parts are littleendian
        memcpy(pParts, p, iNumParts*sizeof(int));
        p += iNumParts*sizeof(int);

        double *pPoints = new double[2*iNumPoints];
        // we can copy because the parts are littleendian
        memcpy(pPoints, p, 2*iNumPoints*sizeof(double));
        p += 2*iNumPoints*sizeof(double);
        
        
        for (int i = 0; i < iNumParts; i++) {
            //            printf("Part %d:\n  ", i);
            int iLast = 0;
            if (i < iNumParts-1) {
                iLast = pParts[i+1];
            } else {
                iLast = iNumPoints;
            }
            int iCur = pParts[i];
            while (iCur < iLast) {
                vCoords.push_back(std::pair<double,double>(pPoints[2*iCur], pPoints[2*iCur+1]));
                iCur++;
            }
        }
        /*
        printf("Total theor: %zd (actually: %d\n", 4*sizeof(double) + 2*sizeof(int) + iNumParts*sizeof(int)+iNumPoints*2*sizeof(double), iBufSize);
        printf("left: %ld\n", p-pBuf);
        */
        iResult = 0;
    } else {
        printf("Couldn't read shape data\n");
    }
    delete[] pBuf;
    return iResult;
}
