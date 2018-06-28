#ifndef __VEGWRITER_H__
#define __VEGWRITER_H__

class Vegetation;

class VegWriter {
public:
    VegWriter(Vegetation *pVeg);
    int writeToQDF(const char *pFileName, float fTime);
    int write(hid_t hFile);

protected:
    Vegetation *m_pVeg;
    int writeVegAttributes(hid_t hGeoGroup);
 
};

#endif
