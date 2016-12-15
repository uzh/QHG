#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include "types.h"
#include "BStateVector.h"

const double EPS = 0.00001;

BStateVector::BStateVector()
    : m_ucState(UNDEF) {
    m_bsVec.clear();
}

void BStateVector::reset() {
    m_ucState = UNDEF;
    m_bsVec.clear();
}

void BStateVector::add(uchar uc) {
    m_bsVec[uc]++;
}


void BStateVector::add(BStateVector bs, float fFactor) {
    if (m_ucState == UNDEF) {
        statevec::iterator iter;
        for (iter = bs.m_bsVec.begin(); iter != bs.m_bsVec.end(); iter++) {
            m_bsVec[iter->first] += iter->second/fFactor;
        }
    }
}

int  BStateVector::reduce(statevec svWeights) {
    std::vector<uchar> vMultiples;
    float fMaxVal  =-1;
    statevec::iterator iter;
    for (iter = m_bsVec.begin(); iter != m_bsVec.end(); iter++) {   
        float fV = iter->second*svWeights[iter->first];
        if (fV > fMaxVal) {
            fMaxVal = fV;
        }
    }
    fMaxVal -= EPS;

    for (iter = m_bsVec.begin(); iter != m_bsVec.end(); iter++) {   
        if (iter->second*svWeights[iter->first] >= fMaxVal) {
            vMultiples.push_back(iter->first);
            m_bsVec[iter->first] = 1;
        } else {
            m_bsVec[iter->first] = 0;
        }
    }
    

    int iNumMult = vMultiples.size();
    if (iNumMult == 1) {
        m_ucState = vMultiples[0];
    }

    return iNumMult;
        
}


uchar BStateVector::collapse(statevec svWeights) {
    if (m_ucState == UNDEF) {
        std::vector<float> vD;
        float fSum=0;
        
        statevec::iterator iter; 
        printf("Size: %zd \n", m_bsVec.size());
 
        for (int i = 0; i < 4; i++) {
            float fV = m_bsVec[i]*svWeights[i];
            fSum += fV;
            vD.push_back(fSum);
        }

        printf("Distr: ");
        for (unsigned int j = 0; j < vD.size(); j++) {
            printf("%f ", vD[j]);
        }
        printf("\n");

        float fR = fSum * (rand() / (RAND_MAX + 1.0));
        printf("R:%f\n", fR);

        unsigned int i = 0;
        while ((fR > vD[i]) && (i < vD.size())) {
            i++;
        }    
        printf("->%d(%f)\n", i, m_bsVec[i]);
        m_ucState = i;
    }
    return m_ucState;
}
