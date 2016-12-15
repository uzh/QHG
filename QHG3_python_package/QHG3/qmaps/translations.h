
#include <map>
#include "types.h"


typedef std::map<int, double> MAP_TRANS; 

class ColorTranslation {
public:
    virtual ~ColorTranslation() {};
    virtual double convert(uchar *pucFirst, int iNum, int iBitDepth)=0; 
    
};

class BWTranslation : public ColorTranslation {
public:
    BWTranslation(const char *pParam);
    virtual double convert(uchar *pucFirst, int iNum, int iBitDepth);
protected:
    double m_dThresh;
};

class GrayTranslation : public ColorTranslation {
public:
    GrayTranslation(const char *p) {};
    virtual double convert(uchar *pucFirst, int iNum, int iBitDepth);
};


class TableTranslation : public ColorTranslation {
public:
    TableTranslation(const char *p);
    virtual double convert(uchar *pucFirst, int iNum, int iBitDepth);
protected:
    int loadTranslation(const char *pTrans);
    MAP_TRANS m_mapT;
    double m_dDefVal;
};
