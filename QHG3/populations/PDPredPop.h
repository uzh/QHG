#ifndef __PDPREDPOP_H__
#define __PDPREDPOP_H__


#include <hdf5.h>

#include "SPopulation.h"
#include "IndexCollector.h"
#include "WeightedMoveRand.h"
//#include "SingleEvaluator.h"
#include "ShareEvaluator.h"
#include "PDHunting.h"
#include "Birther.h"
#include "MassManager.h"
#include "MassInterface.h"

struct PDPredAgent : Agent {
    float m_fMass;
    float m_fBabyMass;
    int   m_iPreyIndex;
};

class PDPredPop : public SPopulation<PDPredAgent>, public MassInterface {
 public:
    PDPredPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~PDPredPop();

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
    IndexCollector<PDPredAgent>   *m_pIC;
    WeightedMoveRand<PDPredAgent> *m_pWM;
    // replaced by shareeval    SingleEvaluator<PDPredAgent> *m_pSE;
    ShareEvaluator<PDPredAgent>   *m_pSE;
    MassManager<PDPredAgent>      *m_pMM;
    Birther<PDPredAgent>          *m_pBB;
    PDHunting<PDPredAgent>        *m_pPDHU;

    double *m_adEnvWeights;
    uint *m_aiAgentCounts;
};

#endif



