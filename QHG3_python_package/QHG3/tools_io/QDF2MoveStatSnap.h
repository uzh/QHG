#ifndef __QDF2MOVESTATSNAP_H__
#define __QDF2MOVESTATSNAP_H__

#include <hdf5.h>
#include "QDF2SnapBase.h"

class QDF2MoveStatSnap : public QDF2SnapBase {
public:
    static QDF2MoveStatSnap *createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);
    virtual ~QDF2MoveStatSnap();

    int init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);

    virtual int fillSnapData(char *pBuffer);
protected:
    QDF2MoveStatSnap();
  
    const char *m_pParam;


};

#endif
