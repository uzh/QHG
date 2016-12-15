#ifndef __SIMPLEPREYPOP_H__
#define __SIMPLEPREYPOP_H__


#include <hdf5.h>


#include "SPopulation.h"
#include "RandomMove.h"
#include "Verhulst.h"
#include "MassInterface.h"
#include "MassManager.h"

struct SimplePreyAgent : Agent {
    float m_fMass;
};

class SimplePreyPop : public SPopulation<SimplePreyAgent>, public MassInterface {
 public:
    SimplePreyPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~SimplePreyPop();

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
 protected:
    RandomMove<SimplePreyAgent> *m_pRM;
    MassManager<SimplePreyAgent> *m_pMM;
    Verhulst<SimplePreyAgent> *m_pVer;
};






#endif

