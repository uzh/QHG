#ifndef __GENEWRITER2_H__
#define __GENEWRITER2_H__


#include "SequenceProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"


#define FORMAT_PLINK "plink"
#define FORMAT_ASC   "asc"
#define FORMAT_BIN   "bin"
#define FORMAT_NUM   "num"

#define CONTENTS_FULL  "full"
#define CONTENTS_RED   "red"

class GeneWriter2 {

public:
    static bool formatAccepted(const char *pFormat);

    static int writeSequence(const char *pFormat, SequenceProvider<ulong> *pGP, const char *pOutputFile,  const loc_data &mLocDefs,  const IDSample *pSample, bool bFull, bool bBitNucs);
 
protected:
 
    static int writeGenesPlink(SequenceProvider<ulong> *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample);
    static int writeGenesAsc(SequenceProvider<ulong> *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bBitNucs);
    static int writeGenesBin(SequenceProvider<ulong> *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull, bool bBitNucs);
    static int writeGenesNum(SequenceProvider<ulong> *pGP, const char *pOutputFile, const IDSample *pSample, bool bFull, bool bBitNucs);

};



#endif

