/*============================================================================
| Observable
| 
|  implementation of the Observable base class.
|  The notifyObservers() method of the Observable class calls the 
|  notify() method of Observer.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

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
