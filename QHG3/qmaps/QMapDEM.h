#ifndef __QMAPDEM_H__
#define __QMAPDEM_H__

#include "DEM.h"
#include "ValReader.h"

class QMapDEM : public DEM {

public:
    QMapDEM();
    ~QMapDEM();

    virtual bool load(char *pName);
    virtual double getAltitude(double dLon, double dLat);

private:
    ValReader *m_pVR;
};

#endif
