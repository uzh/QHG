#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "types.h"
#include "QDFUtils.h"

#include "QDFSequenceExtractor.h"
#include "QDFSequenceExtractor.cpp"
#include "QDFPhenomeExtractor2.h"


//----------------------------------------------------------------------------
// createInstance
//  use if the population file contains geo info
//  pQDFPopFile :     Population QDF file
//  pSpeciesName :    Name of species for which to extract genomes
//                    (if NULL, first species will be used)
//  pAttrGenomeSize : name of genome size attribute
//  pDataSetGenome :  name of genome dataset
//
QDFPhenomeExtractor2 *QDFPhenomeExtractor2::createInstance(const char *pQDFPopFile, 
                                                           const char *pSpeciesName, 
                                                           const char *pAttrPhenomeSize,
                                                           const char *pDataSetPhenome,
                                                           WELL512 *pWELL,
                                                           bool bCartesian) {
    return createInstance(pQDFPopFile, pQDFPopFile, pSpeciesName, pAttrPhenomeSize, pDataSetPhenome, pWELL, bCartesian);
}


//----------------------------------------------------------------------------
// createInstance
//  use if the population file does not contain geo info
//  pQDFPopFile :     Population QDF file
//  pSpeciesName :    Name of species for which to extract genomes
//                    (if NULL, first species will be used)
//  pAttrGenomeSize : name of genome size attribute
//  pDataSetGenome :  name of genome dataset
//
QDFPhenomeExtractor2 *QDFPhenomeExtractor2::createInstance(const char *pQDFGeoFile,
                                                           const char *pQDFPopFile,
                                                           const char *pSpeciesName, 
                                                           const char *pAttrPhenomeSize,
                                                           const char *pDataSetPhenome,
                                                           WELL512 *pWELL,
                                                           bool bCartesian) {
    QDFPhenomeExtractor2 *pQPC = new QDFPhenomeExtractor2(pWELL, bCartesian);
    int iResult = pQPC->init(pQDFGeoFile, pQDFPopFile, pSpeciesName, pAttrPhenomeSize, pDataSetPhenome);
    if (iResult != 0) {
        delete pQPC;
        pQPC = NULL;
    }
    return pQPC;
}



//----------------------------------------------------------------------------
// constructor
//
QDFPhenomeExtractor2::QDFPhenomeExtractor2(WELL512 *pWELL, bool bCartesian)
    : QDFSequenceExtractor(pWELL, bCartesian) {
}



//----------------------------------------------------------------------------
// extractAdditionalAttributes
//
int QDFPhenomeExtractor2::extractAdditionalAttributes() {
    return 0;
}


//----------------------------------------------------------------------------
// calcNumBlocks
//
int QDFPhenomeExtractor2::calcNumBlocks() {
    return m_iSequenceSize;
}


//----------------------------------------------------------------------------
// readQDFSequenceSlab
//
hid_t QDFPhenomeExtractor2::readQDFSequenceSlab(hid_t hDataSet, 
                                               hid_t hMemSpace, 
                                               hid_t hDataSpace, 
                                               float *aBuf) {
    return  H5Dread(hDataSet, H5T_NATIVE_FLOAT, hMemSpace, hDataSpace, H5P_DEFAULT, aBuf);
}
