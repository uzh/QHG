#include <vector>
#include "Observable.h"

#include "Observer.h"
Observable::Observable() {
}
    
void Observable::addObserver(Observer *pObs) {
    m_vObservers.push_back(pObs);
}
 
void Observable::notifyObservers(int iType, const void *pCom) {
    for (unsigned int i = 0; i < m_vObservers.size(); i++) {
        m_vObservers[i]->notify(this, iType, pCom);
    }
}

