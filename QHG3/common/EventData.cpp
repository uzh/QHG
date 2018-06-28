/*============================================================================
| EventData
| 
|  Represents QHG events.
|  Consists of an event id and a std::string containing data.
|  The EDCompare struct is used to order compare EventData objects.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <stdio.h>
#include <string.h>
#include <string>

#include "strutils.h"
#include "EventData.h"

//-----------------------------------------------------------------------------
// constructor
//
EventData::EventData(int iEventType, char *pData) :
    m_iEventType(iEventType),
    m_sData(trim(pData)) {

}


//-----------------------------------------------------------------------------
// parseDef
//
EventData *EventData::parseDef(char *pDef) {
    EventData *pED = NULL;
    int iEventType = -1;
    char *p = nextWord(&pDef);
    if (strToNum(p, &iEventType)) {
        // move to the rest
        p += strlen(p)+1;
        pED = new EventData(iEventType, p);
    }    
    return pED;
}


//-----------------------------------------------------------------------------
// equals
//
bool EventData::equals(EventData *pED) {
    bool bEqual = false;
    if (m_iEventType == pED->m_iEventType) {
        if (m_sData.compare(pED->m_sData) == 0) {
            bEqual = true;
        }
    }
    return bEqual;
}


//-----------------------------------------------------------------------------
// equals
//
void EventData::toString(std::string &sCur) const {
    char sOut[128];
    sprintf(sOut, "(%d)|%s", m_iEventType, m_sData.c_str());
    sCur += sOut;
}
