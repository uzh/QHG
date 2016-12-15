#ifndef __BINCONVOLVER_H__
#define __BINCONVOLVER_H__

#include "ConvKernel.h"


class BinConvolver {

public:
    BinConvolver(ConvKernel *pKernel);
    ~BinConvolver();
    double **convolve(double **pData, int iW, int iH, int iReSamp, 
                      double dBackground, bool bDeleteData, 
                      bool bMaj=false, bool bAllowNaN= false,
                      double **ppResult=NULL);

   
protected:
    void loadNextDataRow(int iRow);
    void convolveElement(int iX, int iY);
    void convolveElementAllowNaN(int iX, int iY);
    void majorityElement(int iX, int iY);
    void prepareMask(int iX, int iY);

    int          m_iW;
    int          m_iH;
    int          m_iWR;
    int          m_iHR;

    ConvKernel  *m_pOrigKernel;
    ConvKernel  *m_pMaskKernel;
    double     **m_pRowBuf;
    double     **m_pRows;
    int          m_iIns;

    bool   m_bDeleteData;
    bool   m_bDeleteResult;
    int    m_iKW;
    int    m_iKH;
    int    m_iReSamp;
    double m_dBackground;
    
    double **m_pData;
    double **m_pResult;
    


};

#endif

