#ifndef __CLIMATEGROUPREADER_H__
#define __CLIMATEGROUPREADER_H__

#include <hdf5.h>

class Climate;

struct ClimateAttributes : Attributes {
    int     m_iNumSeasons;
    bool    m_bDynamic;
};


class ClimateGroupReader : public GroupReader<Climate, ClimateAttributes> {

public:
    static ClimateGroupReader *createClimateGroupReader(const char *pFileName);
    static ClimateGroupReader *createClimateGroupReader(hid_t hFile);

    int tryReadAttributes(ClimateAttributes *pAttributes);
    int readArray(Climate *pGroup, const char *pArrayName);
    int readData(Climate *pGroup);
    
protected:
    ClimateGroupReader();

};




#endif
