#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "Verhulst.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "Anc4AltMoverPop.h"


//----------------------------------------------------------------------------
// constructor
//
Anc4AltMoverPop::Anc4AltMoverPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<Anc4AltMoverAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
      m_pAncBox(NULL), m_pAltPrefName(NULL), m_pOutDir(NULL), m_pPrefix(NULL) {
  
    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

    m_pAltPrefName = new char[64];
    strcpy(m_pAltPrefName, "AltPreference");
    m_pAE = new SingleEvaluator<Anc4AltMoverAgent>(this, m_pCG, m_adEnvWeights, (double*)m_pCG->m_pGeography->m_adAltitude, m_pAltPrefName);
    

    m_pWM = new WeightedMove<Anc4AltMoverAgent>(this, m_pCG, m_apWELL, m_adEnvWeights);

    Anc4AltMoverAgent aama;
    m_pVer = new Verhulst<Anc4AltMoverAgent>(this, m_pCG, m_apWELL, (int)qoffsetof(aama, m_iMateIndex));

    m_pPair = new RandomPair<Anc4AltMoverAgent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<Anc4AltMoverAgent>(this, m_pCG);

    m_prio.addAction(ATTR_WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(ATTR_SINGLEEVAL_NAME, m_pAE);
    m_prio.addAction(ATTR_VERHULST_NAME, m_pVer);
    m_prio.addAction(ATTR_RANDPAIR_NAME, m_pPair);
    m_prio.addAction(ATTR_GETOLD_NAME, m_pGO);
    
}

///----------------------------------------------------------------------------
// destructor
//
Anc4AltMoverPop::~Anc4AltMoverPop() {

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
    if (m_pAncBox != NULL) {
        delete m_pAncBox;
    }
    if (m_pAltPrefName != NULL) {
        delete[] m_pAltPrefName;
    }
    if (m_pOutDir != NULL) {
        delete[] m_pOutDir;
    }
    if (m_pPrefix != NULL) {
        delete[] m_pPrefix;
    }
    
}

///----------------------------------------------------------------------------
// setParams
//
int Anc4AltMoverPop::setParams(const char *pParams) {
    int iResult = 0;
    
    char *sParams = new char[strlen(pParams)+1];
    strcpy(sParams, pParams);
    char *pPos;
    char *p = strtok_r(sParams, "+", &pPos);
    while ((iResult == 0) && (p != NULL)) {
        char *p1 = strchr(p, '=');
        if (p1 != NULL) {
            *p1 = '\0'; 
            p1++;
            if (strcasecmp(p, "outdir") == 0) {
                if (m_pOutDir != NULL) {
                    delete[] m_pOutDir;
                }
                m_pOutDir = new char[strlen(p1) + 1];
                strcpy(m_pOutDir, p1);
            } else if (strcasecmp(p, "prefix") == 0) {
                if (m_pPrefix != NULL) {
                    delete[] m_pPrefix;
                }
                m_pPrefix = new char[strlen(p1) + 1];
                strcpy(m_pPrefix, p1);
            } else {
                iResult = -1;
            }
        } else {
            iResult = -1;
        }
        p = strtok_r(NULL, "+", &pPos);
    }
    printf("Anc4AltMover: Have outdir [%s] and prefix[%s]\n", m_pOutDir, m_pPrefix);
    delete[] sParams;
    return iResult;
}


///----------------------------------------------------------------------------
// preLoop
//
int Anc4AltMoverPop::preLoop() {
    // set output directory and prefix if not already set
    if (m_pOutDir == NULL) {
        m_pOutDir = new char[2];
        strcpy(m_pOutDir, ".");
    }
    if (m_pPrefix == NULL) {
        m_pPrefix = new char[strlen(m_sSpeciesName) + 1];
        strcpy(m_pPrefix, m_sSpeciesName);
    }

    // here we add the initial agents    
    // to the ancestor box
//    sprintf(m_sAncOutFileName,"pop_%d.anc",m_iSpeciesID);
    m_pAncBox = AncestorBox4::createInstance(m_aAgents.getLayerSize());

    int iFirstAgent = getFirstAgentIndex();
    int iLastAgent = getLastAgentIndex();

    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        // code gender in sign of dad id
        m_pAncBox->addBaby(m_aAgents[iA].m_ulID, 0, (m_aAgents[iA].m_iGender != 0)?-1:0, m_fCurTime, m_aAgents[iA].m_ulCellID);
    }

    return 0;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int Anc4AltMoverPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    // here we record parent info into the ancestor box

    Anc4AltMoverAgent* pBaby = &m_aAgents[iAgent];
    Anc4AltMoverAgent* pMother = &m_aAgents[iMother];
    Anc4AltMoverAgent* pFather = &m_aAgents[iFather];
        
    pBaby->m_fAge = 0.0;
    m_pAncBox->addBaby(pBaby->m_ulID, pMother->m_ulID, pFather->m_ulID*((pBaby->m_iGender == 0)?1:-1), m_fCurTime, pBaby->m_ulCellID);

    return 0;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int Anc4AltMoverPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    // here we use a trick:
    // we are not writing to the QDF but to a separate file
    // but it is convenient to do it here because 
    // this function is called just at the right moment 
    printf("******boinky**********\n");
    char *ancfilename = new char[strlen(m_pOutDir)+strlen(m_pPrefix)+16];
    sprintf(ancfilename, "%s/%s_%06d.anc4", m_pOutDir, m_pPrefix, (int)(m_fCurTime+1));
    m_pAncBox->setOutputFile(ancfilename, false);
    delete[] ancfilename;
    return m_pAncBox->writeData();
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int Anc4AltMoverPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void Anc4AltMoverPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    Anc4AltMoverAgent aama;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(aama, m_fAge), H5T_NATIVE_FLOAT);

}
