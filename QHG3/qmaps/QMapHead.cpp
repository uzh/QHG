#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"
#include "types.h"
#include "ParamReader.h"
#include "QMapHeader.h"

const int ACTION_ADD = 1;
const int ACTION_REP = 2;
const int ACTION_UPD = 4;
const int ACTION_LST = 8;

//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pAppName) {
    printf("%s - checking, setting and changing QMap Headers\n", pAppName);
    printf("Usage:\n");
    printf("  %s <inputfile> \n", pAppName);
    printf("or\n");
    printf("  %s <inputfile> (-a <data> | -r <data> | -u <names>) [-l] [-o <outputfile>]\n", pAppName);
    printf("Where\n");
    printf("  inputfile :  name of binary input file\n");
    printf("  outputfile:  name of output file (default: a: append '.map' to input file, r:overwrite input file\n");
    printf("  -a        :  add a QMap Header\n");
    printf("  -r        :  replace QMap Header\n");
    printf("  -u        :  modify coordinate and value names in QMap Header\n");
    printf("  -l        :  show QMap Header\n");
    printf("  data      :  LonMin,LonMax,DLon,LatMin,LatMax,DLat:type[,VName[,XName[,YName]]],\n");
    printf("  names     :  [VName[,XName[,YName]]],\n");
}

//-------------------------------------------------------------------------------------------------
// splitNames
//   split data of the form "lonMin,lonMax,dLon,latMin,latMax,dLat:iType"
//
int splitNames(char *pData, 
              char *pVName, char *pXName, char *pYName) {
    char *pCtx;
    //    printf("Names---[%s]\n", pData);
    strcpy(pVName, DEF_V_NAME);
    strcpy(pXName, DEF_X_NAME);
    strcpy(pYName, DEF_Y_NAME);
    char *p = strtok_r(pData, ",", &pCtx);
    if (p!= NULL) {
        strcpy(pVName, p);
        p = strtok_r(NULL, ",", &pCtx);
        if (p != NULL) {
            strcpy(pXName, p);
            p = strtok_r(NULL, ",", &pCtx);
            if (p != NULL) {
                strcpy(pYName, p);
            }
        }
    }
    //    printf("Have Names |%s|%s|%s|\n", pVName,pXName,pYName);
    return 0;
}

//-------------------------------------------------------------------------------------------------
// splitData
//   split data of the form "lonMin,lonMax,dLon,latMin,latMax,dLat:iType"
//
int splitData(char *pData, 
              double *pdLonMin, double *pdLonMax, double *pdDLon,
              double *pdLatMin, double *pdLatMax, double *pdDLat, int *piType,
              char *pVName, char *pXName, char *pYName) {
    int iResult = -1;
    char *pCtx;
    char *p = strtok_r(pData, ",:", &pCtx);
    if (p != NULL) {
        char *pEnd;
        *pdLonMin = strtod(p, &pEnd);
        if (*pEnd == '\0') {
            p = strtok_r(NULL, ",:", &pCtx);
            if (p != NULL) {
                *pdLonMax = strtod(p, &pEnd);
                if (*pEnd == '\0') {        
                    p = strtok_r(NULL, ",:", &pCtx);
                    if (p != NULL) {
                        *pdDLon = strtod(p, &pEnd);
                        if (*pEnd == '\0') {        
                            p = strtok_r(NULL, ",:", &pCtx);
                            if (p != NULL) {
                                *pdLatMin = strtod(p, &pEnd);
                                if (*pEnd == '\0') {        
                                    p = strtok_r(NULL, ",:", &pCtx);
                                    if (p != NULL) {
                                        *pdLatMax = strtod(p, &pEnd);
                                        if (*pEnd == '\0') {        
                                            p = strtok_r(NULL, ",:", &pCtx);
                                            if (p != NULL) {
                                                *pdDLat = strtod(p, &pEnd);
                                                if (*pEnd == '\0') {        
                                                    p = strtok_r(NULL, ",:", &pCtx);
                                                    if (p != NULL) {
                                                        *piType = (int)strtol(p, &pEnd, 10);
                                                        if (*pEnd == '\0') {        
                                                            printf("Typnum:[%d]end[%s]\n", *piType, pEnd);

                                                            char c = QMapHeader::getTypeChar(*piType);
                                                            if (c != '\0') {
                                                                iResult = 0;
                                                            } else {
                                                                printf("Invalid type char: [%c]\n", c);
                                                            }
                                                        } else {
                                                            *piType = QMapHeader::getTypeID(*p);
                                                            if (*piType != QMAP_TYPE_NONE) {
                                                                iResult = 0;
                                                            } else {
                                                                printf("Bad Number format or bad type char: [%s]\n", p);
                                                            }
                                                        }
                                                        if (iResult == 0) {
                                                            p = strtok_r(NULL, ":", &pCtx);
                                                            iResult = splitNames(p, pVName, pXName, pYName);
                                                        }
                                                    } else {
                                                        printf("Bad Data Format: expected type\n");
                                                    }
                                                } else {
                                                    printf("Bad Number format: [%s]\n", p);
                                                }
                                            } else {
                                                printf("Bad Data Format: expected DLat\n");
                                            }
                                        } else {
                                            printf("Bad Number format: [%s]\n", p);
                                        }
                                    } else {
                                        printf("Bad Data Format: expected LatMax\n");
                                    }
                                } else {
                                    printf("Bad Number format: [%s]\n", p);
                                }
                            } else {
                                printf("Bad Data Format: expected LatMin\n");
                            }
                        } else {
                            printf("Bad Number format: [%s]\n", p);
                        }
                    } else {
                        printf("Bad Data Format: expected DLon\n");
                    }
                } else {
                    printf("Bad Number format: [%s]\n", p);
                }
            } else {
                printf("Bad Data Format: expected LonMax\n");
            }
        } else {
            printf("Bad Number format: [%s]\n", p);
        }
    } else {
        printf("Bad Data Format: expected LonMin\n");
    }
        
    return iResult;
}

//-------------------------------------------------------------------------------------------------
// updateHeader
//   show header contents & size
//
int updateHeader(char *sInputFile, char *pOut, char *pVName, char *pXName, char *pYName) {
    int iResult = -1;
    QMapHeader *pQMH = new QMapHeader();
    iResult = pQMH->readHeader(sInputFile);
    if (iResult == 0) {
        strcpy(pQMH->m_sVName, pVName);
        strcpy(pQMH->m_sXName, pXName);
        strcpy(pQMH->m_sYName, pYName);
        iResult = pQMH->replaceHeader(sInputFile, pOut);
    }
    return iResult;
}
//-------------------------------------------------------------------------------------------------
// showHeader
//   show header contents & size
//
int showHeader(char *sInputFile) {
    QMapHeader *pQMH = new QMapHeader();
    int iResult = pQMH->readHeader(sInputFile);
    if (iResult == 0) {
        //       printf("Names1: Val:%s, X:%s, Y:%s\n", pQMH->m_sVName, pQMH->m_sXName, pQMH->m_sYName);
        printf("File  [%s]:\n", sInputFile);
        printf("type %d (%s)\n", pQMH->m_iType, QMapHeader::getTypeName(pQMH->m_iType));
        printf("Lon: [%f, %f] D:%f\n", pQMH->m_dDataLonMin, pQMH->m_dDataLonMax, pQMH->m_dDLon); 
        printf("Lat: [%f, %f] D:%f\n", pQMH->m_dDataLatMin, pQMH->m_dDataLatMax, pQMH->m_dDLat); 
        printf("Size: %dx%d\n",pQMH->m_iWidth, pQMH->m_iHeight);
        
        int iTypeSize = 0;
        switch (pQMH->m_iType) {
        case QMAP_TYPE_UCHAR:
            iTypeSize = sizeof(uchar);
            break;
        case QMAP_TYPE_SHORT:
            iTypeSize = sizeof(short int);
            break;
        case QMAP_TYPE_INT:
            iTypeSize = sizeof(int);
            break;
        case QMAP_TYPE_LONG:
            iTypeSize = sizeof(long);
            break;
        case QMAP_TYPE_FLOAT:
            iTypeSize = sizeof(float);
            break;
        case QMAP_TYPE_DOUBLE:
            iTypeSize = sizeof(double);
            break;
        }

        printf("Names: Val:%s, X:%s, Y:%s\n", pQMH->m_sVName, pQMH->m_sXName, pQMH->m_sYName);
        int iCalcSize = QMapHeader::getHeaderSize(sInputFile)+pQMH->m_iWidth*pQMH->m_iHeight*iTypeSize;
        struct stat sbuf;
        iResult = stat(sInputFile, &sbuf);
        if (iResult == 0) {
            int iRealSize = sbuf.st_size;
            if (iRealSize == iCalcSize) {
                printf("      %d bytes\n", iCalcSize);
            } else {
                printf("***** calculated size (%d) differs from actual size (%d)!\n", iCalcSize, iRealSize);       
            }
        } else {
            printf("Couldn't measure file size of %s\n", sInputFile);
        }
    } else {
        printf("QMH error: %s\n", pQMH->getErrMess(iResult));
    }
    return iResult;
}

bool getParams(int iArgC, char *apArgV[], char *pInput, char *pData, char *pOutput, int *piAction) {


    bool bOK = false;
    int iI  = 1;
    if (iArgC > iI) {
        strcpy(pInput, apArgV[iI++]);
        bOK = true;
        printf("Input file [%s]\n", pInput);
        if (iArgC > iI) {
            // next should  be -a, -r or -u
            if (strcmp(apArgV[iI], "-a") == 0) {
                *piAction |= ACTION_ADD;
                iI++;
            } else  if (strcmp(apArgV[iI], "-r") == 0) {
                *piAction |= ACTION_REP;
                iI++;
            } else  if (strcmp(apArgV[iI], "-u") == 0) {
                *piAction |= ACTION_UPD;
                iI++;
 
            }
            
            if ((*piAction > 0) && (*piAction != ACTION_LST)) {
                if (iArgC > iI) {
                    strcpy(pData, apArgV[iI++]);
                } else {
                    bOK = false;
                    printf("need argument for option\n");
                }
            }
            if (iArgC > iI) {
                if (strcmp(apArgV[iI], "-l") == 0) {
                    *piAction |= ACTION_LST;
                    iI++;
                }
            }
            if (iArgC > iI) {
                if (strcmp(apArgV[iI], pInput) != 0) {
                    strcpy(pOutput, apArgV[iI++]);
                }
            }
        }
        if (*piAction == 0) {
            *piAction |= ACTION_LST;
        }
    } else {
        bOK = false;
        printf("Need a filename as first argument\n");
    }
    return bOK;

}


//-------------------------------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char sInputFile[MAX_PATH];
    char sOutputFile[MAX_PATH];
    char sData[MAX_PATH];
    char sVName[8];
    char sXName[8];
    char sYName[8];
    bool bList    = false;
    bool bAdd     = false;
    bool bReplace = false;
    bool bUpdate  = false;
    
    *sInputFile  = '\0';
    *sOutputFile = '\0';
    *sData       = '\0';
    int iAction = 0;

    char *pListTarget = sInputFile;
    bool bOK = getParams(iArgC, apArgV, sInputFile, sData, sOutputFile, &iAction); 
    if (bOK) {
        bAdd     = ((iAction & ACTION_ADD) != 0);
        bReplace = ((iAction & ACTION_REP) != 0);
        bUpdate  = ((iAction & ACTION_UPD) != 0);
        bList    = ((iAction & ACTION_LST) != 0);

        if (bAdd || bReplace) {
            double dLonMin;
            double dLonMax;
            double dDLon;
            double dLatMin;
            double dLatMax;
            double dDLat;
            int    iType;
            iResult = splitData(sData, 
                                &dLonMin, &dLonMax, &dDLon, 
                                &dLatMin, &dLatMax, &dDLat,
                                &iType, sVName, sXName, sYName);
            if (iResult == 0) {
                QMapHeader *pQMH = new QMapHeader(iType,
                                                  dLonMin, dLonMax, dDLon, 
                                                  dLatMin, dLatMax, dDLat,
                                                  sVName, sXName, sYName);
                if (bAdd) {
                    if (*sOutputFile == '\0') {
                        sprintf(sOutputFile, "%s.map", sInputFile);
                    }
                    bOK = pQMH->putHeader(sInputFile, sOutputFile);
                } else if (bReplace) {
                    char *pOut = (*sOutputFile == '\0')?NULL:sOutputFile;
                    bOK = pQMH->replaceHeader(sInputFile, pOut);
                } 
            }
            iResult = (bOK?0:-1);

            if ((iResult == 0) && (*sOutputFile != '\0')) {
                pListTarget = sOutputFile;
            }
        } else if (bUpdate) {
            iResult = splitNames(sData, 
                                 sVName, sXName, sYName);
            if (iResult == 0) {
                char *pOut = (*sOutputFile == '\0')?NULL:sOutputFile;
                bOK = updateHeader(sInputFile, pOut, 
                                       sVName, sXName, sYName);
                iResult = (bOK?0:-1);
            }
            if ((iResult == 0) && (*sOutputFile != '\0')) {
                pListTarget = sOutputFile;
            }

        }
        
        if (bList) {
            iResult = showHeader(pListTarget);
        }
    } else {
        usage(apArgV[0]);
    }
      
    return iResult;
}
