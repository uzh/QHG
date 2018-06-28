#ifndef __QDF_GENOME_EXTRACTOR_H__
#define __QDF_GENOME_EXTRACTOR_H__

#include <hdf5.h>
#include "types.h"
#include "WELL512.h"

#include "GenomeProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"

typedef std::map<idtype, ulong *>  genomemap;


class QDFGenomeExtractor : public GenomeProvider {

public:
    static QDFGenomeExtractor *createInstance(const char *pQDFFile, 
                                              const char *pSpeciesName, 
                                              const char *pAttrGenomeSize,
                                              const char *pAttrBitsPerNuc, 
                                              const char *pDataSetGenome,
                                              WELL512 *pWELL,
                                              bool bCartesian=false);

    static QDFGenomeExtractor *createInstance(const char *pGeoFile, 
                                              const char *pQDFFile, 
                                              const char *pSpeciesName, 
                                              const char *pAttrGenomeSize,
                                              const char *pAttrBitsPerNuc, 
                                              const char *pDataSetGenome,
                                              WELL512 *pWELL,
                                              bool bCartesian=false);

    virtual ~QDFGenomeExtractor();
    
    int createSelection(const char *pLocList, const char *pRefFile, bool bDense, int iNumPerBuf=-1);
    int createSelection(const char *pLocList, int iNumSamp, double dSampDist, const char *pRefFile, bool bDense, int iNumPerBuf=-1);

    virtual int getGenomeSize() { return m_iGenomeSize;};
    virtual const ulong *getGenome(idtype iID);

    const loc_data       &getLocData() { return m_mLocData;};
    const genomemap     &getGenomes() { return m_mGenomes;};
    int                  getNumSelected() { return m_iNumSelected;};
    const idset         &getSelectedIDs() { return m_sSelected;};
    const arrpos_ids    &getIndexIDMap() { return m_mSelected;};
    int                  getNumRefSelected() { return m_iNumRefSelected;};
    uint                 getBitsPerNuc() { return m_iBitsPerNuc;};

    const IDSample *getSample() const { return m_pCurSample;};
    const IDSample *getRefSample() const { return m_pRefSample;};
    
    void setVerbose(bool bVerbose) {m_bVerbose = bVerbose;};
protected:
    QDFGenomeExtractor(WELL512 *pWELL, bool bCartesian);
    int init(const char *pGeoFile, 
             const char *pQDFFile, 
             const char *pSpeciesName, 
             const char *pAttrGenomeSize, 
             const char *pAttrBitsPerNuc, 
             const char *pDataSetGenome);

    
    int getSelectedGenesDense(const char *pDataSetGenome, int iNumPerBuf);
    int getSelectedGenesSparse(const char *pDataSetGenome, int iNumPerBuf);
    

    char       *m_pPopName;
    char       *m_pQDFGeoFile;
    stringvec   m_vQDFPopFiles;
    char       *m_pDataSetGenome;

    int         m_iNumSelected;
    idset       m_sSelected;
    arrpos_ids  m_mSelected;

    int         m_iNumRefSelected;
    idset       m_sRefSelected;
    arrpos_ids  m_mRefSelected;

    IDSample   *m_pCurSample;
    IDSample   *m_pRefSample;

    loc_data        m_mLocData;    // location name -> (lon, lat, dist, num)
    

    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    int  m_iGenomeSize;
    uint m_iNumBlocks;
    uint m_iBitsPerNuc;

    genomemap m_mGenomes;

    bool m_bCartesian;

    WELL512 *m_pWELL;

    bool m_bVerbose;

};


#endif

