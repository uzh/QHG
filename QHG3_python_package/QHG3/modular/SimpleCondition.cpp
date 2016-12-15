#include <stdio.h>

#include "MoveCondition.h"
#include "SimpleCondition.h"

// some simple tests
static bool never(double dCur, double dNew) { return false;};
static bool always(double dCur, double dNew) { return true;};
static bool greater(double dCur, double dNew) { return dNew > dCur;};
static bool less(double dCur, double dNew) { return (dNew < 10) && (dNew < dCur);};
static bool equal(double dCur, double dNew) { return dNew == dCur;};
static bool greatereq(double dCur, double dNew) { return dNew >= dCur;};
static bool lesseq(double dCur, double dNew) { return dNew <= dCur;};
static bool different(double dCur, double dNew) { return dNew != dCur;};


//-----------------------------------------------------------------------------
// constructor
//
SimpleCondition::SimpleCondition(double *adRefValues, int iMode) 
    : m_adRefValues(adRefValues),
      m_funcCondition(never) {

    switch (iMode) {
    case ALLOW_NEVER:
        m_funcCondition = never;
        break;
    case ALLOW_ALWAYS:
        m_funcCondition = always;
        break;
    case ALLOW_GREATER:
        m_funcCondition = greater;
        break;
    case ALLOW_LESS:
        m_funcCondition = less;
        break;
    case ALLOW_EQUAL:
        m_funcCondition = equal;
        break;
    case ALLOW_GREATEREQ:
        m_funcCondition = greatereq;
        break;
    case ALLOW_LESSEQ:
        m_funcCondition = lesseq;
        break;
    case ALLOW_DIFFERENT:
        m_funcCondition = different;
        break;
    default:
        printf("WARNING: unknown mode [%d] for SimpleCondition\n", iMode);
        m_funcCondition = never;
    }
    
}


//-----------------------------------------------------------------------------
// destructor
//
SimpleCondition::~SimpleCondition() {
}

//-----------------------------------------------------------------------------
// allow
//  tests the values of the array at iCurIndex and iNewIndex
//  with the chosen  function
//
bool SimpleCondition::allow(int iCurIndex, int iNewIndex) {
    double dCurVal = m_adRefValues[iCurIndex];
    double dNewVal = m_adRefValues[iNewIndex];
    return m_funcCondition(dCurVal, 0.2*dNewVal);
}



