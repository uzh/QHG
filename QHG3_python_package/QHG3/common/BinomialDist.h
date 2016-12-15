#ifndef __BINOMIALDIST_H__
#define __BINOMIALDIST_H__ 

class BinomialDist {
public:
    static BinomialDist *create(double dProb, int n, double dEpsilon);
    ~BinomialDist();
    int getN(double dP);

    void showTable();
protected:
    BinomialDist();
    int init(double dProb, int n, double dEpsilon);
    int buildTable(double dEpsilon);
    int m_N;
    double m_dProb;

    uint m_iNumBins;
    double *m_adLookUp;
};

#endif
