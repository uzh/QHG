#ifndef __DISTMAT_H__
#define __DISTMAT_H__

class DistMat {
public:
    static DistMat *createDistMat(int iGenomeSize, const char **apGenome1, int iN1);
    static DistMat *createDistMat(int iGenomeSize, ulong **ulGenome1, int iN1);
    ~DistMat();

    int **createMatrix();

protected:
    DistMat();
  
    int init(int iGenomeSize, ulong **ulGenome1, int iN1, bool bDeleteArrays);
    void cleanMat();
    int m_iGenomeSize;
    ulong **m_ulGenome1;
    int m_iN1;
    bool m_bDeleteArrays;
    int **m_aaCounts;
    ulong *m_ulG1;
    ulong *m_ulG2;
};


#endif
