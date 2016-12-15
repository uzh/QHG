/*****************************************************************************\
| UDelReader
| derived from TabDataReader
| 
| UDelReader saves the extracted data in a float array
| input files as in
| http://climate.geog.udel.edu/~climate/html_pages/download.html
\*****************************************************************************/

#ifndef __UDELREADER_H__
#define __UDELREADER_H__

#include "ranges.h"
#include "TabDataReader.h"
const int    UDEL_FIELD_SIZE  =   8;
const int    UDEL_DATA_OFFSET =  17;
const int    UDEL_ANN_START   = 113;
const int    UDEL_ANN_END     = UDEL_ANN_START+UDEL_FIELD_SIZE-1;
const float  UDEL_BAD_VAL     = fNaN;


class UDelReader : public TabDataReader {
	
public:

    UDelReader(char *pFileName, 
               float fRangeLonMin, float fRangeLonMax,
               float fRangeLatMin, float fRangeLatMax,
               int iNumVals=1);
    virtual ~UDelReader();
    
    static int convert2QMap(char *pUDelFile, char *pQMap, int iColumnFrom=UDEL_ANN_START, int iColumnTo=UDEL_ANN_END);

protected:
    virtual bool extractLocation(char *pLine, float *pfLon, float *pfLat);


};

#endif
