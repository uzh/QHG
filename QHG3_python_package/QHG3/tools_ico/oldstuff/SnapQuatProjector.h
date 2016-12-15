#ifndef __SNAPQUATPROJECTOR_H__
#define __SNAPQUATPROJECTOR_H__

class ProjInfo;
class Icosahedron;
class ProjType;
class ProjGrid;

class SnapQuatProjector {
public:
    SnapQuatProjector(Icosahedron *pIco, int iW, int iH, double dMaxVal);
    ~SnapQuatProjector();

    
    void setLU(int iLUType, int iNumParams, double *adLUParams) {m_pPI->setLU(iLUType, iNumParams, adLUParams);};
    void setQuat(float *q) { m_pPI->setQuat(q);};
    int  setSnap(char*pSnapFile);

    int drawProjection(char *pSnapFile, float *pQ, char *pOutputFile);
    
    int drawProjection(char *pOutputFile);

protected:
    ProjInfo *m_pPI;
    int   m_iW;
    int   m_iH;
    unsigned char *m_pImage;
    char m_sLastSnap[512];
};

#endif
 
