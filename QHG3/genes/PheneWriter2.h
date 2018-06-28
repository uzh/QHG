#ifndef __PHENEWRITER2_H__
#define __PHENEWRITER2_H__

#include "types.h"
#include "strutils.h"
#include "colors.h"
#include "SequenceProvider.h"
#include "IDSample.h"
#include "IDSampler2.h"

#define FORMAT_ASC   "asc"
#define FORMAT_BIN   "bin"

#define CONTENTS_FULL  "full"
#define CONTENTS_RED   "red"

class PheneWriter2 {

public:
    bool formatAccepted(const char *pFormat);

    static int writeSequence(const char *pFormat, SequenceProvider<float> *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull);
protected:
    static int writePhenesBin(SequenceProvider<float> *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull);
    static int writePhenesAsc(SequenceProvider<float> *pGP, const char *pOutputFile,  const loc_data &mLocDefs, const IDSample *pSample, bool bFull);

};

#endif
