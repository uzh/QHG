#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include <hdf5.h>

#include "SimParams.h"
#include "IDGen.h"


class Simulator : public SimParams {
public:
    Simulator();
    virtual ~Simulator();

    bool isReady();


    int startLoop(); // prepare(); run();cleanup();
    

    int writeState(const char *pQDFOut, char *pSub, int iWhat);

    void showInputs();

private:

    int m_iCurStep;

    // -- looping --
    void checkEvents(float fTime);
    int processEvent(EventData *pED);
    int handleWriteEvent(const char *pDesc);
    int handleClimateEvent(const char *pDesc);
    int handleGeographyEvent(const char *pDesc);
    int handleCommEvent(const char *pDesc);

    int preLoop(); 
    int runLoop(); 
    int postLoop();

    int m_iLastCommTime;

};

#endif

