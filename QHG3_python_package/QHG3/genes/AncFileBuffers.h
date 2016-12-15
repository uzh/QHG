#ifndef __ANCFILEBUFFERS_H__
#define __ANCFILEBUFFERS_H__

#include "types.h"

class AncFileBuffers {
public:
    static AncFileBuffers *createInstance(const char *pTempFile, int iCount, int iBufSize, int iAncSize);
    ~AncFileBuffers();

    const idtype *getSmallestRecord();


    const idtype *getSmallestRecordOld();

protected:
    AncFileBuffers(int iCount, int iBufSize, int iAncSize);
    int init(const char *pTempFile);
    FILE   **m_afIn;
    idtype **m_aaBuf;
    int     *m_aIndexes;
    int     *m_aNumRead;
    int      m_iCount;
    int      m_iBufSize;
    int      m_iAncSize;
    idtype  *m_pData;

    idtype  *m_pData2;
    std::map<long, int> m_mMin;
};


#endif
