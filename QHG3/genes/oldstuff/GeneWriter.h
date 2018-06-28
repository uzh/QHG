#ifndef __GENEWRITER_H__
#define __GENEWRITER_H__

#include "GenomeProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"


#define FORMAT_PLINK "plink"
#define FORMAT_ASC   "asc"
#define FORMAT_BIN   "bin"
#define FORMAT_NUM   "num"

#define CONTENTS_FULL  "full"
#define CONTENTS_RED   "red"

class GeneWriter {

public:
    static bool formatAccepted(const char *pFormat);

    static int writeGenes(const char *pFormat, GenomeProvider *pGP, const char *pOutputFile,  const loc_data &mLocDefs,  const IDSample *pSample, bool bFull, bool bBitNucs);
 
protected:
    /*
    static int writeGenesPlink(GenomeProvider *pGP, const char *pOutputFile, stringvidmap &mvIDs, locdata &mLocDefs, idagdatamap &mAgentData);
    static int writeGenesAsc(GenomeProvider *pGP, const char *pOutputFile, stringvidmap &mvIDs, locdata &mLocDefs, idagdatamap mAgentData);
    static int writeGenesBin(GenomeProvider *pGP, const char *pOutputFile, stringvidmap &mvIDs, locdata &mLocDefs, idagdatamap &mAgentData);
    */

 
    static int writeGenesPlink(GenomeProvider *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample);
    static int writeGenesAsc(GenomeProvider *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bBitNucs);
    static int writeGenesBin(GenomeProvider *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bBitNucs);
    static int writeGenesNum(GenomeProvider *pGP, const char *pOutputFile, const IDSample *pSample, bool bFull, bool bBitNucs);


};



#endif

