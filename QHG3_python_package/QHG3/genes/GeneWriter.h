#ifndef __GENEWRITER_H__
#define __GENEWRITER_H__

#include "GenomeProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"


#define FORMAT_PLINK "plink"
#define FORMAT_ASC   "asc"
#define FORMAT_BIN   "bin"

class GeneWriter {

public:
    static bool formatAccepted(const char *pFormat);

    static int writeGenes(const char *pFormat, GenomeProvider *pGP, const char *pOutputFile,  const locdata &mLocDefs,  const IDSample *pSample);
protected:
    /*
    static int writeGenesPlink(GenomeProvider *pGP, const char *pOutputFile, stringvidmap &mvIDs, locdata &mLocDefs, idagdatamap &mAgentData);
    static int writeGenesAsc(GenomeProvider *pGP, const char *pOutputFile, stringvidmap &mvIDs, locdata &mLocDefs, idagdatamap mAgentData);
    static int writeGenesBin(GenomeProvider *pGP, const char *pOutputFile, stringvidmap &mvIDs, locdata &mLocDefs, idagdatamap &mAgentData);
    */

 
    static int writeGenesPlink(GenomeProvider *pGP, const char *pOutputFile,  const locdata &mLocDefs, const IDSample *pSample);
    static int writeGenesAsc(GenomeProvider *pGP, const char *pOutputFile,  const locdata &mLocDefs, const IDSample *pSample);
    static int writeGenesBin(GenomeProvider *pGP, const char *pOutputFile,  const locdata &mLocDefs, const IDSample *pSample);

};



#endif

