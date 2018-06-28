#ifndef __BSTATEVECTOR_H__
#define __BSTATEVECTOR_H__

#include <map>
#include "types.h"

const unsigned char UNDEF = 0xfe;

typedef std::map<uchar, float> statevec;

class BStateVector {
public:
    statevec m_bsVec;
    uchar    m_ucState;

    BStateVector();
    virtual ~BStateVector(){};
    void reset();
    void add(uchar uc);
    void add(BStateVector bs, float fFactor);
    int  reduce(statevec svWeights);
    uchar collapse(statevec svWeights);

};

#endif
