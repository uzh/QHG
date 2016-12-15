#ifndef __QDF_GENOME_EXTRACTOR_H__
#define __QDF_GENOME_EXTRACTOR_H__

#include <hdf5.h>
#include "types.h"

#include "GenomeProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"

typedef std::map<idtype, ulong *>  genomemap;


class QDFGenomeExtractor : public GenomeProvider {

public:
    static QDFGenomeExtractor *createInstance(const char *pQDFFile, const char *pSpeciesName, const char *pAttrGenomeSize, const char *pDataSetGenome);
    static QDFGenomeExtractor *createInstance(const char *pGeoFile, const char *pQDFFile, const char *pSpeciesName, const char *pAttrGenomeSize, const char *pDataSetGenome);

    virtual ~QDFGenomeExtractor();
    
    int createSelection(const char *pLocList);

    virtual int getGenomeSize() { return m_iGenomeSize;};
    virtual const ulong *getGenome(idtype iID);

    const locdata       &getLocData() { return m_mLocData;};
    const genomemap     &getGenomes() { return m_mGenomes;};
    int                  getNumSelected() { return m_iNumSelected;};
    const idset         &getSelectedIDs() { return m_sSelected;};

    const IDSample *getSample() const { return m_pCurSample;};


protected:
    QDFGenomeExtractor();
    int init(const char *pGeoFile, const char *pQDFFile, const char *pSpeciesName, const char *pAttrGenomeSize, const char *pDataSetGenome);

    
    int getSelectedGenes(const char *pDataSetGenome);
    

    char *m_pPopName;
    char *m_pQDFGeoFile;
    stringvec m_vQDFPopFiles;
    char *m_pDataSetGenome;
    idset m_sSelected;
    indexids m_mSelected;

    IDSample *m_pCurSample;

    locdata        m_mLocData;    // location name -> (lon, lat, dist, num)
    

    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    int m_iGenomeSize;
    int m_iNumBlocks;

    genomemap m_mGenomes;

    int      m_iNumSelected;
};


#endif

