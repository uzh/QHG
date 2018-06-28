#include <string.h>
#include <stdlib.h>
#include "ids.h"



//--------------------------------------------------------------------------------------
// getName
//
const char *getName(const NameIVal *nv, int iSize, int iVal) {
    const char *p = NULL;
    for (int i = 0; (p == NULL) && (i < iSize); ++i) {
        if (nv[i].iVal == iVal) {
            p = nv[i].pName;
        }
    }
    return p;
}


//--------------------------------------------------------------------------------------
// getValue
//  
int getValue(const NameIVal *nv, int iSize, const char *pName) {
    int iVal = -1;
    
    // is it a number?
    char *pEnd;
    int iT = (int)strtol(pName, &pEnd, 10);
    if (*pEnd == '\0') {
        if (getName(nv, iSize, iT) != NULL) {
            iVal = iT;
        }
    } else {

        for (int i = 0; (iVal < 0) && (i < iSize); ++i) {
            // name with or without prefix XXX_
            if ((strcasecmp(pName, nv[i].pName) == 0) || (strcasecmp(pName, 4+nv[i].pName) == 0)) {
                iVal = nv[i].iVal;
            }
        }
    }
    return iVal;
}


//--------------------------------------------------------------------------------------
// spcValue
//  
spcid spcValue(const char *pSpc) {
    int i =  getValue(s_aSpecies2, sizeof(s_aSpecies2)/sizeof(NameIVal), pSpc);
    
    return (i >= 0)?((spcid) i):(spcid)SPC_NONE;
}


//--------------------------------------------------------------------------------------
// spcName
//
const char *spcName(spcid iSpc) {
    return getName(s_aSpecies2, sizeof(s_aSpecies2)/sizeof(NameIVal), GET_SPC_SUFF(iSpc));
}

    
//--------------------------------------------------------------------------------------
// clsValue
//  
spcid clsValue(const char *pCls) {
    int i =  getValue(s_aClasses, sizeof(s_aClasses)/sizeof(NameIVal), pCls);
    return (i >= 0)?((spcid) i):(spcid)CLASS_NONE;
}


//--------------------------------------------------------------------------------------
// clsName
//
const char *clsName(spcid iCls) {
    return getName(s_aClasses, sizeof(s_aClasses)/sizeof(NameIVal), GET_SPC_SUFF(iCls));
}
    
        
