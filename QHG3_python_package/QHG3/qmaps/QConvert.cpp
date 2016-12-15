#include <stdio.h>
#include <string.h>
#include <math.h>

#include "QConverter.h"
#include "QMapHeader.h"


void usage(char *pApp) {
    printf("%s - type conversion for QMAPs\n", pApp);
    printf("Usage:\n");
    printf("  %s <QMapIn> <type> [<QMapOut>]\n", pApp);
    printf("where\n");
    printf("  QMapIn   name of QMAP to convert\n");
    printf("  type     target type, one of 'u', 's', 'i', 'l', 'f', 'd'\n");
    printf("  QMapOut  name of output QMAP (if omitted, QMapIn is used)\n");
    printf("\n");
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char sInput[256];
    int  iTypeOut = QMAP_TYPE_NONE;
    char sOutput[256];
    if (iArgC > 2) {
        strcpy(sInput, apArgV[1]);
        iTypeOut = QMapHeader::getTypeID(*(apArgV[2]));
        if (iTypeOut != QMAP_TYPE_NONE) {
            if (iArgC > 3) {
                strcpy(sOutput, apArgV[3]);
            } else {
                strcpy(sOutput, sInput);
            }
        
            iResult = QConverter::convert(sInput, sOutput, iTypeOut);
            if (iResult == 0) {
                printf("+++ success +++\n");
            } else {
                if (iResult == -2) {
                    printf("Couldn't open ValReader [%s]\n", sInput);
                } else if (iResult == -1) {
                    printf("Couldn't write to [%s]\n", sOutput);
                }
            }

        } else {
            printf("Unknown type [%s]\n", apArgV[2]);
        }

    } else {
        usage(apArgV[0]);
    }


    return iResult;
}
