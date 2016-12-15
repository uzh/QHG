#ifndef __PROJINFO_H__
#define __PROJINFO_H__

#include "icoutil.h"
#include "Observable.h"
#include "IQOverlay.h"

class LookUp;
class GridProjection;
class ProjType;
class ProjGrid;
class Surface;
class ValReader;
class ValueProvider;

class ProjInfo : public Observable {
public:
    ProjInfo(IQOverlay *pOverlay);
    ~ProjInfo();

    void setSurface(Surface *pSurface) { m_pSurface = pSurface;};

    void setLU(int iLUType, int iNumParams, double *adLUParams);

    void setGP(ProjType *pPT, ProjGrid *pPG);

    void setData(ValueProvider *pVP, bool bForceCol);
    void setQuat(float q[4]);

    //   int loadData(const char *pFile, bool bForceCol=false);

    bool readyForAction();
    
    int fillRGBInterpolated(int iW, int iH, unsigned char *pImgData);
    int fillRGBInterpolatedQ(int iW, int iH, unsigned char *pImgData);
    int fillFlatInterpolated(int iW, int iH, unsigned char *pImgData);

    int fillRGBPoints(int iW, int iH, unsigned char *pImgData);
    
    int calcImage(int iW, int iH, const char *pFileName);
    LookUp *getLU() { return m_pLU;};
    GridProjection *getGP() { return m_pGP;};

    ValReader *getValReader() { return m_pVR;};
    ValueProvider *getValueProvider() { return m_pVP;};
    //    int getDataType() { return m_iDataFileType;};
    //    nodelist &getNodeList() { return m_mNodeValues;};
    void getRange(double *pdMin, double *pdMax);
    void getLURange(double *pdMin, double *pdMax);
    void setFlat(bool bFlat) { m_bFlat = bFlat;};

protected:
    int putColorBarycentric(double dLon, double dLat, int iC3, unsigned char *pImgData);
    int putColorDirect(double dLon, double dLat, int iC3, unsigned char *pImgData);
    void translateValArr(int iW, int iH, double **dTempVal, unsigned char *pImgData);

    Surface    *m_pSurface;
    GridProjection *m_pGP;
    
    LookUp         *m_pLU;
    
    ValReader      *m_pVR;
    ValueProvider  *m_pVP;
    bool            m_bFlat;

    float   m_Quat[4];
    float   m_QuatInv[4];

    IQOverlay *m_pOverlay;

};

#endif
