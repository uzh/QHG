#ifndef __GEOGROUPREADER_H__
#define __GEOGROUPREADER_H__

#include <hdf5.h>
#include "GroupReader.h"

class Geography;


struct GeoAttributes : Attributes {
    uint    m_iMaxNeighbors;
    double  m_dRadius;
    double  m_dSeaLevel;
};

class GeoGroupReader : public GroupReader<Geography, GeoAttributes> {

public:
    static GeoGroupReader *createGeoGroupReader(const char *pFileName);
    static GeoGroupReader *createGeoGroupReader(hid_t hFile);

    virtual int tryReadAttributes(GeoAttributes *pAttributes);
    virtual int readArray(Geography *pGroup, const char *pArrayName);
    virtual int readData(Geography *pGroup);

protected:
    GeoGroupReader();
};



#endif
