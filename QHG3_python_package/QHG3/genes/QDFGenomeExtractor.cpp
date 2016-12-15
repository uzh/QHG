#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hdf5.h>

#include "types.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "QDFUtils.h"
#include "GenomeProvider.h"
#include "QDFGenomeExtractor.h"
#include "IDSampler2.h"


#define BUFSIZE_GENOMES 1000000

//----------------------------------------------------------------------------
// createInstance
//  use if the population file contains geo info
//  pQDFPopFile :     Population QDF file
//  pSpeciesName :    Name of species for which to extract genomes
//                    (if NULL, first species will be used)
//  pAttrGenomeSize : name of genome size attribute
//  pDataSetGenome :  name of genome dataset
//
QDFGenomeExtractor *QDFGenomeExtractor::createInstance(const char *pQDFPopFile, 
                                                       const char *pSpeciesName, 
                                                       const char *pAttrGenomeSize, 
                                                       const char *pDataSetGenome) {
    return createInstance(pQDFPopFile, pQDFPopFile, pSpeciesName, pAttrGenomeSize, pDataSetGenome);
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
QDFGenomeExtractor *QDFGenomeExtractor::createInstance(const char *pQDFGeoFile,
                                                       const char *pQDFPopFile,
                                                       const char *pSpeciesName, 
                                                       const char *pAttrGenomeSize, 
                                                       const char *pDataSetGenome) {
    QDFGenomeExtractor *pQPC = new QDFGenomeExtractor();
    int iResult = pQPC->init(pQDFGeoFile, pQDFPopFile, pSpeciesName, pAttrGenomeSize, pDataSetGenome);
    if (iResult != 0) {
        delete pQPC;
        pQPC = NULL;
    }
    return pQPC;
}


//----------------------------------------------------------------------------
// constructor
//
QDFGenomeExtractor::QDFGenomeExtractor()
    : m_pPopName(NULL),
      m_pQDFGeoFile(NULL),
      m_pDataSetGenome(NULL),
      m_pCurSample(NULL),
      m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_iGenomeSize(0),
      m_iNumBlocks(0),
      m_iNumSelected(0) {

}


//----------------------------------------------------------------------------
// destructor
//
QDFGenomeExtractor::~QDFGenomeExtractor() {
    qdf_closeFile(m_hFile);
    qdf_closeGroup(m_hPopulation);
    qdf_closeGroup(m_hSpecies);

    // ... probaly more
    
    if (m_pPopName != NULL) {
        delete[] m_pPopName;
    }
    if (m_pQDFGeoFile != NULL) {
        delete[] m_pQDFGeoFile;
    }
    if (m_pDataSetGenome != NULL) {
        delete[] m_pDataSetGenome;
    }

    if (m_pCurSample != NULL) {
        delete m_pCurSample;
    }
    
    genomemap::const_iterator it;
    for (it =  m_mGenomes.begin(); it != m_mGenomes.end(); ++it) {
        delete[] it->second;
    }
}


//----------------------------------------------------------------------------
// getGenome
//
const ulong *QDFGenomeExtractor::getGenome(idtype iID) {
    const ulong *pGenome = NULL;
    genomemap::const_iterator itm = m_mGenomes.find(iID);
    if (itm != m_mGenomes.end()) {
        pGenome = itm->second;
    }
    return pGenome;
}


//----------------------------------------------------------------------------
// init
//
int QDFGenomeExtractor::init(const char *pQDFGeoFile, 
                             const char *pQDFPopFile, 
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
            if (qdf_checkForGeo(pQDFGeoFile) == 0) {
                m_pDataSetGenome = new char[strlen(pDataSetGenome)+1];
                strcpy(m_pDataSetGenome, pDataSetGenome);
                
                printf("using population [%s]\n", m_pPopName);
                
                m_pQDFGeoFile = new char[strlen(pQDFGeoFile)+1];
                strcpy(m_pQDFGeoFile, pQDFGeoFile);
                
                m_vQDFPopFiles.push_back(pQDFPopFile);
                // open qdf and get species
                // we know file, pop group and species group exist
                m_hFile        = qdf_openFile(pQDFPopFile);
                m_hPopulation  = qdf_openGroup(m_hFile, POPGROUP_NAME);
                m_hSpecies     = qdf_openGroup(m_hPopulation, m_pPopName);
                
                   
                iResult = qdf_extractAttribute(m_hSpecies, pAttrGenomeSize, 1, &m_iGenomeSize);
                if (iResult == 0) {
                    m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
                } else {
                   printf("Couldn't extract ATtribute [%s] for genome size\n", pAttrGenomeSize);
                }
            } else {
                printf("No Grid & Geo in [%s]\n", pQDFGeoFile);
            }
        }
    } else {
        printf("The name for the GenomeSize attribute must not be NULL\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getSelectedGenes
//
int QDFGenomeExtractor::getSelectedGenes(const char *pDataSetGenome) {
    int iResult = 0;


    // read buffer
    uint iReadBufSize = BUFSIZE_GENOMES*2*m_iNumBlocks;
    long *aBuf = new long[iReadBufSize];
   
    // open the DataSet and data space
    printf("Opening data set [%s] in species\n", pDataSetGenome); fflush(stdout);
    hid_t hDataSet = H5Dopen2(m_hSpecies, pDataSetGenome, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    printf("Dataspace extent: %lld\n", dims);

    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    int iGlobalOffset = 0;

    indexids::const_iterator itIdxId = m_mSelected.begin();

    printf("Trying to extract %zd genomes\n", m_mSelected.size());
    // loop until all elements have been read
    while ((iResult == 0) && (dims > 0) && (itIdxId != m_mSelected.end())) {
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
                         hDataSpace, H5P_DEFAULT, aBuf);
        
        if (status >= 0) {
            // distribute the data 
            int iNumThisPass = iCount/(2*m_iNumBlocks);
            for (int i = 0; (iResult == 0) && (i < iNumThisPass) && (itIdxId != m_mSelected.end()); i++) {
                int j = i+iGlobalOffset;
                
                if (j == itIdxId->first) {
                    ulong *pBlock = new ulong[2*m_iNumBlocks];
                    memcpy(pBlock, aBuf+i*2*m_iNumBlocks, 2*m_iNumBlocks*sizeof(ulong));
                    m_mGenomes[itIdxId->second] = pBlock;
                    fprintf(stderr,"%d ",j);
                    ++itIdxId;
                }
            }
            
            iGlobalOffset += iNumThisPass;
            dims          -= iCount;
            iOffset       += iCount;
            
        } else {
            printf("Error during slab reading\n");
            iResult = -1;
        }
    }
    
    fprintf(stderr,"\n");

    if (m_mGenomes.size() == m_mSelected.size()) {
        printf("Successfully read all required genomes\n");
    } else {
        printf("Error: read only %zd of %zd required genomes\n", m_mGenomes.size(), m_mSelected.size());
        iResult = -1;
    }

    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
        
    delete[] aBuf;
    
    return iResult;

}


//----------------------------------------------------------------------------
// createSelection
//   if all reads have been successful, the number of agents is returned.
//   otherwise -1 on error
//
int QDFGenomeExtractor::createSelection(const char *sLocFile) {
    int iResult = -1;

    delete m_pCurSample;
    m_pCurSample = NULL;

    printf("--- creating IDSampler from [%s]\n", m_pQDFGeoFile);
    IDSampler2 *pIS = IDSampler2::createInstance(m_pQDFGeoFile);
    if (pIS != NULL) {
        printf("--- getting samples from [");
        for (uint i = 0; i < m_vQDFPopFiles.size(); ++i) {
            if (i > 0) {
                printf(", ");
            }
            printf("%s", m_vQDFPopFiles[i].c_str());
        }
        printf("]\n");
   
        m_pCurSample =  pIS->getSamples(m_vQDFPopFiles, m_pPopName, sLocFile, m_mLocData);
        if (m_pCurSample != NULL) {
            m_sSelected.clear();
            pIS->getFullIDSet(m_sSelected);
            pIS->getFullIndexIDMap(m_mSelected);
            m_iNumSelected = m_sSelected.size();
            printf("Sampled: Total %d ids\n", m_iNumSelected);fflush(stdout);

            iResult = getSelectedGenes(m_pDataSetGenome);
            //            printf("Got genomes: [%p]\n", m_pDataSetGenome);fflush(stdout);
        } else {
            printf("Couldn't get samples\n");
        }
        delete pIS;
    } else {
        printf("Couldn't create IDSampler for Grid [%s]\n", m_pQDFGeoFile);
    }
    return iResult;
}

