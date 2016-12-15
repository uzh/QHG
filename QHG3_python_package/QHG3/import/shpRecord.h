#ifndef __SHPRECORD_H__
#define __SHPRECORD_H__

#include <stdio.h>
#include <vector>
#include <map>

#include "shpUtils.h"

typedef std::vector<std::pair<double, double> > vecdoubledouble;

class shpRecord {
public:

    shpRecord(FILE *fIn);
    int read(vecdoubledouble &vCoords);
    void display(const char*pCaption);

    
protected:
    int readHeader();
    int readShape(vecdoubledouble &vCoords);
    int readPolyLine(vecdoubledouble &vCoords);
    FILE      *m_fIn;
    int m_iRecNumber;
    int m_iRecLenWords;
    int m_iRecLen;
    int m_iShape;
    //    shpObject *m_pShape;
    long m_lFilePos;
};

#endif
