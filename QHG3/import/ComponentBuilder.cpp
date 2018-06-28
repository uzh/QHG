#include <stdio.h>
#include <string.h>

#include <vector>

#include "SCell.h"
#include "SCellGrid.h"

#include "ComponentBuilder.h"

//----------------------------------------------------------------------------
// createInstance
//
ComponentBuilder *ComponentBuilder::createInstance(SCellGrid *pCG, const cellvec &vSubSet) {
    ComponentBuilder *pCB = new ComponentBuilder(pCG, vSubSet);
    int iResult = pCB->init();
    if (iResult != 0) {
        delete pCB;
        pCB = NULL;
    }
    return pCB;
}


//----------------------------------------------------------------------------
// constructor
//
ComponentBuilder::ComponentBuilder(SCellGrid *pCG, const cellvec &vSubSet)
    : m_pCG(pCG),
      m_vSubSet(vSubSet),
      m_iNumCells(0),
      m_iNumComponents(0),
      m_pIndexToComp(NULL) {

    m_mIdToIndex.clear();
}


//----------------------------------------------------------------------------
// destructor
//
ComponentBuilder::~ComponentBuilder() {
    if (m_pIndexToComp != NULL) {
        delete[] m_pIndexToComp;
    }
}

//----------------------------------------------------------------------------
// init
//
int ComponentBuilder::init() {
    int iResult = 0;
    m_iNumCells = m_vSubSet.size();
    if (m_iNumCells > 0) {
        for (int j = 0; j < m_iNumCells; ++j) {
            m_mIdToIndex[m_vSubSet[j]] = j;
        }
    
        if (iResult == 0) {
            m_pIndexToComp = new gridtype[m_iNumCells];
            for (int j = 0; j < m_iNumCells; ++j) {
                m_pIndexToComp[j] = -1;
            }
        }
        
        
        if (iResult == 0) {
            iResult = calcComponents();
        }
    } else {
        printf("[ComponentBuilder::init] Empty cellvec\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// findNextFree
//
int ComponentBuilder::findNextFree(int iIndex) {
    int iNext = -1;
    
    while ((iIndex < m_iNumCells) && (m_pIndexToComp[iIndex]>= 0)) {
        iIndex++;
    }
     
    if (iIndex < m_iNumCells) {
        iNext = iIndex;
    }
    return iNext;
}
    

//----------------------------------------------------------------------------
// calcComponents
//
int ComponentBuilder::calcComponents() {
    int iResult = -1;

    int iCurIndex = findNextFree(0);
    if (iCurIndex >= 0) {

        iResult = 0;
        m_iNumComponents = 0;
        
        while ((iResult == 0) && (iCurIndex >= 0)) {
            
            iResult = calcComponentFor(iCurIndex);

            m_iNumComponents++;
            iCurIndex = findNextFree(iCurIndex);
        }
            
        
        
    } else {
        printf("no starting point found\n");
    }
    return iResult;
}

//----------------------------------------------------------------------------
// calcComponentsFor
//
int ComponentBuilder::calcComponentFor(int iCurIndex) {
    int iResult = 0;

       cellset apCandidates[2];
    int iWhich = 0;
    apCandidates[iWhich].insert(iCurIndex);

    while (apCandidates[iWhich].size() > 0) {
        cellset::const_iterator it;
        for (it = apCandidates[iWhich].begin(); it != apCandidates[iWhich].end(); ++it) {
            m_pIndexToComp[*it] = m_iNumComponents;
            SCell &sCell = m_pCG->m_aCells[m_vSubSet[*it]];
            for (int k = 0; k < sCell.m_iNumNeighbors; ++k) { 
                int iCurSub =  m_pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID;
                
                idindex::const_iterator itm = m_mIdToIndex.find(iCurSub);
                if (itm != m_mIdToIndex.end()) {
                    if (m_pIndexToComp[itm->second] < 0) {
                        apCandidates[1-iWhich].insert(itm->second);
                    }
                }
            }
        }

        apCandidates[iWhich].clear(); 
        iWhich = 1-iWhich;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getComponentFor
//
int ComponentBuilder::getComponentFor(gridtype iCellID) {
    int iComponent = -1;
    
    idindex::const_iterator it = m_mIdToIndex.find(iCellID);
    if (it != m_mIdToIndex.end()) {
        iComponent = m_pIndexToComp[it->second];
    } else {
        printf("Cell %d is not part of the specified subset\n", iCellID);
    }
    return iComponent;
}
