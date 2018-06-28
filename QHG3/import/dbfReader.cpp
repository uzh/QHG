#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <map>

#include "shpUtils.h"
#include "dbfReader.h"


//----------------------------------------------------------------------------
// constructor
//
dbfReader::dbfReader(FILE *fIn) :
    m_fIn(fIn),
    m_iNumRecords(0),
    m_iHeaderSize(0),
    m_iRecordSize(0) {

}

//----------------------------------------------------------------------------
// read
//
int dbfReader::read(const char *pFieldName, vecdouble &vVals) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = readHeader();
    }

    if (iResult == 0) {
        nameoffsets::const_iterator it;
        /*
        printf("Field names, offsets + lengths\n");
        for (it = m_mOffsets.begin(); it != m_mOffsets.end(); ++it) {
            printf("%12s: %d %d\n", it->first.c_str(), it->second.first, it->second.second);
        }
        */
        if (pFieldName != NULL) {
            it = m_mOffsets.find(pFieldName);
            if (it != m_mOffsets.end()) {
                iResult = readRecords(it->second.first, it->second.second, vVals);
            } else {
                printf("No field with name [%s] found\n", pFieldName);
                iResult =-1;
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readHeader
//
int dbfReader::readHeader() {
    int iResult = -1;
    uchar pBuf[DBF_HEADER_SIZE];

    int iRead = fread(pBuf, 1, DBF_HEADER_SIZE, m_fIn);
    if (iRead == DBF_HEADER_SIZE) {
        uchar *p = pBuf;
        p += 4; // skip to the number of records
        p = shpUtils::getNum(p, &m_iNumRecords, LITTLEENDIAN);
        p = shpUtils::getNum(p, &m_iHeaderSize, LITTLEENDIAN);
        p = shpUtils::getNum(p, &m_iRecordSize, LITTLEENDIAN);
        iResult = 0;
        //        printf("DBF: %d records of size %d starting at pos %d\n", m_iNumRecords, m_iRecordSize, m_iHeaderSize);
        
        m_mOffsets.clear();
        iResult = readFieldDescriptors();
    } else {
        printf("Only read [%d] instead of [%d] bytes\n", iRead, DBF_HEADER_SIZE);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// readFieldDescriptors
//
int dbfReader::readFieldDescriptors() {
    int iResult = 0;
    uchar pBuf[DBF_DESC_SIZE];
    long lLastPos = ftell(m_fIn);
    int iRead = fread(pBuf, 1, DBF_DESC_SIZE, m_fIn);
    int iOffs = 0;
    while ((iResult == 0) && (iRead == DBF_DESC_SIZE) && (*pBuf != 0x0d)) {
        uchar *p = pBuf;
        char sName[11];
        memcpy(sName, pBuf, 11);
        p+= 11;
        char cType   = *p++;
        int iAddr;
        p = shpUtils::getNum(p, &iAddr, LITTLEENDIAN);
        uchar cLen  = *p++;
        //unused        uchar cDec  = *p++;

        //        printf("Field [%10s] offs[%3d], type %c, addr %d, len %3d, count %3d\n", sName, iOffs, cType, iAddr, cLen, cDec);
        if (cType == 'N') {
            m_mOffsets[sName] = std::pair<int,int>(iOffs, cLen);
        }
        iOffs += cLen;
        lLastPos = ftell(m_fIn);        
        iRead = fread(pBuf, 1, DBF_DESC_SIZE, m_fIn);
    }
    if (*pBuf == 0x0d) {
        fseek(m_fIn, lLastPos+1, SEEK_SET);
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readRecords
//
int dbfReader::readRecords(int iFieldOffset, int iLen, vecdouble &vVals) {
    int iResult = 0;  
    printf("Reading records\n");
    uchar *pBuf = new uchar[m_iRecordSize];

    int iCount = 0;
    int iRead = fread(pBuf, 1, m_iRecordSize, m_fIn);
    while ((iResult == 0) && (iRead == m_iRecordSize)) {

        uchar *p = pBuf+iFieldOffset+1;
        char *pVal = new char[iLen+1];
        memcpy(pVal, p, iLen);
        pVal[iLen] = '\0';


        char *pEnd;
        double dVal = strtod(pVal, &pEnd);
        if (*pEnd == '\0') {
            vVals.push_back(dVal);
        } else {
            printf("Non-numeric value found in record #%d: [%s]\n", iCount, pVal);
            iResult = -1;
        }
        iRead = fread(pBuf, 1, m_iRecordSize, m_fIn);
        iCount++;
    }
    //    if (iResult == 0) {
    //        printf("extracted from %d records\n", iCount);
    //    }
    return iResult;
}
