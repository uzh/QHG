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
    

    int writeState(const char *pQDFOut, char *pSub, int iWhat, int iDumpMode);

    void showInputs();
    void setInterrupt() { m_bInterrupt = true;};

private:

    int m_iCurStep;

    // -- looping --
    void checkEvents(float fTime);
    void checkFinalEvents(float fTime);
    int processEvent(EventData *pED);
    int handleWriteEvent(const char *pDesc, int iDumpMode);
    int handleEnvironmentEvent(const char *pDesc);
    int handleEnvArrayEvent(const char *pDesc);
    int handleDumpEvent(const char *pDesc);

    int setGeoArray(hid_t hFile, const char *pArrName);
    int setClimateArray(hid_t hFile, const char *pArrName);
    int setVegArray(hid_t hFile, const char *pArrName);

    int handlePopEvent(const char *pDesc);

    int handleCheckEvent(const char *pDesc);
    int handleCommEvent(const char *pDesc);
    int handleUserEvent(const char *pDesc);
    void writeResumeConfig(const char *pConfigOut);

    int handleCommLine(const char *pLine);

    int preLoop(); 
    int runLoop(); 
    int postLoop();

    int m_iLastCommTime;
    bool m_bInterrupt;
    std::vector<std::string> m_vDumpNames;
};

#endif

