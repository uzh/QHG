#ifndef __MOVESTATREADER_H__
#define __MOVESTATREADER_H__

#include <hdf5.h>


class MoveStatReader {

public:
    static MoveStatReader *createMoveStatReader(const char *pFileName);
    static MoveStatReader *createMoveStatReader(hid_t hFile);
    ~MoveStatReader();

    int readAttributes(uint *piNumCells);
    int readData(MoveStats *pMS);
    
protected:
    MoveStatReader();
    int init(const char *pFileName);
    int init(hid_t hFile);

    hid_t m_hFile;
    hid_t m_hMoveStatGroup;
    uint  m_iNumCells;
   
};




#endif
