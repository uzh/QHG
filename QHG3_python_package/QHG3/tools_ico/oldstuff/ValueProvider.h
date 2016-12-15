#ifndef __VALUEPROVIDER_H__
#define __VALUEPROVIDER_H__


#include "types.h"
#include "utils.h"
#include "icoutil.h"

static const int DATA_FILE_TYPE_NONE = -1;
static const int DATA_FILE_TYPE_QMAP =  0;
static const int DATA_FILE_TYPE_LIST =  1;


class ValueProvider {
public:
    ValueProvider()
        : m_iDataFileType(DATA_FILE_TYPE_NONE),
          m_dMinData(dPosInf),
          m_dMaxData(dNegInf){};
    virtual ~ValueProvider() {};

    virtual int init(const char *pFile)=0;
    virtual double getValue(gridtype lID)=0;

    virtual int addFile(const char *pFile)=0;
    virtual int setMode(const char *pMode)=0;

    void getRange(double *pdMin, double *pdMax) { *pdMin = m_dMinData; *pdMax = m_dMaxData;};
    int getDataType() { return m_iDataFileType;};
    double getMinValue() { return m_dMinData;};
    double getMaxValue() { return m_dMaxData;};
protected:
   
    int m_iDataFileType;
    double m_dMinData;
    double m_dMaxData;

};

#endif
