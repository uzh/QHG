#include <string.h>
#include <omp.h>
#include <math.h>

#include <algorithm>

#include "MessLogger.h"

#include "clsutils.cpp"
#include "WELL512.h"
#include "ArrayShare.h"
#include "PreyDistributor.h"
#include "SPopulation.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "MassInterface.h"
#include "PopFinder.h"
#include "PDHunting.h"

//-----------------------------------------------------------------------------
// constructor
//
template<typename T>
PDHunting<T>::PDHunting(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, PopFinder *pPopFinder)
    : Action<T>(pPop,pCG),
      m_apWELL(apWELL),
      m_dEfficiency(0),
      m_dUsability(0),
      m_pMIPred(NULL),
      m_pPopFinder(pPopFinder){
    
    *m_sRelationInput = '\0';
    
    int iNumCells = this->m_pCG->m_iNumCells;

    // will only be created once
    PreyDistributor::createInstance(iNumCells, apWELL);

}


//-----------------------------------------------------------------------------
// destructor
//
template<typename T>
PDHunting<T>::~PDHunting() {

    if (m_pPreyEff != NULL) {
        delete[] m_pPreyEff;
    }

}


//-----------------------------------------------------------------------------
// preLoop
//  prepare mass-interfaces and share preyratio array
//
template<typename T>
int PDHunting<T>::preLoop() {
    int iResult = 0;
    
    printf("[PDHunting<T>::preLoop] getting relations from [%s]\n", m_sRelationInput);
    iResult = relationFromString(m_sRelationInput);
    printf("[PDHunting<T>::preLoop] number of relations: %zd\n", m_mRelations.size());

    if (iResult == 0) {
        relation::const_iterator it;
        for (it = m_mRelations.begin(); (iResult == 0) && (it != m_mRelations.end()); ++it) {
            PopBase *pPreyPop = m_pPopFinder->getPopByName(it->first.c_str());
            if (pPreyPop != NULL) {    
                // maybe unneeded
                m_mpPreyPop[it->first] = pPreyPop;
                
                MassInterface *pMI = dynamic_cast<MassInterface *>(pPreyPop);
                if (pMI != NULL) {
                    m_mpMIPrey[it->first] = pMI;
                } else {
                    iResult = -1;
                    printf("[PDHunting<T>::preLoop()] couldn't cast prey population [%s] to MassInterface\n", it->first.c_str());
                }
            } else {
                iResult = -1;
                printf("[PDHunting<T>::preLoop()] couldn't find population for species name [%s]\n", it->first.c_str());
            }
        }   
        
        if (iResult == 0) {
            m_pMIPred = dynamic_cast<MassInterface *>(this->m_pPop);
            if (m_pMIPred != NULL) {
                iResult = 0;
            } else {
                iResult = -1;
                printf("[PDHunting<T>::preLoop()] couldn't cast pred population [%s] to MassInterface\n", m_sPredName.c_str());
            }
        }

        // build pPreyEff & share
        if ((iResult == 0) && (m_mRelations.size() > 0)) {
            m_pPreyEff = new preyratio[m_mRelations.size()];
            int i = 0;
            relation::const_iterator it2;
            for (it2 = m_mRelations.begin(); (iResult == 0) && (it2 != m_mRelations.end()); ++it2) {
                m_pPreyEff[i].first  = it2->first;
                m_pPreyEff[i].second = it2->second.first;
                i++;
            }

            char s[256];
            sprintf(s, ATTR_PD_TEMPLATE_PREY, m_sPredName.c_str());
            ArrayShare::getInstance()->shareArray(s, m_mRelations.size(), m_pPreyEff);
            printf("[PDHunting<T>::preLoop()] Shared m_pPreyEff as [%s]\n", s);

            // call PreyDistributor's addRelation (via getinstance etc)
            PreyDistributor::getInstance()->registerPredator(m_sPredName.c_str());
            printf("[PDHunting<T>::preLoop()] added relation [%s]\n", m_sPredName.c_str());

        } else {
            printf("[PDHunting<T>::preLoop()] Need at least one prey relation\n");
            iResult = -1;
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initialize
// here the couples are created
//
template<typename T>
int PDHunting<T>::initialize(float fT) {
    
    int iResult = 0;
    // call PreyDistributor's buildAssignments (via getinstance etc)
    iResult = PreyDistributor::getInstance()->buildAssignments(this->m_pPop->getSpeciesName(), fT);
    
    return iResult;
}


//-----------------------------------------------------------------------------
// action: operator()
//
template<typename T>
int PDHunting<T>::operator()(int iAgentIndex, float fT) {

    int iResult = 0;

    // use shared array "<predname>_ass" to detemine actual hunt results
    char s[256];
    sprintf(s, ATTR_PD_TEMPLATE_ASSMAP, this->m_pPop->getSpeciesName());
    assignmentmap* pAss = (assignmentmap *) ArrayShare::getInstance()->getArray(s);
    if (pAss != NULL) {
        T *pa = &(this->m_pPop->m_aAgents[iAgentIndex]);
        if (pa->m_iLifeState > 0) {
            // m_mpPreyPop: preyname => popBase
            // m_mpMIPrey:  preyname => (MassInterface)preyPop
            // m_pMIPred:   (MassInterface) thisPop
    
            int iCellIndex = pa->m_iCellIndex;
            std::map<std::string, std::vector<int> > mvPreyIndexes;
            
            // loop through preys in assignment map
            assignmentmap::const_iterator itPrey;
            for (itPrey = pAss[iCellIndex].begin(); itPrey != pAss[iCellIndex].end(); ++itPrey) {
                const std::set<intpair> &sPairs = itPrey->second;
                //  loop through vector of pairs
                std::set<intpair>::const_iterator itPairs;
                for (itPairs = sPairs.begin(); itPairs != sPairs.end(); ++itPairs) {
                    if (itPairs->first == iAgentIndex) {
                        // only do stuff to living agents
                        if (this->m_mpPreyPop[itPrey->first]->getAgentLifeState(itPairs->second) > LIFE_STATE_DEAD) {
                            mvPreyIndexes[itPrey->first].push_back(itPairs->second);
                        }
                    }
                }
            }

            // after loop: decide which ones to eat  -> "filter" mvPreyIndexes
            // for now use "as is"
        
            // loop through ids from mvPreyIndexes[preyname]
            std::map<std::string, std::vector<int> >::const_iterator itSel;
            for (itSel = mvPreyIndexes.begin(); itSel != mvPreyIndexes.end(); ++itSel) {
                double dUsability = m_mRelations[itSel->first].second;
                for (uint i = 0; i < itSel->second.size(); ++i) {
                    double dM = m_mpMIPrey[itSel->first]->getMass(itSel->second[i]);
                    m_pMIPred->addMass(iAgentIndex, dM * dUsability);
                    /*
                    if (strcmp(itSel->first.c_str(), "PDSimplePrey") == 0) {
                        printf("*** Killing [%s]#%d (id %ld) (LS %d) in Cell %d\n", 
                           itSel->first.c_str(), 
                           itSel->second[i], 
                           this->m_mpPreyPop[itSel->first]->getAgentID(itSel->second[i]), 
                           this->m_mpPreyPop[itSel->first]->getAgentLifeState(itSel->second[i]), 
                           iCellIndex);
                    }
                    */
                    m_mpPreyPop[itSel->first]->registerDeath(iCellIndex, itSel->second[i]);
                }
            }
        }
    } else {
        /*debug:
        printf("[PDHunting<T>::operator()] no assmap for [%s]\n", this->m_pPop->getSpeciesName());
        */
    }

   
    return iResult;
}


//-----------------------------------------------------------------------------
// postLoop
//  prepare mass-interfaces and share preyratio array
//
template<typename T>
int PDHunting<T>::postLoop() {
    printf("yipppediyiyi postLoop\n");
    PreyDistributor::freeInstance();
    return 0;
}


//-----------------------------------------------------------------------------
// relationFromString
//  fill m_mRelations from string of form
//    <predName>"|"<prey_name>":"<efficiency>":"<usability>[","(<prey_name>":"<efficiency>":"<usability>)]*
//
template<typename T>
int PDHunting<T>::relationFromString(char *pPredRelations) {
    char *pCur1;
    int iResult = -1;
    char *pRelations = strchr(pPredRelations, '|');
    if (pRelations != NULL) {
        *pRelations = '\0';
        pRelations++;
        m_sPredName = pPredRelations;
        iResult = 0;
        char *pNext = strtok_r(pRelations, ",", &pCur1);
        if (pNext != NULL) {
            while ((pNext != NULL) && (iResult == 0)) {
                iResult = -1;
                char *pCur2;
                char *pName =  strtok_r(pNext, ":", &pCur2);
                if (pName != NULL) {
                    double dEfficiency = 0;
                    char *pEff = strtok_r(NULL, ":", &pCur2);
                    if (pEff != NULL) {
                        if (strToNum(pEff, &dEfficiency)) {
                            double dUsability = 0;
                            char *pUse = strtok_r(NULL, ":", &pCur2);
                            if (pUse != NULL) {
                                if (strToNum(pUse, &dUsability)) {
                                    printf("[PDHunting<T>::relationFromString] successsfully extracted relation for %s: %s,%f,%f\n", m_sPredName.c_str(), pName, dEfficiency,dUsability);
                                    m_mRelations[pName] = doublepair(dEfficiency, dUsability);
                                    printf("[PDHunting<T>::relationFromString] num rels: %zd\n", m_mRelations.size());
                                    iResult = 0;
                                } else {
                                    printf("[PDHunting<T>::relationFromString] Expected efficiency (double) [%s]\n", pEff);
                                }
                            } else {
                                printf("[PDHunting<T>::relationFromString] Expected usability\n");
                            }
                        } else {
                        printf("[PDHunting<T>::relationFromString] Expected efficiency (double) [%s]\n", pEff);
                        }
                    } else {
                        printf("[PDHunting<T>::relationFromString] Expected efficiency\n");
                    }
                } else {
                    printf("[PDHunting<T>::relationFromString] Bad string format\n");
                }
                pNext = strtok_r(NULL, ",", &pCur1);
            }
        } else {
            printf("[PDHunting<T>::relationFromString] expected comma-separated list of relations\n");
            iResult = -1;
        }
    } else {
        printf("[PDHunting<T>::relationFromString] Bad string format: expected '<predname>|<relations>'\n");
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// extractParamsQDF
//
//  tries to read the attributes
//    ATTR_PDHUNTING_RELATIONS_NAME
//
template<typename T>
int PDHunting<T>::extractParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_extractSAttribute(hSpeciesGroup, ATTR_PDHUNTING_RELATIONS_NAME, NAME_LEN, m_sRelationInput);
        if (iResult != 0) {
            LOG_ERROR("[PDHunting] couldn't read attribute [%s]", ATTR_PDHUNTING_RELATIONS_NAME);
        }
    }
    
    return iResult; 
}

//-----------------------------------------------------------------------------
// writeParamsQDF
//
//  tries to write the attributes
//    ATTR_PDHUNTING_RELATIONS_NAME
//
template<typename T>
int PDHunting<T>::writeParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;

    // build a string from the relations
    char sRelations[NAME_LEN];
    *sRelations = '\0';
    relation::const_iterator it;
    for (it = m_mRelations.begin(); it != m_mRelations.end(); ++it) {
        char sPart[256];
        // the first '%s' is for the comma if it is not the first part
        sprintf(sPart, "%s%s:%f:%f", (*sRelations != '\0')?",":"" , it->first.c_str(), it->second.first, it->second.second);
        strcat(sRelations, sPart); 
    }

    iResult += qdf_insertSAttribute(hSpeciesGroup, ATTR_PDHUNTING_RELATIONS_NAME, sRelations);

    return iResult; 
}

//-----------------------------------------------------------------------------
// tryReadParamLine
// this is for reading params out of a pop file
//
template<typename T>
int PDHunting<T>::tryReadParamLine(char *pLine) {

   int iResult = 0;
   char *pVal = readKeyString(pLine, ATTR_PDHUNTING_RELATIONS_NAME, "=");
   if (pVal != NULL) {
       strncpy(m_sRelationInput, pVal, NAME_LEN);
       iResult += 1;
   } else {
       iResult += 0;
   }
      
   return iResult;
 }


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename T>
void PDHunting<T>::showAttributes() {
    
    printf("  %s\n", ATTR_PDHUNTING_RELATIONS_NAME);
}
