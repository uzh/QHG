#ifndef __ALPHACOMPOSER_H__
#define __ALPHACOMPOSER_H__

class LookUp;

class AlphaComposer {
public:
    static AlphaComposer *createInstance(int iW, int iH);
    ~AlphaComposer();

    uchar **addMatrix(double **ppdData, LookUp *pLU);
    uchar **addPNGData(uchar **ppdData);
    void    clear();
    uchar **getData() { return m_ppPNGData;};
protected:
    AlphaComposer();
    int init(int iW, int iH);

    int m_iW;
    int m_iH;
    uchar **m_ppPNGData;

};


#endif

