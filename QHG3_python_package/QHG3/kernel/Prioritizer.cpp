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
    }
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
        printf("Action %s tries to read \"%s\" ",it->first.c_str(),pLine);
        iResult = it->second->tryReadParamLine(pLine);
        printf("and returns %d\n", iResult);
    }
    
    return iResult;
}


#endif

