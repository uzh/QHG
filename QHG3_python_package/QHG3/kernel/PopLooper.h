#ifndef __POPLOOPER_H__
#define __POPLOOPER_H__

#include <string.h>
#include <vector>
#include "types.h"
#include "PopFinder.h"

class PopBase;

static const int DEF_CHUNK=16;

typedef std::vector<PopBase *> popvec;

class PopLooper : public PopFinder {
public:
    PopLooper(int iChunkSize=DEF_CHUNK);

    virtual ~PopLooper();

    int addPop(PopBase *pPop);
    int removePopID(spcid iPopID);
    int removePop(uint iIndex);

    int doStep(float fStep);

    size_t getNumPops() { return m_vP.size();};
    const popvec &getPops() { return m_vP;};
    idtype getMaxID() { return m_iMaxID;};

    double dTimeActions;
    double dTimeFinalize;
    

    // PopFinder implementation
    virtual PopBase *getPopByID(idtype iSpeciesID);
    virtual PopBase *getPopByName(const char *pSpeciesName);

protected:
    
    std::set<uint> m_vPrioLevels;
    int m_iChunkSize;
    popvec m_vP;

    idtype m_iMaxID;

 
};
#endif
