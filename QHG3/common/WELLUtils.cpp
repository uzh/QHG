#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

#include <vector>

#include "LineReader.h"
#include "WELL512.h"
#include "WELLUtils.h"

//-----------------------------------------------------------------------------
//  stringToSeed
// 
int stringToSeed(char *sSequence, std::vector<uint32_t> &vulState) {
    int iResult = 0;
    char *p = strtok(sSequence, ",; ");
    while ((p != 0) && (iResult == 0)) {
        int iX = 0;
        char *pEnd;
        iX = (int)strtol(p, &pEnd, STATE_SIZE);
        if (*pEnd == '\0') {
            vulState.push_back(iX & 0xffffffff);
            p = strtok(NULL, ",; ");
        } else {
            printf("Invalid hexnumber given in seed sequence [%s]\n", p);
            iResult = -1;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  phraseToSeed
//    creates a 16 array of int from an arbitrary string
//
int phraseToSeed(const char *pPhrase, std::vector<uint32_t> &vulState) {
    int iResult = -1;

    int iLen = strlen(pPhrase);

    if (iLen > 1) {
        uchar md[SHA512_DIGEST_LENGTH];
        SHA512((uchar *)pPhrase, iLen, md);

        for (uint i = 0; i < STATE_SIZE; i++) {
            vulState.push_back(*((uint32_t *)(md+sizeof(uint)*i)));
        }
        
        iResult = 0;
    } else {
        printf("Phrase for seed must not be empty\n");
        iResult = -1;
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
//  random seed
//    creates a 16 array of long using the rand() function
//
int randomSeed(int iSeed, std::vector<uint32_t> &vulState) {
    
    for (uint i = 0; i < STATE_SIZE; i++) {
        vulState.push_back(rand()%0xffffffff);
    }
    return 0;
}


//-----------------------------------------------------------------------------
//  createWELL
//    creates a WELL and sets seed from phrase
//
WELL512 *createWELL(const char *pPhrase) {

    WELL512 *pWELL = NULL;

    int iLen = strlen(pPhrase);

    std::vector<uint32_t> vState;
    if (iLen > 1) {
        int iResult = phraseToSeed(pPhrase, vState);
        if (iResult == 0) {
            uint32_t aState[STATE_SIZE];
            for (uint i = 0; i < STATE_SIZE; i++) {
                aState[i] = vState[i];
            }

            pWELL = new WELL512(aState);
        }
    } else {
        printf("Phrase for seed must not be empty\n");
    }
    
    return pWELL;
}
