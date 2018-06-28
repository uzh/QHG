#ifndef __QDF_GENOME_EXTRACTOR2_H__
#define __QDF_GENOME_EXTRACTOR2_H__


#include <hdf5.h>
#include "types.h"
#include "WELL512.h"

#include "SequenceProvider.h"
#include "QDFSequenceExtractor.h"

class QDFGenomeExtractor2 : public QDFSequenceExtractor<ulong> {
public:  
    static QDFGenomeExtractor2 *createInstance(const char *pQDFFile, 
                                               const char *pSpeciesName, 
                                               const char *pAttrGenomeSize,
                                               const char *pAttrBitsPerNuc, 
                                               const char *pDataSetGenome,
                                               WELL512 *pWELL,
                                               bool bCartesian=false);
    
    static QDFGenomeExtractor2 *createInstance(const char *pGeoFile, 
                                               const char *pQDFFile, 
                                               const char *pSpeciesName, 
                                               const char *pAttrGenomeSize,
                                               const char *pAttrBitsPerNuc, 
                                               const char *pDataSetGenome,
                                               WELL512 *pWELL,
                                               bool bCartesian=false);

    virtual ~QDFGenomeExtractor2();
    uint                 getBitsPerNuc() { return m_iBitsPerNuc;};

protected:
    QDFGenomeExtractor2(WELL512 *pWELL, bool bCartesian);

    int init(const char *pQDFGeoFile, 
             const char *pQDFPopFile, 
             const char *pSpeciesName, 
             const char *pAttrGenomeSize,
             const char *pAttrBitsPerNuc,
             const char *pDataSetGenome);

    virtual int extractAdditionalAttributes();
    virtual int calcNumBlocks();
    virtual hid_t readQDFSequenceSlab(hid_t hDataSet, hid_t hMemSpace, hid_t hDataSpace, ulong *aBuf);

    uint m_iBitsPerNuc;
    char      *m_pAttrBitsPerNuc;

};


#endif

