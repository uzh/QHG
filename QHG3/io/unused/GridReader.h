#ifndef __GRIDREADER_H__
#define __GRIDREADER_H__

#include <map>
#include <string>
#include <hdf5.h>

class SCellGrid;

typedef std::map<std::string, std::string> stringmap;

class GridReader {

public:
    static GridReader *createGridReader(const char *pFileName);
    static GridReader *createGridReader(hid_t hFile);
    ~GridReader();

    int readAttributes(int *piNumCells, stringmap &smData);
    int readCellData(SCellGrid *pCG);
    
protected:
    GridReader();
    int init(const char *pFileName);
    int init(hid_t hFile);
    hid_t createCellDataType();

    hid_t m_hFile;
    hid_t m_hGridGroup;
    int   m_iNumCells;

};




#endif

