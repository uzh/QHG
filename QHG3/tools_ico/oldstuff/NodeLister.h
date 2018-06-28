#ifndef __NODELISTER_H__
#define __NODELISTER_H__
#include <map>

#include "types.h"
#include "icoutil.h"

typedef std::map<gridtype, double> nodelist;

class NodeLister {
public:

    static int createList(const char *pFile, nodelist &mNodeValues, double *pdMin, double *pdMax);

protected:
    static int createSnapList(FILE *fIn, char cType, nodelist &mNodeValues, double *pdMin, double *pdMax);
    template<class T>
    static int createSnapList(FILE *fIn, int iItemSize, nodelist &mNodeValues, double *pdMin, double *pdMax, T tDummy);
    static int createPopList(const char *pFile, nodelist &mNodeValues);
    


};
#endif
