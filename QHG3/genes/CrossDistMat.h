#ifndef __CROSSDISTMAT_H__
#define __CROSSDISTMAT_H__

#include "types.h"

class CrossDistMat {
public:
    static CrossDistMat *createCrossDistMat(int iGenomeSize, const char **apGenome0, int iN0, const char **apGenome1, int iN1);
    static CrossDistMat *createCrossDistMat(int iGenomeSize, ulong **ulGenome0, int iN0, ulong **ulGenome1, int iN1);
    ~CrossDistMat();

    int **createMatrix();

    void showStats();
protected:
    CrossDistMat();
  
    int init(int iGenomeSize, ulong **ulGenome0, int iN0, ulong **ulGenome1, int iN1, bool bDeleteArrays);
    void cleanMat();
    int m_iGenomeSize;
    ulong **m_ulGenome0;
    int m_iN0;
    ulong **m_ulGenome1;
    int m_iN1;
    bool m_bDeleteArrays;
    int **m_aaCounts;
    ulong *m_ulG1;
    ulong *m_ulG2;

    double m_dAvg;
    int m_iMinDist;
    int m_iMinIdx0;
    int m_iMinIdx1;
    int m_iMaxDist;
    int m_iMaxIdx0;
    int m_iMaxIdx1;
};


#endif
