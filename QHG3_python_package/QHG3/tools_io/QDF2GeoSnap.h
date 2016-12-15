#ifndef __QDF2GEOSNAP_H__
#define __QDF2GEOSNAP_H__

#include <hdf5.h>
#include "QDF2SnapBase.h"

class QDF2GeoSnap : public QDF2SnapBase {
public:
    static QDF2GeoSnap *createInstance(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);
    virtual ~QDF2GeoSnap();

    int init(hid_t hFile, int iNumCells, const char *pAttr, const char *pParam);

    virtual int fillSnapData(char *pBuffer);
protected:
    QDF2GeoSnap();

    int fillIceData(char *pBuffer);
    int fillAvgDistData(char *pBuffer);

    const char *m_pParam;


};

#endif

