#ifndef __GENETICS_H__
#define __GENETICS_H__

#include <hdf5.h>

#include "Action.h"
#include "LayerArrBuf.h"
#include "WELL512.h"

class LBController;
class BinomialDist;

#define GENETICS_NAME          "Genetics"
#define GENETICS_GENOME_SIZE   "Genetics_genome_size"
#define GENETICS_NUM_CROSSOVER "Genetics_num_crossover"
#define GENETICS_MUTATION_RATE "Genetics_mutation_rate"
#define GENOME_DATASET_NAME    "Genome"

// it is not very nice to derive Genetics from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T>
class Genetics : public Action<T> {
public:

    Genetics(SPopulation<T> *pPop, SCellGrid *pCG, LBController *pAgentController, WELL512** apWELL);
    virtual ~Genetics();

    virtual int makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex);

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

    int createInitialGenomes(int iNum, int iNumMutations);
    bool isReady() { return m_bBufferAdded;};

    // from Action
    virtual int initialize(float fTime) { return 0; };
    // operator() does nothing
    virtual int operator()(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };

    virtual int extractParamsQDF(hid_t hSpeciesGroup);
    virtual int writeParamsQDF(hid_t hSpeciesGroup);
    virtual int tryReadParamLine(char *pLine);

    
protected:
    int init();
    void deleteAllocated();
 
    LBController *m_pAgentController;
    WELL512** m_apWELL;
    int           m_iGenomeSize;
    // the genomes must be controlled by ther agent controller m_AgentController
    LayerArrBuf<ulong> m_aGenome; // 1 per cell; the genomes of the agents follow each other; first allele 0 then allele 1

    BinomialDist **m_pBDist;

    int    m_iNumCrossOvers;
    double m_dMutationRate;
    ulong **m_pTempGenome1;
    ulong **m_pTempGenome2;
    int    m_iNumBlocks;

    int m_iNumSetParams;
	int m_iNumThreads;
    bool   m_bBufferAdded;
};


#endif
