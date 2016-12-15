#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <set>

#include <omp.h>

#include "types.h"
#include "colors.h"
#include "BufWriter.h"
#include "SystemInfo.h"
#include "AncestorNode.h"
#include "AncGraphBase.h"
#include "RGeneration.h"
#include "DynAncGraphX.h"

/*
void listIDSet(const char *pCaption, idset sIDs) {
    printf("%s ", pCaption);
    for (idset_cit ii = sIDs.begin(); ii != sIDs.end(); ++ii) {
        printf(" %ld", *ii);
    }
    printf("\n");

}
*/
//----------------------------------------------------------------------------
// createInstance
//   
DynAncGraphX *DynAncGraphX::createInstance() {
    DynAncGraphX *pDAG = new DynAncGraphX();
    return pDAG;
}


//----------------------------------------------------------------------------
// createInstance
//   
DynAncGraphX *DynAncGraphX::createInstance(const char *pAncData, ulong iBlockSize, uint iAncSize) {
    DynAncGraphX *pDAG = new DynAncGraphX();
    int iResult = pDAG->init(pAncData, iBlockSize, iAncSize);
    if (iResult != 0) {
        delete pDAG;
        pDAG = NULL;
    }
    return pDAG;
}
  
 
//----------------------------------------------------------------------------
// createInstance
//   
DynAncGraphX *DynAncGraphX::createInstance(const char *pAncData, ulong iBlockSize, uint iAncSize, const char *pOracleFile, unsigned char *aChecksum, int iCSLen) {
    DynAncGraphX *pDAG = new DynAncGraphX();
    int iResult = pDAG->init(pAncData, iBlockSize, iAncSize, pOracleFile, aChecksum, iCSLen);
    if (iResult != 0) {
        delete pDAG;
        pDAG = NULL;
    }
    return pDAG;
}
   
 
//----------------------------------------------------------------------------
// constructor
//   
DynAncGraphX::DynAncGraphX() 
    : m_fAnc(NULL) ,
      m_lFileSize(0),
      m_lListOffset(0),
      m_iBlockSize(0),
      m_iAncSize(0), 
      m_aAncBuf(NULL),
      m_bKillPoints(false) {

    m_bVer4 = false;
}


//----------------------------------------------------------------------------
// destructor
//   
DynAncGraphX::~DynAncGraphX() {
    if (m_fAnc != NULL) {
        fclose(m_fAnc);
    }
    ancnodelist::iterator it;
    for (it = m_mIndex.begin(); it != m_mIndex.end(); ++it) {
        delete it->second;
    }

    if (m_aAncBuf != NULL) {
        delete[] m_aAncBuf;
    }
}


//----------------------------------------------------------------------------
// init
//   
int DynAncGraphX::init(const char *pAncData, ulong iBlockSize, uint iAncSize) {
    int iResult = -1;
    m_iBlockSize = iBlockSize;
    m_iAncSize = iAncSize;
    m_aAncBuf = new idtype[m_iAncSize*m_iBlockSize];
    m_fAnc = fopen(pAncData, "rb");
    if (m_fAnc != NULL) {
        fseek(m_fAnc, 0, SEEK_END);
        m_lFileSize = ftell(m_fAnc);
        fseek(m_fAnc, 0, SEEK_SET);
        if ((m_lFileSize % m_iAncSize) == 0) {
            iResult = createOracle();
        } else {
            printf("%sAnc file size should be diviible by %d...\n", RED, m_iAncSize);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// init
//   
int DynAncGraphX::init(const char *pAncData, ulong iBlockSize, uint iAncSize, const char *pOracleFile, unsigned char *aChecksum, int iCSLen) {
    int iResult = -1;
    m_iBlockSize = iBlockSize;
    m_iAncSize = iAncSize;
    m_aAncBuf = new idtype[m_iAncSize*m_iBlockSize];
    m_fAnc = fopen(pAncData, "rb");
    if (m_fAnc != NULL) {
        fseek(m_fAnc, 0, SEEK_END);
        m_lFileSize = ftell(m_fAnc);
        fseek(m_fAnc, 0, SEEK_SET);
        if ((m_lFileSize % m_iAncSize) == 0) {
            iResult = loadOracle(pOracleFile, aChecksum, iCSLen);
        } else {
            printf("%sAnc file size should be diviible by %d...\n", RED, m_iAncSize);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// findLowerBound
//   find position in m_aAncBuf less than iResolution items before iVal
//   iResolution must be greater than 0!
//
int DynAncGraphX::findLowerBound(int iLo, int iHi, int iResolution, idtype iVal) {
    while ((iHi - iLo) > iResolution) {
        int iCur = (iHi + iLo)/2;
        if (m_aAncBuf[m_iAncSize*iCur] < iVal) {
            iLo = iCur;
        } else {
            iHi = iCur;
        }
    }
    return iLo;
}


//----------------------------------------------------------------------------
// loadAncs
//   load ancestor info (ID, MomID, DadID) into map mAncs
//
int DynAncGraphX::loadAncs(idset &sItems, std::map<idtype, ancdataX > &mAncs) {
    int iResult = 0;

    // determine which blocks to load
    idset sBlocks;
    idset_cit it;
    idset_cit it2;
    for (it = sItems.begin(); (iResult == 0) && (it != sItems.end()); ++it) {
        if (findAncestorNode(*it) == NULL) {
            oracle::const_iterator ito = m_mOracle.lower_bound(*it);
            if (ito != m_mOracle.end()) {
                sBlocks.insert(ito->first);
            } else {
                printf("%s[DynAncGraph::loadAncs]no upper bound found for id %ld\n", RED, *it);
                iResult = -1;
            }
        }
    }

    it = sItems.begin();
    //    printf("Searching for id %d", *it);
    idset_cit itB = sBlocks.begin();
    while ((iResult == 0) && (itB != sBlocks.end())) {
        idtype iID = *itB;
        filepos iMin = m_mOracle[iID];
        if (iMin*sizeof(idtype) >= m_lFileSize) {
            printf("seeking pos %lu (%ld*%ld)\n", iMin, iMin*sizeof(idtype), m_lFileSize);
        }
        fseek(m_fAnc, iMin*sizeof(idtype), SEEK_SET);
        //        printf("Reading block for %d starting at %d (real %zd)\n", iID, iMin, iMin*sizeof(int));
        size_t iRead = fread(m_aAncBuf, m_iAncSize*sizeof(idtype), m_iBlockSize, m_fAnc);
        if (iRead > 0) {
            // find good starting  point
	    /* this seems to run slower than direct search
            int iResolution = 32;
            int i = findLowerBound(0, iRead, iResolution, *it);
            */
            // now pick only the selected ones
            uint i = 0;
            while (it != sItems.end() && (*it <= iID) && (i < iRead) && (iResult == 0)) {
                while ((i < iRead) && (m_aAncBuf[m_iAncSize*i] < *it)) {
                    i++;
                }

                if (m_aAncBuf[m_iAncSize*i] == *it) {
                    if (m_iAncSize == 4) {
                        mAncs[m_aAncBuf[m_iAncSize*i]] = ancdataX(m_aAncBuf[m_iAncSize*i+1], m_aAncBuf[m_iAncSize*i+2], m_aAncBuf[m_iAncSize*i+3]);
                    } else {
                        mAncs[m_aAncBuf[m_iAncSize*i]] = ancdataX(m_aAncBuf[m_iAncSize*i+1], m_aAncBuf[m_iAncSize*i+2]);
                    }
                    ++it;
                    ++i; 
                    // slower  i = findLowerBound(i, iRead, iResolution, *it);
                } else {
                    printf("%s: Didn't find id %ld in block for %ld starting at %ld\n", RED, *it, iID, iMin);
                    iResult = -1;
                }
            }	           
        } else {
            printf("%sRead error in Anc file\n", RED);
            iResult = -1;
        }
        ++itB;
    }
        
    return iResult;
}


//----------------------------------------------------------------------------
// calcKillPoints
// 
int DynAncGraphX::calcKillPoints(idset &sInitial, int iType, int iLatency) {
    int iResult = 0;
    uint iPrev = 0;

    if (iType > 0) {
        // currently here: 
        m_bKillPoints = true;

        m_bVer4 = false;
        switch (iType) {
        case 1:
            iResult = backReach(sInitial);
            break;
        case 2:
            iResult = backReach2(sInitial);
            break;
        case 3:
        case 5:
            iResult = backReach3(sInitial);
            break;
        case 4:
        case 6:
            m_bVer4 = true;
            iResult = backReach4(sInitial, iLatency);
            break;
        case 7:
            iResult = backReach7(sInitial);
            break;
        default:
            printf("%sIllegal backreach value: %d\n", RED, iType);
            iResult = -1;
        }

        printf("\n"); fflush(stdout);
        int iFirst = 0;
        //:::@@--- here something might need to be corrected for special cases
        if (iResult == 0) {
            for (uint i = 0; i < m_vBackReach.size(); i++) {
                uint iLatest = i;
                for (uint j = iPrev; j < m_vBackReach.size(); j++) {
                    if ((i == m_vBackReach[j]) && (iLatest < j)) {
                        iLatest =  j;
                    }
                }

                if (iLatest == i) {
                    iLatest = iPrev;
                }
                m_mKillPoints[iLatest].insert(i);
        
                iPrev = iLatest;
                if (iFirst == 0) {
                    iFirst = iPrev;
                }
            }
            // if the first N RGenerations are "selfcontained", they will all be
            // saved under m_mKillPoints[0].
            // Here we move everything from m_mKillPoints[0] to m_mKillPoints[iFirst].
            m_mKillPoints[iFirst].insert(m_mKillPoints[0].begin(), m_mKillPoints[0].end());
            m_mKillPoints.erase(0);

            // the last killpoint is for all remaining generations
            uint iLast = 0;
            if (!m_mKillPoints.empty()) {
                iLast = *(m_mKillPoints.rbegin()->second.rbegin());
            } else {
                printf("no killopints found; making final one\n");
            }
            for (uint i = iLast+1; i < m_vGenerations.size(); i++) {
                m_mKillPoints[(int)m_vGenerations.size()-1].insert(i);
            }
            
        }
        
        printf("NumKillpoints at end of calcKillPoints: %zd\n", m_mKillPoints.size());

    }
    return iResult;
}


//----------------------------------------------------------------------------
// backReach
//   for each "backward" generation, determine the "earliest" generation referenced by parents
//   in the generation.
//   ("backwards": latest births:earliest backwards generation)
//   we traverse the tree startingh from a set and work our way backwards via the 
//   parents of this set. It is possible that such a parent has already been a parent
//   of an "earlier" child. In this case the backreach is the index of the "earlier" generation.
//
//   first implementation with many map and set operations (childmap, sCurrent)
//
int DynAncGraphX::backReach(idset &sInitial) {
    int iResult = 0;
    int ig = 0;
    idset sCurrent(sInitial);
    m_sSelected.insert(sInitial.begin(), sInitial.end());

    m_vGenerations.clear();
    childmap mChildMap;

    //  std::vector<int>    vBack;
    printf("checking back reach (%zd curgen, %zd childmap)\n", sCurrent.size(), mChildMap.size());

    while ((sCurrent.size() > 0) || (mChildMap.size() > 0)) {
        if (sCurrent.size() >0) {
            m_vGenerations.push_back(sCurrent);

            printf("Generation %5d: %10zd candidates\r", ig, sCurrent.size());fflush(stdout);
        }
        // do we have to add children from a previous round?
        if (!mChildMap.empty()) {
            int iMinG = ig;
            // childmap's key are parents of the previous set
            childmap::iterator it;
            for (it = mChildMap.begin(); (iResult == 0) && (it != mChildMap.end()); ++it) {
                // check in which previous sets it appears; 
                // i.e. has it been a parent of an id in a "earlier" generation
                bool bSearch = true;
                // we are interested in smallest generation index; i.e. no need to look further than current min generation index
                for (int hh = 0; bSearch && (hh <= iMinG); ++hh) {
                    if (m_vGenerations[hh].find(it->first) != m_vGenerations[hh].end()) {
                        if (hh < iMinG) {
                            iMinG = hh;
                            bSearch = false;
                        }
                    }
                }
             
            }
            m_vBackReach.push_back(iMinG);
            mChildMap.clear();
        }

        idset sTemp;
        // now make child maps from the new nodes
        idset_cit it2;
        std::map<idtype, ancdataX> mAncs;
        iResult = loadAncs(sCurrent, mAncs);
        for (it2 = sCurrent.begin(); (iResult == 0) && (it2 != sCurrent.end()); ++it2) {
            // load anc elements for it2 from anc file
            // map<int, pair<int,int>> ancs
            idtype iMomID = mAncs[*it2].iMomID;
            idtype iDadID = 0;
            if (iMomID != 0) {
                iDadID = (idtype) labs(mAncs[*it2].iDadID);
            }

            if ((iMomID > 0)  && (iDadID > 0)) {
                mChildMap[iMomID].push_back(*it2);
                mChildMap[iDadID].push_back(*it2);
                // only make new nodes if they don't yet exist
                bool bSearching = true;
                for (uint hh = 0; bSearching && (hh < m_vGenerations.size()); ++hh) {
                    if (m_vGenerations[hh].find(iMomID) != m_vGenerations[hh].end()) {
                        bSearching = false;
                    }
                }
                if (bSearching) {
                    sTemp.insert(iMomID);
                }
                bSearching = true;
                for (uint hh = 0; bSearching && (hh < m_vGenerations.size()); ++hh) {
                    if (m_vGenerations[hh].find(iDadID) != m_vGenerations[hh].end()) {
                        bSearching = false;
                    }
                }
                if (bSearching) {
                    sTemp.insert(iDadID);
                }
            } else if ((iMomID == 0)  && (iDadID == 0)) {
                // nothing to do
            } else {
                printf("%s[backreach]Bad parents for %ld\n", RED, *it2);
                iResult = -1;
            }
        }
        
        sCurrent = sTemp;
        //      printf("At end o gen %d: scurr: %zd, childmap. %zd\n", ig, sCurrent.size(), m_mChildMap.size());
        ig++;
    }
    
    
    printf("Created backreach list\n");
    /*    
    printf("Generations: %zd, backreaches %zd\n", m_vGenerations.size(), m_vBackReach.size());
    for (uint hh = 0; hh < m_vBackReach.size(); ++hh) {
        printf("  %d: %zd candidates -> backreach %d\n", hh, m_vGenerations[hh].size(), m_vBackReach[hh]);
    }
    */
    return iResult;
}


//----------------------------------------------------------------------------
// backReach2
//   for each "backward" generation, determine the "earliest" generation referenced by parents
//   in the generation.
//   ("backwards": latest births:earliest backwards generation)
//   we traverse the tree startingh from a set and work our way backwards via the 
//   parents of this set. It is possible that such a parent has already been a parent
//   of an "earlier" child. In this case the backreach is the index of the "earlier" generation.
//
int DynAncGraphX::backReach2(idset &sInitial) {
    SystemInfo *pSys = SystemInfo::createInstance();
    
    // first build all generations
    int iResult = 0;
    uint ig = 0;
    double t0 = omp_get_wtime();
    idset sCurrent(sInitial);
    m_sSelected.insert(sInitial.begin(), sInitial.end());

    m_vGenerations.clear();
  
    printf("checking back reach (2) (%zd curgen)\n", sCurrent.size());

    while ((iResult == 0) && (sCurrent.size() > 0))  {
        m_vGenerations.push_back(sCurrent);
        double t1 = omp_get_wtime();
        printf("Generation %5d: %10zd candidates (%5.3f s) (mem use/peak %12lu / %12lu\r", ig, sCurrent.size(), t1-t0, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
        t0 = t1;

        idset sTemp;
        // now make child maps from the new nodes
        idset_cit it2;
        std::map<idtype, ancdataX> mAncs;
        iResult = loadAncs(sCurrent, mAncs);
        uint iMinG = ig;
        for (it2 = sCurrent.begin(); (iResult == 0) && (it2 != sCurrent.end()); ++it2) {

            // load anc elements for it2 from anc file
            // map<int, pair<int,int>> ancs
            idtype iMomID = mAncs[*it2].iMomID;
            idtype iDadID = 0;
            if (iMomID != 0) {
                iDadID = labs(mAncs[*it2].iDadID);
            }
            if ((iMomID > 0)  && (iDadID > 0)) {
                // only make new nodes if they don't yet exist
                bool bSearching = true;
                for (uint hh = 0; bSearching && (hh < m_vGenerations.size()); ++hh) {
                    //                    if ((iMomID >= *(m_vGenerations[hh].begin())) && (iMomID <= *(m_vGenerations[hh].rbegin()))) {
                        if (m_vGenerations[hh].find(iMomID) != m_vGenerations[hh].end()) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            bSearching = false;
                        }
                        //                    }
                }
                if (bSearching) {
                    sTemp.insert(iMomID);
                }
                bSearching = true;
                for (uint hh = 0; bSearching && (hh < m_vGenerations.size()); ++hh) {
                    //                    if ((iDadID >= *(m_vGenerations[hh].begin())) && (iDadID <= *(m_vGenerations[hh].rbegin()))) {
                        if (m_vGenerations[hh].find(iDadID) != m_vGenerations[hh].end()) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            bSearching = false;
                        }
                        //                    }
                }
                if (bSearching) {
                    sTemp.insert(iDadID);
                }
            } else if ((iMomID == 0)  && (iDadID == 0)) {
                // nothing to do
            } else {
                printf("%s[backreach2]Bad parents for %ld\n", RED, *it2);
                iResult = -1;
            }
        }
        /*printf("\n");*/ fflush(stdout);
        m_vBackReach.push_back(iMinG);

        sCurrent = sTemp;

        ig++;
    }
    delete pSys;
    return iResult;
}


//----------------------------------------------------------------------------
// backReach3
//   for each "backward" generation, determine the "earliest" generation referenced by parents
//   in the generation.
//   ("backwards": latest births:earliest backwards generation)
//   we traverse the tree startingh from a set and work our way backwards via the 
//   parents of this set. It is possible that such a parent has already been a parent
//   of an "earlier" child. In this case the backreach is the index of the "earlier" generation.
//
//   array of sets for current (to prevent copying of large sets)
//
int DynAncGraphX::backReach3(idset &sInitial) {
    SystemInfo *pSys = SystemInfo::createInstance();


    int iResult = 0;
    uint ig = 0;
    double t0 = omp_get_wtime();
   
    // fill the current set with the initial ids 
    idset asCurrent[2];
    asCurrent[0].insert(sInitial.begin(), sInitial.end());
    // current set is set 0
    int iCur = 0;

    // remember the initial ids as the selected ones
    m_sSelected.insert(sInitial.begin(), sInitial.end());
   
    // make sure the vector of generations is clean
    m_vGenerations.clear();

    printf("checking back reach (3) (%zd curgen)\n", asCurrent[iCur].size());
    double tLoadAncs = 0;
    double tGenBuild = 0;
    while ((iResult == 0) && (asCurrent[iCur].size() > 0))  {
        // the newest rgeneration is the current set
        m_vGenerations.push_back(asCurrent[iCur]);
        double t1 = omp_get_wtime();
        printf("Generation %5d: %10zd candidates (%5.3f s) (mem use/peak %12lu / %12lu)\r", ig, asCurrent[iCur].size(), omp_get_wtime()-t0, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
        t0 = t1;
        //        listIDSet("IDs: ", asCurrent[iCur]);
        //        printf("::: ids gen %d\n", ig);

        // now make child maps from the new nodes
        idset_cit it2;
        std::map<idtype, ancdataX> mAncs;
        double t0 = omp_get_wtime();
        iResult = loadAncs(asCurrent[iCur], mAncs);
        tLoadAncs += omp_get_wtime()-t0;
        uint iMinG = ig;
        for (it2 = asCurrent[iCur].begin(); (iResult == 0) && (it2 != asCurrent[iCur].end()); ++it2) {
            double t1 = omp_get_wtime();
            // load anc elements for it2 from anc file
            // map<int, pair<int,int>> ancs
            idtype iMomID = mAncs[*it2].iMomID;
            idtype iDadID = 0;
            if (iMomID != 0) {
                iDadID = labs(mAncs[*it2].iDadID);
            }

            //            printf("::: id %ld: m%ld d%ld\n", *it2, iMomID, iDadID);
            if ((iMomID > 0)  && (iDadID > 0)) {
              
                // only make new nodes if they don't yet exist
                bool bSearching = true;
                for (uint hh = 0; bSearching && (hh < m_vGenerations.size()); ++hh) {
                    if ((iMomID >= *(m_vGenerations[hh].begin())) && (iMomID <= *(m_vGenerations[hh].rbegin()))) {
                        if (m_vGenerations[hh].find(iMomID) != m_vGenerations[hh].end()) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            bSearching = false;
                        }
                    }
                }
                if (bSearching) {
                    //                    printf("::: ids ins %ld\n", iMomID);
                    asCurrent[1-iCur].insert(iMomID);
                }
                bSearching = true;
                for (uint hh = 0; bSearching && (hh < m_vGenerations.size()); ++hh) {
                    if ((iDadID >= *(m_vGenerations[hh].begin())) && (iDadID <= *(m_vGenerations[hh].rbegin()))) {
                        if (m_vGenerations[hh].find(iDadID) != m_vGenerations[hh].end()) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            bSearching = false;
                        }
                    }
                }
                if (bSearching) {
                    //                    printf("::: ids ins %ld\n", iDadID);
                    asCurrent[1-iCur].insert(iDadID);
                }


            } else if ((iMomID == 0)  && (iDadID == 0)) {
                // nothing to do
            } else {
                printf("%s[backreach3]Bad parents for %ld\n", RED, *it2);
                iResult = -1;
            }
            tGenBuild += omp_get_wtime()-t1;

        }
        /*printf("\n");*/ fflush(stdout);

        m_vBackReach.push_back(iMinG);

        asCurrent[iCur].clear();
        iCur = 1-iCur;

        ig++;
    }
    printf("loadAncs: %f\n", tLoadAncs);
    printf("genBuild: %f\n", tGenBuild);

    delete pSys;
    return iResult;
}


#define M_MASK 0x2
#define D_MASK 0x1
#define FULL   0x3    
//----------------------------------------------------------------------------
// backReach7
//   for each "backward" generation, determine the "earliest" generation referenced by parents
//   in the generation.
//   ("backwards": latest births:earliest backwards generation)
//   we traverse the tree startingh from a set and work our way backwards via the 
//   parents of this set. It is possible that such a parent has already been a parent
//   of an "earlier" child. In this case the backreach is the index of the "earlier" generation.
//
//   array of sets for current (to prevent copying of large sets)
//
int DynAncGraphX::backReach7(idset &sInitial) {
    SystemInfo *pSys = SystemInfo::createInstance();


    int iResult = 0;
    uint ig = 0;
    double t0 = omp_get_wtime();
   
    // fill the current set with the initial ids 
    idset asCurrent[2];
    asCurrent[0].insert(sInitial.begin(), sInitial.end());
    // current set is set 0
    int iCur = 0;

    // remember the initial ids as the selected ones
    m_sSelected.insert(sInitial.begin(), sInitial.end());
   
    // make sure the vector of generations is clean
    m_vGenerations.clear();

    printf("checking back reach (7) (%zd curgen)\n", asCurrent[iCur].size());
    double tLoadAncs = 0;
    double tGenBuild = 0;
    while ((iResult == 0) && (asCurrent[iCur].size() > 0))  {
        // the newest rgeneration is the current set
        m_vGenerations.push_back(asCurrent[iCur]);
        double t1 = omp_get_wtime();
        printf("Generation %5d: %10zd candidates (%5.3f s) (mem use/peak %12lu / %12lu)\r", ig, asCurrent[iCur].size(), omp_get_wtime()-t0, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
        t0 = t1;
        //        listIDSet("IDs: ", asCurrent[iCur]);
        //        printf("::: ids gen %d\n", ig);

        // now make child maps from the new nodes
        idset_cit it2;
        std::map<idtype, ancdataX> mAncs;
        double t0 = omp_get_wtime();
        iResult = loadAncs(asCurrent[iCur], mAncs);
        tLoadAncs += omp_get_wtime()-t0;
        uint iMinG = ig;
        for (it2 = asCurrent[iCur].begin(); (iResult == 0) && (it2 != asCurrent[iCur].end()); ++it2) {
            double t1 = omp_get_wtime();
            // load anc elements for it2 from anc file
            // map<int, pair<int,int>> ancs
            idtype iMomID = mAncs[*it2].iMomID;
            idtype iDadID = 0;
            if (iMomID != 0) {
                iDadID = labs(mAncs[*it2].iDadID);
            }

            //            printf("::: id %ld: m%ld d%ld\n", *it2, iMomID, iDadID);
            if ((iMomID > 0)  && (iDadID > 0)) {

                int iSearching = FULL;
                for (uint hh = 0; (iSearching > 0) && (hh < m_vGenerations.size()); ++hh) {
                    if ((iMomID >= *(m_vGenerations[hh].begin())) && (iMomID <= *(m_vGenerations[hh].rbegin()))) {
                        if (m_vGenerations[hh].find(iMomID) != m_vGenerations[hh].end()) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            iSearching &= ~M_MASK;
                        }
                    }
                    if ((iDadID >= *(m_vGenerations[hh].begin())) && (iDadID <= *(m_vGenerations[hh].rbegin()))) {
                        if (m_vGenerations[hh].find(iDadID) != m_vGenerations[hh].end()) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            iSearching &= ~D_MASK;
                        }
                    }
                }
                if ((iSearching & M_MASK) != 0)  {
                    //                    printf("::: ids ins %ld\n", iDadID);
                    asCurrent[1-iCur].insert(iMomID);
                }
                if ((iSearching & D_MASK) != 0)  {
                    //                    printf("::: ids ins %ld\n", iDadID);
                    asCurrent[1-iCur].insert(iDadID);
                }



            } else if ((iMomID == 0)  && (iDadID == 0)) {
                // nothing to do
            } else {
                printf("%s[backreach3]Bad parents for %ld\n", RED, *it2);
                iResult = -1;
            }
            tGenBuild += omp_get_wtime()-t1;

        }
        /*printf("\n");*/ fflush(stdout);

        m_vBackReach.push_back(iMinG);

        asCurrent[iCur].clear();
        iCur = 1-iCur;

        ig++;
    }
    printf("loadAncs: %f\n", tLoadAncs);
    printf("genBuild: %f\n", tGenBuild);

    delete pSys;
    return iResult;
}
    

    
//----------------------------------------------------------------------------
// backReach4
//   for each "backward" generation, determine the "earliest" generation referenced by parents
//   in the generation.
//   ("backwards": latest births:earliest backwards generation)
//   we traverse the tree startingh from a set and work our way backwards via the 
//   parents of this set. It is possible that such a parent has already been a parent
//   of an "earlier" child. In this case the backreach is the index of the "earlier" generation.
//
//   array of sets for current; use of RGenerations ("self loading" sets)
// 
int DynAncGraphX::backReach4(idset &sInitial, int iLatency) {
    SystemInfo *pSys = SystemInfo::createInstance();

    // first build all generations
    int iResult = 0;
    uint ig = 0;
    double t0 = omp_get_wtime();

    //    intset sCurrent(sInitial);
    idset asCurrent[2];
    asCurrent[0].insert(sInitial.begin(), sInitial.end());
    int iCur = 0;

    m_sSelected.insert(sInitial.begin(), sInitial.end());

    m_vRGenerations.clear();

    printf("checking back reach (4) (%zd curgen)\n", asCurrent[iCur].size());

    while ((iResult == 0) && (asCurrent[iCur].size() > 0))  {
    
        RGeneration *pRG = RGeneration::createInstance(ig, asCurrent[iCur], iLatency);
        if (pRG != NULL) {
            m_vRGenerations.push_back(pRG);
            
            double t1 = omp_get_wtime();
            printf("Generation %5d: %10zd candidates (%4.2f s) (mem use/peak %12lu / %12lu)\r", ig, asCurrent[iCur].size(), omp_get_wtime()-t0, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
            t0 = t1;

            idset sTemp;
            // now make child maps from the new nodes
            idset_cit it2;
            std::map<idtype, ancdataX> mAncs;
            iResult = loadAncs(asCurrent[iCur], mAncs);
            uint iMinG = ig;
            for (uint hh = 0; hh < m_vRGenerations.size(); ++hh) {
                m_vRGenerations[hh]->prepare();
            }
            for (it2 = asCurrent[iCur].begin(); (iResult == 0) && (it2 != asCurrent[iCur].end()); ++it2) {

                // load anc elements for it2 from anc file
                // map<int, pair<int,int>> ancs
                idtype iMomID = mAncs[*it2].iMomID;
                idtype iDadID = 0;
                if (iMomID != 0) {
                    iDadID = labs(mAncs[*it2].iDadID);
                }

                if ((iMomID > 0)  && (iDadID > 0)) {
                    // only make new nodes if they don't yet exist
                    bool bSearching = true;
                    for (uint hh = 0; bSearching && (hh < m_vRGenerations.size()); ++hh) {
                        if (m_vRGenerations[hh]->contains(iMomID, ig)) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            bSearching = false;
                        }
                    }
                
                    if (bSearching) {
                        asCurrent[1-iCur].insert(iMomID);
                    }
                    bSearching = true;
                    for (uint hh = 0; bSearching && (hh < m_vRGenerations.size()); ++hh) {
                        if (m_vRGenerations[hh]->contains(iDadID, ig)) {
                            if (hh < iMinG) {
                                iMinG = hh;
                            }
                            bSearching = false;
                        }
                    }
                
                    if (bSearching) {
                        asCurrent[1-iCur].insert(iDadID);
                    }
                } else if ((iMomID == 0)  && (iDadID == 0)) {
                    // roots: nothing to do
                } else {
                    printf("%s[backreach4]Bad parents for %ld\n", RED, *it2);
                    iResult = -1;
                }
            }
            /*printf("\n"); */fflush(stdout);
            m_vBackReach.push_back(iMinG);

            for (uint hh = 0; hh < m_vRGenerations.size(); ++hh) {
                m_vRGenerations[hh]->decideSave(ig);
            }
                        
            asCurrent[iCur].clear();
            iCur = 1-iCur;
            
            ig++;
        } else {
            iResult = -1;
            printf("%sCouldn't build RGeneration\n", RED);
        }
    }

    for (uint i = 0; i < m_vRGenerations.size(); ++i) {
        m_vRGenerations[i]->saveData();
    }

    delete pSys;
    return iResult;
}
    
    
//----------------------------------------------------------------------------
// createGraph2
// 
// At the beginning of each loop, the childmap holds entries of the form
//   parent->{child, child, ...}
// where child is an id of the previous rgeneration.
// The set asCurrent holds the current ids which are, by definition parents
// of the previous rgeneration's.ids.
// First, ancestor nodes are crearted for the current set, and for each
// current id its children (as given by the childmap) are added to its node.
// Then we loop through the current set and collect the ids of their parents
// which are stored in a childmap (which is used in the next iteration)
//
// uses array[2] of set to avoid copying
//
int DynAncGraphX::createGraph2(idset &sInitial, const char *pTempName, intset &sSavePoints) {
    SystemInfo *pSys = SystemInfo::createInstance();

    int iResult = 0;
    int iTempBufSize = 100000;
    int iSum = 0;

    printf("NumKillpoints in createGraph2: %zd\n", m_mKillPoints.size());
    std::map<int, intset>::const_iterator itg = m_mKillPoints.begin();

    int iGen = 0;
    childmap mChildMap;
    printf("Creating graph\n");
    m_sSelected.clear();
    m_sRoots.clear();
    m_sSelected.insert(sInitial.begin(), sInitial.end());
    idset asCurrent[2];
    asCurrent[0].insert(sInitial.begin(), sInitial.end());
    int iCur = 0;

    printf("initial nodes in createGraph2: ");
    for(idset_it its = sInitial.begin(); its != sInitial.end(); ++its) {
        printf("%ld ", *its);
    }
    printf("\n"); fflush(stdout);
    
    printf("Starting loop (current size: %zd; cm size %zd)\n", asCurrent[iCur].size(), mChildMap.size());fflush(stdout);
    while ((asCurrent[iCur].size() > 0) || (mChildMap.size() > 0)) { 

        //        printf("cur %zd, childmap %zd\n", asCurrent[iCur].size(), mChildMap.size());

        // build nodes for the current ids (when not already done so)
        iResult = createNodesForIds(asCurrent[iCur]);
        // do we have to add children from a previous rgeneration?
        if (!mChildMap.empty()) {
            childmap::iterator it;
            for (it = mChildMap.begin(); (iResult == 0) && (it != mChildMap.end()); ++it) {
                AncestorNode *pAN = findAncestorNode(it->first);
                if (pAN != NULL) {
                    for (uint i = 0; i < it->second.size(); i++) {
                        pAN->addChild(it->second[i]);
                    }
                } else {
                    printf("%sMissing node for id %ld\n", RED, it->first);
                    iResult = -1;
                }
            }
            mChildMap.clear();
        }

        // now make child maps from the new nodes: find the current ids' parents
        idset_cit it2;
        for (it2 = asCurrent[iCur].begin(); (iResult == 0) && (it2 != asCurrent[iCur].end()); ++it2) {
            AncestorNode *pAN = findAncestorNode(*it2);
            if (pAN != NULL) {
                idtype iMomID = pAN->getMom();
                idtype iDadID = pAN->getDad();
                if ((iMomID > 0)  && (iDadID > 0)) {
                    mChildMap[iMomID].push_back(*it2);
                    mChildMap[iDadID].push_back(*it2);
                    
                    // some of the selected nodes might be children of other selected nodes.
                    // these relationships must be set explicitly for the first generation.
                    if (itg == m_mKillPoints.begin()) {
                        if (m_sSelected.find(iMomID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (mom of %d) in generation %d\n", iMomID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iMomID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                            } else {
                                printf("%sCouldn't set child %ld to M%ld\n", RED, *it2, iMomID);
                            }
                        }
                        if (m_sSelected.find(iDadID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (dad of %d) in generation %d\n", iDadID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iDadID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                            } else {
                                printf("%sCouldn't set child %ld to D%ld\n", RED, *it2, iDadID);
                            }
                        }
                    }

                    // only make new nodes if they don't yet exist
                    if (findAncestorNode(iMomID) == NULL) {
                        asCurrent[1-iCur].insert(iMomID);
                    }
                    if (findAncestorNode(iDadID) == NULL) {
                        asCurrent[1-iCur].insert(iDadID);
                    }
                } else if  ((iMomID == 0)  && (iDadID == 0)) {
                    m_sRoots.insert(*it2);
                } else {
                    printf("%s[createGraph]Bad parents for %ld\n", RED, *it2);
                    iResult = -1;
                }
            } else {
                printf("%sNo node for %ld\n", RED, *it2);
                iResult = -1;
            }
        }

        if (m_bKillPoints && (itg->first+1 == iGen)) {
            printf("Reached Killpoint %d: ", itg->first);
            sSavePoints.insert(iGen);
            iResult = saveAndDeleteFragement(pTempName, iGen, iTempBufSize, itg->second);
            if (iResult >= 0) {
                iSum += iResult;
                iResult = 0;
            }
            //            printf("  Num of nodes in map after: %zd (first: %d)\n", m_mIndex.size(), m_mIndex.begin()->first);
            ++itg;
        }
        printf("Gen %d: mem use/peak %12lu / %12lu)\r", iGen, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
        // now go on to the next
        ++iGen;
        asCurrent[iCur].clear();
        // next iteration, the current set is the other set
        iCur = 1 - iCur;
    }

    // save and delete the remaining ones
    if ((iResult == 0) && m_bKillPoints  && (itg->first+1 == iGen)) {
        printf("Reached last Killpoint %d: ", itg->first);
        sSavePoints.insert(iGen);
        iResult = saveAndDeleteFragement(pTempName, iGen, iTempBufSize, itg->second);
        if (iResult >= 0) {
            iSum += iResult;
            iResult = 0;
        }
        //        printf("  Num of nodes in map after: %zd (first: %d)\n", m_mIndex.size(), m_mIndex.begin()->first);
        
    }

    if (iResult == 0) {
        if (m_bKillPoints) {
            printf("Saved Ancestorgraph with %d entries (in memory: %zd)\n", iSum, m_mIndex.size());
        } else {
            printf("Created Ancestorgraph with %zd entries\n", m_mIndex.size());
        }
    }
    
    delete pSys;
    return iResult;
}

//----------------------------------------------------------------------------
// createGraph3
// 
// At the beginning of each loop, the childmap holds entries of the form
//   parent->{child, child, ...}
// where child is an id of the previous rgeneration.
// The set m_vGenerations[0] holds the current ids which are, by definition parents
// of the previous rgeneration's.ids.
// First, ancestor nodes are created for the current set, and for each
// current id its children (as given by the childmap) are added to its node.
// Then we loop through the current set and collect the ids of their parents
// which are stored in a childmap (which is used in the next iteration)
//
// uses sets directly from m_vGenerations (calculated in backReach3)
//
int DynAncGraphX::createGraph3(idset &sInitial, const char *pTempName, intset &sSavePoints) {
    SystemInfo *pSys = SystemInfo::createInstance();

    int iResult = 0;
    int iTempBufSize = 100000;
    int iSum = 0;

    double tCreateNodes = 0;
    double tHandleChildMaps = 0;
    double tCreateChildMaps = 0;
    double tSaveDelete = 0;
    double t0 = 0;

    printf("NumKillpoints in createGraph2: %zd\n", m_mKillPoints.size());
    std::map<int, intset>::const_iterator itg = m_mKillPoints.begin();

    childmap mChildMap;
    printf("Creating graph\n");
    m_sRoots.clear();
    m_sSelected.clear();
    m_sSelected.insert(sInitial.begin(), sInitial.end());

    // we need an addittional (empty) set in m_vGenerations
    // to handle the final child map
    idset sDummy;
    m_vGenerations.push_back(sDummy);

    // display initial ids
    printf("initial nodes in createGraph2: ");
    for(idset_it its = sInitial.begin(); its != sInitial.end(); ++its) {
        printf("%ld ", *its);
    }
    printf("\n"); fflush(stdout);
    
    // loop through ar rgenerations
    printf("Starting loop (current size: %zd; cm size %zd)\n", m_vGenerations[0].size(), mChildMap.size());fflush(stdout);
    for (uint iGen = 0; iGen != m_vGenerations.size(); ++iGen) {
        //@@@@            printf("adding -----\n");
            
        // build nodes for the current ids (when not already done so)
        t0 = omp_get_wtime();
        iResult = createNodesForIds(m_vGenerations[iGen]);
        tCreateNodes += omp_get_wtime() - t0;

        
        t0 = omp_get_wtime();
        // do we have to add children from a previous rgeneration?
        if (!mChildMap.empty()) {
            childmap::iterator it;
            for (it = mChildMap.begin(); (iResult == 0) && (it != mChildMap.end()); ++it) {
                AncestorNode *pAN = findAncestorNode(it->first);
                if (pAN != NULL) {
                    for (uint i = 0; i < it->second.size(); i++) {
                        pAN->addChild(it->second[i]);
                        //@@@@                        printf("adding child to %ld : %ld\n", it->first, it->second[i]);
                    }
                } else {
                    printf("%sMissing node for id %ld\n", RED, it->first);
                    iResult = -1;
                }
            }
            mChildMap.clear();
        }
        tHandleChildMaps += omp_get_wtime() - t0;

        
        t0 = omp_get_wtime();
        // now make child maps from the new nodes: find the current ids' parents
        idset_cit it2;
        for (it2 = m_vGenerations[iGen].begin(); (iResult == 0) && (it2 != m_vGenerations[iGen].end()); ++it2) {
            AncestorNode *pAN = findAncestorNode(*it2);
            if (pAN != NULL) {
                idtype iMomID = pAN->getMom();
                idtype iDadID = pAN->getDad();
                if ((iMomID > 0)  && (iDadID > 0)) {
                    mChildMap[iMomID].push_back(*it2);
                    mChildMap[iDadID].push_back(*it2);
                    
                    // some of the selected nodes might be children of other selected nodes.
                    // these relationships must be set explicitly for the first generation.
                    if (itg == m_mKillPoints.begin()) {
                        if (m_sSelected.find(iMomID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (mom of %d) in generation %d\n", iMomID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iMomID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                                //@@@@                                printf("adding child to %ld : %ld i\n", iMomID, *it2);

                            } else {
                                printf("%sCouldn't set child %ld to M%ld\n", RED, iMomID, *it2);
                            }
                        }
                        if (m_sSelected.find(iDadID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (dad of %d) in generation %d\n", iDadID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iDadID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                                //@@@@                                printf("adding child to %ld : %ld i\n", iDadID, *it2);
                            } else {
                                printf("%sCouldn't set child %ld to D%ld\n", RED, *it2, iDadID);
                            }
                        }
                    }
                } else if  ((iMomID == 0)  && (iDadID == 0)) {
                    m_sRoots.insert(*it2);
                } else {
                    printf("%s[createGraph]Bad parents for %ld\n", RED, *it2);
                    iResult = -1;
                }
            } else {
                printf("%sNo node for %ld\n", RED, *it2);
                iResult = -1;
            }
        }
        tCreateChildMaps += omp_get_wtime() - t0;

        if (m_bKillPoints && (itg->first+1 == (int)iGen)) {
            printf("Reached Killpoint %d: ", itg->first);
            sSavePoints.insert(iGen);
            t0 = omp_get_wtime();
            iResult = saveAndDeleteFragement(pTempName, iGen, iTempBufSize, itg->second);
            tSaveDelete += omp_get_wtime() - t0;
            if (iResult >= 0) {
                iSum += iResult;
                iResult = 0;
            }

            ++itg;
        }
        printf("Gen %d: mem use/peak %12lu / %12lu)\r", iGen, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
        // now go on to the next rGeneration
    }

    // save and delete the remaining ones
    int iGLast = (int)m_vGenerations.size();
    if ((iResult == 0) && m_bKillPoints  && (itg->first+1 == iGLast)) {
        printf("Reached last Killpoint %d: ", itg->first);
        sSavePoints.insert(iGLast);
        t0 = omp_get_wtime();
        iResult = saveAndDeleteFragement(pTempName, iGLast, iTempBufSize, itg->second);
        tSaveDelete += omp_get_wtime() - t0;
        if (iResult >= 0) {
            iSum += iResult;
            iResult = 0;
        }
        
    }

    if (iResult == 0) {
        if (m_bKillPoints) {
            printf("Saved Ancestorgraph with %d entries (in memory: %zd)\n", iSum, m_mIndex.size());
        } else {
            printf("Created Ancestorgraph with %zd entries\n", m_mIndex.size());
        }
    }
    
    
    printf("CreateNodes     %f\n", tCreateNodes);
    printf("CreateChildMaps %f\n", tCreateChildMaps);
    printf("HandleChildMaps %f\n", tHandleChildMaps);
    printf("SaveAndDelete   %f\n", tSaveDelete);
    delete pSys;
    return iResult;
}

//----------------------------------------------------------------------------
// createGraph7
// 
// At the beginning of each loop, the childmap holds entries of the form
//   parent->{child, child, ...}
// where child is an id of the previous rgeneration.
// The set m_vGenerations[0] holds the current ids which are, by definition parents
// of the previous rgeneration's.ids.
// First, ancestor nodes are created for the current set, and for each
// current id its children (as given by the childmap) are added to its node.
// Then we loop through the current set and collect the ids of their parents
// which are stored in a childmap (which is used in the next iteration)
//
// uses sets directly from m_vGenerations (calculated in backReach3)
//
int DynAncGraphX::createGraph7(idset &sInitial, const char *pTempName, intset &sSavePoints) {
    SystemInfo *pSys = SystemInfo::createInstance();

    int iResult = 0;
    int iTempBufSize = 100000;
    int iSum = 0;

    double tCreateNodes = 0;
    double tHandleChildMaps = 0;
    double tCreateChildMaps = 0;
    double tSaveDelete = 0;
    double t0 = 0;

    printf("NumKillpoints in createGraph2: %zd\n", m_mKillPoints.size());
    std::map<int, intset>::const_iterator itg = m_mKillPoints.begin();

    childmap mChildMap;
    printf("Creating graph\n");
    m_sRoots.clear();
    m_sSelected.clear();
    m_sSelected.insert(sInitial.begin(), sInitial.end());

    // we need an addittional (empty) set in m_vGenerations
    // to handle the final child map
    idset sDummy;
    m_vGenerations.push_back(sDummy);

    // display initial ids
    printf("initial nodes in createGraph2: ");
    for(idset_it its = sInitial.begin(); its != sInitial.end(); ++its) {
        printf("%ld ", *its);
    }
    printf("\n"); fflush(stdout);
    iResult = createNodesForIds(m_vGenerations[0]);    

    // loop through ar rgenerations
    printf("Starting loop (current size: %zd; cm size %zd)\n", m_vGenerations[0].size(), mChildMap.size());fflush(stdout);
    for (uint iGen = 0; iGen < m_vGenerations.size()-1; ++iGen) {
        printf("adding -----\n");

        // build nodes for the current ids (when not already done so)
        t0 = omp_get_wtime();
        iResult = createNodesForIds(m_vGenerations[iGen+1]);
        tCreateNodes += omp_get_wtime() - t0;

        
        t0 = omp_get_wtime();
        // do we have to add children from a previous rgeneration?
        /*
        if (!mChildMap.empty()) {
            childmap::iterator it;
            for (it = mChildMap.begin(); (iResult == 0) && (it != mChildMap.end()); ++it) {
                AncestorNode *pAN = findAncestorNode(it->first);
                if (pAN != NULL) {
                    for (uint i = 0; i < it->second.size(); i++) {
                        pAN->addChild(it->second[i]);
                    }
                } else {
                    printf("Missing node for id %ld\n", it->first);
                    iResult = -1;
                }
            }
            mChildMap.clear();
        }
        */
        tHandleChildMaps += omp_get_wtime() - t0;

        
        t0 = omp_get_wtime();
        // now make child maps from the new nodes: find the current ids' parents
        idset_cit it2;
        for (it2 = m_vGenerations[iGen].begin(); (iResult == 0) && (it2 != m_vGenerations[iGen].end()); ++it2) {
            AncestorNode *pAN = findAncestorNode(*it2);
            if (pAN != NULL) {
                idtype iMomID = pAN->getMom();
                idtype iDadID = pAN->getDad();
                if ((iMomID > 0)  && (iDadID > 0)) {
                    AncestorNode *pANM = findAncestorNode(iMomID);
                    if (pANM != NULL) {
                        pANM->addChild(*it2);
                        //@@@@                        printf("adding child to %ld : %ld\n", iMomID, *it2);
                        AncestorNode *pAND = findAncestorNode(iDadID);
                        if (pAND != NULL) {
                            pAND->addChild(*it2);
                            //@@@@                            printf("adding child to %ld : %ld\n", iDadID, *it2);
                                                    
                    // some of the selected nodes might be children of other selected nodes.
                    // these relationships must be set explicitly for the first generation.
                            if (itg == m_mKillPoints.begin()) {
                                if (m_sSelected.find(iMomID) != m_sSelected.end()) {
                                    //                        printf("Found selected %d (mom of %d) in generation %d\n", iMomID, *it2, iGen);
                                    pANM->addChild(*it2);
                                    pANM->m_bSelected = true;
                                    //@@@@                                    printf("adding child to %ld : %ld i\n", iMomID, *it2);
                                }
                                if (m_sSelected.find(iDadID) != m_sSelected.end()) {
                                    //                        printf("Found selected %d (dad of %d) in generation %d\n", iDadID, *it2, iGen);
                                    pAND->addChild(*it2);
                                    pAND->m_bSelected = true;
                                    //@@@@                                    printf("adding child to %ld : %ld i\n", iDadID, *it2);
                                }
                            }
                        } else {
                            printf("%sno node for dad %ld of %ld\n", RED, iDadID, *it2);
                        }
                    } else {
                        printf("%sno node for mom %ld of %ld\n", RED, iMomID, *it2);
                    }
                        
                } else if  ((iMomID == 0)  && (iDadID == 0)) {
                    m_sRoots.insert(*it2);
                } else {
                    printf("%s[createGraph]Bad parents for %ld\n", RED, *it2);
                    iResult = -1;
                }
            } else {
                printf("%sNo node for %ld\n", RED, *it2);
                iResult = -1;
            }
        }
        tCreateChildMaps += omp_get_wtime() - t0;

        if (m_bKillPoints && (itg->first+1 == (int)iGen)) {
            printf("Reached Killpoint %d: ", itg->first);
            sSavePoints.insert(iGen);
            t0 = omp_get_wtime();
            iResult = saveAndDeleteFragement(pTempName, iGen, iTempBufSize, itg->second);
            tSaveDelete += omp_get_wtime() - t0;
            if (iResult >= 0) {
                iSum += iResult;
                iResult = 0;
            }

            ++itg;
        }
        printf("Gen %d: mem use/peak %12lu / %12lu)\r", iGen, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
        // now go on to the next rGeneration
    }

    // save and delete the remaining ones
    int iGLast = (int)m_vGenerations.size();
    if ((iResult == 0) && m_bKillPoints  && (itg->first+1 == iGLast)) {
        printf("Reached last Killpoint %d: ", itg->first);
        sSavePoints.insert(iGLast);
        t0 = omp_get_wtime();
        iResult = saveAndDeleteFragement(pTempName, iGLast, iTempBufSize, itg->second);
        tSaveDelete += omp_get_wtime() - t0;
        if (iResult >= 0) {
            iSum += iResult;
            iResult = 0;
        }
        
    }

    if (iResult == 0) {
        if (m_bKillPoints) {
            printf("Saved Ancestorgraph with %d entries (in memory: %zd)\n", iSum, m_mIndex.size());
        } else {
            printf("Created Ancestorgraph with %zd entries\n", m_mIndex.size());
        }
    }
    
    
    printf("CreateNodes     %f\n", tCreateNodes);
    printf("CreateChildMaps %f\n", tCreateChildMaps);
    printf("HandleChildMaps %f\n", tHandleChildMaps);
    printf("SaveAndDelete   %f\n", tSaveDelete);
    delete pSys;
    return iResult;
}



//----------------------------------------------------------------------------
// createGraphX
// 
// At the beginning of each loop, the childmap holds entries of the form
//   parent->{child, child, ...}
// where child is an id of the previous rgeneration.
// The set m_vGenerations[0] holds the current ids which are, by definition parents
// of the previous rgeneration's.ids.
// First, ancestor nodes are created for the current set, and for each
// current id its children (as given by the childmap) are added to its node.
// Then we loop through the current set and collect the ids of their parents
// which are stored in a childmap (which is used in the next iteration)
//
// uses sets directly from m_vRGenerations (calculated in backReach4)
//
int DynAncGraphX::createGraph4(idset &sInitial, const char *pTempName, intset &sSavePoints) {
    SystemInfo *pSys = SystemInfo::createInstance();

    int iResult = 0;
    int iTempBufSize = 100000;
    int iSum = 0;

    printf("NumKillpoints in createGraph2: %zd\n", m_mKillPoints.size());
    std::map<int, intset>::const_iterator itg = m_mKillPoints.begin();

    childmap mChildMap;
    printf("Creating graph\n");
    m_sRoots.clear();
    m_sSelected.clear();
    m_sSelected.insert(sInitial.begin(), sInitial.end());

    // we need an addittional (empty) set in m_vGenerations
    // to handle the final child map
    idset sDummy;
    RGeneration *pRG = RGeneration::createInstance(m_vGenerations.size(), sDummy, 4);
    m_vRGenerations.push_back(pRG);

    printf("initial nodes in createGraph2: ");
    for(idset_it its = sInitial.begin(); its != sInitial.end(); ++its) {
        printf("%ld ", *its);
    }
    printf("\n"); fflush(stdout);
    
    printf("Starting loop (current size: %zd; cm size %zd)\n", m_vRGenerations[0]->getSet().size(), mChildMap.size());fflush(stdout);
    for (uint iGen = 0; iGen != m_vRGenerations.size(); ++iGen) {

        // build nodes for the current ids (when not already done so)
        iResult = createNodesForIds(m_vRGenerations[iGen]->getSet());
        // do we have to add children from a previous rgeneration?
        if (!mChildMap.empty()) {
            childmap::iterator it;
            for (it = mChildMap.begin(); (iResult == 0) && (it != mChildMap.end()); ++it) {
                AncestorNode *pAN = findAncestorNode(it->first);
                if (pAN != NULL) {
                    for (uint i = 0; i < it->second.size(); i++) {
                        pAN->addChild(it->second[i]);
                    }
                } else {
                    printf("%sMissing node for id %ld\n", RED, it->first);
                    iResult = -1;
                }
            }
            mChildMap.clear();
        }

        // now make child maps from the new nodes: find the current ids' parents
        idset_cit it2;
        for (it2 = m_vRGenerations[iGen]->getSet().begin(); (iResult == 0) && (it2 != m_vRGenerations[iGen]->getSet().end()); ++it2) {
            AncestorNode *pAN = findAncestorNode(*it2);
            if (pAN != NULL) {
                idtype iMomID = pAN->getMom();
                idtype iDadID = pAN->getDad();
                if ((iMomID > 0)  && (iDadID > 0)) {
                    mChildMap[iMomID].push_back(*it2);
                    mChildMap[iDadID].push_back(*it2);
                    
                    // some of the selected nodes might be children of other selected nodes.
                    // these relationships must be set explicitly for the first generation.
                    if (itg == m_mKillPoints.begin()) {
                        if (m_sSelected.find(iMomID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (mom of %d) in generation %d\n", iMomID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iMomID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                            } else {
                                printf("%sCouldn't set child %ld to M%ld\n", RED, *it2, iMomID);
                            }
                        }
                        if (m_sSelected.find(iDadID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (dad of %d) in generation %d\n", iDadID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iDadID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                            } else {
                                printf("%sCouldn't set child %ld to D%ld\n", RED, *it2, iDadID);
                            }
                        }
                    }
                } else if  ((iMomID == 0)  && (iDadID == 0)) {
                    m_sRoots.insert(*it2);
                } else {
                    printf("%s[createGraph]Bad parents for %ld\n", RED, *it2);
                    iResult = -1;
                }
            } else {
                printf("%sNo node for %ld\n", RED, *it2);
                iResult = -1;
            }
        }

        if (m_bKillPoints && (itg->first+1 == (int)iGen)) {
            printf("Reached Killpoint %d: ", itg->first);
            sSavePoints.insert(iGen);
            iResult = saveAndDeleteFragement(pTempName, iGen, iTempBufSize, itg->second);
            if (iResult >= 0) {
                iSum += iResult;
                iResult = 0;
            }
            //            printf("  Num of nodes in map after: %zd (first: %d)\n", m_mIndex.size(), m_mIndex.begin()->first);
            ++itg;
        }
        printf("Gen %d: mem use/peak %12lu / %12lu)\r", iGen, pSys->getProcUsedVirtualMemory(), pSys->getPeakVM());fflush(stdout);
        // now go on to the next rGeneration
    }

    // save and delete the remaining ones
    int iGLast = (int)m_vGenerations.size();
    if ((iResult == 0) && m_bKillPoints  && (itg->first+1 == iGLast)) {
        printf("Reached last Killpoint %d: ", itg->first);
        sSavePoints.insert(iGLast);
        iResult = saveAndDeleteFragement(pTempName, iGLast, iTempBufSize, itg->second);
        if (iResult >= 0) {
            iSum += iResult;
            iResult = 0;
        }
        //        printf("  Num of nodes in map after: %zd (first: %d)\n", m_mIndex.size(), m_mIndex.begin()->first);
        
    }

    if (iResult == 0) {
        if (m_bKillPoints) {
            printf("Saved Ancestorgraph with %d entries (in memory: %zd)\n", iSum, m_mIndex.size());
        } else {
            printf("Created Ancestorgraph with %zd entries\n", m_mIndex.size());
        }
    }
    
    
    return iResult;
}


//----------------------------------------------------------------------------
// createGraph
// 
int DynAncGraphX::createGraph(idset &sInitial, const char *pTempName, intset &sSavePoints) {
    int iResult = 0;
    int iTempBufSize = 100000;
    int iSum = 0;

    std::map<int, intset>::const_iterator itg = m_mKillPoints.begin();

    int iGen = 0;
    childmap mChildMap;
    printf("Creating graph\n");
    m_sSelected.clear();
    m_sRoots.clear();
    m_sSelected.insert(sInitial.begin(), sInitial.end());

    
    printf("initial in createGraph1: ");
    for(idset_it its = sInitial.begin(); its != sInitial.end(); ++its) {
        printf("%ld ", *its);
    }
    printf("\n"); fflush(stdout);
    

    while ((sInitial.size() > 0) || (mChildMap.size() > 0)) { 
        //        printf("cur %zd, childmap %zd\n", sInitial.size(), mChildMap.size());
        // build new nodes
        iResult = createNodesForIds(sInitial);
        // do we have to add children from a previous round?
        if (!mChildMap.empty()) {
            childmap::iterator it;
            for (it = mChildMap.begin(); (iResult == 0) && (it != mChildMap.end()); ++it) {
                AncestorNode *pAN = findAncestorNode(it->first);
                if (pAN != NULL) {
                    // perhaps better
                    //   pAN->m_sChildren.insert( it->second.begin(), it->second.end());
                    //
                    for (uint i = 0; i < it->second.size(); i++) {
                        pAN->addChild(it->second[i]);
                    }
                } else {
                    printf("%sMissing node for id %ld\n", RED, it->first);
                    iResult = -1;
                }
            }
            mChildMap.clear();
        }

        idset sTemp;
        // now make child maps from the new nodes
        idset_cit it2;
        for (it2 = sInitial.begin(); (iResult == 0) && (it2 != sInitial.end()); ++it2) {
            AncestorNode *pAN = findAncestorNode(*it2);
            if (pAN != NULL) {
                idtype iMomID = pAN->getMom();
                idtype iDadID = pAN->getDad();
                if ((iMomID > 0)  && (iDadID > 0)) {
                    mChildMap[iMomID].push_back(*it2);
                    mChildMap[iDadID].push_back(*it2);

                    // some of the selected nodes might be children of other selected nodes.
                    // these relationships must be set explicitly.
                    // in a "normal" graph this can only happen for the first generation.
                    if (itg == m_mKillPoints.begin()) {
                        if (m_sSelected.find(iMomID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (mom of %d) in generation %d\n", iMomID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iMomID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                            } else {
                                printf("%sCouldn't set child %ld to M%ld\n", RED, *it2, iMomID);
                            }
                        }
                        if (m_sSelected.find(iDadID) != m_sSelected.end()) {
                            //                        printf("Found selected %d (dad of %d) in generation %d\n", iDadID, *it2, iGen);
                            AncestorNode *pAN2 = findAncestorNode(iDadID);
                            if (pAN2 != NULL) {
                                pAN2->addChild(*it2);
                                pAN2->m_bSelected = true;
                            } else {
                                printf("%sCouldn't set child %ld to D%ld\n", RED, *it2, iDadID);
                            }
                        }
                    }

                    // only make new nodes if they don't yet exist
                    if (findAncestorNode(iMomID) == NULL) {
                        sTemp.insert(iMomID);
                    }
                    if (findAncestorNode(iDadID) == NULL) {
                        sTemp.insert(iDadID);
                    }
                } else if  ((iMomID == 0)  && (iDadID == 0)) {
                    m_sRoots.insert(*it2);
                } else {
                    printf("%s[createGraph]Bad parents for %ld\n", RED, *it2);
                    iResult = -1;
                }
            } else {
                printf("%sNo node for %ld\n", RED, *it2);
                iResult = -1;
            }
        }

        if (m_bKillPoints && (itg->first+1 == iGen)) {
            printf("Reached Killpoint %d: ", itg->first);
            sSavePoints.insert(iGen);
            iResult = saveAndDeleteFragement(pTempName, iGen, iTempBufSize, itg->second);
            if (iResult >= 0) {
                iSum += iResult;
                iResult = 0;
            }
            //            printf("  Num of nodes in map after: %zd (first: %d)\n", m_mIndex.size(), m_mIndex.begin()->first);
            ++itg;
        }
        
        // now go on to the next
        ++iGen;
        sInitial = sTemp;
    }
    
    // save and delete the remaining ones
    if ((iResult == 0) && m_bKillPoints && (itg->first+1 == iGen)) {
        printf("Reached Killpoint %d: ", itg->first);
        sSavePoints.insert(iGen);
        iResult = saveAndDeleteFragement(pTempName, iGen, iTempBufSize, itg->second);
        if (iResult >= 0) {
            iSum += iResult;
            iResult = 0;
        }
        //        printf("  Num of nodes in map after: %zd (first: %d)\n", m_mIndex.size(), m_mIndex.begin()->first);
        
    }

    if (iResult == 0) {
        if (m_bKillPoints) {
            printf("Saved Ancestorgraph with %d entries (in memory: %zd)\n", iSum, m_mIndex.size());
        } else {
            printf("Created Ancestorgraph with %zd entries\n", m_mIndex.size());
        }
    }
    
    
    return iResult;
}


//----------------------------------------------------------------------------
// loadOracle
//   the oracle file is a binary file of int pairs:
//      id_i min_i
//   where id_(i-1) < id <= id_i implies id is found in block starting at min_i
// 
int DynAncGraphX::loadOracle(const char *pOracleFile, unsigned char *aChecksum, uint iCSLen) {
    int iResult = -1;

    m_mOracle.clear();

    FILE *fIn = fopen(pOracleFile, "rb");
    if (fIn != NULL) {
        //        uint aBuf[2*m_iBlockSize];
        iResult = 0;
        
        uchar *pCheckSum = new uchar[iCSLen];
        ulong iRead = fread(pCheckSum, 1, iCSLen, fIn);
        if ((iRead == iCSLen) && (memcmp(aChecksum, pCheckSum, iCSLen) == 0)) {
            // here we abuse m_aAncBuf to read oracle data
            while ((iResult == 0) && (!feof(fIn))) {
                iRead = fread(m_aAncBuf, sizeof(idtype)+sizeof(filepos), m_iBlockSize, fIn);
                if (iRead > 0) {
                    for (uint i = 0; i < iRead; ++i) {
                        filepos iPos = m_aAncBuf[2*i+1];
                        if (iPos < m_lFileSize) {
                            m_mOracle[m_aAncBuf[2*i]] = iPos;
                        } else {
                            printf("%sBad Oracle file: position for id  %ld is %ld > file size %ld\n", RED, m_aAncBuf[2*i], iPos, m_lFileSize);
                            iResult = -1;
                            m_mOracle.clear();
                        }
                    }
                }
            }
        } else {
            printf("%sChecksum does not match. The Oracle is not for this Anc file\n", RED);
            iResult = -1;
        }
        delete[] pCheckSum;
        fclose(fIn);
    } else {
        printf("Couldn't open oracle file [%s]\n", pOracleFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createOracle
//   the oracle is a map id_i->min_i 
//   where id_(i-1) < id <= id_i implies id is found in block starting at min_i
// 
int DynAncGraphX::createOracle() {
    printf("DynAncGraph Creating oracle\n"); fflush(stdout);
    int iResult = -1;
    if (m_fAnc != NULL) {
        iResult = 0;
        // rewind file
        fseek(m_fAnc, 0, SEEK_SET);
        ulong iStep = m_iAncSize*m_iBlockSize;
        ulong iFilePos = m_iAncSize*m_iBlockSize;
        idtype iID[4];
        ulong iPrev = 0;
        printf("Filesize %ld\n", m_lFileSize);
        while ((iResult == 0) && (sizeof(idtype)*iFilePos < m_lFileSize)) {
            fseek(m_fAnc, sizeof(idtype)*(iFilePos-m_iAncSize), SEEK_SET);
            ulong iRead = fread(iID, sizeof(idtype), m_iAncSize, m_fAnc);
            if (iRead == m_iAncSize) {
                m_mOracle[iID[0]] = iPrev;
                //                printf("%ld -> %ld*%ld=%ld\n", iID[0], iPrev, sizeof(idtype), sizeof(idtype)*iPrev);
                iPrev = iFilePos;
                iFilePos += iStep;
            } else {  
                printf("%sRead error in anc file: only read %lu from pos %ld\n", RED, iRead, iFilePos-m_iAncSize);
                if (feof(m_fAnc)) {
                    printf("EOF!\n");
                }
                iResult = -1;
            }
        }
        if (iResult == 0) {
            // add the last element: highest ID + 1 (i.e. greater than all possible IDs)
            if (iPrev < m_lFileSize) {
                fseek(m_fAnc, -(ulong)(m_iAncSize*sizeof(idtype)), SEEK_END);

                ulong iRead = fread(iID, sizeof(idtype), m_iAncSize, m_fAnc);
                if (iRead == m_iAncSize) {
                    m_mOracle[iID[0]+1] = iPrev; 
                    printf("final\n%ld -> %ld\n", iID[0]+1, iPrev);
                } else {
                    printf("%sread %lu instead of %d\n", RED, iRead, m_iAncSize);
                }
            }
        }
    }
    printf("oracle size %zd\n", m_mOracle.size());fflush(stdout);
    return iResult;
}


//----------------------------------------------------------------------------
// createNodesForIds
// 
int DynAncGraphX::createNodesForIds(const idset &sCur) {
    int iResult = 0;

    // determine which blocks to load
    idset sBlocks;
    idset_cit it;
    idset_cit it2;
    for (it = sCur.begin(); (iResult == 0) && (it != sCur.end()); ++it) {
        if (findAncestorNode(*it) == NULL) {
            oracle::const_iterator ito = m_mOracle.lower_bound(*it);
            if (ito != m_mOracle.end()) {
                sBlocks.insert(ito->first);
            } else {
                printf("%s[DynAncGraphX::createNodesForIds ]no upper bound found for id %ld\n", RED, *it);
                iResult = -1;
            }
        }
    }

    
    it = sCur.begin();
    //    printf("Searching for id %d", *it);
    idset_cit itB = sBlocks.begin();
    while ((iResult == 0) && (itB != sBlocks.end())) {
        idtype iID = *itB;
        filepos iMin = m_mOracle[iID];
        //        int aBuf[3*m_iBlockSize];
        fseek(m_fAnc, iMin*sizeof(idtype), SEEK_SET);
        //        printf("Reading block for %d starting at %d (real %zd)\n", iID, iMin, iMin*sizeof(int));
        ulong iRead = fread(m_aAncBuf, m_iAncSize*sizeof(idtype), m_iBlockSize, m_fAnc);
        if (iRead > 0) {
            uint i = 0;
            while (it != sCur.end() && (*it <= iID) && (i < iRead) && (iResult == 0)) {
                while ((i < iRead) && (m_aAncBuf[m_iAncSize*i] < *it)) {
                    i++;
                }

                AncestorNode *pAN0 = findAncestorNode(*it);
                if (pAN0 == NULL) {
                        
                    if (m_aAncBuf[m_iAncSize*i] == *it) {
//                        printf(" -> found in block for %d starting at %d\n", iID, iMin);
                        // create anc node
                        // gender encoded in sign of Dad
                        AncestorNode *pAN = new AncestorNode(m_aAncBuf[m_iAncSize*i], m_aAncBuf[m_iAncSize*i+1], (m_aAncBuf[m_iAncSize*i+1]==0)?0:(idtype)labs(m_aAncBuf[m_iAncSize*i+2]));
                        pAN->m_iGender = (m_aAncBuf[m_iAncSize*i+2] < 0)?1:0;
                        m_mIndex[m_aAncBuf[m_iAncSize*i]] = pAN;

                        //                        printf("%d -> %p\n", aBuf[3*i], pAN);
                        ++it;
                        if (it != sCur.end()) {
                            //                            printf("Searching for id %d (max in block %d)", *it, iID);
                        } else {
                            //                              printf("Done\n");
                        }
                        ++i;
                    } else {
                        printf("%s: Didn't find id %ld in block for %ld starting at %ld\n", RED, *it, iID, iMin);
                        iResult = -1;
                    }
                } else {
                    ++it;
                }
            }

        } else {
            printf("%sRead error in Anc file\n", RED);
            iResult = -1;
        }
        ++itB;
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// saveAndDeleteFragement
// 
int DynAncGraphX::saveAndDeleteFragement(const char *pFileBody, int iGen, int iTempBufSize, const intset &sGenSets) {
    int iResult = 0;

    // collect all ids in the remaining generation sets
    // not needed for creation of graph
    idset sAll;
    intset_cit its;
    if (m_bVer4) {
        for (its = sGenSets.begin(); its != sGenSets.end(); ++its) {
            const idset &s = m_vRGenerations[*its]->getSet();
            sAll.insert(s.begin(), s.end());
            //            printf("deleting RGen #%d\n", *its);
            delete m_vRGenerations[*its];
        }
        
    } else {
        for (its = sGenSets.begin(); its != sGenSets.end(); ++its) {
            sAll.insert(m_vGenerations[*its].begin(), m_vGenerations[*its].end());
            m_vGenerations[*its].clear();
        }
    }
    int iSum = (int)sAll.size();
     
    printf("saveanddelete %d nodes         \n", iSum);
    // save and delete corresponding 
    char sName[64];
    sprintf(sName, "%s_%04d.ag", pFileBody, iGen);
    iResult = saveAndDeleteSet(sName, iTempBufSize, sAll);

    return (iResult == 0)?iSum:iResult;
}


//----------------------------------------------------------------------------
// saveAndDeleteFragement
// 
int DynAncGraphX::saveAndDeleteSet(const char *pFileName, int iBufSize, idset &sIDs) {
    int iResult = 0;

    BufWriter *pBW = BufWriter::createInstance(pFileName, iBufSize);
    if (pBW!= NULL) {

        // write header
        pBW->addLine("AGRB");
        long lListOffset = 0;
        pBW->addChars((char *)&lListOffset, sizeof(long));     
        long iNumNodes = (long)  sIDs.size();
        pBW->addChars((char *)&iNumNodes, sizeof(long));        
        uint iC = 0;
        // write and delete the specified nodes
        idset_cit it;
        for (it = sIDs.begin(); (iResult == 0) && (it != sIDs.end()); ++it) {
            ancnodelist::iterator itm = m_mIndex.find(*it);
            if (itm != m_mIndex.end()) {
                // save it ...
                saveNode(pBW, itm->second);
                iC++;
                // ... delete it ...
                delete itm->second;
                //... and remove it from map
                m_mIndex.erase(itm);
            } else {
                printf("%scouldn't find node for id %ld\n", RED, *it);
            }
        }
        if (iC !=  sIDs.size()) {
            printf("%sWrote %d instead of %zd nodes - deleting [%s]\n", RED, iC, sIDs.size(), pFileName);
            delete pBW;
            // delete file
            remove(pFileName);
            iResult = -1;
        } else {
            // write the footer
            int iListSize = 0;
            lListOffset = pBW->getPos();
            // empty progenitor list
            pBW->addChars((char *) &iListSize, sizeof(int));
            // empty selected list
            pBW->addChars((char *) &iListSize, sizeof(int));
            // empty root list
            pBW->addChars((char *) &iListSize, sizeof(int));
            // empty leaves list
            pBW->addChars((char *) &iListSize, sizeof(int));
            
            delete pBW;
            
            iResult = writeListOffset(pFileName, lListOffset);
        }
    } else {
        iResult = -1;
        printf("%sCouldn't open outputfile [%s]\n", RED, pFileName);
    }
    
    return iResult;
}



//----------------------------------------------------------------------------
// saveBin
//   Format:
//     File         ::= <Header><NodeList><SpecialLists>
//     SpecialLists ::= <Progenitors><Selected><Roots><Leaves>
//     Header       ::= "AGRB" 
//     NodeList     ::= <NumNodes><NodeData>*
//     NodeData     ::= <ID><MomID><DadID><Gender><Children>
//     Children     ::= <IDList>
//     Progenitors  ::= <IDList>
//     Selected     ::= <IDList>
//     Roots        ::= <IDList>
//     Leaves       ::= <IDList>
//     IDList       ::= <NumIDs><ID>*
//   NumNodes : int
//   ID       : int
//   MomID    : int
//   DadID    : int
//   Gender   : int
//   NumIDs   : int
//
int DynAncGraphX::saveBin(BufWriter *pBW, bool bWriteHeader, bool bWriteNodes, bool bWriteFooter) {
    int iResult = 0;

    if (bWriteHeader) {
        pBW->addLine("AGRB");
        long lDummy=0;
        pBW->addChars((char *)&lDummy, sizeof(long));        
        long iNumNodes = (long)  m_mIndex.size();
        pBW->addChars((char *)&iNumNodes, sizeof(long));        
    }
    
    if (bWriteNodes) {
        printf("Writing %zd nodes\n", m_mIndex.size());
        ancnodelist::const_iterator it;
        for (it = m_mIndex.begin(); it != m_mIndex.end(); ++it) {
            saveNode(pBW, it->second);
        }
    }

    if (bWriteFooter) {
        m_lListOffset =  pBW->getPos();
        printf("Writing %zd progs, %zd sels, %zd roots %zd leaves\n", 
               m_sRoots.size(), m_sSelected.size(), m_sRoots.size(), m_sSelected.size());
        idset_cit it2;
        int iNumProgs = (int)m_sRoots.size();
        pBW->addChars((char *) &iNumProgs, sizeof(int));
        for (it2 = m_sRoots.begin(); it2 != m_sRoots.end(); ++it2) {
            idtype iP = *it2;
            pBW->addChars((char *) &iP, sizeof(int));
        }
        int iNumSelected = (int)m_sSelected.size();
        pBW->addChars((char *) &iNumSelected, sizeof(int));
        for (it2 = m_sSelected.begin(); it2 != m_sSelected.end(); ++it2) {
            idtype iS = *it2;
            pBW->addChars((char *) &iS, sizeof(int));
        }
    
        int iNumRoots = (int)m_sRoots.size();
        pBW->addChars((char *) &iNumRoots, sizeof(int));
        for (it2 = m_sRoots.begin(); it2 != m_sRoots.end(); ++it2) {
            idtype iS = *it2;
            pBW->addChars((char *) &iS, sizeof(int));
        }
    
        int iNumLeaves = (int)m_sSelected.size();
        pBW->addChars((char *) &iNumLeaves, sizeof(int));
        for (it2 = m_sSelected.begin(); it2 != m_sSelected.end(); ++it2) {
            idtype iS = *it2;
            pBW->addChars((char *) &iS, sizeof(int));
        }
    }    
    return iResult;
}

//----------------------------------------------------------------------------
// saveBin
//   Format:
//     File         ::= <Header><NodeList><SpecialLists>
//     SpecialLists ::= <Progenitors><Selected><Roots><Leaves>
//     Header       ::= "AGRB"
//     NodeList     ::= <NumNodes><NodeData>*
//     NodeData     ::= <ID><MomID><DadID><Gender><Children>
//     Children     ::= <IDList>
//     Progenitors  ::= <IDList>
//     Selected     ::= <IDList>
//     Roots        ::= <IDList>
//     Leaves       ::= <IDList>
//     IDList       ::= <NumIDs><ID>*
//   NumNodes : int
//   ID       : int
//   MomID    : int
//   DadID    : int
//   Gender   : int
//   NumIDs   : int
//
int DynAncGraphX::saveBin(const char *pFileName) {
    int iResult = -1;

    BufWriter *pBW = BufWriter::createInstance(pFileName);
    if (pBW != NULL) {
        iResult = saveBin(pBW, true, true, true);
        delete pBW;   
        
        iResult = writeListOffset(pFileName, m_lListOffset);
    } else {
        printf("%sCouldn't open [%s] for writing\n", RED, pFileName);
    }
    return iResult;
}


int DynAncGraphX::writeOracle(const char *pOutput, unsigned char *aChecksum, int iCSLen) {
    int iResult = -1;
    int iBufSize = 16636;
    BufWriter *pBW = BufWriter::createInstance(pOutput, iBufSize);
    if (pBW != NULL) {
        pBW->addChars((char *) aChecksum, iCSLen);
        oracle::const_iterator it;

        for (it = m_mOracle.begin(); it != m_mOracle.end(); ++it) {
            pBW->addChars((char *)&(it->first), sizeof(int));
            pBW->addChars((char *)&(it->second), sizeof(uint));
        }

        iResult = 0;
        delete pBW;
    } else {
        printf("%sCouldn't open [%s] for writiong\n", RED, pOutput);
    }

    return iResult;
}
