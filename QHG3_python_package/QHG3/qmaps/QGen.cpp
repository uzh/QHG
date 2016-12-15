#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "QMapHeader.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"

typedef union val {
        unsigned char  c;
        short int s;
        int       i;
        long      l;
        float     f;
        double    d;
} uval;

void usage(char *pApp) {
    printf("%s - generate a unifoem QMap\n", pApp);
    printf("Usage:\n");
    printf("  %s <width>x<height> <type>:<defval> <output>\n", pApp);
    printf("where\n");
    printf("  width:   width of qmap\n");
    printf("  height:  height of qmap\n");
    printf("  type:    datatype (c,s,i,l,f,d)\n");
    printf("  defval:  value to fill the qmap with\n");
    printf("  output:  name of qmap\n");
    printf("\n");
}

int setDefVal(char cType, char *pVal, uval *pdefval) {
    int iResult = -1;

    long l;
    double d;
    char *pEnd;

    switch(cType) {
    case 'c':
    case 's':
    case 'i':
    case 'l':
        l = strtol(pVal, &pEnd, 10);
        if (*pEnd == '\0') {
            switch(cType) {
            case 'c':
                pdefval->c = l & 0x000000ff;
                break;
            case 's':
                pdefval->s = l & 0x0000ffff;
                break;
            case 'i':
                pdefval->s = l & 0xffffffff;
                break;
            case 'l':
                pdefval->l = l;
                break;
            }
            iResult = 0;
        }
        break;
    case 'f':
    case 'd':
        d = strtof(pVal, &pEnd);
        if (*pEnd == '\0') {
            if (cType == 'f') {
                pdefval->f= (float)d;
            } else {
                pdefval->d= d;
            }
            iResult = 0;
        } else {
            if (strcasecmp(pVal, "nan") == 0) {
                if (cType == 'f') {
                    pdefval->f= fNaN;
                } else {
                    pdefval->d= dNaN;
                }
                iResult = 0;
            } else  if (strcasecmp(pVal, "inf") == 0) {
                if (cType == 'f') {
                    pdefval->f= fPosInf;
                } else {
                    pdefval->d= dPosInf;
                }
                iResult = 0;
            } else  if (strcasecmp(pVal, "-inf") == 0) {
                if (cType == 'f') {
                    pdefval->f= fNegInf;
                } else {
                    pdefval->d= dNegInf;
                }
                iResult = 0;
            }
            
        }
    }
    return iResult;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    int iW = 0;
    int iH = 0;
    char cType='\0';
    
    uval val;

    if (iArgC > 3) {
        char *p1 = strchr(apArgV[1], 'x');
        if (p1 != NULL) {
            *p1 = '\0';
            p1++;
            char *pEnd;
            iW = strtol(apArgV[1], &pEnd, 10);
            if (*pEnd == '\0') {
                iH = strtol(p1, &pEnd, 10);
                if (*pEnd == '\0') {
                    char *p2 = strchr(apArgV[2], ':');
                    if (p2 != NULL) {
                        *p2='\0';
                        p2++;

                        cType = *(apArgV[2]);
                        iResult = setDefVal(cType, p2, &val);
                        if (iResult == 0) {
                            iResult = -1;
                            FILE *fOut = fopen(apArgV[3], "wt");
                            if (fOut != NULL) {
                                QMapHeader *pQMH =  new QMapHeader(cType,
                                                                   0, iW, 1.0,
                                                                   0, iH, 1.0);
                                
                                bool bOK = pQMH->addHeader(fOut);
                                if (bOK) {
                                    switch(cType) {
                                    case 'c': {
                                        unsigned char ** pc = QMapUtils::createArray(iW, iH, val.c);
                                        bOK = QMapUtils::writeArray(fOut, iW, iH, pc);
                                        QMapUtils::deleteArray(iW, iH, pc);
                                    }
                                    break;
                                    case 's': {
                                        short int ** ps = QMapUtils::createArray(iW, iH, val.s);
                                        bOK = QMapUtils::writeArray(fOut, iW, iH, ps);
                                        QMapUtils::deleteArray(iW, iH, ps);
                                    }
                                        break;
                                    case 'i': {
                                    int ** pi = QMapUtils::createArray(iW, iH, val.i);
                                    bOK = QMapUtils::writeArray(fOut, iW, iH, pi);
                                    QMapUtils::deleteArray(iW, iH, pi);
                                    }
                                    break;
                                    case 'l': {
                                        long ** pl = QMapUtils::createArray(iW, iH, val.l);
                                        bOK = QMapUtils::writeArray(fOut, iW, iH, pl);
                                        QMapUtils::deleteArray(iW, iH, pl);
                                    }
                                        break;
                                    case 'f': {
                                        float ** pf = QMapUtils::createArray(iW, iH, val.f);
                                        bOK = QMapUtils::writeArray(fOut, iW, iH, pf);
                                        QMapUtils::deleteArray(iW, iH, pf);
                                    }
                                        break;
                                    case 'd': {
                                        double ** pd = QMapUtils::createArray(iW, iH, val.d);
                                        bOK = QMapUtils::writeArray(fOut, iW, iH, pd);
                                        QMapUtils::deleteArray(iW, iH, pd);
                                    }
                                        break;
                                    }
                                    fclose(fOut);
                                    if (bOK) {
                                        iResult = 0;
                                        printf("+++ success +++\n");
                                    }
                                }
                            } else {
                                printf("Couldn't open [%s] for writing\n", apArgV[3]);
                            }
                        } else {
                            printf("Invalid value definition\n");
                        }
                    } else {
                        printf("Expected ':' in value definition\n");
                    }
                } else {
                    printf("Invalid number for height\n");
                }
            } else {
                printf("Invalid number for width\n");
            }
        } else {
            printf("Expected 'x' in size definition\n");
        }
    } 
    if (iResult == -1) {
        usage(apArgV[0]);
    }

}
