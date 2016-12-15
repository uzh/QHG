#ifndef __PDALTPREYPOP_H__
#define __PDALTPREYPOP_H__

#include <hdf5.h>


#include "SPopulation.h"
#include "IndexCollector.h"
#include "SingleEvaluator.h"
#include "WeightedMove.h"
#include "Verhulst.h"
#include "MassInterface.h"
#include "MassManager.h"


struct PDAltPreyAgent : Agent {
    float m_fMass;
};

class PDAltPreyPop : public SPopulation<PDAltPreyAgent>, public MassInterface {
 public:
    PDAltPreyPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~PDAltPreyPop();

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
    virtual double setSecondaryMass(int iAgentIndex, double fMass);


    double **m_afMassArray;
    double  *m_adEnvWeights;
 protected:

    IndexCollector<PDAltPreyAgent>   *m_pIC;
    SingleEvaluator<PDAltPreyAgent>  *m_pSE;
    WeightedMove<PDAltPreyAgent> *m_pWM;
    MassManager<PDAltPreyAgent>      *m_pMM;
    Verhulst<PDAltPreyAgent>         *m_pVer;

};






#endif
