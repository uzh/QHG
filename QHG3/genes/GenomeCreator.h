/*============================================================================
| GenomeCreator
| 
|  Creates a number of initial genome sequences according to various schemes:
|  - completely random               ("random")
|  - variants of a random            ("variants:<NumMutations>")
|  - all genes 0                     ("zero")
|  - hardy-wrinberg distribution     ("hw:<siterate>:<mutrate>")
|
|  GenomeCreator's template is usuallay GeneUtils or BitGeneUtils
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __GENOMECREATOR_H__
#define __GENOMECREATOR_H__

#include <string>
#include "types.h"
#include "LayerArrBuf.h"
#include "WELL512.h"

template<class U>
class GenomeCreator {
public:
    GenomeCreator(uint iNumParents);
    
    int determineInitData(char *pLine);
    const char *getInitString() { return m_sInitString.c_str();};
    int createInitialGenomes(int iGenomeSize, int iNumGenomes, LayerArrBuf<ulong> &aGenome, WELL512** apWELL);

private:
    void buildInitString();

    uint   m_iNumParents;
    int    m_iInitialMutations;
    double m_dInitialMutRate;
    double m_dInitialSiteRate;
    int    m_iInitialType;
    std::string m_sInitString;
};


#endif
