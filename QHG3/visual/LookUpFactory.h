#ifndef  __LOOKUPFACTORY_H__
#define  __LOOKUPFACTORY_H__

class LookUp;

const unsigned int NUM_LOOKUPS    = 13;
const int LOOKUP_UNDEF   = -1;
const int LOOKUP_UCHAR   =  0;
const int LOOKUP_BINVIEW =  1;
const int LOOKUP_VEG     =  2;
const int LOOKUP_POP     =  3;
const int LOOKUP_GEO     =  4;
const int LOOKUP_SUN     =  5;
const int LOOKUP_RAINBOW =  6;
const int LOOKUP_THRESH  =  7;
const int LOOKUP_DENSITY =  8;
const int LOOKUP_BIOME   =  9;
const int LOOKUP_ZEBRA   = 10;
const int LOOKUP_SEGMENT = 11;
const int LOOKUP_GEN     = 12;



class LookUpFactory {
public:
    static LookUpFactory *instance();
    static void free();

    virtual ~LookUpFactory();

    int getNumLookUps();
    char *getLookUpName(unsigned int iIndex);
    LookUp *getLookUp(unsigned int iIndex, double *dParams, unsigned int iNumParams);
    int getNumParams(unsigned int iLU);
    char *getParamName(unsigned int iLU, unsigned int iParam);
    double getParamDefault(unsigned int iLU, unsigned int iParam);

    int getLookUpType(const char *pLUName);
protected:
    LookUpFactory();
    static LookUpFactory *s_pLUF;     
};



#endif
