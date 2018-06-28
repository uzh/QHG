#ifndef __GRADITEMP_H__
#define __GRADITEMP_H__


#include "types.h"
#include "TopoTemp.h"

class Vec3D;
class ValReader;

class GradiTemp : public TopoTemp {
public:
    GradiTemp(char *pAltFile, char *pTempFile);
    GradiTemp(ValReader *pAltReader, char *pTempFile);
    GradiTemp(char *pAltFile, ValReader *pTempReader);
    GradiTemp(ValReader *pAltReader, ValReader *pTempReader);

    virtual ~GradiTemp();

    virtual int adjustTemperature(double fParam, bool bAccumulateTemp);



    //protected:
   
    int findPointAt(Vec3D vQ, double dD, double dAlpha, Vec3D *pvP);
    int getGradient(DPOINT dCenter, double dDistance, Vec3D *pvGradient);


};

#endif
