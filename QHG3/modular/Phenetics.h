#ifndef __PHENETICS_H__
#define __PHENETICS_H__

#include <hdf5.h>

#include <vector>

#include "Action.h"
#include "LayerArrBuf.h"
#include "WELL512.h"
#include "GenomeCreator.h"

class LBController;
class BinomialDist;

typedef float phentype;
const   hid_t hdf_phentype = H5T_NATIVE_FLOAT;
 
#define ATTR_PHENETICS_NAME               "Phenetics"
#define ATTR_PHENETICS_PHENOME_SIZE       "Phenetics_phenome_size"
#define ATTR_PHENETICS_MUTATION_RATE      "Phenetics_mutation_rate"
#define ATTR_PHENETICS_MUTATION_SIGMA     "Phenetics_mutation_sigma"
#define ATTR_PHENETICS_CREATE_NEW_PHENOME "Phenetics_create_new_phenome"
#define ATTR_PHENETICS_INITIAL_SIGMA      "Phenetics_initial_sigma"
#define ATTR_PHENETICS_MIX_AVG            "Phenetics_mix_avg"

#define PHENOME_DATASET_NAME    "Phenome"



// it is not very nice to derive Genetics from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T>
class Phenetics : public Action<T> {
public:

    typedef  void (Phenetics::*mixer)(phentype *pMixedPhenome, phentype *pPhenome, WELL512 *pWELL);

    Phenetics(SPopulation<T> *pPop, SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, WELL512** apWELL);
    Phenetics(SPopulation<T> *pPop, SCellGrid *pCG, LBController *pAgentController, std::vector<int> *pvDeadList, uint iSeed);
    virtual ~Phenetics();

    virtual int makeOffspring(int iBabyIndex, int iMotherIndex, int iFatherIndex);

    virtual int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    virtual int readAdditionalDataQDF(hid_t hSpeciesGroup);

    int createInitialPhenomes(int iNumPhenomes);
    bool isReady() { return m_bBufferAdded;};

    // from Action
    virtual int initialize(float fTime) { return 0; };

    // operator() does nothing
    virtual int operator()(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };

    virtual int extractParamsQDF(hid_t hSpeciesGroup);
    virtual int writeParamsQDF(hid_t hSpeciesGroup);
    virtual int tryReadParamLine(char *pLine);

    int getPhenomeSize() { return m_iPhenomeSize;};
    LayerArrBuf<phentype> *getPhenome() { return &m_aPhenome;};
    phentype *getPhenome(uint i) { return &m_aPhenome[i];};

    void showAttributes();
protected:
    static int NUM_PHENETIC_PARAMS;
protected:
    int init();
    void deleteAllocated();
    void buildWELLs(uint iSeed);

    // "genetic" utilities
    void   mix_avg(phentype *pMixedPhenome, phentype *pPhenome, WELL512 *pWELL);
    void   mix_sel(phentype *pMixedPhenome, phentype *pPhenome, WELL512 *pWELL);
    
    void   mutate(phentype *pPhenome, double dSigma, WELL512 *pWELL);
    double calcDist(phentype *pP1, phentype *pP2); 
    double calcDistAvg(phentype *pP1, phentype *pP2); 

    LBController *m_pAgentController;
    LBController *m_pWriteCopyController;

    WELL512** m_apWELL;
    int           m_iPhenomeSize;
    // the genomes must be controlled by ther agent controller m_AgentController
    LayerArrBuf<phentype> m_aPhenome; // 1 per cell; the genomes of the agents follow each other; first allele 0 then allele 1

    // auxiliary layerbuf for writing
    LayerArrBuf<phentype> m_aWriteCopy; 

    BinomialDist **m_pBDist;

    double  m_dMutationRate;
    double  m_dMutationSigma;
    double  m_dInitialSigma;
    char    m_bCreateNewPhenome;
    char    m_bMixAvg;

    int     m_iNumSetParams;
    int     m_iNumThreads;
    bool    m_bBufferAdded;
    std::vector<int> *m_pvDeadList;

    uint m_iNumParents;
    bool m_bOwnWELL;

    mixer  m_fMixing;
    //    void (Phenetics::*m_fMixing)(phentype *pMixedPhenome, phentype *pPhenome, WELL512 *pWELL);
};


#endif
