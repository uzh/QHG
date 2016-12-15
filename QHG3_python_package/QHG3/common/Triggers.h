#ifndef __TRIGGERS_H__
#define __TRIGGERS_H__

#include <stdio.h>
#include <vector>
#include <string>

#include "utils.h"

class Trigger {
public:
    Trigger(double fFirstTime, double fLastTime, double fStep); 
    double showNext() { return m_fNextTime;};
    void calcNextTime();
    void toString(std::string &sCur) const;
    void revert();
private:
    double m_fNextTime;
    double m_fLastTime;
    double m_fStep;
    double m_fPrevNextTime;
    double m_fOrigStep;
};



class Triggers {
public:
    Triggers();
    static Triggers *createTriggers(char *pTriggersDef);
    ~Triggers();
    double calcNextTriggerTime(bool bAdditional);
    uint addTrigger(Trigger *pTrigger);
    int parseTrigger(char *pTriggerDef);
    void merge(Triggers *pTriggers);
    double getNextTriggerTime() { return m_fNextTriggerTime;};
    void toString(std::string &sCur) const;
protected:
    std::vector<Trigger *> m_vAllTriggers;
    double m_fNextTriggerTime;
    intset m_sChanged;

};

#endif
