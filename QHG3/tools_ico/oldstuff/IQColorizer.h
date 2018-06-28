#ifndef __IQCOLORIZER_H__
#define __IQCOLORIZER_H__

#ifndef NULL 
  #define NULL 0
#endif


class LookUp;

class IQColorizer {
public:
    IQColorizer() : m_bUseColor(true), m_pLookUp(NULL) {};
    virtual ~IQColorizer() {};
    void setLookUp(LookUp *pLookUp) { m_pLookUp = pLookUp;};

    virtual void getCol(double dVal, float fCol[4])=0;

    void setUseColor(bool bUseColor) { m_bUseColor = bUseColor;};
    bool getUseColor() { return m_bUseColor; };

    bool m_bUseColor;
    LookUp *m_pLookUp;

};

#endif

