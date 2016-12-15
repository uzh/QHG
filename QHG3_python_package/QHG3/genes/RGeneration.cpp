#include <stdio.h>
#include <string.h>

#include <set>

#include "types.h"
#include "colors.h"
#include "RGeneration.h"

#define DEF_BODY "rgeneration"
#define BUFSIZE 100000

//----------------------------------------------------------------------------
// createInstance
// 
RGeneration *RGeneration::createInstance(int iCurGen, idset &sIDs, int iLatency, const char *pFileBody) {
    RGeneration *pRG = new RGeneration();
    int iResult = pRG->init(iCurGen, sIDs, iLatency, pFileBody);
    if (iResult != 0) {
        delete pRG;
        pRG = NULL;
    }
    return pRG;
}

//----------------------------------------------------------------------------
// constructor
// 
RGeneration::RGeneration() 
    : m_iMinID(-1),
      m_iMaxID(-1),
      m_iThisGen(-1),
      m_iLatency(-1),
      m_bSaved(false),
      m_bLoaded(false),
      m_pFileName(NULL),
      m_piFileBuf(NULL),
      m_iBufSize(-1) {
}


//----------------------------------------------------------------------------
// destructor
// 
RGeneration::~RGeneration() {
    // delete file
    //    printf("file [%s] is %ssaved\n", m_pFileName, m_bSaved?"":"not");
    if (m_bSaved) {
        remove(m_pFileName);
    }

    if (m_pFileName != NULL) {
        delete[] m_pFileName;
    }
    if (m_piFileBuf != NULL) {
        delete[] m_piFileBuf;
    }
}


//----------------------------------------------------------------------------
// init
// 
int RGeneration::init(int iThisGen, idset &sIDs, int iLatency, const char *pFileBody) {
    int iResult = 0;

    m_iThisGen   = iThisGen;
    m_iLatency   = iLatency;
    m_iLastToucher = iThisGen;
    m_sIDs.insert(sIDs.begin(), sIDs.end());
    if (m_sIDs.empty()) {
        m_iMinID = -1;
        m_iMaxID = -1;
    } else {
        m_iMinID = *(m_sIDs.begin());
        m_iMaxID = *(m_sIDs.rbegin());
    }
    m_bLoaded = true;

    const char *pBody;
    if (pFileBody != NULL) {
        pBody = pFileBody;
    } else {
        pBody = DEF_BODY;
    }
    ulong iL = strlen(pBody)+1+6+4+1;
    m_pFileName = new char[iL];
    sprintf(m_pFileName, "%s_%06d.bin", pBody, m_iThisGen);

    m_iBufSize = BUFSIZE;
    m_piFileBuf = new idtype[m_iBufSize];

    return iResult;
}

//----------------------------------------------------------------------------
// contains
// 
bool RGeneration::contains(idtype iID, int iCurGen) {
    int iResult = -1;
    if ((m_iMinID <= iID) && (m_iMaxID >=  iID)) {

        if (!m_bLoaded) {
            //            printf("hit: %ld <= %ld <= %ld...", m_iMinID, iID, m_iMaxID);
            iResult = loadData();
        } else {
            iResult = 0;
        }

        if ((iResult == 0) && (m_sIDs.find(iID) != m_sIDs.end())) {
            m_iLastToucher = iCurGen;
        } else {
            iResult = -1;
        }
    }
    return (iResult == 0);
}

//----------------------------------------------------------------------------
// decideSave
// 
int RGeneration::decideSave(int iCurGen) {
    int iResult = 0;
    //    printf("RG %d: lastTouch %d diff %d\n", m_iThisGen, m_iLastToucher, iCurGen - m_iLastToucher);
    if ((iCurGen - m_iLastToucher) >= m_iLatency) {
        iResult = saveData();
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getSet
// 
const idset &RGeneration::getSet() { 
   
    if (!m_bLoaded) {
        loadData();
    }
    return m_sIDs;
}


//----------------------------------------------------------------------------
// saveData
// 
int RGeneration::saveData() {
    int iResult = -1;
    if (!m_bSaved) {
        FILE *fOut = fopen(m_pFileName, "wb");
        if (fOut != NULL) {
            iResult = 0;
            uint iSize = (uint)m_sIDs.size();
            idset_cit it = m_sIDs.begin();
            while ((iResult == 0) && (iSize > 0)) {
                // copy next block to buffer
                uint iNum = (iSize>m_iBufSize)?m_iBufSize:iSize;
                idtype *pCur = m_piFileBuf;
                for (uint i = 0; i < iNum; i++) {
                    *pCur = *it;
                    pCur++;
                    it++;
                }
                ulong iWritten = fwrite(m_piFileBuf, sizeof(idtype), iNum, fOut);
                if (iWritten == iNum) {
                    iSize -= iNum;
                } else {
                    printf("%sError while writing to [%s]\n", RED, m_pFileName);
                    iResult = -1;
                }
            }
            /*
            if (iResult == 0) {
                printf("@@saved %d: L:%s, S:%s@@\n", m_iThisGen, m_bLoaded?"yes":"no", m_bSaved?"yes":"no");
            }
            */
            fclose(fOut);
        } else {
            printf("%sCouldn't open [%s] for writing\n", RED, m_pFileName);
        }
    } else {
        iResult = 0;
    }
    if (iResult == 0) {
        if (m_bLoaded) {
            //            printf("Just passing through %d ...\n", m_iThisGen);
            m_sIDs.clear();
            m_bSaved = true;
            m_bLoaded = false;
        }
    }

    return iResult;
}


//----------------------------------------------------------------------------
// loadData
// 
int RGeneration::loadData() {
    int iResult = -1;
    if (m_bSaved) {
        if (!m_bLoaded) {
            FILE *fIn = fopen(m_pFileName, "rb");
            if (fIn != NULL) {
                iResult = 0;
                while ((iResult == 0) && !feof(fIn)) {
                    ulong iRead = fread(m_piFileBuf, sizeof(idtype), m_iBufSize, fIn);
                    if ((iRead == m_iBufSize) || feof(fIn))  {
                        m_sIDs.insert(m_piFileBuf, m_piFileBuf+iRead);

                    } else {
                        iResult = -1;
                        printf("%sError while reading from [%s]\n", RED, m_pFileName);
                    }
                }
                if (iResult == 0) {
                    //                    printf("@@loaded %d: L:%s, S:%s@@\n", m_iThisGen, m_bLoaded?"yes":"no", m_bSaved?"yes":"no");
                    m_bLoaded = true;
                }
                fclose(fIn);
            } else {
                printf("%sCouldn't open [%s] for writing\n", RED, m_pFileName);
            }
        } else {
            // already loaded
            iResult = 0;
        }
    } else {
        printf("%sCan't load unsaved data\n", RED);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// showData
// 
void RGeneration::showData() {

   printf("IDs ");
   for (idset_cit ii = m_sIDs.begin(); ii != m_sIDs.end(); ++ii) {
       printf(" %ld", *ii);
   }
   printf("\n");
}
