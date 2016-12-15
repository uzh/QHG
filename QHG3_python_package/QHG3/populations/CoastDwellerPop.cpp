#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "VerhulstVarK.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "CoastDwellerPop.h"


//----------------------------------------------------------------------------
// constructor
//
CoastDwellerPop::CoastDwellerPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<CoastDwellerAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
    m_pAncBox(NULL) {

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    m_adMoveWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity

    if (m_pCG->m_pVegetation == NULL) {
        fprintf(stderr,"CoastDweller ERROR: No vegetation found, prepare to crash!!! (note: this will not be handled gracefully)\n");
    }
    
    /* old
    std::map<char*, double*>* mEvalInfo = new std::map<char*, double*>;
    

    char *grassprefname = new char[64];
    strcpy(grassprefname, "GrassCapPref,GrassWeight");
    mEvalInfo->insert(std::pair<char*, double*>(grassprefname, (double*)m_pCG->m_pVegetation->m_adANPP[0]));
        
    char *shrubprefname = new char[64];
    strcpy(shrubprefname, "ShrubCapPref,ShrubWeight");
    mEvalInfo->insert(std::pair<char*, double*>(shrubprefname, (double*)m_pCG->m_pVegetation->m_adANPP[1]));
        
    char *treeprefname = new char[64];
    strcpy(treeprefname, "TreeCapPref,TreeWeight");
    mEvalInfo->insert(std::pair<char*, double*>(treeprefname, (double*)m_pCG->m_pVegetation->m_adANPP[2]));

    char *coastprefname = new char[64];
    strcpy(coastprefname, "CoastCapPref,CoastWeight");
    mEvalInfo->insert(std::pair<char*, double*>(coastprefname, (double*)m_pCG->m_pGeography->m_adAltitude));
    */

    evaluatorinfos mEvalInfo;

    SingleEvaluator<CoastDwellerAgent> *pSEGrass = new SingleEvaluator<CoastDwellerAgent>(this, m_pCG, NULL, (double*)m_pCG->m_pVegetation->m_adANPP[0], "GrassCapPref");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("GrassWeight", pSEGrass));

    SingleEvaluator<CoastDwellerAgent> *pSEShrub = new SingleEvaluator<CoastDwellerAgent>(this, m_pCG, NULL, (double*)m_pCG->m_pVegetation->m_adANPP[1], "ShrubCapPref");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("ShrubWeight", pSEShrub));

    SingleEvaluator<CoastDwellerAgent> *pSETree = new SingleEvaluator<CoastDwellerAgent>(this, m_pCG, NULL, (double*)m_pCG->m_pVegetation->m_adANPP[2], "TreeCapPref");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("TreeWeight", pSETree));

    
    m_pME = new MultiEvaluator<CoastDwellerAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, false, &(m_pCG->m_pVegetation->m_bUpdated));

    /* old
    delete[] grassprefname;
    delete[] shrubprefname;
    delete[] treeprefname;
    delete[] coastprefname;
    delete mEvalInfo;
    */
    
    CoastDwellerAgent agent;
    m_pVerVarK = new VerhulstVarK<CoastDwellerAgent>(this, m_pCG, m_apWELL, m_adEnvWeights, (int)qoffsetof(agent, m_iMateIndex));
        
        
    // evaluator for movement
        
    char *moveprefname = new char[64];
    strcpy(moveprefname, "AltMovePref");
    m_pSE = new SingleEvaluator<CoastDwellerAgent>(this, m_pCG, m_adMoveWeights, (double*)m_pCG->m_pGeography->m_adAltitude, moveprefname, true, &(m_pCG->m_pGeography->m_bUpdated));
    delete[] moveprefname;
    
    m_pWM = new WeightedMove<CoastDwellerAgent>(this, m_pCG, m_apWELL, m_adMoveWeights);
    
    m_pPair = new RandomPair<CoastDwellerAgent>(this, m_pCG, m_apWELL);
        
    m_pGO = new GetOld<CoastDwellerAgent>(this, m_pCG);
        
        
    // adding all actions to prioritizer
        
    m_prio.addAction(MULTIEVAL_NAME, m_pME);
    m_prio.addAction(SINGLEEVAL_NAME, m_pSE);
    m_prio.addAction(WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(VERHULSTVARK_NAME, m_pVerVarK);
    m_prio.addAction(RANDPAIR_NAME, m_pPair);
    m_prio.addAction(GETOLD_NAME, m_pGO);
    
}

///----------------------------------------------------------------------------
// destructor
//
CoastDwellerPop::~CoastDwellerPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_adMoveWeights != NULL) {
        delete[] m_adMoveWeights;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pSE != NULL) {
        delete m_pSE;
    }
    if (m_pVerVarK != NULL) {
        delete m_pVerVarK;
    }
    if (m_pAncBox != NULL) {
        delete m_pAncBox;
    }

    if (m_pPair != NULL) {
        delete m_pPair;
    }
    
    if (m_pGO != NULL) {
        delete m_pGO;
    }
    
    if (m_pOutDir != NULL) {
        delete m_pOutDir;
    }
    if (m_pPrefix != NULL) {
        delete m_pPrefix;
    }
}



///----------------------------------------------------------------------------
// setParams
//
int CoastDwellerPop::setParams(const char *pParams) {
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
    delete[] sParams;
    return iResult;
}

///----------------------------------------------------------------------------
// preLoop
//
int CoastDwellerPop::preLoop() {

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

    // sprintf(m_sAncOutFileName,"pop_%d.anc",m_iSpeciesID);
    m_pAncBox = AncestorBoxR::createInstance(m_aAgents.getLayerSize());

    int iFirstAgent = getFirstAgentIndex();
    int iLastAgent = getLastAgentIndex();

    for (int iA = iFirstAgent; iA <= iLastAgent; iA++) {
        m_pAncBox->addBaby(m_aAgents[iA].m_ulID, 0, 0);
    }

    return 0;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int CoastDwellerPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    // here we record parent info into the ancestor box

    m_pAncBox->addBaby(m_aAgents[iAgent].m_ulID, 
                       m_aAgents[iMother].m_ulID, 
                       m_aAgents[iFather].m_ulID);

    m_aAgents[iAgent].m_fAge = 0.0;

    return 0;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int CoastDwellerPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    // here we use a trick:
    // we are not writing to the QDF but to a separate file
    // but it is convenient to do it here because 
    // this function is called just at the right moment 

    char *ancfilename = new char[strlen(m_pOutDir)+strlen(m_pPrefix)+16];
    sprintf(ancfilename, "%s/%s_%06d.anc", m_pOutDir, m_pPrefix, (int)(m_fCurTime+1));
    m_pAncBox->setOutputFile(ancfilename,false);
    delete[] ancfilename;
    return m_pAncBox->writeData();
}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int CoastDwellerPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void CoastDwellerPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    CoastDwellerAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);

}

