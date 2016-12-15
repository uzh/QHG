#include <omp.h>

#include "clsutils.cpp"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "GetOld.h"


//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
GetOld<T>::GetOld(SPopulation<T> *pPop, SCellGrid *pCG) 
    : Action<T>(pPop,pCG) {
    
}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
GetOld<T>::~GetOld() {
    
    // nothing to do here
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int GetOld<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;
    
    T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);

	if (pa->m_iLifeState > 0) {
	    pa->m_fAge = fT - pa->m_fBirthTime;
	}

    return iResult;
}


