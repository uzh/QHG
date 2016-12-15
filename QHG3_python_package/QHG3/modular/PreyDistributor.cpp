#include <stdio.h>
#include <string.h>

#include <omp.h>

#include "WELL512.h"
#include "ArrayShare.h"

#include "PreyDistributor.h"

PreyDistributor *PreyDistributor::s_pPD = NULL;

//---------------------------------------------------------------------------
// createInstance
//
PreyDistributor *PreyDistributor::createInstance(int iNumCells, WELL512 **apWELL) {
    if (s_pPD == NULL) {
        s_pPD = new PreyDistributor(iNumCells, apWELL);
    }
    return s_pPD;
}

//---------------------------------------------------------------------------
// getInstance
//
PreyDistributor *PreyDistributor::getInstance() {
    return s_pPD;
}

//---------------------------------------------------------------------------
// freeInstance
//
void PreyDistributor::freeInstance() {
    if (s_pPD != NULL) {
        delete s_pPD;
        s_pPD = NULL;
    }
}


//---------------------------------------------------------------------------
// constructor
//
PreyDistributor::PreyDistributor(int iNumCells, WELL512 **apWELL)
    : m_iNumCells(iNumCells),
      m_apWELL(apWELL),
      m_fLastTime(-1)  {
    
      m_avNum = new std::map<std::string, std::vector<int> >[iNumCells];  
      m_iNumThreads =  1;
#ifdef OMP_A
      m_iNumThreads = omp_get_max_threads();
#endif

      /*
      //@@ not really necessary: maps for statistics
      m_amUnmatchedBadHunting = new std::map<std::string, intset>[m_iNumThreads];
      m_amUnmatchedNoHunting  = new std::map<std::string, intset>[m_iNumThreads];
      */
}


//---------------------------------------------------------------------------
// destructor
//
PreyDistributor::~PreyDistributor() {
    if (m_avNum != NULL) {
        delete[] m_avNum;
    }

    std::map<std::string, assignmentmap* > ::const_iterator itf;
    for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {
        if (itf->second != NULL) {
	    delete[] itf->second;
	}
    }

    /*
    //@@ not really necessary
    delete[] m_amUnmatchedBadHunting;
    delete[] m_amUnmatchedNoHunting;
    */
}


//---------------------------------------------------------------------------
// registerPredator
//   expects shared preyratio array with name "<predname>_prey"
//
//   list of predator's prey species is added to m_mRelation struct
//
int PreyDistributor::registerPredator(const char *pPredName) {
    int iResult = -1;

    char s1[256];
    sprintf(s1, PD_TEMPLATE_PREY, pPredName);
    preyratio *pPredPrey = (preyratio*) ArrayShare::getInstance()->getArray(s1);
    if (pPredPrey != NULL) {
        printf("[PreyDistributor::registerPredator] adding preyratio array [%s]\n", s1);

        int iNum = ArrayShare::getInstance()->getSize(s1);
        for (int i = 0; i < iNum; ++i) {
            m_mRelations[pPredPrey[i].first].push_back(preyratio(pPredName, pPredPrey[i].second));
        }
        iResult = 0;
    } else {
        printf("[PreyDistributor::registerPredator] couldn't get preyratio array [%s]\n", s1);
    }
    
    // make sure there is an entry for this predator
    m_mbReady[pPredName] = false;
 
    return iResult;
}
    

//---------------------------------------------------------------------------
// getFrequencies
//   fill avNum with the cumulated numbers of predators in each cell
//   e.g.:
//     avNum[iCell][k] = sum_0^k NP(k, iCell)
//   where
//     NP(k, iCell) : number of agents of predatorspecies #k in cell iCell
//
//   expects shared intlist array with name "<predname>_indexes"
//
int PreyDistributor::getFrequencies() {
    int iResult = 0;

    printf("[PreyDistributor::getFrequencies] started\n");
    printf("[PreyDistributor::getFrequencies] doing %zd prey types\n", m_mRelations.size()); 
#ifdef OMP_A
#pragma omp parallel for reduction(+:iResult)
#endif
    for (int iCell = 0;  iCell < m_iNumCells; iCell++) {
        //    for (int iCell = 0; (iResult == 0) && (iCell < m_iNumCells); iCell++) {
        m_avNum[iCell].clear();
        std::map<std::string, relationvec>::const_iterator it;
        for (it = m_mRelations.begin(); (iResult == 0) && (it != m_mRelations.end()); ++it) {
            int iCum = 0;
            relationvec::const_iterator it2;
            for (it2 = it->second.begin(); (iResult == 0) && (it2 != it->second.end()); ++it2) {
                char s[256];
                sprintf(s, PD_TEMPLATE_INDEXES, it2->first.c_str());
                if (iCell == 0) {
                    printf("[PreyDistributor::getFrequencies] looking at pred array [%s] for prey [%s]\n", s, it->first.c_str());
                }
                intlist *pArr = (intlist *)ArrayShare::getInstance()->getArray(s); 
                if (pArr != NULL) {
                    iCum += pArr[iCell].size(); 
                    m_avNum[iCell][it->first].push_back(iCum);
                } else {
                    printf("[PreyDistributor::getFrequencies] required array [%s] not found\n", s);
                    iResult = -1;
                } 
            }
        }
        
        /* debug: show avNum
        if (m_avNum[iCell].size() == 0) {
            printf("Nothing for cell %d\n", iCell);
        } else {
            std::map<std::string, intlist>::const_iterator it2;
            for (it2 = m_avNum[iCell].begin(); it2 != m_avNum[iCell].end(); ++it2) {
                printf("  C%03d[%s]: %zd: ", iCell, it2->first.c_str(), it2->second.size());
                for (uint i = 0; i < it2->second.size(); i++) {
                    printf(" %d", it2->second[i]);
                }
                printf("\n");
            }
        }
        */
    }
    return iResult;
}


//---------------------------------------------------------------------------
// calcAssignments
//
//   m_Ass[predName][iCell][preyName] : vector of pairs (iPredID, iPreyID)
//                                      iPredID: id of agent type predName
//                                      iPreyID: id of agent type preyName
//   e.g., list of assignments to agents of type predName in cell iCell,
//   listed separately for each prey type.
//   
//   expects shared intlist array with name "<preyname>_indexes"
//   expects shared intlist array with name "<predname>_indexes"
//
int PreyDistributor::calcAssignments() {
    int iResult = 0;

    // properly clean up
    std::map<std::string, assignmentmap* > ::const_iterator itf;
    for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {

#ifdef OMP_A
#pragma omp parallel for
#endif
        for (int iCell = 0; iCell < m_iNumCells; iCell++) {
            itf->second[iCell].clear();
        }
        delete[] itf->second;
        
        char s[512];
        sprintf(s, PD_TEMPLATE_ASSMAP, itf->first.c_str());
        ArrayShare::getInstance()->removeArray(s);

    }
    
    m_Ass.clear();
    

    double t0 = omp_get_wtime();
    
    /*
    //@@ not really necessary
#ifdef OMP_A 
#pragma omp parallel
    {
#endif
        m_amUnmatchedBadHunting[omp_get_thread_num()].clear();
        m_amUnmatchedNoHunting[omp_get_thread_num()].clear();
#ifdef OMP_A 
    }
#endif
    */

#ifdef OMP_A 
    omp_lock_t lock0;
    omp_init_lock(&lock0);
#endif
    
    printf("[PreyDistributor::calcAssignments] doing %zd prey types\n", m_mRelations.size()); 
    std::map<std::string, relationvec>::const_iterator it;
    
    for (it = m_mRelations.begin(); (iResult == 0) && (it != m_mRelations.end()); ++it) {
        // get array of prey indexes for current type
        char s[256];
        sprintf(s, PD_TEMPLATE_INDEXES, it->first.c_str());
        printf("Getting indexes for prey [%s]\n", s);
        int iNumPrey = ArrayShare::getInstance()->getSize(s); 
        if (iNumPrey > 0) {
            intlist *pPreyIdx = (intlist *) ArrayShare::getInstance()->getArray(s);
#ifdef OMP_A
#pragma omp parallel for
#endif
            for (int iCell = 0; iCell < m_iNumCells; iCell++) {
                int iThread = omp_get_thread_num();
                
                intlist &v = m_avNum[iCell][it->first];
                if (v.back() > 0) { 

                    for (uint iPreyIndex = 0; iPreyIndex < pPreyIdx[iCell].size(); iPreyIndex++) {
                        int iPreyID = pPreyIdx[iCell][iPreyIndex];
                        
                        // assign a predator agent to prey #iPreyIndex of species s
                        
                        // pick one from total number of predators
                        int iPredIndex = (int) m_apWELL[iThread]->wrandr(0, v.back());
                        uint k = 0;
                        
                        
                        // find index of pred type in this cell containing picked agent
                        while ((k < v.size()) && (iPredIndex >= v[k])) {
                            k++;
                        }
                        if (k > 0) {
                            iPredIndex -= v[k-1];
                        }
                        
                        // find prey ratio (hunt efficiency) in m_mRelations
                        float fPreyRatio = -1;
                        relationvec &vR = m_mRelations[it->first];
                        for (uint j = 0; (fPreyRatio < 0) && (j < vR.size()); j++) {
                            if (strcmp(it->second[k].first.c_str(), vR[j].first.c_str()) == 0) {
                                fPreyRatio = vR[j].second;
                            }    
                        }
                        
                        if (fPreyRatio >= 0) {
                            double dR0 = m_apWELL[iThread]->wrandd();
                            //@@@                            printf("[%s]id %d c %d r:%f/%f\n", it->first.c_str(), pPreyIdx[iCell][iPreyIndex], iCell, dR0, fPreyRatio);
                            
                            if (dR0 < fPreyRatio) {
                                char sPredName[256];
                                sprintf(sPredName, PD_TEMPLATE_INDEXES, it->second[k].first.c_str());
                                intlist *pPredIdx = (intlist*)ArrayShare::getInstance()->getArray(sPredName);
                                
                                if (pPredIdx != NULL) {
                                    if (pPredIdx[iCell].size() > 0) {
                                        // if no assignment has been made for this predator type,
                                        // we need to create array (ATTENTION more than one thread might do this)
                                        std::map<std::string, assignmentmap* > ::const_iterator itf;
                                        itf = m_Ass.find(it->second[k].first);
                                        if (itf == m_Ass.end()) {
                                            // this block only happens a few times (mostly at the beginning)
#ifdef OMP_A
                                            omp_set_lock(&lock0); 
                                            // repeat the find() in case somthing has changed in the mean time
                                            itf = m_Ass.find(it->second[k].first);
                                            if (itf == m_Ass.end()) {
#endif
                                                m_Ass[it->second[k].first] = new assignmentmap[m_iNumCells];
                                                //                                                printf("created for pred[%s]: %p (T:%f)\n", it->second[k].first.c_str(), m_Ass[it->second[k].first], m_fLastTime);
#ifdef OMP_A
                                            }
                                            omp_unset_lock(&lock0);
#endif
                                        }

                                        // get id from index
                                        int iPredID = pPredIdx[iCell][iPredIndex];
                                        // do the assignment
                                        m_Ass[it->second[k].first][iCell][it->first].insert(intpair(iPredID, iPreyID));
                                        //                                        printf("added (%d,%d) to ass[%s][%d][%s]\n", iPredID, iPreyID, it->second[k].first.c_str(), iCell, it->first.c_str());
                                    } else {
                                        // shouldn't happen
                                        printf("o-oh: no agents of [%s] in cell %d\n", it->second[k].first.c_str(), iCell);
                                        iResult = -1;
                                    }
                                } else {
                                    // shouldn't happen
                                    printf("o-oh: array [%s] not found\n", sPredName);
                                    iResult = -1;
                                }
                            } else {
                                //printf("Failed hunt of [%s] on [%s] in cell %d\n", it->second[k].first.c_str(), it->first.c_str(), iCell); 
                                //@@ not really necessary
                                // for statistics: number of failed hunts
                                // m_amUnmatchedBadHunting[iThread][it->first].insert(iPreyID);
                            }
                        } else {
                            // shouldn't happen
                            printf("o-oh: ratio for pred [%s] prey [%s] not found\n", it->second[k].first.c_str(),it->first.c_str());
                            iResult = -1;
                        }
                    }
                } else {
                    /* debug: report unmatched
                    printf("Cell %d: unmatched [%s]: ", iCell, it->first.c_str());
                    for (uint i = 0; i < v.size(); ++i) {
                        printf(" %d", v[i]);
                    }
                    printf("\n");
                    */
                    //@@ not really necessary
                    // for statistics: number of preys in predator-free cells
                    //                    m_amUnmatchedNoHunting[iThread][it->first].insert(iPreyID);
                }  
                
            }
        } else {
            printf("No prey indexes [%s]\n", s);
            iResult = -1;
        }
    }

  
#ifdef OMP_A
    omp_destroy_lock(&lock0);
#endif
    
    printf("Used %fs\n", omp_get_wtime() - t0);
    
    return iResult;
}


//---------------------------------------------------------------------------
// buildAssignments
//  cgets frequencies, calculates assigments and shares assignments
//
//   shares assignmentmap arrays with names "<predname>_ass"
//
int PreyDistributor::buildAssignments(const char *pPredName, float fTime) {
    int iResult = 0;
    m_mbReady[pPredName] = true;
    flagmap::iterator itr;
    bool bReady = true;
    for (itr=m_mbReady.begin(); itr != m_mbReady.end(); itr++) {
        bReady = bReady && itr->second;
    }

    if (bReady && (fTime > m_fLastTime)) {
        for (itr=m_mbReady.begin(); itr != m_mbReady.end(); itr++) {
            itr->second = false;
        }

        m_fLastTime = fTime;

        iResult = getFrequencies();
        if (iResult == 0) {
            iResult = calcAssignments();
         
            if (iResult == 0) {
                printf("[PreyDistributor::buildAssignments] Share arrays for %zd predators\n",  m_Ass.size());
                // share arrays for each predator
                std::map<std::string, assignmentmap* > ::const_iterator itf;
                for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {
                    char s[512];
                    sprintf(s, PD_TEMPLATE_ASSMAP, itf->first.c_str());
                    /*
                    assignmentmap* pAss = (assignmentmap *)ArrayShare::getInstance()->getArray(s);
                    if (pAss != NULL) {
                        printf("c deleted for pred[%s]: %p\n", itf->first.c_str(), pAss);
                        for (int i  = 0; i < m_iNumCells; i++) {
                            pAss[i].clear();
                        }
                        delete[] pAss;
                    }
                    */
                    printf("[PreyDistributor::buildAssignments] sharing array for [%s] (%d):%p\n", s, m_iNumCells,itf->second);
                    ArrayShare::getInstance()->removeArray(s);
                    ArrayShare::getInstance()->shareArray(s, m_iNumCells, itf->second);
                }
            }   
        }
    }

    return iResult;
}


//---------------------------------------------------------------------------
// showRelations
//
void PreyDistributor::showRelations() {
   std::map<std::string, relationvec>::const_iterator it;
   for (it = m_mRelations.begin(); it != m_mRelations.end(); it++) {
       printf("  %s : ", it->first.c_str());
       relationvec::const_iterator it2;
       for (it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
       	   printf(" %s:%.02f", it2->first.c_str(), it2->second);
       }
       printf("\n");
   }
}


//---------------------------------------------------------------------------
// showFrequencies
//
void PreyDistributor::showFrequencies() {

    for (int iCell = 0; iCell < m_iNumCells; iCell++) {
        printf("C%02d:\n", iCell);
        std::map<std::string, intlist>::const_iterator it;
	for (it = m_avNum[iCell].begin(); it != m_avNum[iCell].end(); ++it) {
	    printf("  [%s]: ", it->first.c_str());
	    for (uint j = 0; j < m_avNum[iCell][it->first].size(); j++) {
	        printf(" %d", m_avNum[iCell][it->first][j]);
	    }
	    printf("\n");
	}
    }
}


//---------------------------------------------------------------------------
// showAssignments
//
void PreyDistributor::showAssignments() {

    std::map<std::string, assignmentmap* > ::const_iterator itf;
    for (itf = m_Ass.begin(); itf != m_Ass.end(); ++itf) {
        printf("assignment [%s]\n", itf->first.c_str());
        for (int iCell = 0; iCell < m_iNumCells; iCell++) {
	    printf("  C%02d\n", iCell);
            assignmentmap::const_iterator ita;
            for (ita = itf->second[iCell].begin(); ita != itf->second[iCell].end(); ++ita) {
                printf("     [%s]: ", ita->first.c_str());
                std::set<intpair>::const_iterator itp;
                for (itp = ita->second.begin(); itp != ita->second.end(); ++itp) {
                    printf(" (%d,%d)", itp->first, itp->second);
                }
                ///		for (uint j = 0; j < ita->second.size(); j++) {
                    ///                    printf(" (%d,%d)", ita->second[j].first, ita->second[j].second);
                    ///		}
		printf("\n");
	    }
	}
    }
}

//---------------------------------------------------------------------------
// showAgentAssignments
//
void PreyDistributor::showAgentAssignments(int iCellID, const char *pPredName, int iAgentIndex) {
    assignmentmap mAss = m_Ass[pPredName][iCellID];
    printf("Assignments to id %d of species [%s] (cell %d)\n", iAgentIndex, pPredName, iCellID); 
    assignmentmap::const_iterator ita;
    for (ita = mAss.begin(); ita != mAss.end(); ++ita) {
        printf("     [%s]: ", ita->first.c_str());
        std::set<intpair>::const_iterator itp;
        for (itp = ita->second.begin(); itp != ita->second.end(); ++itp) {
            if  (itp->first == iAgentIndex) {
                printf(" %d", itp->second);
            }
            //        for (uint j = 0; j < ita->second.size(); j++) {
            //            if  (ita->second[j].first == iAgentIndex) {
            //                printf(" %d", ita->second[j].second);
            //            }
        }
        printf("\n");
    }
}


/*
//---------------------------------------------------------------------------
// showUnmatched
//  
//@@ not really necessary
//
void PreyDistributor::showUnmatched() {
    
    // sum "over threads"
    for (int i = 1; i < m_iNumThreads; i++) {
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedBadHunting[i].begin(); it0 != m_amUnmatchedBadHunting[i].end(); ++it0) {
            m_amUnmatchedBadHunting[0][it0->first].insert(it0->second.begin(), it0->second.end());
        }
    }

    for (int i = 1; i < m_iNumThreads; i++) {
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedNoHunting[i].begin(); it0 != m_amUnmatchedNoHunting[i].end(); ++it0) {
            m_amUnmatchedNoHunting[0][it0->first].insert(it0->second.begin(), it0->second.end());
        }
    }


    if (m_amUnmatchedBadHunting[0].size() > 0) {
        printf("Bad Hunting\n");
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedBadHunting[0].begin(); it0 != m_amUnmatchedBadHunting[0].end(); ++it0) {
            printf("  [%s]\n", it0->first.c_str());
            intset::const_iterator it1;
            for (it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
                printf(" %d", *it1);
            }
            printf("\n");
        }
    }

    if (m_amUnmatchedNoHunting[0].size() > 0) {
        printf("No Hunting\n");
        std::map<std::string, intset>::const_iterator it0;
        for (it0 = m_amUnmatchedNoHunting[0].begin(); it0 != m_amUnmatchedNoHunting[0].end(); ++it0) {
            printf("  [%s]\n", it0->first.c_str());
            intset::const_iterator it1;
            for (it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
                printf(" %d", *it1);
            }
            printf("\n");
        }
    }
    
}
*/
