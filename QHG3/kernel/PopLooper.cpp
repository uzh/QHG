#include <stdio.h>
#include <omp.h>

#include "types.h"
#include "PopBase.h"
#include "PopLooper.h"


//----------------------------------------------------------------------------
// constructor
//
PopLooper::PopLooper(int iChunkSize) 
    : m_iChunkSize(iChunkSize),
      m_iMaxID(0) {
    


}


//----------------------------------------------------------------------------
// destructor
//
PopLooper::~PopLooper() {
    for (uint i = 0; i < m_vP.size(); i++) {
        delete m_vP[i];
    }

}

//----------------------------------------------------------------------------
// addPop
//  calls the population's setPrioList() method
//  collects the prio levels
//  and adds it to the vector
//
int PopLooper::addPop(PopBase *pPop) {
    //    int iC =  pPop->getNumAgentsTotal();

    pPop->setPrioList();
    pPop->getPrios(m_vPrioLevels);
    m_vP.push_back(pPop);
    idtype iCurMaxID = pPop->getMaxLoadedID();
    if (iCurMaxID > m_iMaxID) {
        m_iMaxID = iCurMaxID;

    }
    return 0;
}



//----------------------------------------------------------------------------
// doStep
//  this method calls PopBase methods in the required order to perform
//  a single step for all poulations in the population vector
//
int PopLooper::doStep(float fStep) {
    int iResult = 0;
    // before step: call initializeStep for all pops
    for (uint i = 0; i < m_vP.size(); i++) {
        iResult += m_vP[i]->initializeStep(fStep);
    }

    // loop through prio levels and execute every population's
    // functions for this level in each cell
    std::set<uint>::iterator it;
    // all priority levels
    double dTime = omp_get_wtime();

    for (it = m_vPrioLevels.begin(); it != m_vPrioLevels.end(); it++) {

        // all populations
        for (uint i = 0; i < m_vP.size(); i++) {

            iResult += m_vP[i]->doActions(*it, fStep);
            }
        }

    dTimeActions += omp_get_wtime() - dTime;
    dTime = omp_get_wtime();

    // end of step: call finalizeStep for all pops
    for (uint i = 0; i < m_vP.size(); i++) {
        iResult += m_vP[i]->finalizeStep();
    }
    
    dTimeFinalize += omp_get_wtime() - dTime;

    return iResult;

}


//----------------------------------------------------------------------------
// removePopID
//  search for population with given speciesID
//  if found, erase it from vector and delete it
//
int PopLooper::removePopID(spcid iPopID) {
    int iResult = -1;
    popvec::iterator it;
    for (it = m_vP.begin(); (iResult < 0) && (it != m_vP.end()); ++it) {
        if ((*it)->getSpeciesID() == iPopID) {
            //            printf("found it\n");
            // delete the population
            delete *it;
            // remove it from vector
            m_vP.erase(it);
            iResult = 0;
        }
    }
    return iResult;
}

//----------------------------------------------------------------------------
// removePop
//  get Population with given index in vector.
//  if it exists, erase it from vector and delete it
//
int PopLooper::removePop(uint iIndex) {
    int iResult = -1;
    if (iIndex < m_vP.size()) {
        popvec::iterator it =  m_vP.begin()+iIndex;
        delete *it;
        m_vP.erase(it);
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getPopByID
//   PopFinder implementation
//
PopBase *PopLooper::getPopByID(idtype iSpeciesID) {
    PopBase *pPop = NULL;
    popvec::iterator it;
    for (it = m_vP.begin(); (pPop == NULL) && (it != m_vP.end()); ++it) {
        if ((*it)->getSpeciesID() == iSpeciesID) {
            pPop = *it;
        }
    }
    return pPop;
}

//----------------------------------------------------------------------------
// getPopByName
//   PopFinder implementation
//
PopBase *PopLooper::getPopByName(const char *pSpeciesName) {
    PopBase *pPop = NULL;
    printf("Searching for [%s]\n", pSpeciesName);
    popvec::iterator it;
    for (it = m_vP.begin(); (pPop == NULL) && (it != m_vP.end()); ++it) {
        printf("  found [%s]\n", (*it)->getSpeciesName());
        if (strcmp((*it)->getSpeciesName(), pSpeciesName) == 0) {
            pPop = *it;
        }
    }
    return pPop;
}
