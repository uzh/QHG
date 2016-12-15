#ifndef __GEOREADER_H__
#define __GEOREADER_H__

#include <hdf5.h>

class Geography;

class GeoReader {

public:
    static GeoReader *createGeoReader(const char *pFileName);
    static GeoReader *createGeoReader(hid_t hFile);
    ~GeoReader();

    int readAttributes(uint *piNumCells, int *piMaxNeighbors, double *pdRadius, double *pdSeaLevel);
    int readData(Geography *pGG);
    
protected:
    GeoReader();
    int init(const char *pFileName);
    int init(hid_t hFile);

    hid_t m_hFile;
    hid_t m_hGeoGroup;
    uint  m_iNumCells;
    uint  m_iMaxNeighbors;

};




#endif
