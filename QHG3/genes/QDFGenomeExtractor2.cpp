#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "types.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "QDFUtils.h"

#include "QDFSequenceExtractor.h"
#include "QDFSequenceExtractor.cpp"
#include "QDFGenomeExtractor2.h"


//----------------------------------------------------------------------------
// createInstance
//  use if the population file contains geo info
//  pQDFPopFile :     Population QDF file
//  pSpeciesName :    Name of species for which to extract genomes
//                    (if NULL, first species will be used)
//  pAttrGenomeSize : name of genome size attribute
//  pDataSetGenome :  name of genome dataset
//
QDFGenomeExtractor2 *QDFGenomeExtractor2::createInstance(const char *pQDFPopFile, 
                                                         const char *pSpeciesName, 
                                                         const char *pAttrGenomeSize,
                                                         const char *pAttrBitsPerNuc, 
                                                         const char *pDataSetGenome,
                                                         WELL512 *pWELL,
                                                         bool bCartesian) {
    return createInstance(pQDFPopFile, pQDFPopFile, pSpeciesName, pAttrGenomeSize, pAttrBitsPerNuc, pDataSetGenome, pWELL, bCartesian);
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
QDFGenomeExtractor2 *QDFGenomeExtractor2::createInstance(const char *pQDFGeoFile,
                                                         const char *pQDFPopFile,
                                                         const char *pSpeciesName, 
                                                         const char *pAttrGenomeSize,
                                                         const char *pAttrBitsPerNuc, 
                                                         const char *pDataSetGenome,
                                                         WELL512 *pWELL,
                                                         bool bCartesian) {
    QDFGenomeExtractor2 *pQPC = new QDFGenomeExtractor2(pWELL, bCartesian);
    int iResult = pQPC->init(pQDFGeoFile, pQDFPopFile, pSpeciesName, pAttrGenomeSize, pAttrBitsPerNuc, pDataSetGenome);
    if (iResult != 0) {
        delete pQPC;
        pQPC = NULL;
    }
    return pQPC;
}



//----------------------------------------------------------------------------
// constructor
//
QDFGenomeExtractor2::QDFGenomeExtractor2(WELL512 *pWELL, bool bCartesian)
    : QDFSequenceExtractor(pWELL, bCartesian),
      m_iBitsPerNuc(0),
      m_pAttrBitsPerNuc(NULL) {
}


//----------------------------------------------------------------------------
// destructor
//
QDFGenomeExtractor2::~QDFGenomeExtractor2() {
    if (m_pAttrBitsPerNuc != NULL) {
        delete[] m_pAttrBitsPerNuc;
    }
}


//----------------------------------------------------------------------------
// init
//
int QDFGenomeExtractor2::init(const char *pQDFGeoFile, 
                              const char *pQDFPopFile, 
                              const char *pSpeciesName, 
                              const char *pAttrGenomeSize,
                              const char *pAttrBitsPerNuc,
                              const char *pDataSetGenome) {


    int iResult = -1;

    m_pAttrBitsPerNuc = new char[strlen(pAttrBitsPerNuc)+1];
    strcpy(m_pAttrBitsPerNuc, pAttrBitsPerNuc);

    iResult = QDFSequenceExtractor::init(pQDFGeoFile, pQDFPopFile, pSpeciesName, pAttrGenomeSize, pDataSetGenome);
   
    return iResult;
}


//----------------------------------------------------------------------------
// extractAdditionalAttributes
//
int QDFGenomeExtractor2::extractAdditionalAttributes() {
    int iResult =-1;

    iResult = qdf_extractAttribute(m_hSpecies, m_pAttrBitsPerNuc, 1, &m_iBitsPerNuc);
    if (iResult != 0) {
        fprintf(stderr, "WARNING: attribute [%s] not found; using %d\n", m_pAttrBitsPerNuc, GeneUtils::BITSINNUC);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// calcNumBlocks
//
int QDFGenomeExtractor2::calcNumBlocks() {
    int iNumBlocks = 0;


    if (m_pAttrBitsPerNuc != NULL) {
        if (m_iBitsPerNuc == BitGeneUtils::BITSINNUC) {
            iNumBlocks = BitGeneUtils::numNucs2Blocks(m_iSequenceSize);
        } else {
            iNumBlocks = GeneUtils::numNucs2Blocks(m_iSequenceSize);
        }	
    } else {
        m_iBitsPerNuc = GeneUtils::BITSINNUC;
        iNumBlocks = GeneUtils::numNucs2Blocks(m_iSequenceSize);
    }
    return iNumBlocks;
}


//----------------------------------------------------------------------------
// readQDFSequenceSlab
//
hid_t QDFGenomeExtractor2::readQDFSequenceSlab(hid_t hDataSet, 
                                               hid_t hMemSpace, 
                                               hid_t hDataSpace, 
                                               ulong *aBuf) {
    return  H5Dread(hDataSet, H5T_NATIVE_LONG, hMemSpace, hDataSpace, H5P_DEFAULT, aBuf);
}
