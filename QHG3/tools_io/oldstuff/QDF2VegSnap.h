#ifndef __QDF2VEGSNAP_H__
#define __QDF2VEGSNAP_H__

#include <hdf5.h>
#include "QDF2SnapBase.h"

class QDF2VegSnap : public QDF2SnapBase {
public:
    static QDF2VegSnap *createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);
    virtual ~QDF2VegSnap();

    int init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);

    virtual int fillSnapData(char *pBuffer);
protected:
    QDF2VegSnap();

    int fillSubArrayData(char *pBuffer);

    const char *m_pParam;
    int m_iVeg;


};

#endif

