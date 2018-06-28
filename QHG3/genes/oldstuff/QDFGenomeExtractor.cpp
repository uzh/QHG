#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <hdf5.h>

#include "types.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
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
QDFGenomeExtractor *QDFGenomeExtractor::createInstance(const char *pQDFGeoFile,
                                                       const char *pQDFPopFile,
                                                       const char *pSpeciesName, 
                                                       const char *pAttrGenomeSize,
                                                       const char *pAttrBitsPerNuc, 
                                                       const char *pDataSetGenome,
                                                       WELL512 *pWELL,
                                                       bool bCartesian) {
    QDFGenomeExtractor *pQPC = new QDFGenomeExtractor(pWELL, bCartesian);
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
QDFGenomeExtractor::QDFGenomeExtractor(WELL512 *pWELL, bool bCartesian)
    : m_pPopName(NULL),
      m_pQDFGeoFile(NULL),
      m_pDataSetGenome(NULL),
      m_iNumSelected(0),
      m_iNumRefSelected(0),
      m_pCurSample(NULL),
      m_pRefSample(NULL),
      m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_iGenomeSize(0),
      m_iNumBlocks(0),
      m_iBitsPerNuc(0),
      m_bCartesian(bCartesian),
      m_pWELL(pWELL),
      m_bVerbose(false) {

    m_vQDFPopFiles.clear();
    m_sSelected.clear();
    m_mSelected.clear(); 
    m_sRefSelected.clear();
    m_mRefSelected.clear();
    m_mLocData.clear();
    m_mGenomes.clear();
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
    
    if (m_pRefSample != NULL) {
        delete m_pRefSample;
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
                             const char *pAttrBitsPerNuc,
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
                
                if (m_bVerbose) printf("using population [%s]\n", m_pPopName);
                
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
                
                    iResult = qdf_extractAttribute(m_hSpecies, pAttrBitsPerNuc, 1, &m_iBitsPerNuc);
                    if (iResult == 0) {
                        if (m_iBitsPerNuc == BitGeneUtils::BITSINNUC) {
                            m_iNumBlocks = BitGeneUtils::numNucs2Blocks(m_iGenomeSize);
                        } else {
                            m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
                        }	
                    } else {
                        fprintf(stderr, "WARNING: attribute [%s] not found; using %d\n", pAttrBitsPerNuc, GeneUtils::BITSINNUC);
                        m_iBitsPerNuc = GeneUtils::BITSINNUC;
                        m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
                        iResult = 0;
                    }
                } else {
		    fprintf(stderr, "Couldn't extract ATtribute [%s] for genome size\n", pAttrGenomeSize);
                }
            } else {
 	        fprintf(stderr, "No Grid & Geo in [%s]\n", pQDFGeoFile);
            }
        }
    } else {
        fprintf(stderr, "The name for the GenomeSize attribute must not be NULL\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getSelectedGenesDense
//  this algorithm is better if the samples are dense
//
int QDFGenomeExtractor::getSelectedGenesDense(const char *pDataSetGenome, int iNumPerBuf) {
    int iResult = 0;

    printf("getSelectedGenes (dense)\n");

    // read buffer
    ulong iReadBufSize = ((iNumPerBuf<=0)?BUFSIZE_GENOMES:iNumPerBuf)*2*m_iNumBlocks;
    long *aBuf = new long[iReadBufSize];
   
    // open the DataSet and data space
    if (m_bVerbose) { printf("Opening data set [%s] in species\n", pDataSetGenome); fflush(stdout); }
    hid_t hDataSet = H5Dopen2(m_hSpecies, pDataSetGenome, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    if (m_bVerbose) printf("Dataspace extent: %lld\n", dims);

    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    int iGlobalOffset = 0;

    arrpos_ids mAllSelected(m_mSelected);
    mAllSelected.insert(m_mRefSelected.begin(), m_mRefSelected.end());
    //    indexids::const_iterator itIdxId = m_mSelected.begin();
    arrpos_ids::const_iterator itIdxId = mAllSelected.begin();
    
    if (m_bVerbose) printf("Trying to extract %zd genomes\n", mAllSelected.size());
    // loop until all elements have been read
    while ((iResult == 0) && (dims > 0) && (itIdxId != mAllSelected.end())) {
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
            for (int i = 0; (iResult == 0) && (i < iNumThisPass) && (itIdxId != mAllSelected.end()); i++) {
                int j = i+iGlobalOffset;
                
                if (j == itIdxId->first) {
                    ulong *pBlock = new ulong[2*m_iNumBlocks];
                    memcpy(pBlock, aBuf+i*2*m_iNumBlocks, 2*m_iNumBlocks*sizeof(ulong));
                    m_mGenomes[itIdxId->second] = pBlock;
                    //fprintf(stderr,"%d ",j);
                    ++itIdxId;
                }
            }
            
            iGlobalOffset += iNumThisPass;
            dims          -= iCount;
            iOffset       += iCount;
            
        } else {
	    fprintf(stderr, "Error during slab reading\n");
	    iResult = -1;
        }
    }
    
    //fprintf(stderr,"\n");

    if (m_mGenomes.size() == mAllSelected.size()) {
        printf("Successfully read all required %zd genomes\n", m_mGenomes.size());
    } else {
        fprintf(stderr, "Error: read only %zd of %zd required genomes\n", m_mGenomes.size(), m_mSelected.size());
        iResult = -1;
    }

    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
        
    delete[] aBuf;
    
    return iResult;

}

//----------------------------------------------------------------------------
// getSelectedGenesSparse
//  this algorithm is better if the samples are sparse
//
int QDFGenomeExtractor::getSelectedGenesSparse(const char *pDataSetGenome, int iNumPerBuf) {
    int iResult = 0;


    // read buffer
    ulong iReadBufSize = 2*m_iNumBlocks;
    long *aBuf = new long[iReadBufSize];
    printf("getSelectedGenes (sparse)\n");
    // open the DataSet and data space
    if (m_bVerbose) {printf("Opening data set [%s] in species\n", pDataSetGenome); fflush(stdout);}
    hid_t hDataSet = H5Dopen2(m_hSpecies, pDataSetGenome, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    if (m_bVerbose) printf("Dataspace extent: %lld\n", dims);

    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    arrpos_ids mAllSelected(m_mSelected);
    mAllSelected.insert(m_mRefSelected.begin(), m_mRefSelected.end());
    
    arrpos_ids::const_iterator itIdxId;
    for (itIdxId= mAllSelected.begin(); itIdxId != mAllSelected.end(); ++itIdxId) {
        iOffset = 2*m_iNumBlocks*itIdxId->first;
        iCount  = 2*m_iNumBlocks;

        if (iOffset < dims-iCount+1) {
            // read a slab
            hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
            status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                         &iOffset, NULL, &iCount, NULL);
            status = H5Dread(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                             hDataSpace, H5P_DEFAULT, aBuf);
            
            if (status >= 0) {
                ulong *pBlock = new ulong[2*m_iNumBlocks];
                memcpy(pBlock, aBuf, 2*m_iNumBlocks*sizeof(ulong));
                m_mGenomes[itIdxId->second] = pBlock;
                
            } else {
                fprintf(stderr, "Error during slab reading\n");
                iResult = -1;
            }
        } else {
            printf("Index too big: (%lld+1)*2*numblocks > %lld\\n", iOffset, dims);
            iResult = -1;
        }
    }
    //fprintf(stderr,"\n");

    if (m_mGenomes.size() == mAllSelected.size()) {
        printf("Successfully read all required %zd genomes\n", m_mGenomes.size());
    } else {
        fprintf(stderr, "Error: read only %zd of %zd required genomes\n", m_mGenomes.size(), m_mSelected.size());
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
int QDFGenomeExtractor::createSelection(const char *sLocFile, const char *sRefFile, bool bDense, int iNumPerBuf) {
    int iResult = createSelection(sLocFile, 0, 0, sRefFile, bDense, iNumPerBuf);
    return iResult;
}


//----------------------------------------------------------------------------
// createSelection
//   if all reads have been successful, the number of agents is returned.
//   otherwise -1 on error
//
int QDFGenomeExtractor::createSelection(const char *sLocFile, int iNumSamp, double dSampDist, const char *sRefFile, bool bDense, int iNumPerBuf) {
    int iResult = -1;

    delete m_pCurSample;
    m_pCurSample = NULL;
    m_sSelected.clear();
    m_mSelected.clear();

    if (m_bVerbose) printf("--- creating IDSampler from [%s] for locations [%s]\n", m_pQDFGeoFile, sLocFile);
    IDSampler2 *pIS = IDSampler2::createInstance(m_pQDFGeoFile, m_pWELL, m_bCartesian);
    if (pIS != NULL) {
        if (m_bVerbose && false) {
            printf("--- getting samples from [");
            for (uint i = 0; i < m_vQDFPopFiles.size(); ++i) {
                if (i > 0) {
                    printf(", ");
                }
                printf("%s", m_vQDFPopFiles[i].c_str());
            }
            printf("]\n");
        }
       
        locspec ls(sLocFile, dSampDist, iNumSamp);
        locspec rs(sRefFile, dSampDist, iNumSamp);

        //        m_pCurSample =  pIS->getSamples(m_vQDFPopFiles, m_pPopName, sLocFile, iNumSamp, dSampDist, m_mLocData, sRefFile);
        m_pCurSample =  pIS->getSamples(m_vQDFPopFiles, m_pPopName, &ls, m_mLocData, &rs);
        if (m_pCurSample != NULL) {
            m_pRefSample = pIS->getRefSample();
            m_sSelected.clear();
            pIS->getFullIDSet(m_sSelected);
            pIS->getFullIndexIDMap(m_mSelected);
            pIS->getRefIDSet(m_sRefSelected);
            pIS->getRefIndexIDMap(m_mRefSelected);

            m_iNumSelected = m_sSelected.size();
            if (m_bVerbose) {printf("Sampled: Total %d id%s\n", m_iNumSelected, (m_iNumSelected!=1)?"s":"");fflush(stdout);}
            
            double dT0 = omp_get_wtime();
            if (bDense) {
                iResult = getSelectedGenesDense(m_pDataSetGenome, iNumPerBuf);
            } else {
                iResult = getSelectedGenesSparse(m_pDataSetGenome, iNumPerBuf);
            }
            double dT1 = omp_get_wtime();
            if (m_bVerbose) {printf("Got genomes: [%p] R[%d]\n", m_pDataSetGenome, iResult);fflush(stdout);}
            if (m_bVerbose) printf("QDFGenomeExtractor: time to get genomes: %f\n", dT1- dT0);
        } else {
	    fprintf(stderr, "Couldn't get samples\n");
        }
        delete pIS;
    } else {
        fprintf(stderr, "Couldn't create IDSampler for Grid [%s]\n", m_pQDFGeoFile);
    }
    return iResult;
}

