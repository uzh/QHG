/*============================================================================
| Observable
| 
|  implementation of the Observable base class.
|  The notifyObservers() method of the Observable class calls the 
|  notify() method of Observer.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <vector>
#include "Observable.h"

#include "Observer.h"

//----------------------------------------------------------------------------
// constructor
//
Observable::Observable() {
}


//----------------------------------------------------------------------------
// addObserver
//
void Observable::addObserver(Observer *pObs) {
    m_vObservers.push_back(pObs);
}


//----------------------------------------------------------------------------
// notifyObservers
//
void Observable::notifyObservers(int iType, const void *pCom) {
    for (unsigned int i = 0; i < m_vObservers.size(); i++) {
        m_vObservers[i]->notify(this, iType, pCom);
    }
}

