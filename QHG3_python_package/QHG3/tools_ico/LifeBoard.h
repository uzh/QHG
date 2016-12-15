#ifndef __LIFEBOARD_H__
#define __LIFEBOARD_H__

#include "MPIMulti.h"

class LifeTile;

class LifeBoard {
public:
    static LifeBoard *createInstance(int iID, int iNX, int iNY, int iW, int iH, int iHalo);
    int setPattern(std::vector<std::pair<int, int> >&vCoords);
    ~LifeBoard();

    int doStep();


    int writeCurrent(const char *pTemplate);
    
    void showPattern(int z);
protected:
    LifeBoard(int iID, int iNX, int iNY, int iW, int iH, int iHalo);
    int init();
    int createLinks();

    int exchangeData();
    int makeID(int i, int j) { return i*m_iNX+j;};
    int m_iID;
    int m_iNX;
    int m_iNY;
    int m_iW;
    int m_iH;
    int m_iHalo;

    
    LifeTile *m_pLifeTile;
    linkinfo  m_vOutLinks;
    linkinfo  m_vInLinks;
    linkinfo  m_vTags;
    MPIMulti *m_pMM;
};

#endif
