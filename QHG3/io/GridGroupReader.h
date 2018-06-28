#ifndef __GRIDGROUPREADER_H__
#define __GRIDGROUPREADER_H__

#include <map>
#include <string>
#include <hdf5.h>
#include "GroupReader.h"

class SCellGrid;

typedef std::map<std::string, std::string> stringmap;

struct GridAttributes : Attributes {
    stringmap smData;
};

class GridGroupReader : public GroupReader<SCellGrid, GridAttributes> {

public:
    static GridGroupReader *createGridGroupReader(const char *pFileName);
    static GridGroupReader *createGridGroupReader(hid_t hFile);

    virtual int tryReadAttributes(GridAttributes *pAttributes);
    virtual int readArray(SCellGrid *pGroup, const char *pArrayName);
    virtual int readData(SCellGrid *pGroup);

protected:
    GridGroupReader();
};



#endif
