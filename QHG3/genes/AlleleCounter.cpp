#include <stdio.h>
#include <string.h>

#include <vector>
#include <algorithm>
#include <omp.h>

#include "types.h"
#include "colors.h"

#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "QDFArray.h"
#include "QDFUtils.h"

#include "AlleleCounter.h"

#define BUFSIZE_GENOMES 1000000


//----------------------------------------------------------------------------
// createInstance
//
AlleleCounter *AlleleCounter::createInstance(const char *pQDFPopFile, const char *pSpeciesName) {
    AlleleCounter *pNC = new AlleleCounter();
    int iResult = pNC->init(pQDFPopFile, pSpeciesName);
    if (iResult != 0) {
        delete pNC;
        pNC = NULL;
    }
    return pNC;
}


//----------------------------------------------------------------------------
// constructor
//
AlleleCounter::AlleleCounter() 
    : m_iGenomeSize(0),
      m_iNumBlocks(0),
      m_iBitsPerNuc(0),
      m_iNumNucs(0),
      m_iNucsInBlock(0),
      m_iNumGenomes(0),
      m_iNumSelected(0),
      m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_ppGenomes(NULL),
      m_ppCounts(NULL),
      m_bVerbose(true),
      m_pPopName(NULL) {

}


//----------------------------------------------------------------------------
// destructor
//
AlleleCounter::~AlleleCounter() {
    if (m_ppCounts != NULL) {
        for (int i = 0; i < m_iGenomeSize; ++i) {
            delete[] m_ppCounts[i];
        }
        delete[] m_ppCounts;
    }

    if (m_ppGenomes != NULL) {
        for (int i = 0; i < m_iNumSelected; ++i) {
            delete[] m_ppGenomes[i];
        }
        delete[] m_ppGenomes;
    }

    if (m_hFile != H5P_DEFAULT) {
        qdf_closeFile(m_hFile);
    }
    if (m_hPopulation != H5P_DEFAULT) {
        qdf_closeGroup(m_hPopulation);
    }
    if (m_hSpecies != H5P_DEFAULT) {
        qdf_closeGroup(m_hSpecies);
    }

    if (m_pPopName != NULL) {
        delete[] m_pPopName;
    }
}


//----------------------------------------------------------------------------
// init
//
int AlleleCounter::init(const char *pQDFPopFile, const char *pSpeciesName) {
    int iResult = -1;

    if (pSpeciesName == NULL) {
        m_pPopName = qdf_getFirstPopulation(pQDFPopFile);
    } else {
        m_pPopName = qdf_checkForPop(pQDFPopFile, pSpeciesName);
    }
    if (m_pPopName != NULL) {
        m_hFile        = qdf_openFile(pQDFPopFile);
        m_hPopulation  = qdf_openGroup(m_hFile, POPGROUP_NAME);
        m_hSpecies     = qdf_openGroup(m_hPopulation, m_pPopName);
        iResult = qdf_extractAttribute(m_hSpecies, POP_ATTR_GENOME_SIZE, 1, &m_iGenomeSize);
        if (iResult == 0) {
            
            iResult = qdf_extractAttribute(m_hSpecies, POP_ATTR_BITS_PER_NUC, 1, &m_iBitsPerNuc);
            if (iResult != 0) {
                // no such entry -> default
                m_iBitsPerNuc = DEF_BITS_PER_NUC;
                iResult = 0;
            }
            m_iNumNucs = 1 << m_iBitsPerNuc;
            if (m_iBitsPerNuc == 1) {
                m_iNumBlocks = BitGeneUtils::numNucs2Blocks(m_iGenomeSize);
                m_iNucsInBlock = BitGeneUtils::NUCSINBLOCK;
            } else {
                m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
                m_iNucsInBlock = GeneUtils::NUCSINBLOCK;
            }

            double dMutRate = 0;
            qdf_extractAttribute(m_hSpecies, POP_ATTR_MUT_RATE, 1, &dMutRate);
            
            
            // m_iGenomeSize is known, so we can prepare the arrays
            printf("Found attributes: GeneomeSize(%d), bitspernuc (%d), numnucs (%d), numblocks (%d), nucsinblock (%d), urate (%f)\n",
                   m_iGenomeSize, m_iBitsPerNuc, m_iNumNucs, m_iNumBlocks, m_iNucsInBlock,dMutRate);
            prepareArrays();
            
            
        } else {
            fprintf(stderr, "Couldn't extract Attribute [%s] for genome size\n", POP_ATTR_GENOME_SIZE);
        }

    } else {
        if (pSpeciesName == NULL) {
            fprintf(stderr, "Couldn't find any population in [%s]\n", pQDFPopFile);
        } else {
            fprintf(stderr, "Couldn't get population [%s] from [%s]\n", pSpeciesName, pQDFPopFile);
        }
    }


    return iResult;
}

    
//----------------------------------------------------------------------------
// prepareArrays
//   m_iGenomeSize *must* be set!
//
int AlleleCounter::prepareArrays() {
    int iResult = -1;

    if (m_iGenomeSize > 0) {
        m_ppCounts = new uint *[m_iGenomeSize];
        for (int i = 0; i < m_iGenomeSize; i++) {
            m_ppCounts[i] = new uint[m_iNumNucs];
            memset(m_ppCounts[i], 0, m_iNumNucs*sizeof(uint));
        }


        iResult = 0;
    } else {
        fprintf(stderr, "GenomeSize has invalid value: %d (should be positive)\n", m_iGenomeSize);
    }        

    return iResult;
}


//----------------------------------------------------------------------------
// selectIndexes
//   
//
int AlleleCounter::selectIndexes(int iSelectionType) {
    int iResult = -1;
    
    if (m_ppGenomes != NULL) {
        for (int i = 0; i < m_iNumSelected; ++i) {
            delete[] m_ppGenomes[i];
        }
        delete[] m_ppGenomes;
    }
    
    QDFArray *pQA = QDFArray::create(m_hFile);
    if (pQA != NULL) {
        iResult = pQA->openArray(POPGROUP_NAME, m_pPopName, POP_DS_AGENTS);

        if (iResult == 0) {
            m_iNumGenomes = pQA->getSize();

            
            if (iResult == 0) {

                switch (iSelectionType) {
                case SEL_GENDER_F: 
                case SEL_GENDER_M: {
                    int *pGenders  = new int[m_iNumGenomes];
                    if (iResult == 0) {
                        int iCount = pQA->getFirstSlab(pGenders, m_iNumGenomes, "Gender");
                        if (iCount != m_iNumGenomes) {
                            fprintf(stderr, "%sGot %d genders instead of %d%s\n", RED, iCount, m_iNumGenomes, OFF);
                            iResult = -1;
                        }
                    }
                    int iGender = (iSelectionType== SEL_GENDER_F)?0:1;
                    for (int i = 0; i < m_iNumGenomes; i++) {
                        if (pGenders[i] == iGender) {
                            m_vSelectedIndexes.push_back(i);
                        }
                    }
                    
                    break;
                }
                case SEL_ALL:
                    for (int i = 0; i < m_iNumGenomes; i++) {
                        m_vSelectedIndexes.push_back(i);
                    }
                    break;
                default:
                    printf("Unknown selection type [%d]\n", iSelectionType);
                    iResult = -1;
                }
            }

            if (iResult == 0) {
                m_iNumSelected = 2*m_vSelectedIndexes.size();
                m_ppGenomes = new ulong *[m_iNumSelected];
                for (int i = 0; i < m_iNumSelected; i++) {
                    m_ppGenomes[i] = new ulong[m_iNumBlocks];
                    memset(m_ppGenomes[i], 0, m_iNumBlocks*sizeof(ulong));
                }
            
            }

        }
        
        pQA->closeArray();   
    }

    return iResult;
}
 

//----------------------------------------------------------------------------
// countAlleles
//   
//
int AlleleCounter::countAlleles(int iSelectionType) {
    int iResult = 0;
    
    // reset counts
    for (int i = 0; i < m_iGenomeSize; i++) {
        memset(m_ppCounts[i], 0, m_iNumNucs*sizeof(uint));
    }

    // select indexes
    iResult = selectIndexes(iSelectionType);
    if (iResult == 0) {
        iResult = loadGenomes(-1);
    }

    ulong uMask = 0x03;
    if (m_iBitsPerNuc == 1) {
        uMask = 0x01;
    }
    
    // count
    if (iResult == 0) {
        for (int iBlock = 0; iBlock < m_iNumBlocks; ++iBlock) {
            // parallel?
            //#pragma omp parallel for
            for (int iIndex = 0; iIndex < m_iNumSelected; ++iIndex) {
                ulong uBlock = m_ppGenomes[iIndex][iBlock];
                for (int j = 0; j < m_iNucsInBlock; ++j) {
                    int nuc = uBlock & uMask;
                    m_ppCounts[iBlock*m_iNucsInBlock + j][nuc]++;
                    uBlock >>= m_iBitsPerNuc;
                }
            }
        }
    }


    return iResult;
}

//----------------------------------------------------------------------------
// loadGenomes
//   
//
int AlleleCounter::loadGenomes(int iNumPerBuf) {
    int iResult = 0;

    printf("getSelectedGenes (dense)\n");


    // read buffer
    ulong iReadBufSize = ((iNumPerBuf<=0)?BUFSIZE_GENOMES:iNumPerBuf)*2*m_iNumBlocks;
    long *aBuf = new long[iReadBufSize];
   
    // open the DataSet and data space
    if (m_bVerbose) {printf("Opening data set [%s] in species\n", POP_DS_GENOME); fflush(stdout);}
    hid_t hDataSet = H5Dopen2(m_hSpecies, POP_DS_GENOME, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    if (m_bVerbose) printf("Dataspace extent: %lld\n", dims);

    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    int iGlobalOffset = 0;
    int iCurGenome = 0;
    std::vector<int>::const_iterator itIdxId = m_vSelectedIndexes.begin();
    

    if (m_bVerbose) printf("Trying to extract %zd genomes\n", m_vSelectedIndexes.size());
    // loop until all elements have been read
    while ((iResult == 0) && (dims > 0) && (itIdxId != m_vSelectedIndexes.end())) {
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
            for (int i = 0; (iResult == 0) && (i < iNumThisPass) && (itIdxId != m_vSelectedIndexes.end()); i++) {
                int j = i+iGlobalOffset;
                
                if (j == *itIdxId) {
                    ulong *pBlock = m_ppGenomes[iCurGenome++];
                    memcpy(pBlock, aBuf+i*2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
                    pBlock = m_ppGenomes[iCurGenome++];
                    memcpy(pBlock, aBuf+(i*2+1)*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
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


    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
        
    delete[] aBuf;
    
    return iResult;


}

