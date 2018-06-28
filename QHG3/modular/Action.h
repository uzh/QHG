#ifndef __ACTION_H__
#define __ACTION_H__

// This is the base class for all actions

// DO NOT MODIFY THIS CLASS 


#include <hdf5.h>

template<typename T> class SPopulation;
template<typename T> class LayerBuf;

class SCellGrid;

template<typename T>
class Action { 

 public:
    Action(SPopulation<T> *pPop, SCellGrid *pCG);
    virtual ~Action() { };

    virtual int preLoop() { return 0; };
    virtual int initialize(float fTime) { return 0; };
    virtual int operator()(int iA, float fT) { return 0; };
    virtual int finalize(float fTime) { return 0; };
    virtual int postLoop() { return 0; };

    // get action parameters from QDF 
    virtual int extractParamsQDF(hid_t hSpeciesGroup) { return 0; };

    // write action parameters to QDF
    virtual int writeParamsQDF(hid_t hSpeciesGroup) {return 0; };

    // read action parameters from ascii POP file
    virtual int tryReadParamLine(char *pLine) { return 0; }; // we need to return 0 for Prioritizer

    virtual void showAttributes() = 0;
 protected:
    SPopulation<T> *m_pPop;
    SCellGrid *m_pCG;    

    template<typename T2> int readPopKeyVal(char* pLine, const char* pKey, T2* pParam);
    template<typename T2> int readPopKeyArr(char* pLine, const char* pKey, int iNum, T2* pParam);
};

#endif
