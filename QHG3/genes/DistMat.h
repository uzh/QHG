#ifndef __DISTMAT_H__
#define __DISTMAT_H__

template<typename T>
class DistMat {
public:
    typedef float (*calcdist_t)(T *, T *, int);

    //    static DistMat *createDistMat(int iSequenceSize, const char **apGenome1, int iN1);
    static DistMat *createDistMat(int iSequenceSize, T **ulGenome1, int iN1, calcdist_t fCalcDist);

    //    static DistMat *createDistMatRef(int iSequenceSize, const char **apSequence1, int iN1, const char **apSequence2, int iN2);
    static DistMat *createDistMatRef(int iSequenceSize, T **ulSequence1, int iN1, T **ulSequence2, int iN2,  calcdist_t fCalcDist);


    ~DistMat();

    float **createMatrix();

protected:
    DistMat(calcdist_t fCalcDist, bool bDeleteArrays);
  
    int init(int iGenomeSize, T **ulGenome1, int iN1, T **ulGenome2, int iN2);
    void cleanMat();
    int m_iSequenceSize;
    T **m_ulSequence1;
    int m_iN1;
    T **m_ulSequence2;
    int m_iN2;
    bool m_bDeleteArrays;
    float **m_aaDists;
    bool m_bSymmetric;

    calcdist_t  m_fCalcDist;
};


#endif
