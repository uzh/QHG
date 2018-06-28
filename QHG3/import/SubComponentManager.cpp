#include <stdio.h>

#include <map>
#include <vector>

#include "types.h"
#include "SCell.h"
#include "SCellGrid.h"

#include "SubComponent.h"
#include "SubComponentManager.h"


//----------------------------------------------------------------------------
// createInstance
//
SubComponentManager *SubComponentManager::createInstance(SCellGrid *pCG, const distancemap &mDistances, const neighborlist &mNeighbors, bool bVerbosity) {
    SubComponentManager *pSCM = new SubComponentManager(pCG, mDistances, mNeighbors, bVerbosity);
    int iResult = pSCM->init();
    if (iResult != 0) {
        delete pSCM;
        pSCM = NULL;
    }
    return pSCM;
}


//----------------------------------------------------------------------------
// constructor
//
SubComponentManager::SubComponentManager(SCellGrid *pCG, const distancemap &mDistances, const neighborlist &mNeighbors, bool bVerbosity) 
    : m_pCG(pCG),
      m_mDistances(mDistances),
      m_mNeighbors(mNeighbors),
      m_bVerbose(bVerbosity) {
}


//----------------------------------------------------------------------------
// destructor
//
SubComponentManager::~SubComponentManager() {
    subcompmap::const_iterator it;
    for (it = m_mSubComponents.begin(); it != m_mSubComponents.end(); ++it) {
        if (it->second != NULL) {
            delete it->second;
        }
    }
}


//----------------------------------------------------------------------------
// constructor
//
int SubComponentManager::init() {
    int iResult = 0;
    iResult = createSubComponents();
    if (m_bVerbose && (iResult == 0)) {
        printf("~~~~vvv~~~\n");
        showSubComponents();
        printf("~~~~^^^~~~\n");
    }

    return iResult;
}
 
   
//----------------------------------------------------------------------------
// createSubComponents
//
int SubComponentManager::createSubComponents() {
    int iResult = 0;

    int iCurID = 0;

    bool bGoOn = true;
    while (bGoOn && (iResult == 0)) {
        int iCellID1 = firstUnusedCellID();
        if (iCellID1 >= 0) {
            cellchain vChainLeft;
            intset sCur;
            sCur.insert(iCellID1);
         
            vChainLeft.insert(iCellID1);
            m_mSubCompForCell[iCellID1] = iCurID;
            SubComponent *pSC = new SubComponent(iCurID);

 
            intset sNeighbors;
            while (sCur.size() > 0) {
                iResult = unusedNeighborIDs(sCur, sNeighbors);
                if (iResult == 0) {
                    intset sNext;

                
                    intset::const_iterator it2;
                    for (it2 = sNeighbors.begin(); it2 != sNeighbors.end(); ++it2) {
                        vChainLeft.insert(*it2);
                        m_mSubCompForCell[*it2] = iCurID;
                    }
                    sCur= sNeighbors;
                }
            
            }

            pSC->setCells(vChainLeft);
            m_mSubComponents[iCurID] = pSC;
            iCurID++;
        } else {
            bGoOn = false;
        }
    }

    if (m_bVerbose) {
        printf("::::^^^:::\n");
        showSubComponents();
        printf("::::^^^:::\n");
    }
    splitForTarget();

    if (m_bVerbose) {
        printf("****vvv***\n");
        showSubComponents();
        printf("****^^^***\n");
    }
    deleteUnneeded();

    if (m_bVerbose) {
        printf("####vvv###\n");
        showSubComponents();
        printf("####^^^###\n");
    }
    eliminateDuplicates();

    if (m_bVerbose) {
        printf(">>>>vvv<<<\n");
        showSubComponents();
        printf(">>>>vvv<<<\n");
    }
    return iResult;
}






//----------------------------------------------------------------------------
// firstUnusedCellID
//
int SubComponentManager::firstUnusedCellID() {
    int iFirst = -1;
    distancemap::const_iterator it;
    for (it = m_mDistances.begin(); (iFirst < 0) && (it != m_mDistances.end()); ++it) {
     
        if (m_mSubCompForCell.find(it->first) == m_mSubCompForCell.end()) {
            iFirst = it->first;
        }
    }   
    return iFirst; 
}


//----------------------------------------------------------------------------
// unusedNeighborIDs
//
int SubComponentManager::unusedNeighborIDs(intset &sIn, intset &sNeighbors) {
    int iResult = 0;
    sNeighbors.clear();

    intset::const_iterator itIn;
    for (itIn = sIn.begin(); (iResult == 0) && (itIn != sIn.end()); ++itIn) {

    
        neighborlist::const_iterator its = m_mNeighbors.find(*itIn);
        if (its != m_mNeighbors.end()) {
            const intset &sCur = its->second;
            intset::const_iterator itN;
            for (itN = sCur.begin(); itN != sCur.end(); ++itN) {
                if ((m_mSubCompForCell.find(*itN) == m_mSubCompForCell.end()) && 
                    (m_mDistances.find(*itN) != m_mDistances.end())) {
                    sNeighbors.insert(*itN);
                }
            }
        } else {
            printf("No neighbor info for [%d]\n", *itIn);
            iResult = -1;
        }
    }
    return iResult;
}



//----------------------------------------------------------------------------
// splitForTarget
//
void SubComponentManager::splitForTarget() {
    // split cellchains according to target 
    std::map<gridtype, std::map<gridtype, cellchain> > mSplit;
    subcompmap::const_iterator it;
    for (it = m_mSubComponents.begin(); it != m_mSubComponents.end(); ++it) {
        
        const cellchain &vChain = it->second->getChain();
        cellchain::const_iterator itc;
        for (itc = vChain.begin(); itc != vChain.end(); ++itc) {
            const distlist &dl = m_mDistances[*itc];

            distlist::const_iterator it2;
            for (it2 = dl.begin(); it2 != dl.end(); ++it2) {
                mSplit[it->first][ m_mSubCompForCell[it2->first]].insert(*itc);
            }
        }
        // delete the SUbComponent because we don't need it anymore
        delete it->second;
    }
    
    // replace original with split subcomponents
    m_mSubComponents.clear();
    int iCurID = 0;
    std::map<gridtype, std::map<gridtype, cellchain> >::const_iterator itx;
    for (itx = mSplit.begin(); itx != mSplit.end(); ++itx) {
        std::map<gridtype, cellchain>::const_iterator ity;
        for (ity = itx->second.begin(); ity != itx->second.end(); ++ity) {
            SubComponent *pSC = new SubComponent(iCurID);
            /*
            int iMin = 0;
            cellchain::const_iterator itc;
            for (itc = ity->second.begin(); itc != ity->second.end(); ++itc) {
                const distlist &dl = m_mDistances[*itc];
                if (dl.size() > 1) {
                    iMin = 2;
                }
            }
            if (iMin > 1) {
                cellchain vNew;
                cellchain::const_iterator itc;
                for (itc = ity->second.begin(); itc != ity->second.end(); ++itc) {
                    const distlist &dl = m_mDistances[*itc];
                    if ((int)dl.size() >= iMin) {
                        m_mSubCompForCell[*itc] = iCurID;
                        vNew.insert(*itc);
                    }
                }
                pSC->setCells(vNew);
            } else {
                cellchain vNew;
                int iR = (1.0*ity->second.size()*rand())/RAND_MAX;
                cellchain::const_iterator itc;
                for (itc = ity->second.begin(); (iR >= 0) && (itc != ity->second.end()); ++itc,--iR) {
                    if (iR == 0) {
                        m_mSubCompForCell[*itc] = iCurID;
                        vNew.insert(*itc);
                    }
                }
                pSC->setCells(vNew);
            }
            */
            pSC->setCells(ity->second);
            m_mSubComponents[iCurID] = pSC;
            ++iCurID;
        }
    }

    // correct lookup
    m_mSubCompForCell.clear();
    subcompmap::const_iterator it3;
    for (it3 = m_mSubComponents.begin(); it3 != m_mSubComponents.end(); ++it3) {
        const cellchain &vc = it3->second->getChain();
        cellchain::const_iterator itc3;
        for (itc3 = vc.begin(); itc3 != vc.end(); ++itc3) {
            m_mSubCompForCell[*itc3] = it3->first;
        }
    }

}


//----------------------------------------------------------------------------
// deleteUnneeded
//
void SubComponentManager::deleteUnneeded() {
    cellchain sKeep;
    cellchain sKill;
    subcompmap::const_iterator it3;

    // mark multiples and lonely singles for keep, mark single partners of multiple for kill
    for (it3 = m_mSubComponents.begin(); it3 != m_mSubComponents.end(); ++it3) {
        const cellchain &vc = it3->second->getChain();
        cellchain::const_iterator itc;
        int iMin = 0;
        for (itc = vc.begin(); itc != vc.end(); ++itc) {
            const distlist &dl = m_mDistances[*itc];
            if (dl.size() > 1) {
                iMin = 2;
            }
        }
        if (iMin > 1) {
            cellchain::const_iterator itc;
            for (itc = vc.begin(); itc != vc.end(); ++itc) {
                const distlist &dl = m_mDistances[*itc];
                if ((int)dl.size() >= iMin) {
                    sKeep.insert(*itc);
                } else {
                    sKill.insert(*itc);
                }
            }    
        } else {
            if (vc.size() == 1) {
                sKeep.insert(*(vc.begin()));
            }
        }
    }

    //    printf("marked %zd for keep, %zd for kill\n", sKeep.size(), sKill.size());
    // make sure targets of kept are kept; propagate
    bool bFirst = true;
    cellchain sAdditionalKeep;
    cellchain sTest(sKeep);
    cellchain::const_iterator itc;
    while (bFirst || (sAdditionalKeep.size() > 0)) {

        sKeep.insert(sAdditionalKeep.begin(), sAdditionalKeep.end());
        sAdditionalKeep.clear();
        for (itc = sTest.begin(); itc != sTest.end(); ++itc) {
            const distlist &dl = m_mDistances[*itc];
            distlist::const_iterator itd;
            for (itd = dl.begin(); itd != dl.end(); ++itd) {
                if (sKeep.find(itd->first) == sKeep.end()) {
                    sAdditionalKeep.insert(itd->first);
                }
            }
        }
        
        sTest.insert(sAdditionalKeep.begin(), sAdditionalKeep.end());
        bFirst = false;
    }

    // make sure targets of killed are killed; propagate
    bFirst = true;
    cellchain sAdditionalKill;
    while (bFirst || (sAdditionalKill.size() > 0)) {
        sKill.insert(sAdditionalKill.begin(), sAdditionalKill.end());
        sAdditionalKill.clear();
        for (itc = sKill.begin(); itc != sKill.end(); ++itc) {
            const distlist &dl = m_mDistances[*itc];
            distlist::const_iterator itd;
            for (itd = dl.begin(); itd != dl.end(); ++itd) {
                if (sKill.find(itd->first) == sKill.end()) {
                    if (sKeep.find(itd->first) == sKeep.end()) {
                        sAdditionalKill.insert(itd->first);
                    }
                }
            }
        }
        bFirst = false;
    }
        
    // actually kill entries in sKill
    for (it3 = m_mSubComponents.begin(); it3 != m_mSubComponents.end(); ++it3) {
        cellchain &vc = it3->second->getChain();
        cellchain::const_iterator itc3;
        for (itc = sKill.begin(); itc != sKill.end(); ++itc) {
            cellchain::iterator itk = vc.find(*itc);
            if (itk != vc.end()) {
                vc.erase(*itk);
            }
        }
    }
    sKill.clear();
            

    // delete unneeded singlets
    for (it3 = m_mSubComponents.begin(); it3 != m_mSubComponents.end(); ++it3) {
        const cellchain &vc = it3->second->getChain();
        if (vc.size() > 1) {
            bool bSearchKept = true;
            cellchain::const_iterator itc3;
            for (itc3 = vc.begin(); bSearchKept && (itc3 != vc.end()); ++itc3) {
                if (sKeep.find(*itc3) == sKeep.end()) {
                    sKeep.insert(*itc3);
                    sKeep.insert(m_mDistances[*itc3].begin()->first);
                    bSearchKept = false;
                }
            }
            for (itc3 = vc.begin(); itc3 != vc.end(); ++itc3) {
                if (sKeep.find(*itc3) == sKeep.end()) {
                    if (sKeep.find(*itc3) == sKeep.end()) {
                        sKill.insert(*itc3);
                    }
                    if (sKeep.find(m_mDistances[*itc3].begin()->first) == sKeep.end()) {
                        sKill.insert(m_mDistances[*itc3].begin()->first);
                    }
                }
            }
        }
    }

    // actually kill entries in sKill
    for (it3 = m_mSubComponents.begin(); it3 != m_mSubComponents.end(); ++it3) {
        cellchain &vc = it3->second->getChain();
        cellchain::const_iterator itc3;
        for (itc = sKill.begin(); itc != sKill.end(); ++itc) {
            cellchain::iterator itk = vc.find(*itc);
            if (itk != vc.end()) {
                vc.erase(*itk);
            }
        }
    }
    sKill.clear();

    // correct lookup
    m_mSubCompForCell.clear();
    subcompmap::const_iterator it5;
    for (it5 = m_mSubComponents.begin(); it5 != m_mSubComponents.end(); ++it5) {
        const cellchain &vc = it5->second->getChain();
        cellchain::const_iterator itc3;
        for (itc3 = vc.begin(); itc3 != vc.end(); ++itc3) {
            m_mSubCompForCell[*itc3] = it5->first;
        }
    }

}




//----------------------------------------------------------------------------
// eliminateDuplicates
//
void SubComponentManager::eliminateDuplicates() {
    intset sBad;

    // find duplicates
    subcompmap::iterator it1;
    for (it1 = m_mSubComponents.begin(); it1 != m_mSubComponents.end(); ++it1) {

        subcompmap::iterator it2 = it1;
        ++it2;
        for (; it2 != m_mSubComponents.end(); ++it2) {

            bool bEqual = true;
            if (it1->second->getChain().size() == it2->second->getChain().size()) {
                cellchain::const_iterator itc1 = it1->second->getChain().begin();
                cellchain::const_iterator itc2 = it2->second->getChain().begin();
                
                while ((*itc1 == *itc2) && (itc1 != it1->second->getChain().end())) {
                    ++itc1;
                    ++itc2;
                }
                if (itc1 != it1->second->getChain().end()) {
                    bEqual = false;
                }
            } else {
                bEqual = false;
            }
            if (bEqual) {
                sBad.insert(it2->first);
            } 
        }
    }

    // delete duplicates
    intset::iterator iti;
    for (iti = sBad.begin(); iti != sBad.end(); ++iti) {
        subcompmap::iterator itm = m_mSubComponents.find(*iti);
        if (itm != m_mSubComponents.end()) {
            m_mSubComponents.erase(itm);
        }
    }
          
    // correct lookup
    m_mSubCompForCell.clear();
    subcompmap::const_iterator it3;
    for (it3 = m_mSubComponents.begin(); it3 != m_mSubComponents.end(); ++it3) {
        const cellchain &vc = it3->second->getChain();
        cellchain::const_iterator itc3;
        for (itc3 = vc.begin(); itc3 != vc.end(); ++itc3) {
            m_mSubCompForCell[*itc3] = it3->first;
        }
    }
    
}
    


//----------------------------------------------------------------------------
// showSubComponents
//
void SubComponentManager::showSubComponents() {
    subcompmap::const_iterator it;
    for (it = m_mSubComponents.begin(); it != m_mSubComponents.end(); ++it) {
        printf("%d : ", it->first);
        
        const cellchain &vChain = it->second->getChain();
        cellchain::const_iterator itc;
        for (itc = vChain.begin(); itc != vChain.end(); ++itc) {
            printf("  %d (", *itc);
            const distlist &dl = m_mDistances[*itc];

            distlist::const_iterator it2;
            for (it2 = dl.begin(); it2 != dl.end(); ++it2) {
                printf(" %d", m_mSubCompForCell[it2->first]);
            }
            printf(") ");
        }
        
        printf("\n");
    }
}

//----------------------------------------------------------------------------
// createDistanceMap
//
void SubComponentManager::createDistanceMap(distancemap &mDistances) {

    intset sAll;
    subcompmap::const_iterator it;
    for (it = m_mSubComponents.begin(); it != m_mSubComponents.end(); ++it) {
        const cellchain &vChain = it->second->getChain();
        cellchain::const_iterator itc;
        sAll.insert(vChain.begin(), vChain.end());
    }
    printf("\n");
    
    for (it = m_mSubComponents.begin(); it != m_mSubComponents.end(); ++it) {

        const cellchain &vChain = it->second->getChain();
        cellchain::const_iterator itc;
        for (itc = vChain.begin(); itc != vChain.end(); ++itc) {
            const distlist &dl = m_mDistances[*itc];
            distlist dNew;
            distlist::const_iterator itl;
            
                

            for (itl = dl.begin(); itl != dl.end(); ++itl) {
                if (sAll.find(itl->first) != sAll.end()) {
                    m_mSubCompForCell[itl->first];
                    mDistances[*itc][itl->first] = itl->second;
                } else {
                }
            }

 
        }
    }

}
 
