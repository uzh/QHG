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
#include "SequenceProvider.h"
#include "QDFSequenceExtractor.h"
#include "IDSampler2.h"

#include "LineReader.h"

#define BUFSIZE_SEQUENCES 1000000


//----------------------------------------------------------------------------
// constructor
//
template<typename T>
QDFSequenceExtractor<T>::QDFSequenceExtractor(WELL512 *pWELL, bool bCartesian)
    : m_pPopName(NULL),
      m_pQDFGeoFile(NULL),
      m_pSequenceDataSetName(NULL),
      m_iNumSelected(0),
      m_iNumRefSelected(0),
      m_pCurSample(NULL),
      m_pRefSample(NULL),
      m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_iSequenceSize(0),
      m_iNumBlocks(0),
      m_bCartesian(bCartesian),
      m_pWELL(pWELL),
      m_bVerbose(false) {

    m_vQDFPopFiles.clear();
    m_sSelected.clear();
    m_mSelected.clear(); 
    m_sRefSelected.clear();
    m_mRefSelected.clear();
    m_mLocData.clear();
    m_mSequences.clear();
}


//----------------------------------------------------------------------------
// destructor
//
template<typename T>
QDFSequenceExtractor<T>::~QDFSequenceExtractor() {
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
    if (m_pSequenceDataSetName != NULL) {
        delete[] m_pSequenceDataSetName;
    }
    if (m_pCurSample != NULL) {
        delete m_pCurSample;
    }
    
    if (m_pRefSample != NULL) {
        delete m_pRefSample;
    }
   typename  
    QDFSequenceExtractor<T>::sequencemap::const_iterator it;
    for (it =  m_mSequences.begin(); it != m_mSequences.end(); ++it) {
        delete[] it->second;
    }
}


//----------------------------------------------------------------------------
// getSequencesDense
//
template<typename T>
const T *QDFSequenceExtractor<T>::getSequence(idtype iID) {
    const T *pSequence = NULL;
    typename QDFSequenceExtractor<T>::sequencemap::const_iterator itm = m_mSequences.find(iID);
    if (itm != m_mSequences.end()) {
        pSequence = itm->second;
    }
    return pSequence;
}


//----------------------------------------------------------------------------
// init
//
template<typename T>
int QDFSequenceExtractor<T>::init(const char *pQDFGeoFile, 
                                const char *pQDFPopFile, 
                                const char *pSpeciesName, 
                                const char *pAttrSequenceSize,
                                const char *pSequenceDataSetName) {


    int iResult = -1;

    if (pAttrSequenceSize != NULL) {

        m_pPopName = NULL;
        if (pSpeciesName == NULL) {
            m_pPopName = qdf_getFirstPopulation(pQDFPopFile);
        } else {
            m_pPopName = qdf_checkForPop(pQDFPopFile, pSpeciesName);
        }
        if (m_pPopName != NULL) {
            if (qdf_checkForGeo(pQDFGeoFile) == 0) {
                m_pSequenceDataSetName = new char[strlen(pSequenceDataSetName)+1];
                strcpy(m_pSequenceDataSetName, pSequenceDataSetName);
                
                if (m_bVerbose) printf("using population [%s]\n", m_pPopName);
                
                m_pQDFGeoFile = new char[strlen(pQDFGeoFile)+1];
                strcpy(m_pQDFGeoFile, pQDFGeoFile);
                
                m_vQDFPopFiles.push_back(pQDFPopFile);
                // open qdf and get species
                // we know file, pop group and species group exist
                m_hFile        = qdf_openFile(pQDFPopFile);
                m_hPopulation  = qdf_openGroup(m_hFile, POPGROUP_NAME);
                m_hSpecies     = qdf_openGroup(m_hPopulation, m_pPopName);
                
                   
                iResult = qdf_extractAttribute(m_hSpecies, pAttrSequenceSize, 1, &m_iSequenceSize);
                if (iResult == 0) {
                    printf("Sequence size %d\n", m_iSequenceSize);
                    iResult = extractAdditionalAttributes();

                    if (iResult == 0) {
                        m_iNumBlocks = calcNumBlocks();
                    }
                } else {
		    fprintf(stderr, "Couldn't extract Attribute [%s] for sequence size\n", pAttrSequenceSize);
                }
            } else {
 	        fprintf(stderr, "No Grid & Geo in [%s]\n", pQDFGeoFile);
            }
        }
    } else {
        fprintf(stderr, "The name for the sequence size attribute must not be NULL\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getSelectedGenesDense
//  this algorithm is better if the samples are dense
//
template<typename T>
int QDFSequenceExtractor<T>::getSelectedSequencesDense(const char *pSequenceDataSetName, int iNumPerBuf) {
    int iResult = 0;

    printf("getSelectedSequences (dense)\n");

    // read buffer
    ulong iReadBufSize = ((iNumPerBuf<=0)?BUFSIZE_SEQUENCES:iNumPerBuf)*2*m_iNumBlocks;
    T *aBuf = new T[iReadBufSize];
   
    // open the DataSet and data space
    if (m_bVerbose) { printf("Opening data set [%s] in species\n", pSequenceDataSetName); fflush(stdout); }
    hid_t hDataSet = H5Dopen2(m_hSpecies, pSequenceDataSetName, H5P_DEFAULT);
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
        status = readQDFSequenceSlab(hDataSet, hMemSpace, hDataSpace, aBuf);
        
        if (status >= 0) {
            // distribute the data 
            int iNumThisPass = iCount/(2*m_iNumBlocks);
            for (int i = 0; (iResult == 0) && (i < iNumThisPass) && (itIdxId != mAllSelected.end()); i++) {
                int j = i+iGlobalOffset;
                
                if (j == itIdxId->first) {
                    T *pBlock = new T[2*m_iNumBlocks];
                    memcpy(pBlock, aBuf+i*2*m_iNumBlocks, 2*m_iNumBlocks*sizeof(T));
                    m_mSequences[itIdxId->second] = pBlock;

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
    

    if (m_mSequences.size() == mAllSelected.size()) {
        printf("Successfully read all required %zd sequences\n", m_mSequences.size());
    } else {
        fprintf(stderr, "Error: read only %zd of %zd required sequences\n", m_mSequences.size(), m_mSelected.size());
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
template<typename T>
int QDFSequenceExtractor<T>::getSelectedSequencesSparse(const char *pSequenceDataSetName, int iNumPerBuf) {
    int iResult = 0;


    // read buffer
    ulong iReadBufSize = 2*m_iNumBlocks;
    T *aBuf = new T[iReadBufSize];
    printf("getSelectedGenes (sparse)\n");
    // open the DataSet and data space
    if (m_bVerbose) {printf("Opening data set [%s] in species\n", pSequenceDataSetName); fflush(stdout);}
    hid_t hDataSet = H5Dopen2(m_hSpecies, pSequenceDataSetName, H5P_DEFAULT);
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
            status = readQDFSequenceSlab(hDataSet, hMemSpace, hDataSpace, aBuf);
            
            if (status >= 0) {
                T *pBlock = new T[2*m_iNumBlocks];
                memcpy(pBlock, aBuf, 2*m_iNumBlocks*sizeof(T));
                m_mSequences[itIdxId->second] = pBlock;
                
            } else {
                fprintf(stderr, "Error during slab reading\n");
                iResult = -1;
            }
        } else {
            printf("Index too big: (%lld+1)*2*numblocks > %lld\\n", iOffset, dims);
            iResult = -1;
        }
    }

    if (m_mSequences.size() == mAllSelected.size()) {
        printf("Successfully read all required %zd sequences\n", m_mSequences.size());
    } else {
        fprintf(stderr, "Error: read only %zd of %zd required sequences\n", m_mSequences.size(), m_mSelected.size());
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
//   uses number and sampling distance from file
//   otherwise -1 on error
//
template<typename T>
int QDFSequenceExtractor<T>::createSelection(const char *sLocFile, const char *sRefFile, bool bDense, int iNumPerBuf, const char *pSampIn, const char *pSampOut) {
    // "0, 0" use num samp and dist from file
    int iResult = createSelection(sLocFile, 0, 0, sRefFile, bDense, iNumPerBuf, pSampIn, pSampOut);
    return iResult;
}


//----------------------------------------------------------------------------
// createSelection
//   if all reads have been successful, the number of agents is returned.
//   otherwise -1 on error
//
template<typename T>
int QDFSequenceExtractor<T>::createSelection(const char *sLocFile, int iNumSamp, double dSampDist, const char *sRefFile, bool bDense, int iNumPerBuf, const char *pSampIn, const char *pSampOut) {
    int iResult = -1;

    if (m_pCurSample != NULL) {
        delete m_pCurSample;
        m_pCurSample = NULL;
    }
    m_sSelected.clear();
    m_mSelected.clear();

    locspec ls(sLocFile, dSampDist, iNumSamp);
    locspec rs(sRefFile, dSampDist, iNumSamp);

    if (m_bVerbose) printf("--- creating IDSampler from [%s] for locations [%s] (pSampIn:[%s])\n", m_pQDFGeoFile, sLocFile, pSampIn);
    if (pSampIn == NULL) {
        IDSampler2 *pIS = IDSampler2::createInstance(m_pQDFGeoFile, m_pWELL, m_bCartesian);
        if (pIS != NULL) {
            iResult = 0;
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

            m_pCurSample = pIS->getSamples(m_vQDFPopFiles, m_pPopName, &ls, m_mLocData, &rs, pSampOut);
            m_pRefSample = pIS->getRefSample();

            if ((iResult == 0) && (pSampOut != NULL)) {
                printf("writing samples to [%s]\n", pSampOut);
                m_pCurSample->write(pSampOut);
            }
            

            delete pIS;

        } else {
            iResult = -1;
            fprintf(stderr, "Couldn't create IDSampler for Grid [%s]\n", m_pQDFGeoFile);
        }
    } else {
        m_pCurSample = new IDSample();
        m_pRefSample = new IDSample();
        iResult = m_pCurSample->read(pSampIn);
        fillLocData(&ls, m_mLocData);
        if (iResult != 0) {
            fprintf(stderr, "Couldn't read IDSample from [%s]\n", pSampIn);

            delete m_pCurSample;
            m_pCurSample = NULL;
            delete m_pRefSample;
            m_pRefSample = NULL;
        }
    }

    if (m_pCurSample != NULL) {
        //        m_pCurSample->display();
        iResult = 0;
        if (m_pCurSample != NULL) {
            m_sSelected.clear();
            m_pCurSample->getFullIDSet(m_sSelected);
            m_pCurSample->getFullIndexIDMap(m_mSelected);
            m_pRefSample->getFullIDSet(m_sRefSelected);
             m_pRefSample->getFullIndexIDMap(m_mRefSelected);
            //           pIS->getLocationIDSet();

            m_iNumSelected = m_sSelected.size();
            if (m_bVerbose) {printf("Sampled: Total %d id%s\n", m_iNumSelected, (m_iNumSelected!=1)?"s":"");fflush(stdout);}
            
            double dT0 = omp_get_wtime();
            if (bDense) {
                iResult = getSelectedSequencesDense(m_pSequenceDataSetName, iNumPerBuf);
            } else {
                iResult = getSelectedSequencesSparse(m_pSequenceDataSetName, iNumPerBuf);
            }
            double dT1 = omp_get_wtime();
            if (m_bVerbose) {printf("Got sequences: [%p] R[%d]\n", m_pSequenceDataSetName, iResult);fflush(stdout);}
            if (m_bVerbose) printf("QDFSequenceExtractor: time to get genomes: %f\n", dT1- dT0);
        } else {
	    fprintf(stderr, "Couldn't get samples\n");
        }
        
    }
    return iResult;
}

