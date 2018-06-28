#ifndef __NAVGROUPREADER_H__
#define __NAVGROUPREADER_H__

#include <hdf5.h>

class Navigation;

struct NavAttributes : Attributes {
    uint     m_iNumPorts;
    uint     m_iNumDests;
    uint     m_iNumDists;
    double   m_dSampleDist;
};


class NavGroupReader : public GroupReader<Navigation, NavAttributes> {

public:
    static NavGroupReader *createNavGroupReader(const char *pFileName);
    static NavGroupReader *createNavGroupReader(hid_t hFile);

    int tryReadAttributes(NavAttributes *pAttributes);
    int readArray(Navigation *pGroup, const char *pArrayName);
    int readData(Navigation *pGroup);
    
protected:
    NavGroupReader();

};




#endif
