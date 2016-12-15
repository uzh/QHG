#ifndef __LOOKUP_H__
#define __LOOKUP_H__


class LookUp {
public:
    LookUp(double dMinLevel, double dMaxLevel) : m_dMinLevel(dMinLevel), m_dMaxLevel(dMaxLevel) {};
    virtual ~LookUp() {};
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha)=0;
    virtual void getColor(double dValue, unsigned char &uRed, unsigned char &uGreen, unsigned char &uBlue, unsigned char &uAlpha);

    double m_dMinLevel;
    double m_dMaxLevel;
};

#endif
