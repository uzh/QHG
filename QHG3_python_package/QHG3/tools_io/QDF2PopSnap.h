#ifndef __QDF2POPSNAP_H__
#define __QDF2POPSNAP_H__

#include <hdf5.h>

class QDF2PopSnap {
public:
    static QDF2PopSnap *createInstance(hid_t hFile, int iNumCells, const char *pSpecies, const char *pGridFile);
    ~QDF2PopSnap();

    int init(hid_t hFile, int iNumCells, const char *pSpecies, const char *pGridFile);

    int fillSnapData(char *pBuffer);
protected:
    QDF2PopSnap();
    hid_t createSCellAgentType();

    int   m_iNumCells;
    hid_t m_hSpeciesGroup;
    hid_t m_hDataSetPop;
    hid_t m_hDataSpacePop; 
    hid_t m_hGridGroup;
    hid_t m_hDataSetGrid;
    hid_t m_hDataSpaceGrid;

    const char *m_pSpecies;


};

#endif
