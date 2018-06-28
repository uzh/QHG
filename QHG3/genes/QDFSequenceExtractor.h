#ifndef __QDF_SEQUENCE_EXTRACTOR_H__
#define __QDF_SEQUENCE_EXTRACTOR_H__

#include <hdf5.h>
#include "types.h"

#include "SequenceProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"


template<typename T>
class QDFSequenceExtractor : public SequenceProvider<T> {

public:
    typedef std::map<idtype, T *>  sequencemap;

  
    virtual ~QDFSequenceExtractor();
    
    // use number & distance from file
    virtual int createSelection(const char *pLocList, const char *pRefFile, bool bDense, int iNumPerBuf, const char *pSampIn, const char *pSampOut);
    // specify number and distance
    virtual int createSelection(const char *pLocList, int iNumSamp, double dSampDist, const char *pRefFile, bool bDense, int iNumPerBuf, const char *pSampIn, const char *pSampOut);

    virtual int getSequenceSize() { return m_iSequenceSize;};
    virtual const T *getSequence(idtype iID);

    int                  getNumSelected() { return m_iNumSelected;};
    
    const loc_data      &getLocData() { return m_mLocData;};
    const sequencemap   &getSequences() { return m_mSequences;};
    const idset         &getSelectedIDs() { return m_sSelected;};
    const arrpos_ids    &getIndexIDMap() { return m_mSelected;};
    int                  getNumRefSelected() { return m_iNumRefSelected;};
    
    const loc_ids       &getLocIDs() {return m_mLocIDs;};

    const IDSample *getSample() const { return m_pCurSample;};
    const IDSample *getRefSample() const { return m_pRefSample;};
    
    void setVerbose(bool bVerbose) {m_bVerbose = bVerbose;};
protected:
    QDFSequenceExtractor(WELL512 *pWELL, bool bCartesian);
    int init(const char *pGeoFile, 
             const char *pQDFFile, 
             const char *pSpeciesName, 
             const char *pAttrSequenceSize, 
             const char *pSequenceDataSetName);

    
    int getSelectedSequencesDense(const char *pSequenceDataSetName, int iNumPerBuf);
    int getSelectedSequencesSparse(const char *pSequenceDataSetName, int iNumPerBuf);

    virtual int extractAdditionalAttributes() = 0;
    virtual int calcNumBlocks() = 0;
    virtual hid_t readQDFSequenceSlab(hid_t hDataSet, hid_t hMemSpace, hid_t hDataSpace, T*aBuf)=0;

    char      *m_pPopName;
    char      *m_pQDFGeoFile;
    stringvec  m_vQDFPopFiles;
    char      *m_pSequenceDataSetName;

    int        m_iNumSelected;
    idset      m_sSelected;
    arrpos_ids m_mSelected;

    int        m_iNumRefSelected;
    idset      m_sRefSelected;
    arrpos_ids m_mRefSelected;

    IDSample  *m_pCurSample;
    IDSample  *m_pRefSample;

    loc_data   m_mLocData;    // location name -> (lon, lat, dist, num)
    

    hid_t m_hFile;
    hid_t m_hPopulation;
    hid_t m_hSpecies;

    int  m_iSequenceSize;
    uint m_iNumBlocks;

    sequencemap m_mSequences;
    loc_ids     m_mLocIDs;

    bool m_bCartesian;
    WELL512 *m_pWELL;
    bool m_bVerbose;
};


#endif
