#ifndef __TEMPRAINCAP_H__
#define __TEMPRAINCAP_H__

class ValReader;

class TempRainCap {

public:
    TempRainCap();
    ~TempRainCap();
    int  setFiles(char *pTempFile, char *pPrecFile, char *pCapacityFile=NULL);
    void setDiffs(double dTempDiff, double dPrecDiff);
    int  calculate(double dTempBin, double dPrecBin);
    int  fill(double dRadius);
    int  writeFiles(const char *pOutBody);

    int  getNumEmpty() { return m_iEmpty; };

    int  checkResult(char *pTable, char *pOut, bool bCalcMass);

private:
    int  checkFiles(char *pTempFile, char *pPrecFile, char *pCapacityFile);

    int sumUp(int iU, int iV, double *pdAvg, double *pdStd);
    int getAverages(int iX, int iY, double dR2, int iLim,  double *pdAvg, double *pdStd);

    /*
    int  findClosestFilledCells(int iX, int iY, int *piX, int *piY, int iRadius);
    int  calcBaryCentricCoords2(int iX, int iY, int *piX, int *piY, double *pdC);
    int  calcBaryCentricCoords1(int iX, int iY, int *piX, int *piY, double *pdC);
    int  check(int iU, int iu, int iV, int iv, double *pDists, int *piX, int *piY);
    */
    int  writeFile(const char *pOutBody, const char *pID, double **aadData);

    unsigned int m_iW;
    unsigned int m_iH;


    ValReader *m_pVRTemp;
    ValReader *m_pVRPrec;
    ValReader *m_pVRCap;
    
    double m_dMinTemp;
    double m_dMaxTemp;
    double m_dTempBin;

    double m_dMinPrec;
    double m_dMaxPrec;
    double m_dPrecBin;

    int m_iMaxTemp;
    int m_iMaxPrec;

    int    ***m_aaiCount;

    double **m_aadAvg;
    double **m_aadStd;
    double **m_aadAvg0;
    double **m_aadStd0;

    int m_iCur;
    int m_iPrev;
    int m_iEmpty;

    double m_dTempDiff;
    double m_dPrecDiff;
};

#endif



