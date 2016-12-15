#ifndef __PDPREYPOP_H__
#define __PDPREYPOP_H__


#include <hdf5.h>


#include "SPopulation.h"
#include "IndexCollector.h"
#include "RandomMove.h"
#include "Verhulst.h"
#include "MassInterface.h"
#include "MassManager.h"


struct PDPreyAgent : Agent {
    float m_fMass;
};

class PDPreyPop : public SPopulation<PDPreyAgent>, public MassInterface {
 public:
    PDPreyPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState);
    ~PDPreyPop();

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

    IndexCollector<PDPreyAgent> *m_pIC;
    RandomMove<PDPreyAgent>     *m_pRM;
    MassManager<PDPreyAgent>    *m_pMM;
    Verhulst<PDPreyAgent>       *m_pVer;

};






#endif

