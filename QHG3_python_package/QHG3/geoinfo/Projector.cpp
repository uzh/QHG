#include <string.h>
#include "utils.h"
#include "Projector.h"


//----------------------------------------------------------------------------
// constructor
//
Projector::Projector(int iID, const char *pName, double dLambda0, double dPhi0) 
    : m_iID(iID),
      m_dLambda0(dLambda0),
      m_dPhi0(dPhi0) {
    
    strncpy(m_sName, pName, NAME_LEN);
    m_sName[NAME_LEN-1] = NUL;    
}

//----------------------------------------------------------------------------
// setAdditional
//
void Projector::setAdditional(int iNumAdd, const double *pdAdd) {
    // do nothing
}
