#ifndef __GENETICS_H__
#define __GENETICS_H__

#include <hdf5.h>

#include <vector>

#include "Action.h"
#include "LayerArrBuf.h"
#include "WELL512.h"
#include "GenomeCreator.h"

class LBController;
class BinomialDist;

#define ATTR_GENETICS_NAME              "Genetics"
#define ATTR_GENETICS_GENOME_SIZE       "Genetics_genome_size"
#define ATTR_GENETICS_NUM_CROSSOVER     "Genetics_num_crossover"
#define ATTR_GENETICS_MUTATION_RATE     "Genetics_mutation_rate"
#define ATTR_GENETICS_INITIAL_MUTS      "Genetics_initial_muts"
#define ATTR_GENETICS_CREATE_NEW_GENOME "Genetics_create_new_genome"
#define ATTR_GENETICS_BITS_PER_NUC      "Genetics_bits_per_nuc"

#define GENOME_DATASET_NAME    "Genome"
        

// it is not very nice to derive Genetics from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T, class U>
class Genetics : public Action<T> {
public:

    Genetics(SPopulation<T> *pPop, SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL);
    Genetics(SPopulation<T> *pPop, SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed);
    virtual ~Genetics();

    virtual int makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex);

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int writeAdditionalDataQDFOld(hid_t hSpeciesGroup);
    virtual int writeAdditionalDataQDFSafe(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

    int createInitialGenomes(int iNumGenomes);
    bool isReady() { return m_bBufferAdded;};

    // from Action
    virtual int initialize(float fTime) { return 0; };

    // operator() does nothing
    virtual int operator()(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };

    virtual int extractParamsQDF(hid_t hSpeciesGroup);
    virtual int writeParamsQDF(hid_t hSpeciesGroup);
    virtual int tryReadParamLine(char *pLine);

    int getGenomeSize() { return m_iGenomeSize;};
    LayerArrBuf<ulong> *getGenome() { return &m_aGenome;};
    ulong *getGenome(uint i) { return &m_aGenome[i];};
    int getNumBlocks() { return m_iNumBlocks; };

    void showAttributes();
protected:
    static int NUM_GENETIC_PARAMS;
protected:
    int init();
    void deleteAllocated();
    void buildWELLs(uint iSeed);

    LBController *m_pAgentController;
    LBController *m_pWriteCopyController;

    WELL512** m_apWELL;
    int           m_iGenomeSize;
    // the genomes must be controlled by ther agent controller m_AgentController
    LayerArrBuf<ulong> m_aGenome; // 1 per cell; the genomes of the agents follow each other; first allele 0 then allele 1

    // auxiliary layerbuf for writing
    LayerArrBuf<ulong> m_aWriteCopy; 

    BinomialDist **m_pBDist;

    int     m_iNumCrossOvers;
    double  m_dMutationRate;
    ulong **m_pTempGenome1;
    ulong **m_pTempGenome2;
    int     m_iNumBlocks;

    char    m_bCreateNewGenome;
    uint    m_iBitsPerNuc;

    int     m_iNumSetParams;
    int     m_iNumThreads;
    bool    m_bBufferAdded;
    std::vector<int> *m_pvDeadList;

    GenomeCreator<U> *m_pGenomeCreator;
    uint m_iNumParents;
    bool m_bOwnWELL;
};


#endif
