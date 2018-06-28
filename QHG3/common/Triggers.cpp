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
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <string>


#include "utils.h"
#include "Triggers.h"

bool g_bVerbose = false;

//-----------------------------------------------------------------------------
// class Trigger
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor
//
Trigger::Trigger(double fFirstTime, double fLastTime, double fStep) 
    : m_fNextTime(fFirstTime),
      m_fLastTime(fLastTime),
      m_fStep(fStep),
      m_fPrevNextTime(fFirstTime),
      m_fOrigStep(fStep) {
}


//-----------------------------------------------------------------------------
// getNextTime
//
void Trigger::calcNextTime() {
    m_fPrevNextTime = m_fNextTime;
    m_fNextTime += m_fStep;
    if (m_fNextTime > m_fLastTime) {
        m_fNextTime = fPosInf;
        m_fStep = fPosInf;
    }
}

//-----------------------------------------------------------------------------
// revert
//
void Trigger::revert() {
    m_fNextTime = m_fPrevNextTime;
    m_fStep     = m_fOrigStep;
}


//-----------------------------------------------------------------------------
// show
//
void Trigger::toString(std::string &sCur) const {
    char sOut[128];
    sprintf(sOut, "[%f:%f]%f{%p}", m_fNextTime, m_fLastTime, m_fStep, this);
    sCur += sOut;
}



//-----------------------------------------------------------------------------
// class Triggers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor
//
Triggers::Triggers(double dOffset) 
    : m_fNextTriggerTime(fPosInf),
      m_dOffset(dOffset),
      m_bFinal(false) {
}


//-----------------------------------------------------------------------------
// destructor
//
Triggers::~Triggers() {
    for (uint i = 0; i < m_vAllTriggers.size(); i++) {
        delete m_vAllTriggers[i];
    }
}


//-----------------------------------------------------------------------------
// addTrigger
//
uint Triggers::addTrigger(Trigger *pTrigger) {
    m_vAllTriggers.push_back(pTrigger);
    return (uint) m_vAllTriggers.size();
}


//-----------------------------------------------------------------------------
// calcNextTriggerTime
//   find next trigger time.
//   if calcNextTriggerTime is called because of a newly added event,
//   no new set is created if the new events time is greater than m_fNextTriggerTime
//
double Triggers::calcNextTriggerTime(bool bAdditional) {
    double fNext = fPosInf;
 
    // determine new smallest time value
    for (uint i = 0; i < m_vAllTriggers.size(); i++) {
        if (m_vAllTriggers[i]->showNext() < fNext) {
            fNext = m_vAllTriggers[i]->showNext();
        }
    }
    
    if (g_bVerbose) printf("[Triggers::calcNextTriggerTime]{%p} have new next %f, old %f\n", this, fNext,m_fNextTriggerTime);

    // new time earlier: revert triggers for previous set of smallest times
    if (fNext < m_fNextTriggerTime) {
        // revert all in set
        intset::const_iterator it;
        for (it = m_sChanged.begin(); it != m_sChanged.end(); ++it) {
            m_vAllTriggers[*it]->revert();
            if (g_bVerbose) printf("[Triggers::calcNextTriggerTime]{%p} reverting Trigger %d:{%p}\n", this, *it, m_vAllTriggers[*it]);
        }
    }

    m_sChanged.clear();

    if (!bAdditional || (fNext < m_fNextTriggerTime)) {
        // select all with new triggertime
        for (uint i = 0; i < m_vAllTriggers.size(); i++) {
            if (m_vAllTriggers[i]->showNext() == fNext) {
                m_vAllTriggers[i]->calcNextTime();
                // reember them in case of needed revert
            m_sChanged.insert(i);
            if (g_bVerbose) printf("[Triggers::calcNextTriggerTime]{%p} adding Trigger %d:{%p}\n", this, i, m_vAllTriggers[i]);
            
            }
        }
        m_fNextTriggerTime = fNext;
    }
    if (g_bVerbose) printf("[Triggers::calcNextTriggerTime] {%p} %f\n", this, m_fNextTriggerTime);
    return m_fNextTriggerTime;
}


//-----------------------------------------------------------------------------
// parseTrigger
//   a trigger definition has the form
//   trigger-def       ::=  <normal-trigger> | <point-trigger> | <final-trigger>
//   normal-trigger    ::= [<trigger-interval>] <step-size>
//   trigger-interval  ::= "[" [<minval>] : [<maxval>] "]"
//   point-trigger     ::= "[" <time> "]"
//   final-trigger     ::= "final"
//
int Triggers::parseTrigger(char *pTriggerDef) {
    int iResult = 0;

    double dMin  = dPosInf;
    double dMax  = dNegInf;
    double dStep = dNaN;

    char *p1 = strchr(pTriggerDef, '[');
    char *p2 = strchr(pTriggerDef, ':');
    char *p3 = strchr(pTriggerDef, ']');

    if ((p1 != NULL) && (p3 != NULL)) {
        
        p1++;
        if (p2 != NULL) {
            *p2 = '\0';
            p2++;
        }
        *p3 = '\0';
        p3++;

        char *pEnd;
        double d1;
        double d2;
        double d3;
        if (iResult == 0) {
            if (*p1 == '\0') {
                // normal-trigger : minval = 0
                d1 = 0;
            } else {
                // normal-trigger : minval
                d1 = strtod(p1, &pEnd);
                if (*pEnd != '\0') {
                    iResult = -1;
                }
            }
        }

        if (p2 != NULL) {
            if (iResult == 0) {
                if (*p2 == '\0') {
                    // normal-trigger : maxval = posInf
                    d2 = dPosInf;
                } else {
                    // normal-trigger : maxval
                    d2 = strtod(p2, &pEnd);
                    if (*pEnd != '\0') {
                        iResult = -1;
                    }
                }
            }
        } else {
            // point-trigger 
            // [<number>] is the same as [<number>:<number>]
            d2 = d1;
        }

        if (iResult == 0) {
            // step size
            if (*p3 != '\0') {
                d3 = strtod(p3, &pEnd);
                if (*pEnd != '\0') {
                    iResult = -1;
                }
            } else {
                // no step -> 1
                d3 = 1;
            }
        }
        
        if(iResult == 0) {
            dMin  = d1;
            dMax  = d2;
            dStep = d3;
        }

    } else if ((p1 == NULL) && (p2 == NULL) && (p3 == NULL)) { 
        // single number: eternal step size
        char *pEnd;
        double dV = strtod(pTriggerDef, &pEnd);
        if (*pEnd == '\0') {
            if (dV < 0) {
                dV = -dV;
                dMin  = m_dOffset-dV*floor(m_dOffset/dV);
                dMax  = dPosInf;
                dStep = dV;
            } else {
                dMin  = 0;
                dMax  = dPosInf;
                dStep = dV;
            }
        } else if (strcmp("final", pTriggerDef) == 0) {
            m_bFinal = true;
        } else {
            iResult = -1;
        }
    } else {
        iResult = -1;
    }

    // create the trigger
    if (iResult == 0) {
        if (dMax <= 0) {
            if ((dMax < 0) || (dMin < 0)) {
                dMax += m_dOffset;
            }
        }
        if (dMin < 0) {
            dMin += m_dOffset;
        }
        if (dStep < 0) {
            dStep = -dStep;
        }
        
        printf("Setting trigger %f, %f, %f\n", dMin, dMax, dStep);
        Trigger *pT = new Trigger(dMin, dMax, dStep);
        addTrigger(pT);
    }

    return iResult;

}


//-----------------------------------------------------------------------------
// merge
//
void Triggers::merge(Triggers *pTriggers) {
    m_vAllTriggers.insert(m_vAllTriggers.end(),  
                          pTriggers->m_vAllTriggers.begin(),
                          pTriggers->m_vAllTriggers.end());
}

 
//-----------------------------------------------------------------------------
// createTriggers
//
Triggers *Triggers::createTriggers(char *pTriggersDef,double fOffset) {
    Triggers *pT = new Triggers(fOffset);
    int iResult = 0;
    char *p = strtok(pTriggersDef, "+");
    while ((iResult == 0) && (p != NULL)) {
        iResult = pT->parseTrigger(p);
        p = strtok(NULL, "+");
    }
    if (iResult != 0) {
        delete pT;
        pT = NULL;
    }
    return pT;
}


//-----------------------------------------------------------------------------
// toString
//
void Triggers::toString(std::string &sCur) const {
    char sOut[64];
    sprintf(sOut, "{{%p}}m[%f]", this, m_fNextTriggerTime);
    sCur += sOut;
    for (uint i = 0; i < m_vAllTriggers.size(); i++) {
        if (i > 0) {
            sCur += '+';
        }
        m_vAllTriggers[i]->toString(sCur);
    }
}

