#include <string.h>
#include <vector>
#include <algorithm>

#include "utils.h"
#include "types.h"
#include "SeqInterval.h"

// A sequence if intervals has the form
// seqint ::= <firstInterval>["+"<fullInterval>]*
// firstinterval ::= <interval-size> | <fullInterval>
// fullInterval  ::= <interval-def> <interval-size>
// intervaldef   ::= "[" [<minval>] ":" [<maxval>] "]"
// if minval is omitted, it is set to fNegInf
// if maxval is omitted, it is set to fPosInf
// example
//    20+[300:]500+[250:600]10

//-----------------------------------------------------------------------------
// marker_comp
//
bool marker_comp(const marker *pm1, const marker *pm2) {
    return (pm1->_dTime < pm2->_dTime);
}

//-----------------------------------------------------------------------------
// event_comp 
//
bool event_comp(const DPOINT &dp1, const DPOINT &dp2) {
    return (dp1.first < dp2.first);
}


//-----------------------------------------------------------------------------
// constructor
//
SeqInterval::SeqInterval() {
    _vIntervals.clear();
    _vmInput.clear();
}

//-----------------------------------------------------------------------------
// constructor
//
SeqInterval::SeqInterval(VVI &vIntervals)
    : _vIntervals(vIntervals) {
    _vmInput.clear();
}


//-----------------------------------------------------------------------------
// destructor
//
SeqInterval::~SeqInterval() {
    for (unsigned int i = 0; i < _vmInput.size(); i++) {
        delete _vmInput[i];
    }
}

//-----------------------------------------------------------------------------
// addInterval
//
void SeqInterval::addInterval(const vinterval &vi) {
    _vIntervals.push_back(vi);
   
    _vmInput.push_back(new marker(vi._dMin, vi,  marker::START));
    _vmInput.push_back(new marker(vi._dMax, vi, marker::STOP));
}

//-----------------------------------------------------------------------------
// addIntervals
//
void SeqInterval::addIntervals(const VVI & vIntervals) {
    for (unsigned int i = 0; i < vIntervals.size(); i++) {
        addInterval(vIntervals[i]);
    }
}


//-----------------------------------------------------------------------------
// prepare
//
uint SeqInterval::prepare() {
    _vmInput.clear();
    for (unsigned int i = 0; i < _vIntervals.size(); i++) {
        addInterval(_vIntervals[i]);
    }
    //    std::sort(_vmInput.begin(), _vmInput.end(), marker_comp);
    return (uint) _vmInput.size();
}

//-----------------------------------------------------------------------------
// linearize
//
int SeqInterval::linearize() {
    int iResult = 0;
    VVI      vStack;
    std::sort(_vmInput.begin(), _vmInput.end(), marker_comp);
    vStack.clear();
    _vEvents.clear();

    for (unsigned int i = 0; (iResult == 0) && (i < _vmInput.size()); i++) {
        marker *pm = _vmInput[i];
        double dT = pm->_dTime;
        if (pm->_iEvent == marker::START) {
            vStack.push_back(pm->_vi);
            push_back_event(dT, pm->_vi._dVal);
            //            _vEvents.push_back(DPOINT(dT, pm->_vi._dVal));
            //                        printf("Got START EVENT (%f, %f)->%zd\n", pm->_dTime, pm->_vi._dVal, vStack.size());
            //                        printf("pushing event1 %f,%f\n", dT, pm->_vi._dVal);
        } else if (pm->_iEvent == marker::STOP) {
            //            printf("Got STOP EVENT (%f, %f)-> %zd\n", pm->_dTime, pm->_vi._dVal, vStack.size());
            VVI::iterator it = std::find(vStack.begin(), vStack.end(), pm->_vi);
            if (it != vStack.end()) {
                bool bTop = false;
                if (*it == vStack.back()) {
                    bTop = true;
                }
                vStack.erase(it);
                if (bTop) {
                    if (vStack.empty()) {
                       
                        // stop event
                        if ((_vEvents.size() > 0) && (_vEvents.back().first ==  dT)) {
                            _vEvents.pop_back();
                        }
                        push_back_event(dT, dPosInf);
                        
                    } else {
                        vinterval viCur = *(vStack.rbegin());
                        double t1 = viCur._dMin + ceil((pm->_dTime - viCur._dMin)/viCur._dVal)*viCur._dVal;
                        if ((_vEvents.size() == 0) || (_vEvents.back().first !=  t1)) {

                            push_back_event(t1, viCur._dVal);
                        }
                    }
                }
          
            } else {
                iResult = -1;
            }
        } else {
            iResult = -1;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// display
//
void SeqInterval::display() {
    printf("%zd events:\n", _vEvents.size());
    for (unsigned i = 0; i < _vEvents.size(); i++) {
        printf("  %f -> %f\n", _vEvents[i].first, _vEvents[i].second);
    }
}


vinterval SeqInterval::_viRes;

//-----------------------------------------------------------------------------
// parseInterval
//
vinterval &SeqInterval::parseInterval(char *pI) {
    double dMin = dPosInf;
    double dMax = dNegInf;
    double dVal = dNaN;
    bool   bForce = false;

    char *p1 = strchr(pI, '[');
    char *p2 = strchr(pI, ':');
    char *p3 = strchr(pI, ']');

    if ((p1 != NULL) && (p2 != NULL) && (p3 != NULL)) {
        p1++;
        *p2 = '\0';
        p2++;
        *p3 = '\0';
        p3++;

        int iResult = 0;
        char *pEnd;
        double d1;
        double d2;
        double d3;
        if (iResult == 0) {
            if (*p1 == '\0') {
                d1 = 0;
            } else {
                d1 = strtod(p1, &pEnd);
                if (*pEnd != '\0') {
                    iResult = -1;
                }
            }
        }

        if (iResult == 0) {
            if (*p2 == '\0') {
                d2 = dPosInf;
            } else {
                d2 = strtod(p2, &pEnd);
                if (*pEnd != '\0') {
                    iResult = -1;
                }
            }
        }

        if (iResult == 0) {
            if (*p3 != '\0') {
                d3 = strtod(p3, &pEnd);
                if (*pEnd == '!') {
                    bForce = true;
                } else if (*pEnd != '\0') {
                    iResult = -1;
                }
            } else {
                iResult = -1;
            }
        }
        
        if(iResult == 0) {
            dMin = d1;
            dMax = d2;
            dVal = d3;
        }
    } else if ((p1 == NULL) && (p2 == NULL) && (p3 == NULL)) { 
        double dV;
        char *pEnd;
        dV = strtod(pI, &pEnd);
        if (*pEnd == '\0') {
            dMin = 0;
            dMax = dPosInf;
            dVal = dV;
        }
    }

    _viRes._dMin = dMin;
    _viRes._dMax = dMax;
    _viRes._dVal = dVal;
    _viRes._bForce = bForce;
    //    printf("[]at end min %f, max %f, val %f\n", dMin,dMax,dVal);
    return _viRes;
}

//-----------------------------------------------------------------------------
// createSeqInterval
//
SeqInterval *SeqInterval::createSeqInterval(char *pIDef) {
    SeqInterval *pSI = new SeqInterval();
    char *p = strtok(pIDef, "+");
    while ((pSI != NULL) && (p != NULL)) {
        //        printf("next is [%s]\n", p);
        vinterval &vi = SeqInterval::parseInterval(p);
        if (!isnan(vi._dVal)) {
            pSI->addInterval(vi);
            p = strtok(NULL, "+");
        } else {
            printf("invalid interval def: [%s]\n", p);
            delete pSI;
            pSI = NULL;
        } 
    }
    return pSI;
}

//-----------------------------------------------------------------------------
// merge
//   merge the intervals in pSI with this one's.
//
int SeqInterval::merge(SeqInterval *pSI) {

    const VDP vOtherEv = pSI->getEvents();
    printf("this: %zd evs, other %zd evs\n", _vEvents.size(), vOtherEv.size());
    _vEvents.insert(_vEvents.begin(), vOtherEv.begin(), vOtherEv.end());
    printf("combined: %zd evs\n", _vEvents.size());

    std::sort(_vEvents.begin(), _vEvents.end(), event_comp);
    return -1;
}
