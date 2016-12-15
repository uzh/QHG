#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include "crypto.h"

#define BLOCK_SIZE 32768

//----------------------------------------------------------------------------
// shasum
//
int shasum(const char *pFile, unsigned char *pSHAsum) {
    int iResult = -1;

    unsigned char aBuf[BLOCK_SIZE];
    FILE *fIn = fopen(pFile, "rb");
    if (fIn != NULL) {
        SHA_CTX c;
        SHA1_Init(&c);
        while (!feof(fIn)) {
            unsigned long iRead = fread(aBuf, 1, BLOCK_SIZE*sizeof(unsigned char), fIn);
            SHA1_Update(&c, aBuf, iRead);
        }

        SHA1_Final(pSHAsum, &c);
        iResult = 0;
        fclose(fIn);
    } else {
        memset(pSHAsum, 0, SHA_SIZE);
        iResult = -1;
    }

    return iResult;

}

//----------------------------------------------------------------------------
// md5sum
//
int md5sum(const char *pFile, unsigned char *pMD5sum) {
    int iResult = -1;

    unsigned char aBuf[BLOCK_SIZE];
    FILE *fIn = fopen(pFile, "rb");
    if (fIn != NULL) {
        MD5_CTX c;
        MD5_Init(&c);
        while (!feof(fIn)) {
            unsigned long iRead = fread(aBuf, 1, BLOCK_SIZE*sizeof(unsigned char), fIn);
            MD5_Update(&c, aBuf, iRead);
        }

        MD5_Final(pMD5sum, &c);
        iResult = 0;
        fclose(fIn);
    } else {
        memset(pMD5sum, 0, MD5_SIZE);
        iResult = -1;
    }

    return iResult;

}


//----------------------------------------------------------------------------
// ruipsum
//
int ripsum(const char *pFile, unsigned char *pRIPsum) {
    int iResult = -1;

    unsigned char aBuf[BLOCK_SIZE];
    FILE *fIn = fopen(pFile, "rb");
    if (fIn != NULL) {
        RIPEMD160_CTX c;
        RIPEMD160_Init(&c);
        while (!feof(fIn)) {
            unsigned long iRead = fread(aBuf, 1, BLOCK_SIZE*sizeof(unsigned char), fIn);
            RIPEMD160_Update(&c, aBuf, iRead);
        }

        RIPEMD160_Final(pRIPsum, &c);
        iResult = 0;
        fclose(fIn);
    } else {
        memset(pRIPsum, 0, RIP_SIZE);
        iResult = -1;
    }

    return iResult;

}


   
