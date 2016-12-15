/*****************************************************************************\
| FAOReader
| derived from TabDataReader
| 
| FAOReader saves the extracted data in a float array
| input files as in
| http://user.uni-frankfurt.de/~grieser/downloads/NetPrimaryProduction/npp.htm
\*****************************************************************************/

#ifndef __FAOREADER_H__
#define __FAOREADER_H__

#include "TabDataReader.h"


const int DEF_FIELD = 4;


class FAOReader : public TabDataReader {
	
public:
    FAOReader(char *pFileName, 
            float fRangeLonMin, float fRangeLonMax,
            float fRangeLatMin, float fRangeLatMax);
    virtual ~FAOReader();
    

protected:
    virtual bool extractLocation(char *pLine, float *pfLon, float *pfLat);


};

#endif
