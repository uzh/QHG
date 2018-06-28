#ifndef __QDF_PHENOME_EXTRACTOR2_H__
#define __QDF_PHENOME_EXTRACTOR2_H__


#include <hdf5.h>
#include "types.h"

#include "SequenceProvider.h"
#include "QDFSequenceExtractor.h"

class QDFPhenomeExtractor2 : public QDFSequenceExtractor<float> {
public:  
    static QDFPhenomeExtractor2 *createInstance(const char *pQDFFile, 
                                                const char *pSpeciesName, 
                                                const char *pAttrGenomeSize,
                                                const char *pDataSetGenome,
                                                WELL512 *pWELL,
                                                bool bCartesian=false);
    
    static QDFPhenomeExtractor2 *createInstance(const char *pGeoFile, 
                                                const char *pQDFFile, 
                                                const char *pSpeciesName, 
                                                const char *pAttrGenomeSize,
                                                const char *pDataSetGenome,
                                                WELL512 *pWELL,
                                                bool bCartesian=false);

    virtual ~QDFPhenomeExtractor2() {};

protected:
    QDFPhenomeExtractor2(WELL512 *pWELL, bool bCartesian);

    virtual int extractAdditionalAttributes();
    virtual int calcNumBlocks();
    virtual hid_t readQDFSequenceSlab(hid_t hDataSet, hid_t hMemSpace, hid_t hDataSpace, float *aBuf);

};


#endif

