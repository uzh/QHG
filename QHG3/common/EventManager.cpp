/*============================================================================
| EventManager
| 
|  Manages QHG EventData associated with Trigger objects determining their
|  activation.
| 
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "utils.h"
#include "types.h"
#include "Triggers.h"
#include "EventData.h"
#include "EventManager.h"

//--------------------------------------------------------------
// constructor
//
EventManager::EventManager() 
    :   m_dNextTime(dNegInf),
        m_bStarted(false) {

}


//--------------------------------------------------------------
// destructor
//
EventManager::~EventManager() {
    evttrig::const_iterator it;
    for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
        delete it->first;
        delete it->second;
    }
}


//--------------------------------------------------------------
// start
//
void EventManager::start() {
    if (!m_bStarted) {
        m_vEDSet.clear();
        evttrig::const_iterator it;
        for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
            it->second->calcNextTriggerTime(false);
        }
        m_bStarted = true;
    }
}


//--------------------------------------------------------------
// clear
//
void EventManager::clear() {
    m_vEDT.clear();
}


//--------------------------------------------------------------
// force
//
const std::vector<EventData*> &EventManager::force() {
    m_vEDSet.clear();
    evttrig::const_iterator it;
    for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
        m_vEDSet.push_back(it->first);
    }
    return m_vEDSet;
}


//--------------------------------------------------------------
// load
//
int EventManager::loadEventItem(EventData *pED, Triggers *pT, float fCurTime) {
    int iResult = -1;
    
    if ((pED != NULL) && (pT != NULL)) {
        evttrig::iterator it;
        it =  m_vEDT.find(pED);
        if (it != m_vEDT.end()) {
            Triggers *pT2 = m_vEDT[pED];
            pT2->merge(pT);
        } else {
            m_vEDT[pED] = pT;
        }

        if (fCurTime >= 0) {
            m_vEDT[pED]->calcNextTriggerTime(true);
        } else {
            updateEventList();
        }

        iResult =0;
    } else {
        iResult = -1;
    }
    

    return iResult;
}


//--------------------------------------------------------------
// hasNewInterval
//
bool EventManager::hasNewEvent(float fT) {
    if (isinf(m_dNextTime)) {
        m_dNextTime = findNextEvent();
    }
    return (fT >= m_dNextTime) || isinf(fT);

}


//--------------------------------------------------------------
// forwardTo
//
void EventManager::forwardTo(float fStartTime) {
    if (isinf(m_dNextTime)) {
        m_dNextTime = findNextEvent();
    }
    while (m_dNextTime < fStartTime) {
        updateEventList();
        m_dNextTime = findNextEvent();
    }
}


//--------------------------------------------------------------
// triggerAll
//
void EventManager::triggerAll(float fTime) {
    char sTrigger[32];

    evttrig::const_iterator it;
    for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
        sprintf(sTrigger, "[%f]", fTime);
        Triggers *pT = Triggers::createTriggers(sTrigger, it->second->getOffset());
        it->second->merge(pT);
    }
}


//--------------------------------------------------------------
// findNextEvent
//
double EventManager::findNextEvent() {
    double dMin=dPosInf;
    m_vEDSet.clear();
    // first find smallest time in all Trigger objects
    evttrig::const_iterator it;
    for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
        if (it->second->getNextTriggerTime() < dMin) {
            dMin = it->second->getNextTriggerTime();
        }
    }
    // now find all events scheduled for that time
    for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
        if (it->second->getNextTriggerTime() == dMin) {
            bool bNew = true;
            for (unsigned int i = 0; bNew && (i < m_vEDSet.size()); i++) {
                if (it->first->equals(m_vEDSet[i])) {
                    bNew = false;
                }
            }
            if (bNew) {
                m_vEDSet.push_back(it->first);
            }
        }
    }
    printf("[EventManager::findNextEvent()] next time: %f\n", dMin);
    return dMin;
}


//--------------------------------------------------------------
// findFinalEvents
//
void EventManager::findFinalEvents() {

    m_vEDSet.clear();
    evttrig::const_iterator it;
    for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
        if (it->second->hasFinal()) {
            m_vEDSet.push_back(it->first);
        }
    }
    printf("[EventManager::findFinalEvents] found %zd \"final\" triggers\n", m_vEDSet.size());

}


//--------------------------------------------------------------
// getEventData
//
const std::vector<EventData*> &EventManager::getEventData() {
    if (isinf(m_dNextTime)) {
        //printf("...iiinf\n");
        m_dNextTime = findNextEvent();
    }
    
    updateEventList();
    return m_vEDSet;
}


//--------------------------------------------------------------
// updateEventList
//
void EventManager::updateEventList() {
    double dMin = fPosInf;
    for (unsigned int i =0; i < m_vEDSet.size(); i++) {
        Triggers *pT =  m_vEDT[m_vEDSet[i]];
        double dNext = pT->calcNextTriggerTime(false);
        if (dNext < dMin) {
            dMin = dNext;
        }
    }
    m_dNextTime = dNegInf; 
}


//--------------------------------------------------------------
// toString
//
void EventManager::toString(std::vector<std::string> &vsCur) const  {
    evttrig::const_iterator it;
    for (it = m_vEDT.begin(); it != m_vEDT.end(); it++) {
        std::string sCur;
        it->first->toString(sCur);
        sCur += '@';
        it->second->toString(sCur);
        vsCur.push_back(sCur);
    }
}
