#include <stdio.h>
#include <string.h>

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include <omp.h>


#include "types.h"
#include "strutils.h"
#include "geomutils.h"
#include "ParamReader.h"

#include "SCell.h"
#include "SCellGrid.h"
#include "Geography.h"

#include "ComponentBuilder.h"

#include "CoastalDistances.h"



//----------------------------------------------------------------------------
// createInstance
//
CoastalDistances *CoastalDistances::createInstance(SCellGrid *pCG, double dMaxDist, boxlist &vBoxes, bool bVerbosity) {
    CoastalDistances *pCD = new CoastalDistances(pCG, dMaxDist, vBoxes, bVerbosity);
    int iResult = pCD->init();
    if (iResult != 0) {
        delete pCD;
        pCD = NULL;
    }
    return pCD;
}


//----------------------------------------------------------------------------
// constructor
//
CoastalDistances::CoastalDistances(SCellGrid *pCG, double dMaxDist, boxlist &vBoxes, bool bVerbosity) 
    : m_pCG(pCG),
      m_iNumCells(m_pCG->m_iNumCells),
      m_dMaxDist(dMaxDist),
      m_vBoxes(vBoxes),
      m_bVerbose(bVerbosity) {
    

}


//----------------------------------------------------------------------------
// destructor
//
CoastalDistances::~CoastalDistances() {
}

//----------------------------------------------------------------------------
// init
//
int CoastalDistances::init() {
    int iResult = 0;
    
    iResult = collectCoastCells();
    
    if (iResult == 0) {
        
        printf("Collected %zd coastal cells\n", m_vCoastCells.size());
                        
                            
        iResult = findCoastalNeighborhoods();
        if (iResult == 0) {
            iResult = removeConnectedComponents();
            if (iResult == 0) {
                iResult = selectShortestComponentConnections();
                if (iResult == 0) {
                    // OK
                } else {
                    printf("Couldn't select shortest component connections\n");
                }
            } else {
                printf("Couldn't remove connected components\n");
            }
            
        } else {
            printf("Couldn't find coastal neighborhoods\n");
        }        
        
    } else {
        printf("Couldn't collect coast cells\n");
    } 
       
    return iResult;

}



//----------------------------------------------------------------------------
// isCoastalCell
//
bool CoastalDistances::isCoastalCell(int iIndex) {

    bool bCoastal = false;
    bool bIsInBoxes = false;
    boxlist::const_iterator itB;
    for (itB = m_vBoxes.begin(); !bIsInBoxes && (itB != m_vBoxes.end()); ++itB) {
        if ((m_pCG->m_pGeography->m_adLongitude[iIndex] > itB->dLonMin) &&
            (m_pCG->m_pGeography->m_adLatitude[iIndex]  > itB->dLatMin) &&
            (m_pCG->m_pGeography->m_adLongitude[iIndex] < itB->dLonMax) &&
            (m_pCG->m_pGeography->m_adLatitude[iIndex]  < itB->dLatMax)) {
            bIsInBoxes = true;
        }
    }

    if (bIsInBoxes && (m_pCG->m_pGeography->m_adAltitude[iIndex] > 0)) {
            
        SCell &sCell = m_pCG->m_aCells[iIndex];
        for (int k = 0; (!bCoastal) && (k < sCell.m_iNumNeighbors); ++k) { 
            int iCurSub =  m_pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID;
            if (m_pCG->m_pGeography->m_adAltitude[iCurSub] < 0) {
                bCoastal = true;
            }
        }
    }
    return bCoastal;
}



//----------------------------------------------------------------------------
// collectCoastCells
//
int CoastalDistances::collectCoastCells() {

    int iResult = 0;
    printf("collectCoastCells\n");
    m_vCoastCells.clear();
    cellvec *avCoastCells = new cellvec[omp_get_max_threads()];
    
#pragma omp parallel for
    for (int iIndex =  0; iIndex < m_iNumCells; iIndex++) {
        int iT = omp_get_thread_num();

        if (isCoastalCell(iIndex)) {
            avCoastCells[iT].push_back(iIndex);
        }
    }

    // accumulate from threads
    for (int i = 0; i < omp_get_max_threads(); ++i) {
        m_vCoastCells.insert(m_vCoastCells.end(), avCoastCells[i].begin(), avCoastCells[i].end());
    }


    // fill look-up id -> coast index
    for (uint j = 0; j < m_vCoastCells.size(); ++j) {
        m_mIdToIndex[m_vCoastCells[j]] = j;
    }
    

    delete[] avCoastCells;
    return iResult;
}


//----------------------------------------------------------------------------
// findCoastalNeighborhoods
//
int CoastalDistances::findCoastalNeighborhoods() {
    int iResult = 0;
    printf("findCoastalNeighborhoods\n");
    m_mCoastRanges.clear();
    distlist *vDist = new distlist[omp_get_max_threads()];
    Geography *pGeo = m_pCG->m_pGeography;
    for (uint iCurIndex = 0; iCurIndex < m_vCoastCells.size(); ++iCurIndex) {
        int iCurCell = m_vCoastCells[iCurIndex];
        if (m_bVerbose) {
            if (iCurIndex%100 == 0) {
                printf("\r                          \rCell #%d: %d", iCurIndex, iCurCell);fflush(stdout);
            }
        }
        for (int iT = 0; iT < omp_get_max_threads(); ++iT) {
            vDist[iT].clear();
        }
        double dLon0 = pGeo->m_adLongitude[iCurCell];
        double dLat0 = pGeo->m_adLatitude[iCurCell];

        cellset *asCoastRanges = new cellset[omp_get_max_threads()];

#pragma omp parallel for
        for (int iIndex = 0; iIndex < m_iNumCells; ++iIndex) {
            double dLon = pGeo->m_adLongitude[iIndex];
            double dLat = pGeo->m_adLatitude[iIndex];
            
            if  ((iIndex != iCurCell) && isCoastalCell(iIndex)) {
                double dCurDist = spherdist(dLon0*M_PI/180, dLat0*M_PI/180, dLon*M_PI/180, dLat*M_PI/180, pGeo->m_dRadius);
                if (dCurDist < m_dMaxDist) {
                    // save distance information
                    asCoastRanges[omp_get_thread_num()].insert(iIndex);
                    vDist[omp_get_thread_num()][iIndex]= dCurDist;
                }
            }
        }

        // accumulate
        for (int iT = 0; iT < omp_get_max_threads(); ++iT) {
            if (asCoastRanges[iT].size() > 0) {
                m_mCoastRanges[iCurCell].insert(asCoastRanges[iT].begin(), asCoastRanges[iT].end());
            }
            m_mDistances[iCurCell].insert(vDist[iT].begin(), vDist[iT].end());

        }
        delete[] asCoastRanges;

    }
    delete[] vDist;

    return iResult;
}


//----------------------------------------------------------------------------
// removeConnectedComponent
//
int CoastalDistances::removeConnectedComponent(int iKey, cellset &sRange) {
    int iResult = 0;

    cellset sCur[2];
    cellset sFinal;
    int iWhich = 0;
    sCur[iWhich].insert(iKey);
        
    while (sCur[iWhich].size() > 0) {
        cellset::const_iterator itc;
        cellset sUsed;
        for (itc = sCur[iWhich].begin(); itc != sCur[iWhich].end(); ++itc) {
            SCell &sCell = m_pCG->m_aCells[*itc];

            for (int k = 0; k < sCell.m_iNumNeighbors; ++k) { 
                int iCurSub =  m_pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID;
                if (sRange.find(iCurSub) != sRange.end()) {
                    sCur[1-iWhich].insert(iCurSub);
                }
            }
        }
        sUsed.insert(sCur[1-iWhich].begin(), sCur[1-iWhich].end());

        cellset::const_iterator it2;
        for (it2 = sUsed.begin(); it2 != sUsed.end(); ++it2) {
            if (*it2 != iKey) {
                sRange.erase(*it2);

                const distlist::iterator itd = m_mDistances[iKey].find(*it2);
                m_mDistances[iKey].erase(itd);
            }
        }

        sCur[iWhich].clear();

        iWhich = 1-iWhich;
    }
    return iResult;
}   


//----------------------------------------------------------------------------
// removeConnectedComponents
//
int CoastalDistances::removeConnectedComponents() {
    int iResult = 0;
    printf("removeConnectedComponents\n");
    int *aKeys = new int[m_mCoastRanges.size()];
    int i = 0;
    cellrange::iterator it;
    for (it = m_mCoastRanges.begin(); it != m_mCoastRanges.end(); ++it) {
        aKeys[i++] = it->first;
    }

#pragma omp parallel for
    for (uint i = 0; i < m_mCoastRanges.size(); ++i) {

        int iKey = aKeys[i];
        int iRes =  removeConnectedComponent(iKey, m_mCoastRanges[iKey]);
        if (iRes != 0) {
            printf("Failed to remove connected components of rangs [%d]\n", iKey);
        }
    }

    delete[] aKeys;
    return iResult;
}




//----------------------------------------------------------------------------
// showDistances
//
void CoastalDistances::showDistances() {
    showDistances(m_mDistances);
}


//----------------------------------------------------------------------------
// showDistances
//
void CoastalDistances::showDistances(const distancemap &mDistances) {
    printf("Showing %zd distanecs\n", mDistances.size());
    distancemap::const_iterator it;

    for (it = mDistances.begin(); it != mDistances.end(); ++it) {
        printf("Orig %d(%s): (%zd entries)\n", it->first, ""/*mnames[it->first].c_str()*/, it->second.size());
        
        distlist::const_iterator itd;
        for (itd = it->second.begin(); itd != it->second.end(); ++itd) {
            printf("   %d(%s) : %f\n", itd->first, ""/*mnames[itd->first].c_str()*/, itd->second);
        }
        
    }
}

bool endpointcomp (const std::pair<gridtype, double>  &lhs, const std::pair<gridtype, double>  &rhs) {return (lhs.second < rhs.second) || (lhs.first < rhs.first);};

//----------------------------------------------------------------------------
// selectShortestComponentConnections
//
//  typedef std::pair<gridtype, double>  endpoint;
//  typedef _std::vector<endpoint>        distlist;
//  typedef std::map<gridtype, distlist> distancemap;
//
int CoastalDistances::selectShortestComponentConnections() {
    int iResult = 0;

    ComponentBuilder *pCB = ComponentBuilder::createInstance(m_pCG, m_vCoastCells);
    if (pCB != NULL) {
        printf("Found %d components\n", pCB->getNumComponents());



        printf("selectShortestComponentConnections\n");
        distancemap mReduced0;

        if (m_bVerbose) {
          printf("====vvv===\n");
          showDistances();
          printf("===^^^===\n");
        }
        printf("SORTING NOW\n"); fflush(stdout);
        distancemap::const_iterator it;
        for (it = m_mDistances.begin(); it != m_mDistances.end(); ++it) {
            const distlist &dl = it->second;
            
            // component -> endpointlist
            distancemap mTemp;
            for (distlist::const_iterator iti = dl.begin(); iti != dl.end(); ++iti) {
                int iOtherID = iti->first;
                mTemp[pCB->getComponentFor(iOtherID)][iti->first] = iti->second;
            }
            // i1d -> endpointlist
            distancemap::const_iterator itt;
            for (itt = mTemp.begin(); itt != mTemp.end(); ++itt) {
                /*
                if (itt->second.size() > 0) {
                    std::vector<std::pair<gridtype, double> >v(itt->second.begin(), itt->second.end());
                    std::sort(v.begin(), v.end(), endpointcomp);
                    mReduced0[it->first][v[0].first] = v[0].second;
                }
                */
                double dMin = 1.0e9;
                int    iMin = -1;
                for (distlist::const_iterator iti = itt->second.begin(); iti != itt->second.end(); ++iti) {
                    if (iti->second < dMin) {
                        dMin = iti->second;
                        iMin = iti->first;
                    }
                }
                if (iMin >= 0) {
                    mReduced0[it->first][iMin] = dMin;
                }
                
                // why can't we sort?
                // std::sort(itt->second.begin(),  itt.second.end(), endcomp);
            }
        }
        printf("SORTING DONE\n"); fflush(stdout);

        if (m_bVerbose) {
          printf("----vvv---\n");
          showDistances(mReduced0);
          printf("----^^^---\n");
        }
        
        distancemap::const_iterator itt;
        for (itt = mReduced0.begin(); itt != mReduced0.end(); ++itt) {
            for (distlist::const_iterator iti =itt->second.begin(); iti != itt->second.end(); ++iti) {
                int iOther = iti->first;
                distancemap::const_iterator itd = mReduced0.find(iOther);
                if (itd != mReduced0.end()) {
                    distlist::const_iterator iti2 = itd->second.find(itt->first); 
                    if (iti2 != itd->second.end()) {
                        // if we don't exclude targets on the same coast we get connections from
                        // one side of an island to the other.
                        // if we do exclude it we get no bay crossings, i.e. no crossing of 
                        // gibraltar when africa is connected to europe
                        if (pCB->getComponentFor(itt->first) != pCB->getComponentFor(iti->first)) {
                            m_mReduced[itt->first][iti->first] = iti->second;
                        }                
                    }
                }
            }
        }
        if (m_bVerbose) {
          printf("++++vvv++++\n");
          showDistances(m_mReduced);
          printf("++++^^^+++\n");
        }
        delete pCB;
    } else {
        printf("Couldn't create components\n");
        iResult = -1;
    }

    return iResult;
}
                                    

//----------------------------------------------------------------------------
// getNeighborWater
//
void CoastalDistances::getNeighborWater(int iCelliD, intset &sWater) {
    SCell &sCell = m_pCG->m_aCells[iCelliD];
    for (int k = 0; k < sCell.m_iNumNeighbors; ++k) { 
        int iCurSub =  m_pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID;
        if (iCurSub >= 0) {
            
            if (m_pCG->m_pGeography->m_adAltitude[iCurSub] < 0) {
                sWater.insert(iCurSub);
            }
        }
    }
}


//----------------------------------------------------------------------------
// createNeighborList
//
int CoastalDistances::createNeighborList() {
    int iResult = 0;

    intset sTemp(m_vCoastCells.begin(), m_vCoastCells.end());
     cellvec::const_iterator it;    
     for (it = m_vCoastCells.begin(); (iResult == 0) && (it != m_vCoastCells.end()); ++it) {
        SCell &sCell = m_pCG->m_aCells[*it];
        intset sWater;
        intset sCoast;
        for (int k = 0; k < sCell.m_iNumNeighbors; ++k) { 
            int iCurSub =  m_pCG->m_aCells[sCell.m_aNeighbors[k]].m_iGlobalID;
            if (iCurSub >= 0) {
                if (m_pCG->m_pGeography->m_adAltitude[iCurSub] > 0) {
                    if (sTemp.find(iCurSub) != sTemp.end()) {
                        sCoast.insert(iCurSub);
                    }
                } else {
                    sWater.insert(iCurSub);
                }
            }
        }

        intset sFinal;

        if (sCoast.size() > 2) {
            intset::const_iterator it2;    
            for (it2 = sCoast.begin(); it2 != sCoast.end(); ++it2) {
                intset sOtherWater;
                getNeighborWater(*it2, sOtherWater);
               
                std::vector<int> v(sWater.size());
                std::vector<int>::iterator it = std::set_intersection(sWater.begin(), sWater.end(), sOtherWater.begin(), sOtherWater.end(), v.begin());
                /*
                v.resize(it-v.begin());
                if (v.size() > 0) {
                */
                if (it != v.begin()) {
                    sFinal.insert(*it2);
                }
            }
        } else {
            sFinal.insert(sCoast.begin(), sCoast.end());
        }

        m_mNeighbors[*it] = sFinal;
            
    }
    return iResult;
}
