#include "Action.h"
#include "clsutils.h"


template<typename T>
Action<T>::Action(SPopulation<T> *pPop, SCellGrid *pCG) 
    : m_pPop(pPop),
      m_pCG(pCG) {
   
}


template<typename T>
template<typename T2>
int Action<T>::readPopKeyVal(char* pLine, const char* pKey, T2* pParam) {

    T2 pDummy = 0;
    int iResult = 1;    // we want final result =0 for fail, =1 for success
    
    iResult += readKeyVal(pLine, pKey, "=", &pDummy);
    if (iResult == 1) {
        *pParam = pDummy;
    }

    return iResult;
}


template<typename T>
template<typename T2>
int Action<T>::readPopKeyArr(char* pLine, const char* pKey, int iNum, T2* pParam) {

    int iResult = 1;    // we want final result =0 for fail, =1 for success
    
    iResult += readKeyArr(pLine, pKey, "=", iNum, pParam);

    return iResult;
}


