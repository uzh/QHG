#ifndef __PRIORITIZER_CPP__
#define __PRIORITIZER_CPP__

#include <map>
#include <vector>
#include <set>
#include <string>

#include "Prioritizer.h"
#include "Action.h"


//-----------------------------------------------------------------------------
// setPrio
//
template<typename A>
void Prioritizer<A>::setPrio(uint iPrio, std::string name) {
    //    m_prios[iPrio].push_back(act);
    if (m_names.find(name) != m_names.end()) {
        m_prios[iPrio].push_back(m_names[name]);
        printf("Prioritizer set Prio for [%s] to %u\n", name.c_str(), iPrio);
    } else {
        printf("WARNING: Prioritizer tried to add non-existing action %s\n",name.c_str());
    }
}


//-----------------------------------------------------------------------------
// addAction
//
template<typename A>
void Prioritizer<A>::addAction(std::string name, Action<A> *act) {
    if (m_names.find(name) != m_names.end()) {
        printf("WARNING: there already is an action with name [%s]\n", name.c_str());
    } else {
        m_names[name] = act;
        printf("Prioritizer added actions [%s]\n", name.c_str());
    }
}


//-----------------------------------------------------------------------------
// removeAction
//
template<typename A>
int Prioritizer<A>::removeAction(std::string name) {
    int iResult = -1;
    // find the action for the name
    typename namelist::iterator itN = m_names.find(name);
    if (itN != m_names.end()) {
        std::set<uint> sDeletable;
        Action<A> *pAction = itN->second;

        // loop through meth list  erasing action from every vector
        typename methlist::iterator itM;
        for (itM = m_prios.begin(); itM != m_prios.end(); ++itM) {
            typename std::vector<Action<A> *>::iterator itV;
            for (itV = itM->second.begin(); itV != itM->second.end(); ) {
                if (*itV == pAction) {
                    itM->second.erase(itV);
                } else {
                    itV++;
                }
            }
            if (itM->second.size()==0) {
                // if vector is empty, this entry may be deleted
                sDeletable.insert(itM->first);
            }
        }

        // delete entries with empty vectors
        std::set<uint>::const_iterator itS;
        for (itS = sDeletable.begin(); itS != sDeletable.end(); itS++) {
            m_prios.erase(*itS);
        }
	m_names.erase(itN);
        
    } else {
        printf("Action [%s] not found\n", name.c_str());
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getPrios
//
template<typename A>
uint Prioritizer<A>::getPrios( std::set<uint> &vPrios) {
    typename methlist::const_iterator it;
    for (it = m_prios.begin(); it != m_prios.end(); ++it) {
        vPrios.insert(it->first);
    }
    return (uint) vPrios.size();
}


//-----------------------------------------------------------------------------
// extractActionParamsQDF
//
template<typename A>
int Prioritizer<A>::extractActionParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        iResult = it->second->extractParamsQDF(hSpeciesGroup);
    }
    if (iResult != 0) {
        showAttributes();
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// writeActionParamsQDF
//
template<typename A>
int Prioritizer<A>::writeActionParamsQDF(hid_t hSpeciesGroup) {
    
    int iResult = 0;
    typename namelist::const_iterator it;
    
    // we cycle through all actions and extract parameters
    // until we finish or until we find an error
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        fprintf(stderr,"adding parameters for action %s\n",it->first.c_str());
        iResult = it->second->writeParamsQDF(hSpeciesGroup);
    }
    if (iResult != 0) {
        showAttributes();
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// readActionParamLine
//
template<typename A>
int Prioritizer<A>::readActionParamLine(char *pLine) {

    int iResult = 0;
    
    // we try reading this param line 
    // until some action succeeds in interpreting it
    // if none does, we return -1 to SPopulation::readSpeciesData
    typename namelist::const_iterator it;
    for (it = m_names.begin(); iResult == 0 && it != m_names.end(); ++it) {
        //printf("Action %s tries to read \"%s\" ",it->first.c_str(),pLine);
        iResult = it->second->tryReadParamLine(pLine);
        //printf("and returns %d\n", iResult);
    }
    // here 0 means failure
    if (iResult == 0) {
        printf("ERROR Line [%s] not recognized by any Action\n", pLine);
        showAttributes();
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// showAttributes
//
template<typename A>
void Prioritizer<A>::showAttributes() {
    printf("++++++++++++++++++++++++++++++++++++++++\n");
    printf("Required attributes for this population:\n");
    typename namelist::const_iterator it;
    for (it = m_names.begin(); it != m_names.end(); ++it) {
        printf("[%s]\n", it->first.c_str());
        it->second->showAttributes();
    }
    printf("++++++++++++++++++++++++++++++++++++++++\n");
}


#endif

