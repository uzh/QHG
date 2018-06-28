/*============================================================================
| Observer
| 
|  implementation of the Observer interface.
|  The Observable class calls the notify() method of Observer.
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __OBSERVER_H__
#define __OBSERVER_H__

class Observable;

class Observer {
public:
    virtual void notify(Observable *pObs, int iType, const void *pCom)=0;
};

#endif
