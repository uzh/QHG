#ifndef __SEQINTERVAL_H__
#define __SEQINTERVAL_H__

#include <vector>

#include "utils.h"
#include "types.h"
#include <deque>

//typedef std::vector<DPOINT>   VDP;
typedef std::deque<DPOINT>   VDP;
class vinterval {
public:
    vinterval() : _dMin(dPosInf), _dMax(dNegInf), _dVal(dNaN), _bForce(false) {};
    vinterval(double dMin, double dMax, double dVal, bool bForce) : _dMin(dMin), _dMax(dMax), _dVal(dVal), _bForce(bForce) {};
    bool    operator==(const vinterval &other) const {return (_dMin == other._dMin) && (_dMax == other._dMax) && (_dVal == other._dVal);};
    double _dMin;
    double _dMax;
    double _dVal;
    bool   _bForce;
};
typedef std::vector<vinterval> VVI;


class marker {
public:
    enum etype {START, STOP};
    marker(double dTime, const vinterval &vi, etype iEvent, bool bForce=false) : _dTime(dTime), _vi(vi), _iEvent(iEvent) {};
    double       _dTime;
    vinterval    _vi;
    etype        _iEvent;
};

typedef std::vector<marker *> VMARKERS;


class SeqInterval {
public:

    SeqInterval();
    SeqInterval(VVI &vintervals);
    ~SeqInterval();
    void addInterval(const vinterval &vi);
    void addIntervals(const VVI &vIntervals);
    uint prepare();
    int linearize();
    void pop_front() { _vEvents.pop_front();};
    uint getNumEvents() const { return (uint) _vEvents.size();};
    const DPOINT &front() const { return _vEvents.front();};
    const DPOINT &getEvent(int iIndex) const { return _vEvents[iIndex];};
    void push_back_event(DPOINT event) { _vEvents.push_back(event);};
    void push_back_event(double dT, double dI) { push_back_event(DPOINT(dT, dI));};
    void push_front_event(DPOINT event) { _vEvents.push_front(event);};
    void push_front_event(double dT, double dI) { push_front_event(DPOINT(dT, dI));};

    static SeqInterval *createSeqInterval(char *pIDef);
    static vinterval &parseInterval(char *pI);

    const VDP &getEvents() const { return _vEvents;};
    const VVI &getIntervals() const { return _vIntervals;};
    void display();
    int merge(SeqInterval *pSI);

protected:
    VVI      _vIntervals;
    VMARKERS _vmInput;
    VVI      _vStack;
    VDP      _vEvents;

    static vinterval _viRes;
};


#endif
