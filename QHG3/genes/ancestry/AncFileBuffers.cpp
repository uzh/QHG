#include <stdio.h>
#include <string.h>
#include <omp.h>
#include "AncFileBuffers.h"

//----------------------------------------------------------------------------
// createInstance
//
AncFileBuffers *AncFileBuffers::createInstance(const char *pTempFile, int iCount, int iBufSize, int iAncSize) {
    AncFileBuffers *pAB = new AncFileBuffers(iCount, iBufSize, iAncSize);
    int iResult = pAB->init(pTempFile);
    if (iResult != 0) {
        delete pAB;
        pAB = NULL;
    }
    return pAB;
}

//----------------------------------------------------------------------------
// constructor
//
AncFileBuffers::AncFileBuffers(int iCount, int iBufSize, int iAncSize) 
    : m_afIn(NULL),
      m_aaBuf(NULL),
      m_aIndexes(NULL),
      m_aNumRead(NULL),
      m_iCount(iCount),
      m_iBufSize(iBufSize),
      m_iAncSize(iAncSize) {
    m_pData = new idtype[m_iAncSize];
    memset(m_pData, 0, m_iAncSize*sizeof(idtype));

}

//----------------------------------------------------------------------------
// destructor
//
AncFileBuffers::~AncFileBuffers() {
    if (m_afIn != NULL) {
        for (int i = 0; i < m_iCount; i++) {
            if (m_afIn[i] != NULL) {
                fclose(m_afIn[i]);
            }
        }
        delete[] m_afIn;
    }

    if (m_aaBuf != NULL) {
        for (int i = 0; i < m_iCount; i++) {
            delete[] m_aaBuf[i];
        }
        delete[] m_aaBuf;
    }
    if (m_aIndexes != NULL) {
        delete[] m_aIndexes;
    }
    if (m_aNumRead != NULL) {
        delete[] m_aNumRead;
    }

    delete[] m_pData;
}

//----------------------------------------------------------------------------
// init
//
int AncFileBuffers::init(const char *pTempFile) {
    int iResult = 0;

    

    m_afIn     = new FILE *[m_iCount];
    memset(m_afIn, 0, m_iCount*sizeof(FILE*));
    m_aaBuf    = new idtype*[m_iCount];
    memset(m_aaBuf, 0, m_iCount*sizeof(idtype*));
    m_aIndexes = new int[m_iCount];
    memset(m_aIndexes, 0, m_iCount*sizeof(int));
    m_aNumRead = new int[m_iCount];
    memset(m_aNumRead, 0, m_iCount*sizeof(int));
    /*    
#pragma omp parallel for 
    for (int i = 0; i < m_iCount; i++) {
    */
    for (int i = 0; (iResult == 0) && (i < m_iCount); i++) {
        m_aaBuf[i] = new idtype[m_iBufSize];
        
        char sFileName[256];
        sprintf(sFileName, "%s_%06d", pTempFile, i);
        m_afIn[i] = fopen(sFileName, "rb");
        if (m_afIn[i] != NULL) {
            
            m_aIndexes[i] = 0;
            m_aNumRead[i] = (int)fread(m_aaBuf[i], sizeof(idtype), m_iBufSize, m_afIn[i]);
            if (m_aNumRead[i] > 0) {
                //                printf("loaded %d records from file [%s] (bufsize %d)\n", m_aNumRead[i], sFileName, m_iBufSize);
            } else {
                    printf("failed to read from file [%s] (bufsize %d)\n",sFileName, m_iBufSize);
                    iResult--;
            }
        } else {
            printf("Thread %d: Couldn't open input file [%s]\n", omp_get_thread_num(), sFileName);
            iResult--;
        }
        m_mMin[m_aaBuf[i][0]] = i;

    }

    return iResult;
}


//----------------------------------------------------------------------------
// getSmallestRecord
//
const idtype *AncFileBuffers::getSmallestRecord() {
    const idtype *p=NULL;

    if (m_mMin.size() > 0) {
        int iMinIndex = m_mMin.begin()->second;

        memcpy(m_pData, m_aaBuf[iMinIndex]+m_aIndexes[iMinIndex], m_iAncSize*sizeof(idtype));
        p = m_pData; 
        m_mMin.erase(m_mMin.begin());

        bool bUpdate = true;
        m_aIndexes[iMinIndex] += m_iAncSize;
        if (m_aIndexes[iMinIndex] >= m_aNumRead[iMinIndex]) {
            m_aNumRead[iMinIndex] = (int)fread(m_aaBuf[iMinIndex], sizeof(idtype), m_iBufSize, m_afIn[iMinIndex]);
            if (m_aNumRead[iMinIndex] > 0) {
                // new block read: reset index
                m_aIndexes[iMinIndex] = 0;
            } else {
	        // file finished: delete buffer and map entry
                delete[] m_aaBuf[iMinIndex];
                m_aaBuf[iMinIndex] = NULL;

                bUpdate = false;
            }
        }
        if (bUpdate) {
            // insert next minimal val for this index
            m_mMin[m_aaBuf[iMinIndex][m_aIndexes[iMinIndex]]] = iMinIndex;
        }
    }
    return p;

}

//----------------------------------------------------------------------------
// getSmallestRecordOld
//
const idtype *AncFileBuffers::getSmallestRecordOld() {
    idtype  iMin = -1;
    int     iMinIndex = -1;
    idtype *p = NULL;

    for (int i = 0; i < m_iCount; i++) {
        if ((m_aaBuf[i] != NULL) && ((iMin < 0) || (m_aaBuf[i][m_aIndexes[i]] < iMin))) {
            iMin = m_aaBuf[i][m_aIndexes[i]];
            iMinIndex = i;
        }
    }
    if (iMinIndex >= 0) {
        memcpy(m_pData, m_aaBuf[iMinIndex]+m_aIndexes[iMinIndex], m_iAncSize*sizeof(idtype));
        p = m_pData;       
        m_aIndexes[iMinIndex] += m_iAncSize;
        if (m_aIndexes[iMinIndex] >= m_aNumRead[iMinIndex]) {
            m_aNumRead[iMinIndex] = (int)fread(m_aaBuf[iMinIndex], sizeof(idtype), m_iBufSize, m_afIn[iMinIndex]);
            if (m_aNumRead[iMinIndex] > 0) {
//	        printf("loaded additional %d records from [%d]\n", m_aNumRead[iMinIndex], iMinIndex); 
                m_aIndexes[iMinIndex] = 0;
            } else {
	        // nothing left for next round
                delete[] m_aaBuf[iMinIndex];
                m_aaBuf[iMinIndex] = NULL;
            }
        }
    }
    return p;

}
