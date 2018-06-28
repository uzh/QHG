#ifndef __QDFALLGENOMES_H__
#define __QDFALLGENOMES_H__

#include <hdf5.h>
#include "types.h"


class QDFAllGenomes  {

public:
    static QDFAllGenomes *createInstance(const char *pQDFFile, const char *pSpeciesName, const char *pAttrGenomeSize, const char *pDataSetGenome);

    virtual ~QDFAllGenomes();
    

    int extractGenomes();

    int getGenomeSize() { return m_iGenomeSize;};
    int getNumGenomes() { return m_iNumGenomes;};
    int getNumBlocks()  { return m_iNumBlocks;};

    ulong **getGenomes() { return m_ppGenomes;};
    

protected:
    QDFAllGenomes();
    int init(const char *pQDFFile, const char *pSpeciesName, const char *pAttrGenomeSize, const char *pDataSetGenome);

    
    

    char *m_pPopName;
    char *m_pDataSetGenome;


    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    int m_iGenomeSize;
    int m_iNumBlocks;

    int      m_iNumGenomes;
    ulong **m_ppGenomes;
    ulong *m_pGenomeData;
};


#endif

