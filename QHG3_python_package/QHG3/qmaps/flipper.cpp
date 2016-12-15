#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "strutils.h"
#include "ParamReader.h"
#include "QMapHeader.h"

const int BUFSIZE = 4096;
const int SHORT_INPUT = 64;
//-----------------------------------------------------------------------------
// Usage
//
void Usage(char *pProg) {
    printf("%s - flip binary file\n", pProg);
    printf("Usage:\n");
    printf("%s -i <Input> [-o <Output>]\n", pProg);
    printf("   -h <headersize>\n");
    printf("   -s <width>x<height>\n");
    printf("   -t <datatype> \n");
    printf("or:\n");
    printf("%s -i <Input> [-o <Output>]\n", pProg);
    printf("where\n");
    printf("  Input:     Name of binary input file \n");
    printf("  Output:    Body of output file name \n");
    printf("             if omitted, Input body with appended '_f.map' will be \n"); 
    printf("             written to ./\n");
    printf("  DataType:  Datatype of file\n");
    printf("              u: unsigned char\n");
    printf("              d: double\n");
    printf("              f: float\n");
    printf("              s: short\n");
    printf("              i: int\n");
    printf("  width:     Width\n");
    printf("  height:    Height\n");
    printf("  headersize: size of header to skip\n");
    printf(" the second form is only intended for BMAP files\n");
}



int main(int iArgC, char *apArgV[]) {
    char sInput[SHORT_INPUT];
    char sOutput[SHORT_INPUT];
    char sDataType[SHORT_INPUT];
    char sSize[SHORT_INPUT];
    
    int    iW   = -1;
    int    iH   = -1;
    int    iHeaderSize = 0;

    
    *sInput    = '\0';
    *sOutput   = '\0';
    *sDataType = '\0';
    *sSize     = '\0';

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(5,
                               "-i:s!",  sInput,
                               "-o:s",  sOutput,
                               "-s:s",  sSize,
                               "-t:s",  sDataType,
                               "-h:i",  &iHeaderSize);
    if (bOK) {     
        int iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {

            if ((*sDataType == '\0') || (*sSize == '\0')) {
                QMapHeader *pQMH = new QMapHeader();
                iResult = pQMH->readHeader(sInput);
                if (iResult == 0) {
                    printf("Header Info for %s\n", sInput);
                    *sDataType = QMapHeader::getTypeChar(pQMH->m_iType);
                    iW = pQMH->m_iWidth;
                    iH = pQMH->m_iHeight;
                    iHeaderSize = QMapHeader::getHeaderSize(sInput);
                    bOK = true;
                } else {
                    bOK = false;
                    printf("Couldn't read binmap [%s]: %s\n", sInput, pQMH->getErrMess(iResult));
                }
                delete pQMH;
            } else {

                bOK = splitSizeString(sSize, &iW, &iH);
            }
            if (bOK) {
                if (*sOutput == '\0') {
                    strcpy(sOutput, sInput);
                    // remove suffix
                    char *p = strrchr(sOutput, '.');
                    if (p != NULL) {
                        *p = '\0';
                    }

                    p = strrchr(sOutput, '/');
                    if (p == NULL) {
                        p = sOutput;
                    } else {
                        p++;
                    }
                    sprintf(sOutput,"%s_f.map", p);
                }


                
                int iSize = -1;
                switch (*sDataType) {
                case 'u':
                    iSize = sizeof(unsigned char);;
                    break;
                case 's':
                    iSize = sizeof(short);
                    break;
                case 'i':
                    iSize = sizeof(int);
                    break;
                case 'l':
                    iSize = sizeof(long);
                    break;
                case 'f':
                    iSize = sizeof(float);
                    break;

                case 'd':
                    iSize = sizeof(double);
                    break;
                }
                if (iSize > 0) {
                    printf("DataSize is %d\n", iSize);
                    printf("Map %dx%d\n", iW, iH);
                    printf("Output: %s\n", sOutput);
                    FILE * fIn = fopen(sInput, "rb");
                    if (fIn != NULL) {
                        FILE *fOut = fopen(sOutput, "wb");
                        if (fOut != NULL) {
                            char *sBuf = new char[iW*iSize];
                            long iCount = iHeaderSize;
                            printf("Doing Header (%d)\n", iHeaderSize);
                            while (iCount > 0) {
                                int iNum = iCount;
                                if (iNum > BUFSIZE) {
                                    iNum = BUFSIZE;
                                }
                                int iRead = fread(sBuf, sizeof(char), iNum, fIn);
                                fwrite(sBuf, sizeof(char), iNum, fOut);
                                iCount -= iRead;
                            }

                            long iRowStart = iHeaderSize+iW*(iH-1)*iSize; 
                            printf("Doing Lines starting at %ld\n", iRowStart);

                            while (iRowStart >= iHeaderSize) {
                                int iRes = fseek(fIn, iRowStart, SEEK_SET);
                                if (iRes == 0){
                                    //      printf("Seeked to %d\n", iRowStart);
                                } else {
                                    printf("seek problem\n");
                                }
                                fread(sBuf, iSize, iW, fIn);
                                fwrite(sBuf, iSize, iW,  fOut);
                            
                                iRowStart -= iW*iSize;
                            }
                            delete[] sBuf;
                            fclose(fOut);
                        } else {
                            printf("couldn't open %s\n", sOutput);
                        }
                        fclose(fIn);
                    } else {
                        printf("couldn't open %s\n", sInput);
                    }
                } else {
                    printf("Invalid type:[%s]\n", sDataType);
                }
            } else {
                printf("Invalid size: [%s]\n", sSize);
            }
        } else {
            Usage(apArgV[0]);
        }
    } else {
        printf("Error in setOptions\n");
    }
    return 0;
}
