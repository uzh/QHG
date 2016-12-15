#include <stdio.h>
#include <string>
#include <map>

#include "Pattern.h"
#include "ArrayShare.h"

ArrayShare *ArrayShare::s_pAS = NULL;
ArrayShare *ArrayShare::getInstance() {
    if (s_pAS == NULL) {
        s_pAS = new ArrayShare();
    }
    return s_pAS;
}
void ArrayShare::freeInstance() {
    if (s_pAS != NULL) {
        delete s_pAS;
        s_pAS = NULL;
    }
}


//-----------------------------------------------------------------------------
// constructor
//
ArrayShare::ArrayShare() {
}

//-----------------------------------------------------------------------------
// destructor
//
ArrayShare::~ArrayShare() {
    arraymap::iterator it;
    printf("deleting arrayshare\n");
    for (it = m_mArrays.begin(); it != m_mArrays.end(); ++it) {
        if (it->second != NULL) {
            delete it->second;
        }
    }
}


//-----------------------------------------------------------------------------
// addArray
//
int ArrayShare::shareArray(const char *pName, int iSize, void *pdArr) {
    int iResult = -1;
    arraymap::iterator it = m_mArrays.find(pName);
    if (it == m_mArrays.end()) {
        m_mArrays[pName] = new arraystruct(iSize, pdArr);
        iResult = 0;
    } else {
        printf("[ArrayShare::shareArray] name [%s] already in list\n", pName);
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getSize
//
int ArrayShare::getSize(const char *pName) {
    int iSize = -1;
    arraymap::iterator it = m_mArrays.find(pName);
    if (it != m_mArrays.end()) {
        iSize = it->second->m_iSize;
    } else {
        printf("[ArrayShare::getSize] no array with name [%s]\n", pName);
        iSize = -1;
    }
    return iSize;   
}


//-----------------------------------------------------------------------------
// getArray
//
void *ArrayShare::getArray(const char *pName) {
    void *pdArr = NULL;
    arraymap::iterator it = m_mArrays.find(pName);
    if (it != m_mArrays.end()) {
        pdArr = it->second->m_pdArr;
    } else {
        //        printf("[ArrayShare::getArray] no array with name [%s]\n", pName);
    }
    return pdArr;
}


//-----------------------------------------------------------------------------
// getArrayStruct
//
arraystruct *ArrayShare::getArrayStruct(const char *pName) {
    arraystruct *pas = NULL;
    arraymap::iterator it = m_mArrays.find(pName);
    if (it != m_mArrays.end()) {
        pas = it->second;
    } else {
        //        printf("[ArrayShare::getArrayStruct] no array with name [%s]\n", pName);
    }
    return pas;
}


//-----------------------------------------------------------------------------
// removeArray
//
int ArrayShare::removeArray(const char *pName) {
    int iResult = -1;

    arraymap::iterator it = m_mArrays.find(pName);
    if (it != m_mArrays.end()) {
        arraystruct *pas = it->second;
        m_mArrays.erase(it);
        delete pas;
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getNamesLike
//
const stringlist &ArrayShare::getNamesLike(const char *pNamePattern) {
    m_vNameMatches.clear();
    arraymap::iterator it;
    Pattern *pPat = new Pattern();
    pPat->addPattern(pNamePattern);

    stringlist vCurMatches;
    for (it = m_mArrays.begin(); it != m_mArrays.end(); ++it) {
        bool bMatch = pPat->matchPattern(0, it->first.c_str(), vCurMatches);
        if (bMatch) {
            m_vNameMatches.push_back(it->first);
        }
    }
    delete pPat;
    return m_vNameMatches;
}



//-----------------------------------------------------------------------------
// display
//
void ArrayShare::display() {
    arraymap::iterator it ;
    for (it = m_mArrays.begin(); it != m_mArrays.end(); ++it) {
        printf("  [%s] -> %p (%d)\n", it->first.c_str(), it->second->m_pdArr, it->second->m_iSize);
    }
}    


