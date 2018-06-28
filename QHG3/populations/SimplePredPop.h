#ifndef __SIMPLEPREDPOP_H__
#define __SIMPLEPREDPOP_H__


#include <hdf5.h>

#include "SPopulation.h"
#include "WeightedMoveRand.h"
//#include "SingleEvaluator.h"
#include "ShareEvaluator.h"
#include "Hunting.h"
#include "Birther.h"
#include "MassManager.h"
#include "MassInterface.h"

struct SimplePredAgent : Agent {
    float m_fMass;
    float m_fBabyMass;
    int   m_iPreyIndex;
};

class SimplePredPop : public SPopulation<SimplePredAgent>, public MassInterface {
 public:
    SimplePredPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~SimplePredPop();

    int preLoop();
    int postLoop();
    virtual int initializeStep(float fTime);
    
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);

    //massinterface
    virtual double setMass(int iAgentIndex, double fMass);
    virtual double addMass(int iAgentIndex, double fMass);
    virtual double getMass(int iAgentIndex);
    virtual double getTotalMass(int iCellIndex);
    virtual double *getTotalMassArray();
    virtual double setSecondaryMass(int iAgentIndex, double dMass);

    double **m_afMassArray;
 protected:
    WeightedMoveRand<SimplePredAgent> *m_pWM;
    // replaced by shareeval    SingleEvaluator<SimplePredAgent> *m_pSE;
    ShareEvaluator<SimplePredAgent> *m_pSE;
    MassManager<SimplePredAgent> *m_pMM;
    Birther<SimplePredAgent> *m_pBB;
    Hunting<SimplePredAgent> *m_pHU;

    double *m_adEnvWeights;
    uint *m_aiAgentCounts;
};






#endif
