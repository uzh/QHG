#include <omp.h>
#include <hdf5.h>

#include "GeneUtils.h"

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "Verhulst.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "Genetics.cpp"
#include "GenomeCreator.cpp"
#include "GeneticMoverPop.h"


//----------------------------------------------------------------------------
// constructor
//
GeneticMoverPop::GeneticMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<GeneticMoverAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
      m_pGenetics(NULL), m_pAltPrefName(NULL), m_bCreateGenomes(true) {
  
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

    m_pAltPrefName = new char[64];
    strcpy(m_pAltPrefName, "AltPreference");
    m_pAE = new SingleEvaluator<GeneticMoverAgent>(this, m_pCG, m_adEnvWeights, (double*)m_pCG->m_pGeography->m_adAltitude, m_pAltPrefName);
    

    m_pWM = new WeightedMove<GeneticMoverAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    GeneticMoverAgent aama;
    m_pVer = new Verhulst<GeneticMoverAgent>(this, m_pCG, m_apWELL, (int)qoffsetof(aama, m_iMateIndex));

    m_pPair = new RandomPair<GeneticMoverAgent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<GeneticMoverAgent>(this, m_pCG);

    m_pGenetics = new Genetics<GeneticMoverAgent,GeneUtils>(this, m_pCG, m_pAgentController, &m_vMergedDeadList, m_apWELL);

    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(ATTR_SINGLEEVAL_NAME, m_pAE);
    m_prio.addAction(ATTR_VERHULST_NAME, m_pVer);
    m_prio.addAction(ATTR_RANDPAIR_NAME, m_pPair);
    m_prio.addAction(ATTR_GETOLD_NAME, m_pGO);
    m_prio.addAction(ATTR_GENETICS_NAME, m_pGenetics);
    
}

///----------------------------------------------------------------------------
// destructor
//
GeneticMoverPop::~GeneticMoverPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pAE != NULL) {
        delete m_pAE;
    }
    if (m_pVer != NULL) {
        delete m_pVer;
    }
    if (m_pPair != NULL) {
        delete m_pPair;
    }
    if (m_pGO != NULL) {
        delete m_pGO;
    }
    if (m_pAltPrefName != NULL) {
        delete[] m_pAltPrefName;
    }
    if (m_pGenetics != NULL) {
        delete m_pGenetics;
    }

    
}


///----------------------------------------------------------------------------
// setParams
//
int GeneticMoverPop::setParams(const char *pParams) {
    int iResult = 0;
  
    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int GeneticMoverPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = 0;
   
    if (m_bCreateGenomes) {
        // we assume the agents start at position 0

        int iN = getNumAgentsTotal();
        
        iResult = m_pGenetics->createInitialGenomes(iN);
    } else {
        if (m_pGenetics->isReady()) {
            iResult = 0;
        }
    }

    return iResult;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int GeneticMoverPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    // here we call the Genetics method to create a genome for the baby

    int iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int GeneticMoverPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    return m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int GeneticMoverPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void GeneticMoverPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    GeneticMoverAgent aama;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(aama, m_fAge), H5T_NATIVE_FLOAT);

}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int GeneticMoverPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {
    // we read the genome from a QDF: no need to create it
    m_bCreateGenomes = false;
    return m_pGenetics->readAdditionalDataQDF(hSpeciesGroup);
}


