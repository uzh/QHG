#ifndef __TOPOTEMP_H__
#define __TOPOTEMP_H__

class ValReader;

class TopoTemp {
public:
    TopoTemp(char *pAltFile, char *pTempFile);
    TopoTemp(ValReader *pAltReader, char *pTempFile);
    TopoTemp(char *pAltFile, ValReader *pTempReader);
    TopoTemp(ValReader *pAltReader, ValReader *pTempReader);
    virtual ~TopoTemp();

    virtual int adjustTemperature(double fParam, bool bAccumulateTemp)=0;

    int writeToFile(char *pOutputFile);

    bool isReady() { return m_bReady; };

    void setVerbose(bool bV) { m_bVerbose = bV; };
protected:
    void init();

    ValReader     *m_pQMRAltitude;
    ValReader     *m_pQMRTemp;
    float        **m_ppData;
    unsigned int   m_iW;
    unsigned int   m_iH;
    bool           m_bReady;
    bool           m_bDelAltReader;
    bool           m_bDelTempReader;
    bool           m_bVerbose;

};

#endif

