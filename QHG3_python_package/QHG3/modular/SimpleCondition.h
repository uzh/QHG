#ifndef __SIMPLECONDITION_H__
#define __SIMPLECONDITION_H__

#include "MoveCondition.h"

#define ALLOW_NEVER     0
#define ALLOW_ALWAYS    1
#define ALLOW_GREATER   2
#define ALLOW_LESS      3
#define ALLOW_EQUAL     4
#define ALLOW_GREATEREQ 5
#define ALLOW_LESSEQ    6
#define ALLOW_DIFFERENT 7

typedef bool (*condfunc)(double, double);

class SimpleCondition : public MoveCondition {
public:
    SimpleCondition(double *adRefValues, int iMode);
    virtual ~SimpleCondition();

    virtual bool allow(int iCurIndex, int iNewIndex);

protected:
    double *m_adRefValues;
    condfunc m_funcCondition;
};


#endif
