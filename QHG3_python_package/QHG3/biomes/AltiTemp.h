#ifndef __ALTITEMP_H__
#define __ALTITEMP_H__

#include "TopoTemp.h" 

class ValReader;

class AltiTemp : public TopoTemp  {
public:
    AltiTemp(char *pAltFile, char *pTempFile);
    AltiTemp(ValReader *pAltReader, char *pTempFile);
    AltiTemp(char *pAltFile, ValReader *pTempReader);
    AltiTemp(ValReader *pAltReader, ValReader *pTempReader);

    virtual ~AltiTemp();

    virtual int adjustTemperature(double fParam, bool bAccumulateTemp);


};


#endif
