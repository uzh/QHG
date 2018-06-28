#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include <openssl/sha.h>

#include <vector>
#include <algorithm>

#include "MessLogger.h"
#include "strutils.h"

#include "BinomialDist.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "QDFUtils.h"

#include "clsutils.h"
#include "Genetics.h"

// needed for BinomialDistribution
#define EPS 1e-6

static const uint  BUFSIZE_READ_GENOMES=1000;


#define MAX_INIT_NAME 32

// this number must changed if the parameters change
template<typename T, class U>
int Genetics<T,U>::NUM_GENETIC_PARAMS = 6;

//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
Genetics<T,U>::Genetics(SPopulation<T> *pPop,  SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL)  
    : Action<T>(pPop, pCG),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(apWELL),
      m_iGenomeSize(-1),
      m_pBDist(NULL),
      m_iNumCrossOvers(-1),
      m_dMutationRate(-1),
      m_pTempGenome1(NULL),
      m_pTempGenome2(NULL),
      m_iNumBlocks(0),
      m_bCreateNewGenome(0),
      m_iBitsPerNuc(U::BITSINNUC),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_pGenomeCreator(NULL),
      m_iNumParents(2),
      m_bOwnWELL(false) {


    m_iNumThreads = omp_get_max_threads();

    // GenomeCreator must exist before init() is called
    m_pGenomeCreator = new GenomeCreator<U>(m_iNumParents);

}


//-----------------------------------------------------------------------------
// constructor
//
template<typename T, class U>
Genetics<T,U>::Genetics(SPopulation<T> *pPop,  SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed)  
    : Action<T>(pPop, pCG),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(NULL),
      m_iGenomeSize(-1),
      m_pBDist(NULL),
      m_iNumCrossOvers(-1),
      m_dMutationRate(-1),
      m_pTempGenome1(NULL),
      m_pTempGenome2(NULL),
      m_iNumBlocks(0),
      m_bCreateNewGenome(0),
      m_iBitsPerNuc(U::BITSINNUC),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_pGenomeCreator(NULL),
      m_iNumParents(2),
      m_bOwnWELL(true) {


    m_iNumThreads = omp_get_max_threads();

    // GenomeCreator must exist before init() is called
    m_pGenomeCreator = new GenomeCreator<U>(m_iNumParents);

    // we need to build our own WELLs    
    printf("[Genetics::Genetics] using %u as seed for WELLs\n", iSeed);
    buildWELLs(iSeed);
   
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T, class U>
Genetics<T,U>::~Genetics() {
    deleteAllocated();
    if (m_pGenomeCreator != NULL) {
        delete m_pGenomeCreator;
    }

    if (m_bOwnWELL && (m_apWELL != NULL)) {
        for  (int iT = 0; iT < m_iNumThreads; iT++) {
            delete m_apWELL[iT];
        }
        delete[] m_apWELL;
    }
}


//-----------------------------------------------------------------------------
// deleteAllocated
//
template<typename T, class U>
void Genetics<T,U>::deleteAllocated() {

    if (m_pTempGenome1 != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pTempGenome1[iT] != NULL) {
                delete[] m_pTempGenome1[iT];
            }
        }
        delete[] m_pTempGenome1;
    }

    if (m_pTempGenome2 != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pTempGenome2[iT] != NULL) {
                delete[] m_pTempGenome2[iT];
            }
        }
        delete[] m_pTempGenome2;
    }

    if (m_pBDist != NULL) {
        for (int iT = 0; iT < m_iNumThreads; iT++) {
            if (m_pBDist[iT] != NULL) {
                delete m_pBDist[iT];
            }
        }
        delete[] m_pBDist;
    }

    delete m_pWriteCopyController;

    if (m_bBufferAdded) {
        m_pAgentController->removeBuffer(static_cast<LBBase *>(&m_aGenome));
        m_bBufferAdded = false;
    }
}


//-----------------------------------------------------------------------------
// buildWELLs
//   build genetic's own WELL array
//  
template<typename T, class U>
void Genetics<T,U>::buildWELLs(uint iSeed) {

    uint32_t temp[STATE_SIZE];
    m_apWELL = new WELL512 *[m_iNumThreads];
    char sPhrase[256];
    for  (int iT = 0; iT < m_iNumThreads; iT++) {
        sprintf(sPhrase, "seed for %d:%u", iT, iSeed);

        uchar md[SHA512_DIGEST_LENGTH];
        SHA512((uchar *)sPhrase, strlen(sPhrase), md);
        for (uint i = 0; i < STATE_SIZE; i++) {
            temp[i] = *((uint32_t *)(md+sizeof(uint)*i));
        }
        m_apWELL[iT] = new WELL512(temp);
    }
}


//-----------------------------------------------------------------------------
// init
//   must be called *after* params have been read, 
//   but *before* reading the genome
//
template<typename T, class U>
int Genetics<T,U>::init() {
    int iResult = -1;
    printf("init called\n");
    deleteAllocated();
    if ((m_iGenomeSize > 0) && 
        /*(m_dMutationRate >= 0) && */(m_dMutationRate <= 1)) {
        
        if (m_iBitsPerNuc == U::BITSINNUC) {
            // number of longs needed to hold genome
            m_iNumBlocks  = U::numNucs2Blocks(m_iGenomeSize);

            // initialize the buffer ...
            printf("initializing m_aGenome with (%d, %d)\n", m_pAgentController->getLayerSize(), m_iNumParents*m_iGenomeSize);
            m_aGenome.init(m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
        
            // ... and add it to the AgentController
            iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aGenome));


            // layeredBuffer and controller for writing a copy of the actual agent array
            if (iResult == 0) {

                m_pWriteCopyController = new LBController;

                m_aWriteCopy.init( m_pAgentController->getLayerSize(), m_iNumParents*m_iNumBlocks);
                m_pWriteCopyController->init( m_pAgentController->getLayerSize());
                iResult = m_pWriteCopyController->addBuffer(static_cast<LBBase *>(&m_aWriteCopy));
                m_pWriteCopyController->addLayer();
            }

            if (iResult == 0) {
            
                m_bBufferAdded = true;
            
            
                m_pBDist = new BinomialDist*[m_iNumThreads];
                // for each thread we need two temporary arrays for the crossing-over 
                m_pTempGenome1 = new ulong*[m_iNumThreads];
                m_pTempGenome2 = new ulong*[m_iNumThreads];
        
#pragma omp parallel
                {
                    int iT = omp_get_thread_num();
                    m_pBDist[iT] = BinomialDist::create(m_dMutationRate, m_iNumParents*m_iGenomeSize, EPS);
                    if (m_pBDist[iT] != NULL) {

                        // two arrays for crossing over calculations
                        m_pTempGenome1[iT] = new ulong[m_iNumParents*m_iGenomeSize];
                        m_pTempGenome2[iT] = new ulong[m_iNumParents*m_iGenomeSize];
                    
                        iResult = 0;
                    } else {
                        printf("Couldn't create BinomialDistribution\n");
                    }
                }

            } else {
                printf("Couldn't add buffer to controller\n");
            }
        } else {
            printf("[Genetics] This module expects %d bit nucleotides, but the attruibute specifies %d bit nucleotides\n", U::BITSINNUC, m_iBitsPerNuc);
        }

    } else {
        printf("Bad values for genome size or num crossovers or mutation rate\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// makeOffspring
//   actual creation of babies
//   (called from SPopulation::performBirths()
//
template<typename T, class U>
int  Genetics<T,U>::makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex) {
    int iResult = 0;

    // we have to manipulate inside of a layer buf array
    // better to treat them as normal arrays
    // this is safe because a genome is alway inside  single layer
    ulong *pBabyGenome  = &(m_aGenome[iBabyIndex]);
    ulong *pGenome1     = &(m_aGenome[iMotherIndex]);
    ulong *pGenome2     = &(m_aGenome[iFatherIndex]);

    // clear baby genome
    memset(pBabyGenome, 0, m_iNumParents*m_iNumBlocks*sizeof(ulong));

    int iT = omp_get_thread_num();

    // which strand to chose from genome 1 for baby genome
    int i1 = (int)(m_iNumParents*1.0*m_apWELL[iT]->wrandd());
    // which strand to chose from genome 2 for baby genome
    int i2 = (int)(m_iNumParents*1.0*m_apWELL[iT]->wrandd());


    if (m_iNumCrossOvers > 0) {
        // crossover wants the genome size, not the number of blocks
        U::crossOver(m_pTempGenome1[iT], pGenome1, m_iGenomeSize, m_iNumCrossOvers, m_apWELL[iT]);
        U::crossOver(m_pTempGenome2[iT], pGenome2, m_iGenomeSize, m_iNumCrossOvers, m_apWELL[iT]);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempGenome1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempGenome2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == -1) {
        // full recombination
        U::freeReco(m_pTempGenome1[iT], pGenome1, m_iNumBlocks, m_apWELL[iT]);
        U::freeReco(m_pTempGenome2[iT], pGenome2, m_iNumBlocks, m_apWELL[iT]);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempGenome1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempGenome2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == 0) {
        // no cross over: copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              pGenome1+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, pGenome2+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
    }

    // perhaps some mutations...
    if (m_dMutationRate > 0) {
        int iNumMutations = m_pBDist[iT]->getN(m_apWELL[iT]->wrandd());
        U::mutateNucs(pBabyGenome, m_iNumParents*m_iGenomeSize, iNumMutations, m_apWELL[iT]);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//  write additional data to the group
//  Here: create a new Dataset for the genome.
//  Write genomes sequentially from all cells.
//
//  Go through all cells, and use the BufLayers of m_aGenome as slabs
//
template<typename T, class U>
int Genetics<T,U>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = -1;
    iResult = writeAdditionalDataQDFSafe(hSpeciesGroup);
    //iResult = writeAdditionalDataQDFOld(hSpeciesGroup);
    //    printf("After writeAdditionalAgentDataQDF:\n");
    //    this->m_pPop->checkLists();

    return iResult;
}

    
//----------------------------------------------------------------------------
// writeAdditionalDataQDFSafe
//  write additional data to the group
//  Here: create a new Dataset for the genome.
//  Write genomes sequentially from all cells.
//  This method does not change the agent- or genome array
//  Go through all cells, and use the BufLayers of m_aGenome as slabs 
//
template<typename T, class U>
int Genetics<T,U>::writeAdditionalDataQDFSafe(hid_t hSpeciesGroup) {
    //    int iResult = 0;
    printf("[Genetics<T,U>::writeAdditionalDataQDFSafe]\n");
    herr_t status=-1;
    int iTotalKill = 0;
    uint iNumWritten = 0;
    uint iWrittenLongs = 0;

    // amount of elements we have to save 
    hsize_t dims = this->m_pPop->getNumAgentsEffective()*m_iNumParents*m_iNumBlocks; 
    //                    printf("  [%s] numagents total: %lld\n", it->first->getSpeciesName(), dims);

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, GENOME_DATASET_NAME, H5T_NATIVE_LONG, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        
        if (hDataSet > 0) {
            hsize_t dimsm = m_pAgentController[0].getLayerSize()*m_iNumParents*m_iNumBlocks;
            hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

            hsize_t iOffset = 0;
            hsize_t iCount  = 0;  
            status = 0;

            uint iD = 0;
            int iLayerSize = m_pWriteCopyController->getLayerSize();
            // does the cell contain agents?
            if (m_pAgentController->getNumUsed() > 0) {
                // loop over BufLayers
                for (uint j = 0; j < m_aGenome.getNumUsedLayers(); j++) {
                    // avoid empty layers
                    if (m_pAgentController->getNumUsed(j) > 0) {
                    
                        // write agents of layer j as hyperslab
                        const ulong* pSlab0 = m_aGenome.getLayer(j);
                        m_aWriteCopy.copyLayer(0, pSlab0);
                        m_pWriteCopyController->setL2List(m_pAgentController->getL2List(j), 0);

                        int iNumKilled = 0;
                        // remove dead in slab (the dead list has been filled in writeAgentQDFSafe())
                        while ((iD < m_pvDeadList->size()) && (m_pvDeadList->at(iD) < (int)(j+1)*iLayerSize)) {
                            m_pWriteCopyController->deleteElement(m_pvDeadList->at(iD) - j*iLayerSize);
                            iD++;
                            iNumKilled++;
                        }
                        iTotalKill += iNumKilled;

                        // here it is important not to have an empty slab
                        m_pWriteCopyController->compactData();
                    
                        const ulong* pSlab = m_aWriteCopy.getLayer(0);
                        // printf("layer %u: %d genomes\n", j, m_pWriteCopyController->getNumUsed(0));
                        iNumWritten +=  m_pWriteCopyController->getNumUsed(0);
                        // adapt memspace if size of slab must change
                        iCount =  m_pWriteCopyController->getNumUsed(0)*m_iNumParents*m_iNumBlocks;
                        if (iCount != dimsm) {
                            qdf_closeDataSpace(hMemSpace); 
                            dimsm = iCount;
                            hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
                        }
                        
                        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                                     &iOffset, NULL, &iCount, NULL);
                        
                        status = H5Dwrite(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                                          hDataSpace, H5P_DEFAULT, pSlab);
                        iOffset += iCount;
                        iWrittenLongs += iCount;
                    } else {
                        printf("[Genetics<T,U>::writeAdditionalDataQDFSafe] ignored layer %d because it's empty\n", j);
                    }
                }
                
            }
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }

    printf("[Genetics<T,U>::writeAdditionalDataQDFSafe] written %u items (%u longs), killed %d\n", iNumWritten, iWrittenLongs, iTotalKill); fflush(stdout);
    printf("[Genetics<T,U>::writeAdditionalDataQDFSafe] end with status %d\n", status); fflush(stdout);

    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// writeAdditionalDataQDFOld
//  write additional data to the group
//  Here: create a new Dataset for the genome.
//  Write genomes sequentially from all cells.
//
//  Go through all cells, and use the BufLayers of m_aGenome as slabs
//
template<typename T, class U>
int Genetics<T,U>::writeAdditionalDataQDFOld(hid_t hSpeciesGroup) {
    //    int iResult = 0;
    printf("writeAdditionalDataQDFOld\n");
    herr_t status=-1;
    uint iNumWritten = 0;
    uint iWrittenLongs = 0;
    // amount of elements we have to save
    hsize_t dims = this->m_pPop->getNumAgentsTotal()*m_iNumParents*m_iNumBlocks; 
    //                    printf("  [%s] numagents total: %lld\n", it->first->getSpeciesName(), dims);

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
       
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, GENOME_DATASET_NAME, H5T_NATIVE_LONG, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
       
        if (hDataSet > 0) {
            hsize_t dimsm = m_pAgentController[0].getLayerSize()*m_iNumParents*m_iNumBlocks;
            hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

            hsize_t iOffset = 0;
            hsize_t iCount  = 0; 
            status = 0;

            // data should be compacted (no holes)
            this->m_pPop->compactData();

            // does the cell contain agents?
            if (m_pAgentController->getNumUsed() > 0) {
               
                // loop over BufLayers
                for (uint j = 0; j < m_aGenome.getNumUsedLayers(); j++) {
                   
                    // adapt memspace if size of slab must change
                    iCount =  m_pAgentController->getNumUsed(j)*m_iNumParents*m_iNumBlocks;
                    if (iCount != dimsm) {
                        qdf_closeDataSpace(hMemSpace); 
                        dimsm = iCount;
                        hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
                    }
                    printf("layer %u: %d genomes\n", j, m_pAgentController->getNumUsed(j));
                    iNumWritten +=  m_pAgentController->getNumUsed(j);
                   
                    // write agents of layer j as hyperslab
                    const ulong* pSlab = m_aGenome.getLayer(j);
                    status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                                 &iOffset, NULL, &iCount, NULL);
                   
                    status = H5Dwrite(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                                      hDataSpace, H5P_DEFAULT, pSlab);
                    iOffset += iCount;
                    iWrittenLongs+= iCount;
                }
               
            }
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }

    printf("[Genetics<T,U>::writeAdditionalDataQDFOld] written %u items (%u longs)\n", iNumWritten, iWrittenLongs); fflush(stdout);
    printf("[Genetics<T,U>::writeAdditionalDataQDFOld] end with status %d\n", status); fflush(stdout);

    return (status >= 0)?0:-1;
}


//----------------------------------------------------------------------------
// readAdditionalDataQDF
//  read additional data from the group
//  Read genomes sequentially from all cells which contain agents (1 per agent)
//
// Ass: agents have been read
//      There is exactly one genome per agent
//      Genomes ordered as agents
//  
// The difficulty is that a buffer containing a slab of data from the file
// may contain genome for one or more cells, od the genome for a cell may be 
// spread over several slabs.
// 
template<typename T, class U>
int Genetics<T,U>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = -1;

    
    if (qdf_exists(hSpeciesGroup, GENOME_DATASET_NAME)) {
        iResult = 0;
        
        // the buffer must hold a multiple of the array length
        uint iReadBufSize = BUFSIZE_READ_GENOMES*m_iNumParents*m_iNumBlocks;
        long *aBuf = new long[iReadBufSize];

        // open the data set
        hid_t hDataSet = H5Dopen2(hSpeciesGroup, GENOME_DATASET_NAME, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        // get toal number of elements in dataset
        hsize_t dims;
        herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
        printf("Dataspace extent: %lld\n", dims);

        // initialize some counters and indexes
        int iFirstIndex = 0;
        int iTotalToDo = dims;
        hsize_t iCount;
        hsize_t iOffset = 0;
    
        // loop until all elements have been read
        while ((iResult == 0) && (dims > 0)) {
            // can we get a full load of the buffer?
            if (dims > iReadBufSize) {
                iCount = iReadBufSize;
            } else {
                iCount = dims;
            }
            //@@            printf("Slabbing offset %lld, count %lld\n", iOffset,iCount);
        
            // read a slab
            hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
            status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                         &iOffset, NULL, &iCount, NULL);
            status = H5Dread(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                             hDataSpace, H5P_DEFAULT, aBuf);
        
        
            if (status >= 0) {
                // distribute the data 
                int iArrOffset = 0;
                int iNumThisPass = iCount/(m_iNumParents*m_iNumBlocks);
                int iNumToCopy = iNumThisPass;
                //@@                printf("starting inner loop: iTotalToDo %d, iNumThisPass %d\n", iTotalToDo, iNumThisPass);
                // while we still have genomes to fill and data from the slab not used up
                while ((iTotalToDo > 0) && (iNumThisPass > 0)) {
                    
                    // creation of agents has already created space in the genome buffer
                    // copy from the current point in the buffer to the current index
                    long *pCur = aBuf+iArrOffset;
                    // the positions in the LayerBuf are already have the correct L2List  links
                    // because reserveSpace2() has been called by readAgentDataQDF()
                    m_aGenome.copyBlock(iFirstIndex, (ulong *) pCur, iNumToCopy);

                    // update counters and indexes
                    iFirstIndex += iNumToCopy;
                    iTotalToDo  -= iNumToCopy*m_iNumParents*m_iNumBlocks;
                    iArrOffset  += iNumToCopy*m_iNumParents*m_iNumBlocks;
                    //@@                    printf("after copyblock: firstindex %d, numforcell: %d, totaltodo %d\n", iFirstIndex, iNumForCell,iTotalToDo);
                    iNumThisPass -= iNumToCopy;
                    iNumToCopy    = iNumThisPass;
                }

            } else {
                printf("Error during slab reading\n");
                iResult = -1;
            }
        
            dims    -= iCount;
            iOffset += iCount;
            
        }
        

        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSet(hDataSet);
        
        delete[] aBuf;
    } else {
        printf("WARNING: no dataset [%s] found\n", GENOME_DATASET_NAME);
    }
    return iResult;

}


//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_GENETICS_GENOME_SIZE, 
//    ATTR_GENETICS_NUM_CROSSOVER
//    ATTR_GENETICS_MUTATION_RATE
//    ATTR_GENETICS_INITIAL_MUTS
//    ATTR_GENETICS_CREATE_NEW_GENOME
//    ATTR_GENETICS_BITS_PER_NUC 
//
template<typename T, class U>
int Genetics<T,U>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_MUTATION_RATE,  1, &m_dMutationRate);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
    }
    if (iResult == 0) {
        const char *pInitData = m_pGenomeCreator->getInitString();
        iResult = qdf_insertSAttribute(hSpeciesGroup, ATTR_GENETICS_INITIAL_MUTS, pInitData);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_CREATE_NEW_GENOME, 1, &m_bCreateNewGenome);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_GENETICS_BITS_PER_NUC, 1, &m_iBitsPerNuc);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_GENETICS_GENOME_SIZE, 
//    ATTR_GENETICS_NUM_CROSSOVER
//    ATTR_GENETICS_MUTATION_RATE
//    ATTR_GENETICS_INITIAL_MUTS
//    ATTR_GENETICS_CREATE_NEW_GENOME
//    ATTR_GENETICS_BITS_PER_NUC 
//
template<typename T, class U>
int Genetics<T,U>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_GENOME_SIZE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_MUTATION_RATE,  1, &m_dMutationRate);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_MUTATION_RATE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_NUM_CROSSOVER);
        }
    }
    
    if (iResult == 0) {
        char sTemp[MAX_INIT_NAME];
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_GENETICS_INITIAL_MUTS, MAX_INIT_NAME, sTemp);
        if (iResult != 0) {
            LOG_ERROR("[Genetics] couldn't read attribute [%s]", ATTR_GENETICS_INITIAL_MUTS);
        } else {
            iResult = m_pGenomeCreator->determineInitData(sTemp);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_CREATE_NEW_GENOME, 1, &m_bCreateNewGenome);
        if (iResult != 0) {
            LOG_WARNING("[Genetics] couldn't read attribute [%s]; setting value to false", ATTR_GENETICS_CREATE_NEW_GENOME);
            m_bCreateNewGenome = 0;
            iResult = 0;
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_GENETICS_BITS_PER_NUC, 1, &m_iBitsPerNuc);
        if (iResult != 0) {
            LOG_WARNING("[Genetics] couldn't read attribute [%s]; setting value to %d", ATTR_GENETICS_BITS_PER_NUC, U::BITSINNUC);
            m_iBitsPerNuc = U::BITSINNUC;
            iResult = 0;
        } else {
            if (m_iBitsPerNuc != U::BITSINNUC) {
                iResult = -1;
                LOG_ERROR("[Genetics] value of [%s] does not match current GeneUtils::BITSINNUC", ATTR_GENETICS_BITS_PER_NUC);
            }
        }
    }
    
    printf("[Genetics] ExtractParamsQDF:res %d\n", iResult);
    
    if (iResult == 0) {
        // we must call init() before callnig readAdditionalDataQDF()
        iResult = init();
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T, class U>
int Genetics<T,U>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;
    
    iResult += this->readPopKeyVal(pLine, ATTR_GENETICS_GENOME_SIZE,       &m_iGenomeSize);
    iResult += this->readPopKeyVal(pLine, ATTR_GENETICS_NUM_CROSSOVER,     &m_iNumCrossOvers);
    iResult += this->readPopKeyVal(pLine, ATTR_GENETICS_MUTATION_RATE,     &m_dMutationRate);
    iResult += this->readPopKeyVal(pLine, ATTR_GENETICS_CREATE_NEW_GENOME, &m_bCreateNewGenome);
    iResult += this->readPopKeyVal(pLine, ATTR_GENETICS_BITS_PER_NUC,      &m_iBitsPerNuc);
    
    if (strstr(pLine, ATTR_GENETICS_INITIAL_MUTS) == pLine) {
        int iRes = m_pGenomeCreator->determineInitData(pLine);
        if (iRes == 0) {
            iResult = 1;
        } else {
            printf("value for [%s] is malformed or unknown: [%s]\n", ATTR_GENETICS_INITIAL_MUTS, pLine);
            iResult = 0;
        }

    }
    
    if (iResult != 0) {
        m_iNumSetParams += iResult;
        
        if (!m_bBufferAdded && (m_iNumSetParams >= NUM_GENETIC_PARAMS)) {
            // we must call init() before calling readAdditionalDataQDF()
            iResult = 1 + init(); // init:-1 => res: 0; init:0 => res 1 
        }
    }
   
    return iResult;
}


//-----------------------------------------------------------------------------
// createInitialGenomes
//   only do this if the buffer has been already added to the controller
//
template<typename T, class U>
int Genetics<T,U>::createInitialGenomes(int iNumGenomes) {
    int iResult = -1;
    if (m_bBufferAdded) {
        if (m_bCreateNewGenome != 0) {
            iResult = 0;
            m_bCreateNewGenome = false;
            iResult = m_pGenomeCreator->createInitialGenomes(m_iGenomeSize, iNumGenomes, m_aGenome, m_apWELL);
        } else {
            printf("No new Genome created\n");
            iResult = 0;
        }
    } else {
        printf("Genetics has not been initialized\n");
    }   
    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T, class U>
void Genetics<T,U>::showAttributes() {
    printf("  %s\n", ATTR_GENETICS_GENOME_SIZE);
    printf("  %s\n", ATTR_GENETICS_NUM_CROSSOVER);
    printf("  %s\n", ATTR_GENETICS_MUTATION_RATE);
    printf("  %s\n", ATTR_GENETICS_INITIAL_MUTS);
    printf("  %s\n", ATTR_GENETICS_CREATE_NEW_GENOME);
    printf("  %s\n", ATTR_GENETICS_BITS_PER_NUC);
}

