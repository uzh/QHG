#ifndef __OOANAVGEN2BITRANDDISTRPOP_H__
#define __OOANAVGEN2BITRANDDISTRPOP_H__

#include "GeneUtils.h"
#include "SPopulation.h"
#include "WeightedMove.h"
//#include "ConfinedMove.h"
#include "SingleEvaluator.h"
#include "MultiEvaluator.h"
#include "VerhulstVarK.h"
#include "RandDistrPair.h"
#include "GetOld.h"
#include "OldAgeDeath.h"
#include "Fertility.h"
#include "NPPCapacity.h"
#include "Genetics.h"
#include "Navigate.h"


struct OoANavGen2bitRandDistrAgent : Agent {

    float m_fAge;
    float m_fLastBirth;
    int m_iMateIndex;
    int m_iNumBabies;
};

class OoANavGen2bitRandDistrPop : public SPopulation<OoANavGen2bitRandDistrAgent> {
public:
    OoANavGen2bitRandDistrPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState, uint *aiSeeds);
    ~OoANavGen2bitRandDistrPop();

    int preLoop();

    int setParams(const char *pParams);
    int makePopSpecificOffspring(int iAgent, int iMother, int iFather);
    int writeAdditionalDataQDF(hid_t hSpeciesGroup);
    int readAdditionalDataQDF(hid_t hSpeciesGroup);
    int addPopSpecificAgentData(int iAgentIndex, char **ppData);
    void addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType);
    virtual int updateEvent(int EventID, char *pData, float fT);
    virtual void flushEvents(float fT);

 protected:
    WeightedMove<OoANavGen2bitRandDistrAgent> *m_pWM;
    //    ConfinedMove<OoANavGen2bitRandDistrAgent> *m_pCM;
    MultiEvaluator<OoANavGen2bitRandDistrAgent> *m_pME;
    VerhulstVarK<OoANavGen2bitRandDistrAgent> *m_pVerVarK;
    RandDistrPair<OoANavGen2bitRandDistrAgent,GeneUtils> *m_pPair;
    GetOld<OoANavGen2bitRandDistrAgent> *m_pGO;
    OldAgeDeath<OoANavGen2bitRandDistrAgent> *m_pOAD;
    Fertility<OoANavGen2bitRandDistrAgent> *m_pFert;
    NPPCapacity<OoANavGen2bitRandDistrAgent> *m_pNPPCap;
    Genetics<OoANavGen2bitRandDistrAgent,GeneUtils> *m_pGenetics;
    Navigate<OoANavGen2bitRandDistrAgent> *m_pNavigate;

    double *m_adEnvWeights;
    double *m_adCapacities;

    bool m_bCreateGenomes;
    bool m_bUpdateNeeded;

};


#endif
