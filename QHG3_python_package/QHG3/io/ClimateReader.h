#ifndef __CLIMATEREADER_H__
#define __CLIMATEREADER_H__

#include <hdf5.h>

class Climate;
class ClimateReader {

public:
    static ClimateReader *createClimateReader(const char *pFileName);
    static ClimateReader *createClimateReader(hid_t hFile);
    ~ClimateReader();

    int readAttributes(uint *piNumCells, int *piNumSeason, bool *bpDynamic);
    int readData(Climate *pC);
    
protected:
    ClimateReader();
    int init(const char *pFileName);
    int init(hid_t hFile);

    hid_t m_hFile;
    hid_t m_hClimateGroup;
    uint  m_iNumCells;
   
   
};




#endif
