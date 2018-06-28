#include <string.h>
#include <omp.h>

#include "utils.h" /// for dNegInf
#include "clsutils.cpp"
#include "EventConsts.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "PolyLine.h"
#include "Evaluator.h"
#include "MultiEvaluator.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
MultiEvaluator<T>::MultiEvaluator(SPopulation<T> *pPop, SCellGrid *pCG, double *adOutputWeights, evaluatorinfos &mEvaluators, int iMode, bool bDeleteEvaluators)
    : Action<T>(pPop,pCG),
      m_bDeleteEvaluators(bDeleteEvaluators),
      m_adOutputWeights(adOutputWeights),
      m_bFirst(true),
      m_adSingleEvalWeights(NULL),
      m_acAllowed(NULL) {

    
    init(mEvaluators, iMode);
}    


//-----------------------------------------------------------------------------
// init
//
template<typename T>
void MultiEvaluator<T>::init(evaluatorinfos &mEvalInfo, int iMode) {
    fprintf(stderr,"creating MultiEvaluator\n");
    m_iMode = iMode;

    m_iMaxNeighbors = this->m_pCG->m_iConnectivity;
    m_adSingleEvalWeights = new double[this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1)];

    if ((m_iMaxNeighbors != 0) && (m_iMode == MODE_ADD_BLOCK)) {
        m_acAllowed = new uchar[this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1)];
    }

    // mEvalInfo is the map of preference info parameter names
    // and relative arrays of data (e.g. altitude) and weights
    
    m_iNumEvals = (int)mEvalInfo.size();
    m_aEvaluators = new Evaluator*[m_iNumEvals];
    m_asCombinationWeightNames = new char*[m_iNumEvals];
    m_adCombinationWeights     = new double[m_iNumEvals];
    
    int iEval = 0;

    evaluatorinfos::iterator it;

    for (it = mEvalInfo.begin(); it != mEvalInfo.end(); ++it, ++iEval) {
	        
        m_asCombinationWeightNames[iEval] = new char[it->first.length()+1];
        strcpy(m_asCombinationWeightNames[iEval], it->first.c_str()); // this ensures that sPreference is null-terminated
        
        //bool bCumulate = (m_iMode == MODE_MUL_SIMPLE || m_iMode == MODE_ADD_BLOCK) ? false : true;
        m_aEvaluators[iEval] = it->second;
        m_aEvaluators[iEval]->setOutputWeights(m_adSingleEvalWeights);
        addObserver(m_aEvaluators[iEval]);
        printf("MultiEvaluator added Evaluator for [%s]\n",   m_asCombinationWeightNames[iEval]);
        
        m_adCombinationWeights[iEval] = 0; // initialization to 0
    }

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
MultiEvaluator<T>::~MultiEvaluator() {
    

    if (m_adSingleEvalWeights != NULL) {
        delete[] m_adSingleEvalWeights;
    }
    
    if (m_aEvaluators != NULL) {
        if (m_bDeleteEvaluators) {
            for (int i = 0; i < m_iNumEvals; i++) {
                if (m_aEvaluators[i] != NULL) {
                    delete m_aEvaluators[i];
                }
            }
        }
        delete[] m_aEvaluators;
    }

    if (m_asCombinationWeightNames != NULL) {
        for (int i = 0; i < m_iNumEvals; i++) {
            if (m_asCombinationWeightNames[i] != NULL) {
                delete[] m_asCombinationWeightNames[i];
            }
        }
        delete[] m_asCombinationWeightNames;
    }
    
    if (m_adCombinationWeights != NULL) {
        delete[] m_adCombinationWeights;
    }

    if (m_acAllowed != NULL) {
        delete[] m_acAllowed;
    }

}

//-----------------------------------------------------------------------------
// preLoop
//   basically call all evaluatzors' preLoop
//
template<typename T>
int MultiEvaluator<T>::preLoop() {
    int iResult = 0;
    for (int iEval = 0; (iResult == 0) && (iEval < m_iNumEvals); iEval++) {
        iResult = m_aEvaluators[iEval]->preLoop();
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// postLoop
//   basically call all evaluatzors' preLoop
//
template<typename T>
int MultiEvaluator<T>::postLoop() {
    int iResult = 0;
    for (int iEval = 0; (iResult == 0) && (iEval < m_iNumEvals); iEval++) {
        iResult = m_aEvaluators[iEval]->postLoop();
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
// here the weights are calculated if needed
//
template<typename T>
int MultiEvaluator<T>::initialize(float fT) {

    int iResult = 0;

    // previously    if (m_bNeedUpdate || (fT==0)) {
    if (m_bNeedUpdate || m_bFirst) {
        m_bFirst = false;
        // printf("[MultiEvaluator::initialize] updating...\n");

        switch (m_iMode) {
        case MODE_ADD_SIMPLE:
            iResult = addSingleWeights(fT);
            break;
        case MODE_ADD_BLOCK:
            iResult = addSingleWeightsBlock(fT);
            break;
        case MODE_MUL_SIMPLE:
            iResult = multiplySingleWeights(fT);
            break;
        case MODE_MAX_SIMPLE:
            iResult = maxSingleWeights(fT);
            break;
        case MODE_MAX_BLOCK:
            iResult = maxSingleWeightsBlock(fT);
            break;
        case MODE_MIN_SIMPLE:
            iResult = minSingleWeights(fT);
            break;
        default:
            printf("Unknown mode [%d]\n", m_iMode);
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// finalize
//   reset m_bNeeedUpdate to false
//
template<typename T>
int MultiEvaluator<T>::finalize(float fT) {
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        m_aEvaluators[iEval]->finalize(fT);
    }
    m_bNeedUpdate = false;
    return 0;
}


//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void MultiEvaluator<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    
    notifyObservers(iEvent, pData);

    if (iEvent == EVENT_ID_FLUSH) {   
        m_bNeedUpdate = false;
        for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
            m_bNeedUpdate |= m_aEvaluators[iEval]->needUpdate();
        }
    }
}


//----------------------------------------------------------------------------
// addEnvWeights
//  add weighted contributions, no blocking
//
template<typename T>
int MultiEvaluator<T>::addSingleWeights(float fT) {
   
    int iResult = 0;
    printf("addEnvWeights\n");

   int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);


    memset(m_adOutputWeights, 0, iArrSize*sizeof(double));
     
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
            
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            m_adOutputWeights[iArrIndex] += m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
        }
        
    }

    return iResult;
}


//----------------------------------------------------------------------------
// addEnvWeightsBlock
//  add weighted contributions, blocking
//
//  ATTENTION: SingleEvaluators must be non-cumulating
//
template<typename T>
int MultiEvaluator<T>::addSingleWeightsBlock(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);
    
    printf("addEnvWeightsBlock\n");
    findBlockings(fT);

    memset(m_adOutputWeights, 0, iArrSize*sizeof(double));
     
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
            
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            if (m_acAllowed[iArrIndex] > 0) {
                m_adOutputWeights[iArrIndex] += m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
            }
        }
    }
    // now let's do the cumulant, since we have been getting non-cumulated values from this Evaluator
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    


    return iResult;
}


//----------------------------------------------------------------------------
// multiplyEnvWeights
//  multiply contributions, no blocking
//
//  ATTENTION: SingleEvaluator must be non-cumulating
//
template<typename T>
int MultiEvaluator<T>::multiplySingleWeights(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);
    printf("multiplyEnvWeights\n");

    
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = 1.0;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            m_adOutputWeights[iArrIndex] *= m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
        }

    }

    // now let's do the cumulant, since we have been getting non-cumulated values from SingleEvaluator
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// maxEnvWeights
//  max contributions
//
template<typename T>
int MultiEvaluator<T>::maxSingleWeights(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);

    printf("maxEnvWeights\n");

    // set all elements to negative infinity
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = dNegInf;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            double dValNew = m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
            if (m_adOutputWeights[iArrIndex] < dValNew) {
                m_adOutputWeights[iArrIndex] = dValNew; 
            }
        }
        
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// maxEnvWeightsBlock
//  max contributions
//
//  ATTENTION: SingleEvaluator must be non-cumulating
//
template<typename T>
int MultiEvaluator<T>::maxSingleWeightsBlock(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);

    printf("maxEnvWeightsBlock\n");
    findBlockings(fT);

    // set all elements to negative infinity
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = dNegInf;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            if (m_acAllowed[iArrIndex] > 0) {
                double dValNew = m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
                if (m_adOutputWeights[iArrIndex] < dValNew) {
                    m_adOutputWeights[iArrIndex] = dValNew; 
                }
            }
        }
        
    }

    // now let's do the cumulant, since we have been getting non-cumulated values from SingleEvaluator
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// minEnvWeights
//  min contributions
//
template<typename T>
int MultiEvaluator<T>::minSingleWeights(float fT) {
   
    int iResult = 0;

    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);
    printf("minEnvWeights\n");

    // set all elements to negative infinity
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
        m_adOutputWeights[iArrIndex] = dPosInf;
    }
    
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {
        
        memset(m_adSingleEvalWeights, 0, iArrSize*sizeof(double));
        
        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            double dValNew = m_adSingleEvalWeights[iArrIndex] * m_adCombinationWeights[iEval];
            if (m_adOutputWeights[iArrIndex] > dValNew) {
                m_adOutputWeights[iArrIndex] = dValNew; 
            }
        }
        
    }
    
    // now let's do the cumulant, since we have been getting non-cumulated values from SingleEvaluator
#ifdef OMP_A
#pragma omp parallel for
#endif
    for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex += m_iMaxNeighbors+1) {
        for (int iC = iArrIndex + 1; iC < iArrIndex + m_iMaxNeighbors + 1; iC++) {
            m_adOutputWeights[iC] += m_adOutputWeights[iC-1];
        }
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
template<typename T>
int MultiEvaluator<T>::extractParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    for (int i = 0; i < m_iNumEvals && iResult == 0; i++) {
        iResult = qdf_extractAttribute(hSpeciesGroup, m_asCombinationWeightNames[i], 1, &m_adCombinationWeights[i]);
        if (iResult == 0) {
            iResult = m_aEvaluators[i]->extractParamsQDF(hSpeciesGroup);
        }
    }

    return iResult;
}



//-----------------------------------------------------------------------------
// writeParamsQDF
//
template<typename T>
int MultiEvaluator<T>::writeParamsQDF(hid_t hSpeciesGroup) {

    int iResult = 0;

    for (int i = 0; i < m_iNumEvals && iResult == 0; i++) {
        //        printf("writing att [%s]:%f\n",  m_asCombinationWeightNames[i], m_adCombinationWeights[i]);
        iResult = qdf_insertAttribute(hSpeciesGroup, m_asCombinationWeightNames[i], 1, &m_adCombinationWeights[i]);
        if (iResult == 0) {
            iResult = m_aEvaluators[i]->writeParamsQDF(hSpeciesGroup);
        }
    }
    //    printf("finished writing atts\n");

    return iResult;

}



//-----------------------------------------------------------------------------
// tryReadParamLine
//
template<typename T>
int MultiEvaluator<T>::tryReadParamLine(char *pLine) {

    int iResult = 0;

    for (int i = 0; i < m_iNumEvals && iResult == 0; i++) {
        iResult += this->readPopKeyVal(pLine, m_asCombinationWeightNames[i], &m_adCombinationWeights[i]);
        if (iResult == 0) {
            iResult += m_aEvaluators[i]->tryReadParamLine(pLine);
            if (iResult == 1) {
                printf("Evaluator #%d recognized \"%s\"\n",i,pLine);
            }
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// findBlockings
//  m_acAllowed[i] is 0 if any of the single evaluator arrays is zero at position i.
//  otherwise, it is 1
//
template<typename T>
int MultiEvaluator<T>::findBlockings(float fT) {
    int iResult = 0;
    int iArrSize = this->m_pCG->m_iNumCells * (m_iMaxNeighbors + 1);

    
    memset(m_acAllowed, 0x01, iArrSize*sizeof(uchar));
    for (int iEval = 0; iEval < m_iNumEvals; iEval++) {

        iResult = m_aEvaluators[iEval]->initialize(fT); // this will fill m_adSingleEvalWeights
        
#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iArrIndex = 0; iArrIndex < iArrSize; iArrIndex++) {
            if (m_adSingleEvalWeights[iArrIndex] <= 0) {
                m_acAllowed[iArrIndex] = 0;
            }
        }
        
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void MultiEvaluator<T>::showAttributes() {
    
    for (int i = 0; i < m_iNumEvals; i++) {
        printf("  %s\n", m_asCombinationWeightNames[i]);
    }

}
