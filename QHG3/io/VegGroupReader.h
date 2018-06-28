#ifndef __VEGGROUPREADER_H__
#define __VEGGROUPREADER_H__

#include <hdf5.h>

class Vegetation;

struct VegAttributes : Attributes {
    int     m_iNumVegSpc;
};


class VegGroupReader : public GroupReader<Vegetation, VegAttributes> {

public:
    static VegGroupReader *createVegGroupReader(const char *pFileName);
    static VegGroupReader *createVegGroupReader(hid_t hFile);

    int tryReadAttributes(VegAttributes *pAttributes);
    int readArray(Vegetation *pGroup, const char *pArrayName);
    int readData(Vegetation *pGroup);
    
protected:
    VegGroupReader();

};




#endif
