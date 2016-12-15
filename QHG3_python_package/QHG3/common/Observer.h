#ifndef __OBSERVER_H__
#define __OBSERVER_H__

class Observable;

class Observer {
public:
    virtual void notify(Observable *pObs, int iType, const void *pCom)=0;
};

#endif
