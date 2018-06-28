#ifndef __LIFETILE_H__
#define __LIFETILE_H__

#include "types.h"
#define DIR_E  0
#define DIR_NE 1
#define DIR_N  2
#define DIR_NW 3
#define DIR_W  4
#define DIR_SW 5
#define DIR_S  6
#define DIR_SE 7
#define NUM_DIR 8

class LifeTile {
public:
    static LifeTile *createInstance(int iID, int iW, int iH, int iHalo);
    ~LifeTile();

    int setInitialPattern(uchar *pData);
    int getFinalPattern(uchar *pData);

    int exchangeData();
    int doStep();

    int getSendSize(int iDir);

    int setSendData(int iDir, uchar *pData);
    int getRecvData(int iDir, const uchar *pData);

    void showInterior(bool bShowHalo);
    int writeCurrent(const char *pTemplate);

protected:
    LifeTile();
    int init(int iD, int iW, int iH, int iHalo);

    int m_iID;
    int m_iW; 
    int m_iH; 
    int m_iHalo;
    int m_iCur;
    uchar ***m_ppField;

};


#endif

