#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "BinomialDist.h"
#include "LBController.h"
#include "LayerArrBuf.h"
#include "QDFUtils.h"
#include "GeneUtils.h"
#include "clsutils.h"
#include "Genetics.h"

// needed for BinomialDistribution
#define EPS 1e-6

static const uint  BUFSIZE_READ_ADDITIONAL=1000;


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Genetics<T>::Genetics(SPopulation<T> *pPop,  SCellGrid *pCG, LBController *pAgentController, WELL512** apWELL)  
    : Action<T>(pPop, pCG),
      m_pAgentController(pAgentController),
      m_apWELL(apWELL),
      m_iGenomeSize(0),
      m_pBDist(NULL),
      m_iNumCrossOvers(0),
      m_dMutationRate(0),
      m_pTempGenome1(NULL),
      m_pTempGenome2(NULL),
      m_iNumBlocks(0),
      m_iNumSetParams(0),
      m_bBufferAdded(false)  {


    m_iNumThreads = omp_get_max_threads();
}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Genetics<T>::~Genetics() {
    deleteAllocated();}

//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
void Genetics<T>::deleteAllocated() {

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

    if (m_bBufferAdded) {
        m_pAgentController->removeBuffer(static_cast<LBBase *>(&m_aGenome));
        m_bBufferAdded = false;
    }
}

//-----------------------------------------------------------------------------
// init
//
template<typename T>
int Genetics<T>::init() {
    int iResult = -1;
    printf("init called\n");
    deleteAllocated();
    if ((m_iGenomeSize > 0) && 
        //        (m_iNumCrossOvers >= 0) &&   // m_iNumCrossOvers = -1 for full recombination
        (m_dMutationRate >= 0) && (m_dMutationRate <= 1)) {
        
        m_pBDist = new BinomialDist*[m_iNumThreads];
        m_pTempGenome1 = new ulong*[m_iNumThreads];
        m_pTempGenome2 = new ulong*[m_iNumThreads];
        
        // number of longs needed to hold genome
        m_iNumBlocks  = GeneUtils::numNucs2Blocks(m_iGenomeSize);

        // initialize the buffer ...
        printf("initializing m_aGenome with (%d, %d)\n", m_pAgentController->getLayerSize(), 2*m_iGenomeSize);
        m_aGenome.init(m_pAgentController->getLayerSize(), 2*m_iNumBlocks);
        
        // ... and add it to the AgentController
        iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aGenome));

        if (iResult == 0) {
            
            m_bBufferAdded = true;
            
            //#ifdef OMP_A
#pragma omp parallel
        {
            //#endif
            int iT = omp_get_thread_num();
            m_pBDist[iT] = BinomialDist::create(m_dMutationRate, 2*m_iGenomeSize, EPS);
            if (m_pBDist[iT] != NULL) {

                // two arrays for crossing over calculations
                m_pTempGenome1[iT] = new ulong[2*m_iGenomeSize];
                m_pTempGenome2[iT] = new ulong[2*m_iGenomeSize];
                    
                iResult = 0;
            } else {
                printf("Couldn't create BinomialDistribution\n");
            }
            //#ifdef OMP_A
        }
        //#endif
        } else {
            printf("Couldn't add buffer to controller\n");
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
template<typename T>
int  Genetics<T>::makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex) {
    int iResult = 0;

    // we have to manipulate inside of a layer buf array
    // better to treat them as normal arrays
    // this is safe because a genome is alway inside  single layer
    ulong *pBabyGenome  = &(m_aGenome[iBabyIndex]);
    ulong *pGenome1     = &(m_aGenome[iMotherIndex]);
    ulong *pGenome2     = &(m_aGenome[iFatherIndex]);

    // clear baby genome
    memset(pBabyGenome, 0, 2*m_iNumBlocks*sizeof(ulong));

    int iT = omp_get_thread_num();

    // which strand to chose from genome 1 for baby genome
    int i1 = (int)(2.0*m_apWELL[iT]->wrandd());
    // which strand to chose from genome 2 for baby genome
    int i2 = (int)(2.0*m_apWELL[iT]->wrandd());


    if (m_iNumCrossOvers > 0) {
        // crossover wants the genome size, not the number of blocks
        GeneUtils::crossOver(m_pTempGenome1[iT], pGenome1, m_iGenomeSize, m_iNumCrossOvers, m_apWELL[iT]);
        GeneUtils::crossOver(m_pTempGenome2[iT], pGenome2, m_iGenomeSize, m_iNumCrossOvers, m_apWELL[iT]);
        // now copy a randomly chosen strand from each parent
        memcpy(pBabyGenome,              m_pTempGenome1[iT]+i1*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));
        memcpy(pBabyGenome+m_iNumBlocks, m_pTempGenome2[iT]+i2*m_iNumBlocks, m_iNumBlocks*sizeof(ulong));

    } else if (m_iNumCrossOvers == -1) {
        // full recombination
        GeneUtils::freeReco(m_pTempGenome1[iT], pGenome1, m_iNumBlocks, m_apWELL[iT]);
        GeneUtils::freeReco(m_pTempGenome2[iT], pGenome2, m_iNumBlocks, m_apWELL[iT]);
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
        // anywhere on the two chromosomes
        GeneUtils::mutateNucs(pBabyGenome, 2*m_iGenomeSize, iNumMutations, m_apWELL[iT]);
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
template<typename T>
int Genetics<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    //    int iResult = 0;
    herr_t status=-1;

    // amount of elements we have to save 
    hsize_t dims = this->m_pPop->getNumAgentsTotal()*2*m_iNumBlocks; 
    //                    printf("  [%s] numagents total: %lld\n", it->first->getSpeciesName(), dims);

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, GENOME_DATASET_NAME, H5T_NATIVE_LONG, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        
        if (hDataSet > 0) {
            hsize_t dimsm = m_pAgentController[0].getLayerSize()*2*m_iNumBlocks;
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
                    iCount =  m_pAgentController->getNumUsed(j)*2*m_iNumBlocks;
                    if (iCount != dimsm) {
                        qdf_closeDataSpace(hMemSpace); 
                        dimsm = iCount;
                        hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
                    }
                    
                    // write agents of layer j as hyperslab
                    const ulong* pSlab = m_aGenome.getLayer(j);
                    status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                                 &iOffset, NULL, &iCount, NULL);
                    
                    status = H5Dwrite(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                                      hDataSpace, H5P_DEFAULT, pSlab);
                    iOffset += iCount;
                }
                
            }
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }

    printf("[Genetics<T>::writeAdditionalDataQDF] end with status %d\n", status); fflush(stdout);

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
template<typename T>
int Genetics<T>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = 0;

    // the buffer must hold a multiple of the array length
    uint iReadBufSize = BUFSIZE_READ_ADDITIONAL*2*m_iNumBlocks;
    long *aBuf = new long[iReadBufSize];

    // open the DataSet and data space
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
            int iNumThisPass = iCount/(2*m_iNumBlocks);
            int iNumToCopy = iNumThisPass;
            //@@                printf("starting inner loop: iTotalToDo %d, iNumThisPass %d\n", iTotalToDo, iNumThisPass);
            // while we still have genomes to fill and data from the slab not used up
            while ((iTotalToDo > 0) && (iNumThisPass > 0)) {

                // creation of agents has already created space in the genome buffer
                // copy from the current point in the buffer to the current index
                long *pCur = aBuf+iArrOffset;
                m_aGenome.copyBlock(iFirstIndex, (ulong *) pCur, iNumToCopy);

                // update counters and indexes
                iFirstIndex += iNumToCopy;
                iTotalToDo  -= iNumToCopy*2*m_iNumBlocks;
                iArrOffset  += iNumToCopy*2*m_iNumBlocks;
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
    
    return iResult;

}

//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
template<typename T>
int Genetics<T>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, GENETICS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, GENETICS_MUTATION_RATE,  1, &m_dMutationRate);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, GENETICS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
template<typename T>
int Genetics<T>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    iResult += qdf_extractAttribute(hSpeciesGroup, GENETICS_GENOME_SIZE,  1, (int *) &m_iGenomeSize);
    iResult += qdf_extractAttribute(hSpeciesGroup, GENETICS_MUTATION_RATE,  1, &m_dMutationRate);
    iResult += qdf_extractAttribute(hSpeciesGroup, GENETICS_NUM_CROSSOVER, 1, &m_iNumCrossOvers);
    printf("ExtractParamsQDF:res %d\n", iResult);
    if (iResult == 0) {
        iResult = init();
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int Genetics<T>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;
    
    iResult += this->readPopKeyVal(pLine, GENETICS_GENOME_SIZE, &m_iGenomeSize);
    iResult += this->readPopKeyVal(pLine, GENETICS_NUM_CROSSOVER, &m_iNumCrossOvers);
    iResult += this->readPopKeyVal(pLine, GENETICS_MUTATION_RATE, &m_dMutationRate);
    
    m_iNumSetParams += iResult;

    if (m_iNumSetParams == 3) {
        iResult = 1 + init();
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// createInitialGenomes
//   only do this if the buffer has been already added to the controller
//
template<typename T>
int Genetics<T>::createInitialGenomes(int iNum, int iNumMutations) {
    int iResult = -1;
    if (m_bBufferAdded) {
        ulong *pG0 = GeneUtils::createRandomAlleles(m_iGenomeSize, iNumMutations, m_apWELL[omp_get_thread_num()]);
        for (int i = 0; i < iNum; i++) {
            // go to position for next genome
            ulong *pGenome = &(m_aGenome[i]); 
            GeneUtils::copyMutatedGenes(m_iGenomeSize, iNumMutations, pG0, pGenome);
        }
        delete[] pG0;
        iResult = 0;
    } else {
        printf("Genetics has not been initialized\n");
    }
    return iResult;
}


