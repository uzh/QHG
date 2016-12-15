#ifndef __VEGREADER_H__
#define __VEGREADER_H__

#include <hdf5.h>

class Vegetation;
class VegReader {

public:
    static VegReader *createVegReader(const char *pFileName);
    static VegReader *createVegReader(hid_t hFile);
    ~VegReader();

    int readAttributes(uint *piNumCells, int *piNumVegSpc, bool *bpDynamic);
    int readData(Vegetation *pV);
    
protected:
    VegReader();
    int init(const char *pFileName);
    int init(hid_t hFile);

    hid_t m_hFile;
    hid_t m_hVegGroup;
    uint  m_iNumCells;
    int   m_iNumVegSpc;
   
   
};




#endif

