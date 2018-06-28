#ifndef __POPREADER_H__
#define __POPREADER_H__

#include <vector>
#include <hdf5.h>
#include "types.h"

#define NAMESIZE 128

class PopBase;
typedef struct {
    int  m_iClassID;
    char m_sClassName[NAMESIZE];
    int  m_iSpeciesID;
    char m_sSpeciesName[NAMESIZE];
    int  m_iNumCells;
} popinfo;

typedef std::vector<popinfo> popinfolist;

const int POP_READER_ERR_NO_POP_GROUP      = -1;
const int POP_READER_ERR_CELL_MISMATCH     = -2;
const int POP_READER_ERR_READ_SPECIES_DATA = -3;
const int POP_READER_ERR_NO_SPECIES_GROUP  = -4;
class PopReader {
public:

    static PopReader *create(const char *pFilename);
    static PopReader *create(hid_t hFile);
    ~PopReader();
    
    int open(const char *pFilename);
    int open(hid_t hFile);
    const popinfolist &getPopList() { return m_vPopList;};
    int read(PopBase *pPB, const char *pSpeciesName, int iNumCells, bool bRestore);

protected:
    PopReader();
   
    popinfolist m_vPopList;
    hid_t       m_hFile;

    
    hid_t       m_hPopGroup;
    hid_t       m_hSpeciesGroup;
   
    bool        m_bOpenedFile;
};


#endif
