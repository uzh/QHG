#ifndef __MOVESTATGROUPREADER_H__
#define __MOVESTATGROUPREADER_H__

#include <hdf5.h>

class MoveStats;

struct MoveStatAttributes : Attributes {
};


class MoveStatGroupReader : public GroupReader<MoveStats, MoveStatAttributes> {

public:
    static MoveStatGroupReader *createMoveStatGroupReader(const char *pFileName);
    static MoveStatGroupReader *createMoveStatGroupReader(hid_t hFile);

    int tryReadAttributes(MoveStatAttributes *pAttributes);
    int readArray(MoveStats *pGroup, const char *pArrayName);
    int readData(MoveStats *pGroup);
    
protected:
    MoveStatGroupReader();

};




#endif
