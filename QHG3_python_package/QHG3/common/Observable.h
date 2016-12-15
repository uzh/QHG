#ifndef __OBSERVABLE_H__
#define __OBSERVABLE_H__

#include <vector>
class Observer;

class Observable {
public:
    Observable();
    
    void addObserver(Observer *pObs);
    void notifyObservers(int iType, const void *pCom);

protected:
    std::vector<Observer *> m_vObservers;
};

#endif
