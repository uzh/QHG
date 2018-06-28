#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
#include "Phenetics.h"

// needed for BinomialDistribution
#define EPS 1e-6

static const uint  BUFSIZE_READ_PHENOMES=8192;


#define MAX_INIT_NAME 32

// this number must changed if the parameters change
template<typename T>
int Phenetics<T>::NUM_PHENETIC_PARAMS = 5;


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Phenetics<T>::Phenetics(SPopulation<T> *pPop,  SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL)  
    : Action<T>(pPop, pCG),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(apWELL),
      m_iPhenomeSize(-1),
      m_pBDist(NULL),
      m_dMutationRate(-1),
      m_dMutationSigma(-1),
      m_dInitialSigma(0),
      m_bCreateNewPhenome(0),
      m_bMixAvg(1),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_iNumParents(2),
      m_bOwnWELL(false)   {

    m_fMixing = &Phenetics<T>::mix_avg;

    m_iNumThreads = omp_get_max_threads();


}


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Phenetics<T>::Phenetics(SPopulation<T> *pPop,  SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed)  
    : Action<T>(pPop, pCG),
      m_pAgentController(pAgentController),
      m_pWriteCopyController(NULL),
      m_apWELL(NULL),
      m_iPhenomeSize(-1),
      m_pBDist(NULL),
      m_dMutationRate(-1),
      m_dMutationSigma(-1),
      m_bCreateNewPhenome(0),
      m_iNumSetParams(0),
      m_bBufferAdded(false),
      m_pvDeadList(pvDeadList),
      m_iNumParents(2),
      m_bOwnWELL(true) {

  m_fMixing = &Phenetics<T>::mix_avg;
    m_iNumThreads = omp_get_max_threads();

    // we need to build our own WELLs    
    printf("[Phenetics::Phenetics] using %u as seed for WELLs\n", iSeed);
    buildWELLs(iSeed);
   
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Phenetics<T>::~Phenetics() {
    deleteAllocated();

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
template<typename T>
void Phenetics<T>::deleteAllocated() {

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
        m_pAgentController->removeBuffer(static_cast<LBBase *>(&m_aPhenome));
        m_bBufferAdded = false;
    }
}


//-----------------------------------------------------------------------------
// buildWELLs
//   build genetic's own WELL array
//  
template<typename T>
void Phenetics<T>::buildWELLs(uint iSeed) {

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
template<typename T>
int Phenetics<T>::init() {
    int iResult = -1;
    printf("init called\n");
    deleteAllocated();
    if ((m_iPhenomeSize > 0) && 
        /*(m_dMutationRate >= 0) && */(m_dMutationRate <= 1)) {
        

        // initialize the buffer ...
        printf("initializing m_aPhenome with (%d, %d)\n", m_pAgentController->getLayerSize(), m_iNumParents*m_iPhenomeSize);
        m_aPhenome.init(m_pAgentController->getLayerSize(), m_iNumParents*m_iPhenomeSize);
        
        // ... and add it to the AgentController
        iResult = m_pAgentController->addBuffer(static_cast<LBBase *>(&m_aPhenome));
        
        
        // layeredBuffer and controller for writing a copy of the actual agent array
        if (iResult == 0) {
            
            m_pWriteCopyController = new LBController;
            
            m_aWriteCopy.init( m_pAgentController->getLayerSize(), m_iNumParents*m_iPhenomeSize);
            m_pWriteCopyController->init( m_pAgentController->getLayerSize());
            iResult = m_pWriteCopyController->addBuffer(static_cast<LBBase *>(&m_aWriteCopy));
            m_pWriteCopyController->addLayer();
        }

        if (iResult == 0) {
            
            m_bBufferAdded = true;
            

            // bionomial distribution (to get number of mutations (maybe not needed )
            m_pBDist = new BinomialDist*[m_iNumThreads];
        
#pragma omp parallel
            {
                int iT = omp_get_thread_num();
                m_pBDist[iT] = BinomialDist::create(m_dMutationRate, m_iNumParents*m_iPhenomeSize, EPS);
                if (m_pBDist[iT] != NULL) {
                    
                                        
                    iResult = 0;
                } else {
                    printf("Couldn't create BinomialDistribution\n");
                }
            }
            
        } else {
            printf("Couldn't add buffer to controller\n");
        }

        if (m_bMixAvg) {
            m_fMixing = &Phenetics<T>::mix_avg;
        } else {
            m_fMixing = &Phenetics<T>::mix_sel;
        }
        
    } else {
        printf("Bad values for phenome size or mutation rate\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// makeOffspring
//   actual creation of babies
//   (called from SPopulation::performBirths()
//
template<typename T>
int  Phenetics<T>::makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex) {
    int iResult = 0;

    // we have to manipulate inside of a layer buf array
    // better to treat them as normal arrays
    // this is safe because a genome is alway inside  single layer
    phentype *pBabyPhenome  = &(m_aPhenome[iBabyIndex]);
    phentype *pPhenome1     = &(m_aPhenome[iMotherIndex]);
    phentype *pPhenome2     = &(m_aPhenome[iFatherIndex]);

    // clear baby genome
    memset(pBabyPhenome, 0, m_iNumParents*m_iPhenomeSize*sizeof(phentype));

    int iT = omp_get_thread_num();

    // average or parent1's phenome into baby's first strand
    (this->*m_fMixing)(pBabyPhenome, pPhenome1, m_apWELL[iT]);
    // average or parent2's phenome into baby's second strand
    (this->*m_fMixing)(pBabyPhenome+m_iPhenomeSize, pPhenome2, m_apWELL[iT]);


    // perhaps some mutations...
    if (m_dMutationRate > 0) {
        //        int iNumMutations = m_pBDist[iT]->getN(m_apWELL[iT]->wrandd());
        mutate(pBabyPhenome, m_dMutationSigma, m_apWELL[iT]);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// mix_avg
//  a sort of "crossing-over": take the average of a strand of each parent
//   
template<typename T>
void Phenetics<T>::mix_avg(phentype *pMixedPhenome, phentype *pPhenome, WELL512 *pWELL) {
    for (int i = 0; i < m_iPhenomeSize; i++) {
        pMixedPhenome[i] = (pPhenome[i] + pPhenome[i+m_iPhenomeSize])/2;
    }
}


//----------------------------------------------------------------------------
// mix_sel
//  a sort of "free recombination": select elements randomly from parents
//
template<typename T>
void Phenetics<T>::mix_sel(phentype *pMixedPhenome, phentype *pPhenome, WELL512 *pWELL) {
    for (int i = 0; i < m_iPhenomeSize; i++) {
        pMixedPhenome[i] = (pWELL->wrandd() <0.5) ? pPhenome[i] : pPhenome[i+m_iPhenomeSize];
    }
}


//----------------------------------------------------------------------------
// mutate
//   mutation by adding normally distributed nudge to all traits
//
template<typename T>
void Phenetics<T>::mutate(phentype *pPhenome, double dSigma, WELL512 *pWELL) {
    for (int i = 0; i < 2*m_iPhenomeSize; i++) {
        pPhenome[i] +=  pWELL->wgauss(dSigma);
    }
}


//----------------------------------------------------------------------------
// calcDist
//   ordinary euclidean distance
//
template<typename T>
double Phenetics<T>::calcDist(phentype *pP1, phentype *pP2) {
    double dS = 0;
    for (int i = 0; i < m_iNumParents*m_iPhenomeSize; i++) {
        dS += (pP1[i]-pP2[i])*(pP1[i]-pP2[i]);
    }
    return sqrt(dS);
}


//----------------------------------------------------------------------------
// calcDistAvg
//   ordinary euclidean distance betwen the averaged phenomes
//
template<typename T>
double Phenetics<T>::calcDistAvg(phentype *pP1, phentype *pP2) {
    double dS = 0;
    for (int i = 0; i < m_iPhenomeSize; i++) {
        double k = (pP1[i]+pP1[i+m_iPhenomeSize]-pP2[i]-pP2[i+m_iPhenomeSize])/2;
        dS += k*k;
    }
    return sqrt(dS);
}



//----------------------------------------------------------------------------
// writeAdditionalDataQDF
//  write additional data to the group
//  Here: create a new Dataset for the genome.
//  Write genomes sequentially from all cells.
//  This method does not change the agent- or genome array
//  Go through all cells, and use the BufLayers of m_aGenome as slabs 
//
template<typename T>
int Phenetics<T>::writeAdditionalDataQDF(hid_t hSpeciesGroup) {
    //    int iResult = 0;
    printf("[Phenetics<T>::writeAdditionalDataQDFSafe]\n");
    herr_t status=-1;
    int iTotalKill = 0;
    uint iNumWritten = 0;
    uint iWrittenLongs = 0;

    // amount of elements we have to save 
    hsize_t dims = this->m_pPop->getNumAgentsEffective()*m_iNumParents*m_iPhenomeSize; 
    //                    printf("  [%s] numagents total: %lld\n", it->first->getSpeciesName(), dims);

    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
    if (hDataSpace > 0) {
        
        // Create the dataset
        hid_t hDataSet = H5Dcreate2(hSpeciesGroup, PHENOME_DATASET_NAME, hdf_phentype, hDataSpace, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        
        if (hDataSet > 0) {
            hsize_t dimsm = m_pAgentController[0].getLayerSize()*m_iNumParents*m_iPhenomeSize;
            hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 

            hsize_t iOffset = 0;
            hsize_t iCount  = 0;  
            status = 0;

            uint iD = 0;
            int iLayerSize = m_pWriteCopyController->getLayerSize();
            // does the cell contain agents?
            if (m_pAgentController->getNumUsed() > 0) {
                // loop over BufLayers
                for (uint j = 0; j < m_aPhenome.getNumUsedLayers(); j++) {
                    // avoid empty layers
                    if (m_pAgentController->getNumUsed(j) > 0) {
                    
                        // write agents of layer j as hyperslab
                        const phentype* pSlab0 = m_aPhenome.getLayer(j);
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
                    
                        const phentype *pSlab = m_aWriteCopy.getLayer(0);
                        // printf("layer %u: %d genomes\n", j, m_pWriteCopyController->getNumUsed(0));
                        iNumWritten +=  m_pWriteCopyController->getNumUsed(0);
                        // adapt memspace if size of slab must change
                        iCount =  m_pWriteCopyController->getNumUsed(0)*m_iNumParents*m_iPhenomeSize;
                        if (iCount != dimsm) {
                            qdf_closeDataSpace(hMemSpace); 
                            dimsm = iCount;
                            hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
                        }
                        
                        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                                     &iOffset, NULL, &iCount, NULL);
                        
                        status = H5Dwrite(hDataSet, hdf_phentype, hMemSpace,
                                          hDataSpace, H5P_DEFAULT, pSlab);
                        iOffset += iCount;
                        iWrittenLongs += iCount;
                    } else {
                        printf("[Phenetics<T>::writeAdditionalDataQDFSafe] ignored layer %d because it's empty\n", j);
                    }
                }
                
            }
            qdf_closeDataSpace(hMemSpace); 
 
            qdf_closeDataSet(hDataSet);
        }
        qdf_closeDataSpace(hDataSpace);
    }

    printf("[Phenetics<T>::writeAdditionalDataQDFSafe] written %u items (%u longs), killed %d\n", iNumWritten, iWrittenLongs, iTotalKill); fflush(stdout);
    printf("[Phenetics<T>::writeAdditionalDataQDFSafe] end with status %d\n", status); fflush(stdout);

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
int Phenetics<T>::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    int iResult = -1;

    
    if (qdf_exists(hSpeciesGroup, PHENOME_DATASET_NAME)) {
        iResult = 0;
        
        // the buffer must hold a multiple of the array length
        uint iReadBufSize = BUFSIZE_READ_PHENOMES*m_iNumParents*m_iPhenomeSize;
        float *aBuf = new float[iReadBufSize];

        // open the data set
        hid_t hDataSet = H5Dopen2(hSpeciesGroup, PHENOME_DATASET_NAME, H5P_DEFAULT);
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
            status = H5Dread(hDataSet, hdf_phentype, hMemSpace,
                             hDataSpace, H5P_DEFAULT, aBuf);
        
        
            if (status >= 0) {
                // distribute the data 
                int iArrOffset = 0;
                int iNumThisPass = iCount/(m_iNumParents*m_iPhenomeSize);
                int iNumToCopy = iNumThisPass;
                //@@                printf("starting inner loop: iTotalToDo %d, iNumThisPass %d\n", iTotalToDo, iNumThisPass);
                // while we still have genomes to fill and data from the slab not used up
                while ((iTotalToDo > 0) && (iNumThisPass > 0)) {
                    
                    // creation of agents has already created space in the genome buffer
                    // copy from the current point in the buffer to the current index
                    phentype *pCur = aBuf+iArrOffset;
                    // the positions in the LayerBuf are already have the correct L2List  links
                    // because reserveSpace2() has been called by readAgentDataQDF()
                    m_aPhenome.copyBlock(iFirstIndex, (phentype *) pCur, iNumToCopy);

                    // update counters and indexes
                    iFirstIndex += iNumToCopy;
                    iTotalToDo  -= iNumToCopy*m_iNumParents*m_iPhenomeSize;
                    iArrOffset  += iNumToCopy*m_iNumParents*m_iPhenomeSize;
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
        printf("WARNING: no dataset [%s] found\n", PHENOME_DATASET_NAME);
    }
    return iResult;

}


//-----------------------------------------------------------------------------
// writeSpeciesDataQDF
//
//  tries to write the attributes
//    ATTR_PHENETICS_PHENOME_SIZE, 
//    ATTR_PHENETICS_MUTATION_RATE
//    ATTR_PHENETICS_MUTATION_SIGMA
//    ATTR_PHENETICS_INITIAL_SIGMA
//    ATTR_PHENETICS_CREATE_NEW_PHENOME
//    ATTR_PHENETICS_MIX_AVG
//
template<typename T>
int Phenetics<T>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_PHENOME_SIZE,  1, (int *) &m_iPhenomeSize);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_RATE,  1, &m_dMutationRate);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_SIGMA,  1, &m_dMutationSigma);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_INITIAL_SIGMA,  1,  &m_dInitialSigma);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_CREATE_NEW_PHENOME, 1, &m_bCreateNewPhenome);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hSpeciesGroup, ATTR_PHENETICS_MIX_AVG,  1,  &m_bMixAvg);
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readSpeciesDataQDF
//
//  tries to read the attributes
//    ATTR_PHENETICS_PHENOME_SIZE, 
//    ATTR_PHENETICS_MUTATION_RATE
//    ATTR_PHENETICS_MUTATION_SIGMA
//    ATTR_PHENETICS_INITIAL_SIGMA
//    ATTR_PHENETICS_CREATE_NEW_PHENOME
//    ATTR_PHENETICS_MIX_AVG
//
template<typename T>
int Phenetics<T>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_PHENOME_SIZE,  1, (int *) &m_iPhenomeSize);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_PHENOME_SIZE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_RATE,  1, &m_dMutationRate);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_MUTATION_RATE);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_MUTATION_SIGMA,  1, &m_dMutationSigma);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_MUTATION_SIGMA);
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_INITIAL_SIGMA, 1,  &m_dInitialSigma);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_INITIAL_SIGMA);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_CREATE_NEW_PHENOME, 1, &m_bCreateNewPhenome);
        if (iResult != 0) {
            LOG_WARNING("[Phenetics] couldn't read attribute [%s]; setting value to false", ATTR_PHENETICS_CREATE_NEW_PHENOME);
            m_bCreateNewPhenome = 0;
            iResult = 0;
        }
    }
    
    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_PHENETICS_MIX_AVG, 1, &m_bMixAvg);
        if (iResult != 0) {
            LOG_ERROR("[Phenetics] couldn't read attribute [%s]", ATTR_PHENETICS_MIX_AVG);
        }
    }


    
    printf("[Phenetics] ExtractParamsQDF:res %d\n", iResult);
    
    if (iResult == 0) {
        // we must call init() before callnig readAdditionalDataQDF()
        iResult = init();
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int Phenetics<T>::tryReadParamLine(char *pLine) {
    
    int iResult = 0;
    
    iResult += this->readPopKeyVal(pLine, ATTR_PHENETICS_PHENOME_SIZE,       &m_iPhenomeSize);
    iResult += this->readPopKeyVal(pLine, ATTR_PHENETICS_MUTATION_RATE,      &m_dMutationRate);
    iResult += this->readPopKeyVal(pLine, ATTR_PHENETICS_MUTATION_SIGMA,     &m_dMutationSigma);
    iResult += this->readPopKeyVal(pLine, ATTR_PHENETICS_INITIAL_SIGMA,      &m_dInitialSigma);
    iResult += this->readPopKeyVal(pLine, ATTR_PHENETICS_CREATE_NEW_PHENOME, &m_bCreateNewPhenome);
    iResult += this->readPopKeyVal(pLine, ATTR_PHENETICS_MIX_AVG,            &m_bMixAvg);
   
    
    if (iResult != 0) {
        m_iNumSetParams += iResult;
        
        if (!m_bBufferAdded && (m_iNumSetParams >= NUM_PHENETIC_PARAMS)) {
            // we must call init() before calling readAdditionalDataQDF()
            iResult = 1 + init(); // init:-1 => res: 0; init:0 => res 1 
        }
    }
   
    return iResult;
}


//-----------------------------------------------------------------------------
// createInitialPhenomes
//   only do this if the buffer has been already added to the controller
//
template<typename T>
int Phenetics<T>::createInitialPhenomes(int iNumPhenomes) {
    int iResult = -1;
    if (m_bBufferAdded) {
        if (m_bCreateNewPhenome != 0) {
            iResult = 0;
            m_bCreateNewPhenome = false;
            printf("Creating %d mutated variants with sigma %f\n", iNumPhenomes, m_dInitialSigma);
            phentype *pG0 = new phentype[m_iNumParents*m_iPhenomeSize];
            memset(pG0, 0,  m_iNumParents*m_iPhenomeSize*sizeof(phentype));
            for (int i = 0; i < iNumPhenomes; i++) {
                // go to position for next phenome
                phentype *pPhenome = &(m_aPhenome[i]); 
                memcpy(pPhenome, pG0, m_iNumParents*m_iPhenomeSize*sizeof(phentype));
                mutate(pPhenome, m_dInitialSigma, m_apWELL[omp_get_thread_num()]);
            }
            delete[] pG0;
        }

    } else {
        printf("Phenetics has not been initialized\n");
    }   
    return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void Phenetics<T>::showAttributes() {
    printf("  %s\n", ATTR_PHENETICS_PHENOME_SIZE);
    printf("  %s\n", ATTR_PHENETICS_MUTATION_RATE);
    printf("  %s\n", ATTR_PHENETICS_MUTATION_SIGMA);
    printf("  %s\n", ATTR_PHENETICS_INITIAL_SIGMA);
    printf("  %s\n", ATTR_PHENETICS_CREATE_NEW_PHENOME);
    printf("  %s\n", ATTR_PHENETICS_MIX_AVG);

}

