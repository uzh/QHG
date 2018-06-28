#ifndef __PDALTPREDPOP_H__
#define __PDALTPREDPOP_H__


#include <hdf5.h>

#include "SPopulation.h"
#include "IndexCollector.h"
#include "WeightedMoveRand.h"
#include "MultiEvaluator.h"
#include "PDHunting.h"
#include "Birther.h"
#include "MassManager.h"
#include "MassInterface.h"

struct PDAltPredAgent : Agent {
    float m_fMass;
    float m_fBabyMass;
    int   m_iPreyIndex;
};

class PDAltPredPop : public SPopulation<PDAltPredAgent>, public MassInterface {
public:
    PDAltPredPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~PDAltPredPop();

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
    IndexCollector<PDAltPredAgent>    *m_pIC;
    WeightedMoveRand<PDAltPredAgent>  *m_pWM;
    MultiEvaluator<PDAltPredAgent>    *m_pME;
    MassManager<PDAltPredAgent>       *m_pMM;
    Birther<PDAltPredAgent>           *m_pBB;
    PDHunting<PDAltPredAgent>         *m_pPDHU;

    double *m_adEnvWeights;
    uint *m_aiAgentCounts;
};

#endif


