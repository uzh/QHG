#ifndef __QDF2CLIMATESNAP_H__
#define __QDF2CLIMATESNAP_H__

#include <hdf5.h>
#include "QDF2SnapBase.h"

class QDF2ClimateSnap : public QDF2SnapBase {
public:
    static QDF2ClimateSnap *createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);
    virtual ~QDF2ClimateSnap();

    int init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);

    virtual int fillSnapData(char *pBuffer);
protected:
    QDF2ClimateSnap();


    const char *m_pParam;


};

#endif

