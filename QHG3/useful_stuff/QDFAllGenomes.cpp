#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hdf5.h>

#include "types.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "QDFUtils.h"
#include "QDFAllGenomes.h"

#define BUFSIZE_GENOMES 1000000


//----------------------------------------------------------------------------
// createInstance
//  use if the population file contains geo info
//  pQDFPopFile :     Population QDF file
//  pSpeciesName :    Name of species for which to extract genomes
//                    (if NULL, first species will be used)
//  pAttrGenomeSize : name of genome size attribute
//  pDataSetGenome :  name of genome dataset
QDFAllGenomes *QDFAllGenomes::createInstance(const char *pQDFPopFile,
                                                       const char *pSpeciesName, 
                                                       const char *pAttrGenomeSize, 
                                                       const char *pDataSetGenome) {
    QDFAllGenomes *pQPC = new QDFAllGenomes();
    int iResult = pQPC->init(pQDFPopFile, pSpeciesName, pAttrGenomeSize, pDataSetGenome);
    if (iResult != 0) {
        delete pQPC;
        pQPC = NULL;
    }
    return pQPC;
}


//----------------------------------------------------------------------------
// constructor
//
QDFAllGenomes::QDFAllGenomes()
    : m_pPopName(NULL),
      m_pDataSetGenome(NULL),
      m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_iGenomeSize(0),
      m_iNumBlocks(0),
      m_iNumGenomes(0),
      m_ppGenomes(NULL),
      m_pGenomeData(NULL) {

}


//----------------------------------------------------------------------------
// destructor
//
QDFAllGenomes::~QDFAllGenomes() {
     // ... probaly more
    
    if (m_pPopName != NULL) {
        delete[] m_pPopName;
    }
    if (m_pDataSetGenome != NULL) {
        delete[] m_pDataSetGenome;
    }

    if (m_ppGenomes != NULL) {
        delete[] m_ppGenomes;
    }

    if (m_pGenomeData != NULL) {
        delete[] m_pGenomeData;
    }

}



//----------------------------------------------------------------------------
// init
//
int QDFAllGenomes::init(const char *pQDFPopFile, 
                             const char *pSpeciesName, 
                             const char *pAttrGenomeSize,
                             const char *pDataSetGenome) {


    int iResult = -1;

    if (pAttrGenomeSize != NULL) {

        m_pPopName = NULL;
        if (pSpeciesName == NULL) {
            m_pPopName = qdf_getFirstPopulation(pQDFPopFile);
        } else {
            m_pPopName = qdf_checkForPop(pQDFPopFile, pSpeciesName);
        }
        if (m_pPopName != NULL) {
            m_pDataSetGenome = new char[strlen(pDataSetGenome)+1];
            strcpy(m_pDataSetGenome, pDataSetGenome);
                
            printf("using population [%s]\n", m_pPopName);
                
                
            // open qdf and get species
            // we know file, pop group and species group exist
            m_hFile        = qdf_openFile(pQDFPopFile);
            m_hPopulation  = qdf_openGroup(m_hFile, POPGROUP_NAME);
            m_hSpecies     = qdf_openGroup(m_hPopulation, m_pPopName);
                
                   
            iResult = qdf_extractAttribute(m_hSpecies, pAttrGenomeSize, 1, &m_iGenomeSize);
            if (iResult == 0) {
                m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);

                hid_t hDataSet = H5Dopen2(m_hSpecies, pDataSetGenome, H5P_DEFAULT);
                hid_t hDataSpace = H5Dget_space(hDataSet);
                int iNumDims = H5Sget_simple_extent_ndims(hDataSpace);
                if (iNumDims == 1) {
                    hsize_t iNumLongs = 0;
                    H5Sget_simple_extent_dims(hDataSpace, &iNumLongs, NULL);
                    m_iNumGenomes = iNumLongs/m_iNumBlocks;
                    printf("GenomeSIze %d, NumBlocks %d, numlongs %lld, num genomes %d\n",
                           m_iGenomeSize, m_iNumBlocks, iNumLongs, m_iNumGenomes);
                    m_pGenomeData = new ulong[m_iNumGenomes*m_iNumBlocks];
                    m_ppGenomes   = new ulong *[m_iNumGenomes];
                    for (int i = 0; i < m_iNumGenomes; i++) {
                        m_ppGenomes[i] = m_pGenomeData + (i*m_iNumBlocks);
                    }
                } else {
                    printf("Wrong number of dimensions for genome data set: %d\n", iNumDims);
                }
                qdf_closeDataSpace(hDataSpace);
                qdf_closeDataSet(hDataSet);
            } else {
                printf("Couldn't extract ATtribute [%s] for genome size\n", pAttrGenomeSize);
            }
        }
    } else {
        printf("The name for the GenomeSize attribute must not be NULL\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// extractGenomes
//
int QDFAllGenomes::extractGenomes() {
    int iResult = 0;

    // read buffer
    uint iReadBufSize = BUFSIZE_GENOMES*m_iNumBlocks;
    ulong *pBuf = m_pGenomeData;
   
    // open the DataSet and data space
    printf("Opening data set [%s] in species\n", m_pDataSetGenome); fflush(stdout);
    hid_t hDataSet = H5Dopen2(m_hSpecies, m_pDataSetGenome, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    printf("Dataspace extent: %lld\n", dims);

    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    printf("Trying to extract all genomes\n");
    // loop until all elements have been read
    while ((iResult == 0) && (dims > 0)) {
        // can we get a full load of the buffer?
        if (dims > iReadBufSize) {
            iCount = iReadBufSize;
        } else {
            iCount = dims;
        }
        
        // read a slab
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, NULL, &iCount, NULL);
        status = H5Dread(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pBuf);
        
        if (status >= 0) {
            pBuf += iCount;
            
            dims          -= iCount;
            iOffset       += iCount;
            
        } else {
            printf("Error during slab reading\n");
            iResult = -1;
        }
    }
    
    if (iResult == 0) {
        printf("Extracted all genomes (%lld)\n", iOffset);
    }

    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
        
    qdf_closeGroup(m_hSpecies);
    qdf_closeGroup(m_hPopulation);
    qdf_closeFile(m_hFile);


    return iResult;
}


