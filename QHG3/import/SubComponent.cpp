#include <stdio.h>

#include <map>
#include <vector>

#include "types.h"

#include "SubComponent.h"

//----------------------------------------------------------------------------
// constructor
//
SubComponent::SubComponent(int iID) 
    : m_iID(iID) {
    
}


//----------------------------------------------------------------------------
// destructor
//
SubComponent::~SubComponent() {
    // nothing to be done    
}

//----------------------------------------------------------------------------
// setCells
//
void SubComponent::setCells(cellchain vCellChain) {
    m_vCellChain = vCellChain;
}
