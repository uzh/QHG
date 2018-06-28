#ifndef __QSPLINE_H__
#define __QSPLINE_H__

class QSpline {
public:
    QSpline(float *pqIn, int iN, int iEqMethod=0, int iSamples=0);
    ~QSpline();
    
    void getValue(int n, float t, float qCur[4]);
    void getValue(float t, float qCur[4]);
    
protected:
    int   equalizeQ();
    int   equalizeQ2();
    int   equalizeV();
    float *m_pqPoints;
    int    m_iN;
    float *m_pqInter;
    bool   m_bEqualize;
    int    m_iSamples;
    float *m_afTable;
};


#endif

