#ifndef __RGENERATION_H__
#define __RGENERATION_H__

#include "types.h"


class RGeneration {

public:
    static RGeneration *createInstance(int iThisGen, idset &sIDs, int iLatency, const char *pFileBody=NULL);
    ~RGeneration();

    bool contains(idtype iID, int iCurGen);
    idtype getMin() {return m_iMinID;};
    idtype getMax() {return m_iMaxID;};
    const idset &getSet();
    int saveData();
    void prepare() { m_iLastToucher = m_iThisGen;};
    int decideSave(int iCurGen);
    void showData();
private:
    RGeneration();
    int init(int iThisGen, idset &sIDs, int iLatency, const char *pFileBody);
    int loadData();
    idset m_sIDs;
    idtype m_iMinID;
    idtype m_iMaxID;
    int m_iThisGen;
    int m_iLatency;
    int m_iLastToucher;
    int m_iUntouched;
    bool m_bSaved;
    bool m_bLoaded;
    char *m_pFileName;
    idtype *m_piFileBuf;
    uint m_iBufSize;
};


#endif

