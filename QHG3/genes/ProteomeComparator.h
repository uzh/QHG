#ifndef __PROTEOMECOMPARATOR_H__
#define __PROTEOMECOMPARATOR_H__

#include "ProteinBuilder.h"

class ProteomeComparator {
public:

    static int countProteinMatches(const proteome &vProteome1, const proteome &vProteome2);

    static int countProtHashMatches(const prothash &vProthash1, const prothash &vProtHash2);

};


#endif
