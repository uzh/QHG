#include <stdio.h>
#include <omp.h>

#include "MessLogger.h"
#include "EventConsts.h"
#include "WELL512.h"

#include "Action.h"
#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "Navigation.h"
#include "Navigate.h"

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
Navigate<T>::Navigate(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL) 
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dDecay(0),
      m_dDist0(0),
      m_dProb0(0),
      m_dMinDens(0),
      m_dA(0),
      m_bNeedUpdate(true) {

    pPop->addObserver(this);

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
Navigate<T>::~Navigate() {
    // delete targdist arrays in map
    cleanup();

}


//-----------------------------------------------------------------------------
// cleanup
//
template<typename T>
void Navigate<T>::cleanup() {
    distprobmap::iterator it;
    for (it = m_mDistProbs.begin(); it != m_mDistProbs.end(); ++it) {
        delete[] it->second.second;
    }
    m_mDistProbs.clear();
}

//-----------------------------------------------------------------------------
// notify
//
template<typename T>
void Navigate<T>::notify(Observable *pObs, int iEvent, const void *pData) {
    if ((iEvent == EVENT_ID_GEO) || (iEvent == EVENT_ID_NAV)) { 
        m_bNeedUpdate = true;
        
    } else if (iEvent == EVENT_ID_FLUSH) {
        // no more events: recalculate
        recalculate();
    }
}


//-----------------------------------------------------------------------------
// recalculate
//
template<typename T>
void Navigate<T>::recalculate() {

    if (m_bNeedUpdate) {
        printf("Navigate::recalculate\n");
        m_dA = m_dProb0/(exp(m_dDecay*m_dDist0));

        cleanup();
        int iResult = 0;
        distancemap::const_iterator it;
        Navigation *pNav = this->m_pCG->m_pNavigation; 
        for (it = pNav->m_mDestinations.begin(); (iResult == 0) && (it != pNav->m_mDestinations.end()); ++it) {
            targdist *pTD = new targdist[it->second.size()+1];
            distlist::const_iterator itl;

            double dProbSum = 0;
            int i = 1;
            for (itl = it->second.begin(); itl != it->second.end(); ++itl) {
                double dProb = m_dA*exp(m_dDecay*itl->second);
                dProbSum += dProb;
                pTD[i++]=targdist(itl->first, dProb);
            }
            if (dProbSum < 1) {
                pTD[0] = targdist(-1, 1-dProbSum);
                m_mDistProbs[it->first] = std::pair<int, targdist*>(it->second.size(), pTD);
            } else {
                printf("[Navigate] probabilities for port [%d] add up to %f\n",  it->first, dProbSum);fflush(stdout);
                LOG_ERROR("[Navigate] probabilities for port [%d] add up to %f\n",  it->first, dProbSum);
                iResult = -1;
            }
        }
        m_bNeedUpdate = false;
    }    
}


//-----------------------------------------------------------------------------
// preloop
//
template<typename T>
int Navigate<T>::preLoop() {
    int iResult = 0;
    // we need 
    if ((this->m_pCG->m_pGeography != NULL) && 
        (this->m_pCG->m_pNavigation != NULL)) {
        cleanup();
        recalculate();
    } else {
        iResult = -1;
        if (this->m_pCG->m_pGeography != NULL) {
            printf("[Navigate] m_pGeography is NULL!\n");
            LOG_ERROR("[Navigate] m_pGeography is NULL!\n");
            printf("  Make sure your gridfile has a geography group!\n");
        }
        if (this->m_pCG->m_pNavigation != NULL) {
            printf("[Navigate] m_pNavigation is NULL!\n");
            LOG_ERROR("[Navigate] m_pNavigation is NULL!\n");
            printf("  Make sure your gridfile has a climate group!\n");
        }
    }

    return iResult;
}




//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int Navigate<T>::operator()(int iAgentIndex, float fT) {

    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);


    int iCellIndex = pa->m_iCellIndex;
    distprobmap::const_iterator it = m_mDistProbs.find(iCellIndex);
    if (it != m_mDistProbs.end()) {

        // we only ship alive and not moving agents
        if ((pa->m_iLifeState > 0) && ((pa->m_iLifeState & LIFE_STATE_MOVING) == 0)) {
            int iNumDests = it->first;
            
            int iThread = omp_get_thread_num();
            double dR =  m_apWELL[iThread]->wrandd();

            int i = 0;
            while ((i < iNumDests) && (dR > it->second.second[i].second)) {
                dR -= it->second.second[i++].second;
            }

            if (i > 0) {
                int iNewCellIndex = it->second.second[i].first;
                if ((this->m_pCG->m_pGeography == NULL) || (!this->m_pCG->m_pGeography->m_abIce[iCellIndex])) {
                    this->m_pPop->registerMove(iCellIndex, iAgentIndex, iNewCellIndex);
                }
            }
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_NAVIGATE_DECAY_NAME
//    ATTR_NAVIGATE_DIST0_NAME
//    ATTR_NAVIGATE_PROB0_NAME
//    ATTR_NAVIGATE_MINDENS_NAME
//
template<typename T>
int Navigate<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE_DECAY_NAME, 1, &m_dDecay);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE_DECAY_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE_DIST0_NAME, 1, &m_dDist0);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE_DIST0_NAME);
        }
    }

    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE_PROB0_NAME, 1, &m_dProb0);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE_PROB0_NAME);
        }
    }


    if (iResult == 0) {
        iResult = qdf_extractAttribute(hSpeciesGroup, ATTR_NAVIGATE_MINDENS_NAME, 1, &m_dMinDens);
        if (iResult != 0) {
            LOG_ERROR("[Navigate] couldn't read attribute [%s]", ATTR_NAVIGATE_MINDENS_NAME);
        }
    }

    return iResult; 
}


//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_NAVIGATE_DECAY_NAME
//    ATTR_NAVIGATE_DIST0_NAME
//    ATTR_NAVIGATE_PROB0_NAME
//    ATTR_NAVIGATE_MINDENS_NAME
//
template<typename T>
int Navigate<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE_DECAY_NAME,   1, &m_dDecay);

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE_DIST0_NAME,   1, &m_dDist0);

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE_PROB0_NAME,   1, &m_dProb0);

    iResult += qdf_insertAttribute(hSpeciesGroup, ATTR_NAVIGATE_MINDENS_NAME, 1, &m_dMinDens);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int Navigate<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;

   iResult += this->readPopKeyVal(pLine, ATTR_NAVIGATE_DECAY_NAME,   &m_dDecay);
   iResult += this->readPopKeyVal(pLine, ATTR_NAVIGATE_DIST0_NAME,   &m_dDist0);
   iResult += this->readPopKeyVal(pLine, ATTR_NAVIGATE_PROB0_NAME,   &m_dProb0);
   iResult += this->readPopKeyVal(pLine, ATTR_NAVIGATE_MINDENS_NAME, &m_dMinDens);

   return iResult;
}

//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void Navigate<T>::showAttributes() {
    printf("  %s\n", ATTR_NAVIGATE_DECAY_NAME);
    printf("  %s\n", ATTR_NAVIGATE_DIST0_NAME);
    printf("  %s\n", ATTR_NAVIGATE_PROB0_NAME);
    printf("  %s\n", ATTR_NAVIGATE_MINDENS_NAME);
}
