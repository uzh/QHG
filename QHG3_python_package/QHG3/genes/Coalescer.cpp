#include <stdio.h>
#include <string.h>

#include <omp.h>

#include <iostream>
#include <string>
#include <set>
#include <algorithm>

#include "types.h"
#include "Coalescer.h"



template<typename T>
void showset(std::string sCaption, const std::set<T> &sData) {
    std::cout  << sCaption << " ";
    for (idset_cit it = sData.begin(); it != sData.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}


double dBlockTot = 0.0;
double dFindTot  = 0.0;
double dMergeTot = 0.0;

//----------------------------------------------------------------------------
// init
//
int Coalescer::init(char *pAncFile, uint iAncSize, int iBlockSize) {
    int iResult = 0;
    // create Oracle for ANCFile
    m_iAncSize   = iAncSize;
    m_iBlockSize = iBlockSize;
    m_aAncBuf = new idtype[m_iAncSize*m_iBlockSize];
    m_fAnc = fopen(pAncFile, "rb");
    if (m_fAnc != NULL) {
        fseek(m_fAnc, 0, SEEK_END);
        m_lFileSize = ftell(m_fAnc);
        fseek(m_fAnc, 0, SEEK_SET);
        if ((m_lFileSize % m_iAncSize) == 0) {
            iResult = createOracle();
        } else {
            printf("Anc file size should be diviible by three...\n");
        }
    }

    // prepare arrays for parallel operation
    if (iResult == 0) {
        printf("Preparing for parallel operation\n");
        m_iNumBlocks = m_mOracle.size();
        m_iNumThreads = omp_get_max_threads();
        m_afIn = new FILE*[m_iNumThreads];
        m_asAncestors = new idset[m_iNumThreads];
        m_apBR = new BufReader*[m_iNumThreads];

        m_asBlockIDs = new blockset[m_iNumBlocks];
        int iC = 0;
        oracle::const_iterator ita;
        for (ita = m_mOracle.begin(); ita != m_mOracle.end(); ++ita) {
            m_asBlockIDs[iC++].iIDFirst = ita->first;
        }
        m_aiIndexes = new int[m_iNumThreads];
        m_aiCounts  = new int[m_iNumThreads];

#pragma omp parallel
        {
            int iT = omp_get_thread_num();
            m_afIn[iT] = fopen(pAncFile, "rb");
            m_apBR[iT] = BufReader::createInstance(m_afIn[iT], iBlockSize);
            m_aiIndexes[iT] = iT;
            m_aiCounts[iT] = 0;
        }
    } 
    return iResult;
}

//----------------------------------------------------------------------------
// createInstance
//
Coalescer *Coalescer::createInstance(char *pAncFile, int iBlockSize) {
    return createInstance(pAncFile, ANCTYPE_SPACETIME, iBlockSize);
}


//----------------------------------------------------------------------------
// createInstance
//
Coalescer *Coalescer::createInstance(char *pAncFile, uint iAncSize, int iBlockSize) {
    Coalescer *pCC = new Coalescer();
    int iResult = pCC->init(pAncFile, iAncSize, iBlockSize);
    if (iResult != 0) {
        delete pCC;
        pCC = NULL;
    }
    return pCC;
}

//----------------------------------------------------------------------------
// constructor
//
Coalescer::Coalescer() 
    : m_iNum(0),
      m_iComparesToDo(0),
      m_asRTree(NULL),
      m_aasCurIDs(NULL),
      m_asComparables(NULL),
      m_aasCoalescents(NULL),
      m_iBlockSize(0),
      m_lFileSize(0),
      m_fAnc(NULL),
      m_iAncSize(0),
      m_aAncBuf(NULL),
      
      m_iNumThreads(0),
      m_iNumBlocks(0),
      m_asBlockIDs(NULL),
      m_afIn(NULL),
      m_asAncestors(NULL),
      m_aiIndexes(NULL) {

}

//----------------------------------------------------------------------------
// destructor
//
Coalescer::~Coalescer() {
    clearSets();

    // other clean1ups (oracle?)
    if (m_fAnc != NULL) {
        fclose(m_fAnc);
    }

    if (m_aAncBuf != NULL) {
        delete[] m_aAncBuf;
    }

    for (int i = 0; i < m_iNumThreads; ++i) {
        fclose(m_afIn[i]);
        delete m_apBR[i];
    }
    delete[] m_afIn;
    delete[] m_asAncestors;
    delete[] m_apBR;
    delete[] m_asBlockIDs;
    delete[] m_aiIndexes;
    delete[] m_aiCounts;

}

    

//----------------------------------------------------------------------------
// clearSets
//
void Coalescer::clearSets() {
   
    if (m_asRTree != NULL) {
        delete[] m_asRTree;
    }


    if (m_aasCurIDs != NULL) {
        for (uint k = 0; k > m_iNum; ++k) {
            delete[] m_aasCurIDs[k];
        }
        delete[] m_aasCurIDs;
    }

    if (m_aasCoalescents != NULL) {
        for (uint k = 0; k > m_iNum; ++k) {
            delete[] m_aasCoalescents[k];
        }
        delete[] m_aasCoalescents;
    }

    if (m_asComparables != NULL) {
        delete[] m_asComparables;
    }

}

//----------------------------------------------------------------------------
// initSets
//
int Coalescer::initSets(const idset &sOriginals) {
    int iResult = 0;
    m_iNum = sOriginals.size();

    m_asRTree        = new idset[m_iNum];
    m_aasCurIDs      = new idset*[m_iNum];
    m_asComparables  = new idset[m_iNum];
    m_aasCoalescents = new timeset*[m_iNum];
    
    m_iComparesToDo = 0;
    
    for (uint i = 0; i < m_iNum; ++i) {
        m_aasCurIDs[i] = new idset[2];
        m_aasCoalescents[i] = new timeset[m_iNum];

        //  m_asComparables[i]: set of indexes with which index i still has to do comparisons
        for (uint l = i+1; l < m_iNum; ++l) {
            m_asComparables[i].insert(l);
        }
        m_iComparesToDo += m_asComparables[i].size();
    }

    // the current sets contain i element each of the original set
    int iC = 0;
    for (idset_cit it = sOriginals.begin(); it != sOriginals.end(); ++it) {
        m_aasCurIDs[iC++][0].insert(*it);
    }

    return iResult;
}

bool tcomp(const timeentry &te1, const timeentry &te2) {
    return te1.second.fTime > te2.second.fTime;
}

//----------------------------------------------------------------------------
// calcIntersection
//
int Coalescer::calcIntersection(idset &s1, idset &s2, timeset &sCoal) {
    int iResult = 0;
    uint iSize = (s1.size() < s2.size())?s1.size():s2.size();
    std::vector<idtype> v1(iSize);
    std::vector<idtype>::iterator citv1;        
    citv1 = std::set_intersection(s1.begin(), s1.end(),
                                  s2.begin(), s2.end(),
                                  v1.begin());
    // if the intersection is not empty
    if (citv1 != v1.begin()) {
        // trim the vector
        v1.resize(citv1-v1.begin()); 

        // create a timelist from the vector
        timelist sT;
        getTimesForIDs(v1, sT);
        // sort it to get latest time first
        std::sort(sT.begin(), sT.end(), tcomp);

        // print the findings
        timelist::iterator cit2;
        printf("\n");
        for (cit2=sT.begin(); cit2 != sT.end(); ++cit2) {
            printf("%ld[%f;%d] ", cit2->first, cit2->second.fTime, cit2->second.iCellID);
        }
        printf("\n");
        //sCoal.insert(sCoal.begin(), sT.begin(), sT.end());

        //insert it to the global timelist
        sCoal.insert(sT.begin(), sT.end());
        
        iResult = 1;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// doCoalesce
//
int Coalescer::doCoalesce(idset &sOriginal, bool bParallel) {
    int iResult = 0;
    bool bSingle = false;

    clearSets();

    printf("doCoalesce %s\n", bParallel?"parallel":"normal");
    bool bVerbose = true;
    double tTotal     = 0;
    double tAncestors = 0;
    double tIntersect = 0;
    iResult =  initSets(sOriginal);
    int *aiFound = new int[m_iNum];
    bool *abNeeded = new bool[m_iNum];
    for (uint k = 0; k < m_iNum; ++k) {
        abNeeded[k] = true;
    }

    int iCur =  0;
    int iC = 0;
    double tT0 = omp_get_wtime();
    while ((iResult == 0) && (m_iComparesToDo > 0)) {
        printf("-----------rG %d\n", iC++); fflush(stdout);
        double tA0 = omp_get_wtime(); 
        if (bParallel) {
            for (uint k = 0; (iResult == 0) && (k < m_iNum); ++k) {
                if (abNeeded[k]) {
                    iResult = findAncestorsForIDsPar(m_aasCurIDs[k][iCur],  m_aasCurIDs[k][1-iCur]);
                
                    m_asRTree[k].insert(m_aasCurIDs[k][iCur].begin(), m_aasCurIDs[k][iCur].end());
                }
                m_aasCurIDs[k][iCur].clear();                       
            }
        } else {
            for (uint k = 0; (iResult == 0) && (k < m_iNum); ++k) {
                if (abNeeded[k]) {
                    iResult = findAncestorsForIDs(m_aasCurIDs[k][iCur],  m_aasCurIDs[k][1-iCur]);
                    
                    m_asRTree[k].insert(m_aasCurIDs[k][iCur].begin(), m_aasCurIDs[k][iCur].end());
                }
                m_aasCurIDs[k][iCur].clear();                       
            }
        }
        tAncestors += omp_get_wtime() - tA0;

        iCur = 1-iCur;
        m_iComparesToDo = 0;
        
        if (bVerbose) {
            printf("-----------Cur "); fflush(stdout);
            for (uint k = 0; k < m_iNum; ++k) {
                printf ("%zd ",  m_aasCurIDs[k][iCur].size());
            }
            printf("\n"); fflush(stdout);
            printf("-----------RTrees "); fflush(stdout);
            for (uint k = 0; k < m_iNum; ++k) {
                printf ("%zd ",  m_asRTree[k].size());
            }
            printf("\n"); fflush(stdout);
        }

        if (iResult == 0) {
            double tI0 = omp_get_wtime();
            for (uint k = 0; k < m_iNum; ++k) {
                memset(aiFound, 0, m_iNum*sizeof(int));
                for (idset_cit it = m_asComparables[k].begin(); it != m_asComparables[k].end(); ++it) {
                    //                printf("Intersection Cur %d - Cur %ld :",k, *it);
                    int iInter = calcIntersection(m_aasCurIDs[k][iCur], m_aasCurIDs[*it][iCur],  m_aasCoalescents[k][*it]);
                    aiFound[*it] += iInter;
                    if (iInter != 0) {
                        printf("  (%03d: Intersection Cur %d - Cur %ld)\n", iC-1, k, *it);
                    }

                    if (!bSingle || (aiFound[*it] == 0)) {
                        int iInter = calcIntersection(m_aasCurIDs[k][iCur], m_asRTree[*it],  m_aasCoalescents[k][*it]);
                        aiFound[*it] += iInter;
                        if (iInter != 0) {
                            printf("  (%03d: Intersection Cur %d - RTree %ld)\n", iC-1, k, *it);
                        }
                    }

                    if (!bSingle || (aiFound[*it] == 0)) {
                        int iInter = calcIntersection(m_asRTree[k], m_aasCurIDs[*it][iCur],  m_aasCoalescents[k][*it]);
                        aiFound[*it] += iInter;
                        if (iInter != 0) {
                            printf("  (%03d: Intersection RTree %d - Cur %ld)\n", iC-1, k, *it);
                        }
                    }

                    if  (aiFound[*it] != 0) {
                        printf("sorting m_aasCoalescents[%d][%zd]\n", k, *it);
                        //std::sort(m_aasCoalescents[k][*it].begin(), m_aasCoalescents[k][*it].end(), tcomp);

                        timeset::const_iterator its;
                        for (its = m_aasCoalescents[k][*it].begin(); its != m_aasCoalescents[k][*it].end(); ++its) {
                            printf(" %f", its->second.fTime);
                        }
                        printf("\n");
                          
                    }
                }
                if (!m_asComparables[k].empty()) {
                    for (uint j = 0; j < m_iNum; ++j) {
                        if (aiFound[j] > 0) {
                            m_asComparables[k].erase(j);
                        }
                    }
                }
                m_iComparesToDo += m_asComparables[k].size();
            }
            tIntersect += omp_get_wtime() - tI0;
        }

        
        for (uint k = 0; k < m_iNum; ++k) {
            if (m_asComparables[k].empty()) {
                bool bAppears = false;
                for (uint j = 0; (!bAppears) && (j < k); ++j) {
                    if (m_asComparables[j].find(k) != m_asComparables[j].end()) {
                        bAppears = true;
                    }
                }
                if (!bAppears) {
                    abNeeded[k] = false;
                }
            }
        }
        

    }
    
    tTotal = omp_get_wtime() - tT0;
    printf("doCoalesce ends with ComparesTodo:%d res %d\n", m_iComparesToDo, iResult);
    printf("time used:\n");
    printf("Ancestors    %f\n", tAncestors);
    printf("  Block     %f\n", dBlockTot);
    printf("  Find      %f\n", dFindTot);
    printf("  Merge     %f\n", dMergeTot);
    printf("Intersect    %f\n", tIntersect);
    printf("Total        %f\n", tTotal);
    return iResult;

}

//----------------------------------------------------------------------------
// createOracle
//   the oracle is a map id_i->min_i 
//   where id_(i-1) < id <= id_i implies id is found in block starting at min_i
// 
int Coalescer::createOracle() {
    printf("DynAncGraph Creating oracle\n"); fflush(stdout);
    int iResult = -1;
    if (m_fAnc != NULL) {
        iResult = 0;
        // rewind file
        fseek(m_fAnc, 0, SEEK_SET);
        ulong iStep = m_iAncSize*m_iBlockSize;
        ulong iFilePos = m_iAncSize*m_iBlockSize;
        idtype iID[m_iAncSize];
        ulong iPrev = 0;
        printf("Filesize %ld\n", m_lFileSize);
        while ((iResult == 0) && (sizeof(idtype)*iFilePos < m_lFileSize)) {
            fseek(m_fAnc, sizeof(idtype)*(iFilePos-m_iAncSize), SEEK_SET);
            ulong iRead = fread(iID, sizeof(idtype), m_iAncSize, m_fAnc);
            if (iRead == m_iAncSize) {
                m_mOracle[iID[0]] = sizeof(idtype)*iPrev;
                //                printf("%ld -> %ld*%ld=%ld; \n", iID[0], iPrev, sizeof(idtype), sizeof(idtype)*iPrev);
                iPrev = iFilePos;
                iFilePos += iStep;
            } else {  
                printf("Read error in anc file: only read %lu from pos %ld\n", iRead, iFilePos-m_iAncSize);
                if (feof(m_fAnc)) {
                    printf("EOF!\n");
                }
                iResult = -1;
            }
        }
        if (iResult == 0) {
            // add the last element: highest ID + 1 (i.e. greater than all possible IDs)
            if (iPrev <= m_lFileSize-m_iAncSize*sizeof(idtype)) {
                fseek(m_fAnc, -(ulong)(m_iAncSize*sizeof(idtype)), SEEK_END);
                ulong iRead = fread(iID, sizeof(idtype), m_iAncSize, m_fAnc);
                if (iRead == m_iAncSize) {
                    m_mOracle[iID[0]] = iPrev; 
                    printf("final\n%ld -> %ld\n", iID[0], iPrev);
                } else {
                    printf("read %lu instead of %d\n", iRead, m_iAncSize);
                }
            }
        }
    }
    printf("oracle size %zd\n", m_mOracle.size());fflush(stdout);
    return iResult;
}


//----------------------------------------------------------------------------
// findAncestorsForIDs
// 
int Coalescer::findAncestorsForIDs(const idset &sIDs, idset &sAncestors) {
    int iResult = 0;

    double dT0 = omp_get_wtime();
    // determine which blocks to load
    idset sBlocks;
    idset_cit it;
    idset_cit it2;
    for (it = sIDs.begin(); (iResult == 0) && (it != sIDs.end()); ++it) {
        oracle::const_iterator ito = m_mOracle.lower_bound(*it);
        if (ito != m_mOracle.end()) {
            sBlocks.insert(ito->first);
        } else {
            printf("no upper bound found for id %ld\n", *it);
            printf("id %ld not present in AncFile\n", *it);
            iResult = -1;
        }
    }
    double dTBlocks = omp_get_wtime() - dT0;

    
    dT0 = omp_get_wtime();
    it = sIDs.begin();

    idset_cit itB = sBlocks.begin();
    while ((iResult == 0) && (itB != sBlocks.end())) {
        idtype iID = *itB;
        filepos iMin = m_mOracle[iID];

        fseek(m_fAnc, iMin, SEEK_SET);

        ulong iRead = fread(m_aAncBuf, m_iAncSize*sizeof(idtype), m_iBlockSize, m_fAnc);
        if (iRead > 0) {
            uint i = 0;
            while (it != sIDs.end() && (*it <= iID) && (i < iRead) && (iResult == 0)) {
                while ((i < iRead) && (m_aAncBuf[m_iAncSize*i] < *it)) {
                    i++;
                }

                        
                if (m_aAncBuf[m_iAncSize*i] == *it) {
                    // create anc node
                    // gender encoded in sign of Dad
                    if ((m_aAncBuf[m_iAncSize*i+1] != 0) && (m_aAncBuf[m_iAncSize*i+2]!=0)) {
                        sAncestors.insert(m_aAncBuf[m_iAncSize*i+1]);
                        sAncestors.insert((m_aAncBuf[m_iAncSize*i+1]==0)?0:(idtype)labs(m_aAncBuf[m_iAncSize*i+2]));
                    }
                    ++it;
                    ++i;
                } else {
                    printf(": Didn't find id %ld in block for %ld starting at %ld\n", *it, iID, iMin);
                    iResult = -1;
                }
                
            }

        } else {
            printf("Read error in Anc file\n");
            iResult = -1;
        }
        ++itB;
    }
    double dTFind = omp_get_wtime() - dT0;
    printf("... block: %f\n", dTBlocks);
    printf("... find : %f\n", dTFind);

    dBlockTot += dTBlocks;
    dFindTot  += dTFind;
    return iResult;
}

//----------------------------------------------------------------------------
// findAncestorsForIDsPar
// 
int Coalescer::findAncestorsForIDsPar(const idset &sIDs, idset &sAncestors) {
    int iResult = 0;

    double dT0 = omp_get_wtime();

    // find blocks containing node IDs
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < m_iNumBlocks; ++i) {
        idset_it its;
        for (its = sIDs.begin(); its != sIDs.end(); ++its) {
            oracle::const_iterator ito = m_mOracle.lower_bound(*its);
            if (ito != m_mOracle.end()) {
                if (ito->first == m_asBlockIDs[i].iIDFirst) {
                    m_asBlockIDs[i].sLocalIDs.insert(*its);
                }
            } else {
                printf("no upper bound found for id %ld\n", *its);
                printf("id %ld not present in AncFile\n", *its);
                iResult = -1;
            }
        }
    }
    double dTBlocks = omp_get_wtime() - dT0;
 
    dT0 = omp_get_wtime();
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < m_iNumBlocks; ++i) {
        int iRes = 0;
        int iT = omp_get_thread_num();
        idtype iIDFirst = m_asBlockIDs[i].iIDFirst;
        long lMin = m_mOracle[iIDFirst];
        idset_it its = m_asBlockIDs[i].sLocalIDs.begin();

        fseek(m_afIn[iT], lMin, SEEK_SET);
        m_apBR[iT]->reload();

        if (m_asBlockIDs[i].sLocalIDs.size() > 0) {
            ancitem sAncItem;
            while ((iRes == 0) && (its != m_asBlockIDs[i].sLocalIDs.end()) && (*its <= iIDFirst)) {
                iRes = m_apBR[iT]->getBlock((char *)&sAncItem, sizeof(ancitem));
                if (iRes == 0) {
                    if (sAncItem.iID == *its)  {
                        // only look for ancestors with IDs > 0
                        if ((sAncItem.iMomID != 0) && (sAncItem.iDadID !=0 )) {
                    
                            m_asAncestors[iT].insert(sAncItem.iMomID);
                            m_asAncestors[iT].insert((sAncItem.iMomID==0)?0:(idtype)labs(sAncItem.iDadID));
                        }
                        // in any case move to the next
                        ++its;
                        ++m_aiCounts[iT];
                        
                    }
                } else {
                    printf("[%d] got a res: %d working on its %ld with block %d (firstid %ld)\n", iT, iRes, *its, i, iIDFirst); fflush(stdout);
                }
            }

            // is everything ok?
            if (its != m_asBlockIDs[i].sLocalIDs.end()) {
                printf("T%d ,%d: Couldn't find all IDs: %ld\n", iT, i, *its);
                iRes = -1;
            }
        }
        // clean local set for next round        
        m_asBlockIDs[i].sLocalIDs.clear();

        // set main result code if necessary
        if (iRes != 0) {
            iResult = iRes;
        }
    }
    double dTFind = omp_get_wtime() - dT0;

    dT0 = omp_get_wtime();
    // merge the sets created by the threads into the main set:
    // successively merge pairs of sets in parallel;
    // merge final one with main set
    if (iResult == 0) {
        int iD = 1;
        int iNumSets = m_iNumThreads;
        while (iNumSets > 1) {
            // merge in parallel
#pragma omp parallel for
            for (int j = 0; j < iNumSets/2; ++j) {
                int k1=m_aiIndexes[2*j*iD];
                int k2=m_aiIndexes[(2*j+1)*iD];
                m_asAncestors[k1].insert(m_asAncestors[k2].begin(),m_asAncestors[k2].end()); 
            }
            iNumSets = (iNumSets+1)/2;
            iD *= 2;
        }
        // now merge with main
        sAncestors.insert(m_asAncestors[0].begin(),m_asAncestors[0].end());
    }
    
#pragma omp parallel 
    {
        m_asAncestors[omp_get_thread_num()].clear();
    }
    double dTMerge = omp_get_wtime() - dT0;

    printf("... block: %f\n", dTBlocks);
    printf("... find : %f\n", dTFind);
    printf("... merge: %f\n", dTMerge);

    dBlockTot += dTBlocks;
    dFindTot  += dTFind;
    dMergeTot += dTMerge;

    if (iResult != 0) {
        int iCount = 0;
        for (int i = 0; i < m_iNumThreads; ++i) {
            iCount += m_aiCounts[i];
            m_aiCounts[i] = 0;
        }
        
        printf("Loaded %d/%zd nodes\n", iCount, sIDs.size());
    }
    

    
    
    return iResult;
}


//----------------------------------------------------------------------------
// getLocTimesForIDs
//   for each of the ids in vIDs search the AncFile to get the relevant data:
//    time and cell ID
//
int Coalescer::getLocTimesForIDs(const std::vector<idtype> &vIDs, timelist &sTimeList) {

    int iResult = 0;

    // determine which blocks to load
    idset sBlocks;
    std::vector<idtype>::const_iterator it;
    idset_cit it2;
    for (it = vIDs.begin(); (iResult == 0) && (it != vIDs.end()); ++it) {
        oracle::const_iterator ito = m_mOracle.lower_bound(*it);
        if (ito != m_mOracle.end()) {
            sBlocks.insert(ito->first);
        } else {
            printf("no upper bound found for id %ld\n", *it);
            iResult = -1;
        }
    }
    
    
    it = vIDs.begin();
    //    printf("Searching for id %d", *it);
    idset_cit itB = sBlocks.begin();
    while ((iResult == 0) && (itB != sBlocks.end())) {
        idtype iID = *itB;
        filepos iMin = m_mOracle[iID];

        fseek(m_fAnc, iMin, SEEK_SET);
        //        printf("Reading block for %d starting at %d (real %zd)\n", iID, iMin, iMin*sizeof(int));
        ulong iRead = fread(m_aAncBuf, m_iAncSize*sizeof(idtype), m_iBlockSize, m_fAnc);
        if (iRead > 0) {
            uint i = 0;
            while (it != vIDs.end() && (*it <= iID) && (i < iRead) && (iResult == 0)) {
                while ((i < iRead) && (m_aAncBuf[m_iAncSize*i] < *it)) {
                    i++;
                }

                        
                if (m_aAncBuf[m_iAncSize*i] == *it) {
                    // found the anc record; now extract loc and time
                    idtype loc = m_aAncBuf[m_iAncSize*i+3];
                    float fTime = (float) (loc >> 32);
                    locts loce(fTime, loc & 0xffffffff); 
                    sTimeList.push_back(timeentry(*it, loce));
                    
                    ++it;
                    ++i;
                } else {
                    printf("[getTimeForIDs]: Didn't find id %ld in block for %ld starting at %ld\n", *it, iID, iMin);
                    iResult = -1;
                }
                
            }

        } else {
            printf("Read error in Anc file\n");
            iResult = -1;
        }
        ++itB;
    }
    
    return iResult;
}
