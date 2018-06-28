#include <stdio.h>
#include <algorithm>

#include "ProteinBuilder.h"
#include "ProteomeComparator.h"

int ProteomeComparator::countProteinMatches(const proteome &vProteome1, const proteome &vProteome2) {
    int iMatches = 0;
    for (uint i = 0; i < vProteome1.size(); i++) {
        for (uint j = 0; j < vProteome2.size(); j++) {
            const protein &v1 = vProteome1[i];
            const protein &v2 = vProteome2[j];
            if (v1.size() == v2.size()) {
                if (std::equal(v1.begin(), v1.end(), v2.begin())) {
                    //printf("%d <-> %d: match\n", i, j); 
                    iMatches++;
                }
            }
        }
    }
    return iMatches;
}

//assume sorted
int ProteomeComparator::countProtHashMatches(const prothash &vProtHash1, const prothash &vProtHash2) {
    int iMatches = 0;
    /*
    uint i1 = 0;
    uint i2 = 0;
    
    while ((i1 < vProtHash1.size()) && (i2 < vProtHash2.size())) {
        //        printf("[%016lx %016lx] [%016lx %016lx]\n", vProtHash1[i1][0], vProtHash1[i1][1], vProtHash2[i2][0], vProtHash2[i2][1]);
        if ((vProtHash1[i1][0] == vProtHash2[i2][0]) && (vProtHash1[i1][1] == vProtHash2[i2][1])) {
            iMatches++;
            printf("matchs %d-%d: [%016lx %016lx] [%016lx %016lx]\n", i1, i2, vProtHash1[i1][0], vProtHash1[i1][1], vProtHash2[i2][0], vProtHash2[i2][1]);
            i1++;
            i2++;

        } else if ((vProtHash1[i1][0] < vProtHash2[i2][0]) || ((vProtHash1[i1][0] == vProtHash2[i2][0]) && (vProtHash1[i1][1] == vProtHash2[i2][1]))) {
            i1++;
        } else {
            i2++;
        }
    }
    */
            
    for (uint i1 = 0; i1 < vProtHash1.size(); i1++) {
        for (uint i2 = 0; i2 < vProtHash2.size(); i2++) {
            // we only have 2 longs to compare: unroll
            if ((vProtHash1[i1][0] == vProtHash2[i2][0]) && (vProtHash1[i1][1] == vProtHash2[i2][1])) {
            //            if (vProtHash1[i1] == vProtHash2[i2]) {
                //printf("matchn %d-%d: [%016lx %016lx] [%016lx %016lx]\n", i1, i2, vProtHash1[i1][0], vProtHash1[i1][1], vProtHash2[i2][0], vProtHash2[i2][1]);
                iMatches++;
            }
        }
    }
        
    //    printf("returning %d matches\n", iMatches);
    return iMatches;
}
