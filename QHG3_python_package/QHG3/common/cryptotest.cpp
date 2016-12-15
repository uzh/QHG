#include <stdio.h>
#include <string.h>

#include "crypto.h"

const int   ID_SHA  = 1;
const int   ID_MD5  = 2;
const int   ID_RIP  = 4;

const char *asNames[] = {"sha", "md5", "rip"};
const int  aIDs[] = {ID_SHA, ID_MD5, ID_RIP};

void showHex(unsigned char *pSum, int iLen) {
    for (int i = 0; i < iLen; i++) {
        printf("%02x", pSum[i]);
    }
    printf("\n");
}

int main (int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 1) {
        int iWhich = 0;

        if (iArgC > 2) {
            int i = 2;
            while (i < iArgC) {
                for (unsigned int j = 0; j < sizeof(asNames)/sizeof(char *); j++) {
                    if (strcmp(apArgV[i], asNames[j]) == 0) {
                        iWhich |= aIDs[j];
                    }
                }
                i++;
            }
        } else {
            for (unsigned int j = 0; j < sizeof(asNames)/sizeof(char *); j++) {
                iWhich |= aIDs[j];
            }
        }
        
        if ((iResult == 0) && ((iWhich & ID_MD5) != 0)) {
            printf("MD5\n");
            unsigned char aMD5[MD5_SIZE];
            iResult = md5sum(apArgV[1], aMD5);
            if (iResult == 0) {
                showHex(aMD5, MD5_SIZE);
            } else {
                printf("couldn't open [%s]\n", apArgV[1]);
            }
        }

        if ((iResult == 0) && ((iWhich & ID_SHA) != 0)) {
            printf("SHA\n");
            unsigned char aSHA[SHA_SIZE];
            iResult = shasum(apArgV[1], aSHA);
            if (iResult == 0) {
                showHex(aSHA, SHA_SIZE);
            } else {
                printf("couldn't open [%s]\n", apArgV[1]);
            }
        }

        if ((iResult == 0) && ((iWhich & ID_RIP) != 0)) {
            printf("RIP\n");
            unsigned char aRIP[RIP_SIZE];
            iResult = ripsum(apArgV[1], aRIP);
            if (iResult == 0) {
                showHex(aRIP, RIP_SIZE);
            } else {
                printf("couldn't open [%s]\n", apArgV[1]);
            }
        }


    } else {
        iResult = -1;
        printf("usage: %s <file>\n", apArgV[0]);
    }

    return iResult;
}

