#ifndef __SNAPVALUEPROVIDER_H__
#define __SNAPVALUEPROVIDER_H__

#include <stdio.h>
#include <map>

#include "types.h"
#include "icoutil.h"
#include "ValueProvider.h"

class SnapValueProvider : public ValueProvider {
public:
    static ValueProvider *createValueProvider(const char *pFile);
    virtual int init(const char *pFile);
    virtual double getValue(gridtype lID);

    virtual int addFile(const char *pFile) { return 0;};
    virtual int setMode(const char *pMode) { return 0;};

protected:
    SnapValueProvider();

    int createSnapList(FILE *fIn, char cType);

    template<class T>
    int createSnapList(FILE *fIn, int iItemSize, T tDummy);

    std::map<gridtype, double>  m_mNodeValues;

};

#endif
