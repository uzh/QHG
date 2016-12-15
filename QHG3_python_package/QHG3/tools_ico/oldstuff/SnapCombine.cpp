#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>

#include "utils.h"
#include "strutils.h"
#include "icoutil.h"
#include "NodeLister.h"
#include "SnapHeader.h"
#include "LineReader.h"

typedef std::vector<std::pair<long, double> > plist;
#define NUMITS 1000

const int OP_PLUS   =  0;
const int OP_MINUS  =  1;
const int OP_TIMES  =  2;
const int OP_DIV    =  3;
const int OP_MIN    =  4;
const int OP_MAX    =  5;
const int OP_LT     =  6;
const int OP_LE     =  7;
const int OP_EQ     =  8;
const int OP_GE     =  9;
const int OP_GT     = 10;
const int OP_NE     = 11;
const int OP_ISNAN  = 12;
const int OP_ISINF  = 13;
const int OP_ISNUM  = 14;
const int OP_SETNAN = 15;
const int OP_SETINF = 16;
const int NUM_OP    = 17;

// operator names
static const char *sOps[] = {
    "plus",
    "minus",
    "times",
    "div",
    "min",
    "max",
    "lt",
    "le",
    "eq",
    "ge",
    "gt",
    "ne",
    "isnan",
    "isinf",
    "isnum",
    "setnan",
    "setinf",
};
// operator description
static const char *sDesc[] = {
    "add",
    "subtract",
    "multiply",
    "divide",
    "minimum",
    "maximum",
    "less than",
    "less or equal",
    "equals",
    "greater or equal",
    "greater than",
    "not equal",
    "is NaN (second operand must be 0)",
    "is inf (second operand must be 0)",
    "is numeric (second operand must be 0)",
    "set NaN value to number",
    "set inf value to number",
};


typedef double binop(double, double);

double plus(double d1,  double d2) { return d1 + d2;};
double minus(double d1, double d2) { return d1 - d2;};
double times(double d1, double d2) { return d1 * d2;};
double div(double d1,   double d2) { return d1 / d2;};
double min(double d1,   double d2) { return (d1 < d2)?d1:d2;};
double max(double d1,   double d2) { return (d1 < d2)?d2:d1;};
double lt(double d1,    double d2) { return d1 < d2;};
double le(double d1,    double d2) { return d1 <= d2;};
double eq(double d1,    double d2) { return d1 == d2;};
double ge(double d1,    double d2) { return d1 > d2;};
double gt(double d1,    double d2) { return d1 >= d2;};
double ne(double d1,    double d2) { return d1 != d2;};
double isn(double d1,   double d2) { return isnan(d1);};
double isi(double d1,   double d2) { return isinf(d1);};
double isnum(double d1,   double d2) { return isnormal(d1);};
double setnan(double d1,   double d2) { return isnan(d1)?d2:d1;};
double setinf(double d1,   double d2) { return isinf(d1)?d2:d1;};

static const binop *aops[] = {
    plus, 
    minus,
    times,
    div,  
    min,  
    max,  
    lt,   
    le,   
    eq,   
    ge,   
    gt,   
    ne,    
    isn,  
    isi,
    isnum,
    setnan,
    setinf,
};

//-------------------------------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - combine 2 Ico-Snaps of equal geometry with an operation\n", pApp);
    printf("usage:\n");
    printf("  %s <Snap1> <op> <Snap2> <SnapOut>\n", pApp);
    printf("or\n");
    printf("  %s <Snap1> <op> <Val2> <SnapOut>\n", pApp);
    printf("or\n");
    printf("  %s <Val1> <op> <Snap2> <SnapOut>\n", pApp);
    printf("where\n");
    printf("  Snap1    : name of first input Snap\n");
    printf("  Snap2    : name of second input Snap\n");
    printf("  Val1     : value of left operand (case 3),  may also be nan,inf,-inf\n");
    printf("  Val2     : value of right operand (case 2), may also be nan,inf,-inf\n");
    printf("  SnapOut  : name for result QMap\n");
    printf("  op       : an operator:\n");

    for (int i = 0; i <  NUM_OP; i++) {
        printf("               %s - %s\n", sOps[i], sDesc[i]);
    }
    printf("\n");
    printf("Note that the Snaps must be built for the same icosahedron,\n");
    printf("otherwise the results are undefined\n");
    printf("\n");
}

//-------------------------------------------------------------------------------------------------
// listoplist
//
int listoplist(binop *fun, nodelist &nlVal1, nodelist &nlVal2, plist &vRes) {
    int iResult = 0;
    nodelist::const_iterator it1 = nlVal1.begin();
    nodelist::const_iterator it2 = nlVal2.begin();

    while ((it1 != nlVal1.end()) || (it2 != nlVal2.end())) {
        if ((it1 != nlVal1.end()) && (it2 != nlVal2.end())) {
            while ((it1 != nlVal1.end()) && (it1->first < it2->first)) {
                vRes.push_back(std::pair< long, double>(it1->first, it1->second));
                it1++;
            }
            while ((it2 != nlVal1.end()) && (it1->first > it2->first)) {
                vRes.push_back(std::pair< long, double>(it2->first, it2->second));
                it2++;
            }
        }
        if ((it1 != nlVal1.end()) && (it2 != nlVal2.end())) {
            if (it1->first == it2->first) {
                double dVal;
                dVal= fun(it1->second, it2->second);
                vRes.push_back(std::pair< long, double>(it1->first, dVal));
                it1++;
                it2++;
            }
            
        } else if (it1 != nlVal1.end())  {
            vRes.push_back(std::pair< long, double>(it1->first, it1->second));
            it1++;
        } else if (it2 != nlVal2.end())  {
            vRes.push_back(std::pair< long, double>(it2->first, it2->second));
            it2++;
        }
        
    }
    return iResult;
}

//-------------------------------------------------------------------------------------------------
// listopnum
//
int listopnum(binop *fun, nodelist &nlVal1, double dVal2, plist &vRes) {
    int iResult = 0;
    printf("Doing listopnum\n");
    nodelist::const_iterator it1 = nlVal1.begin();
            
    while (it1 != nlVal1.end()) {
        double dVal = fun(it1->second, dVal2);
        vRes.push_back(std::pair< long, double>(it1->first, dVal));
        it1++;
    }
    printf("Done %d\n", iResult);
    return iResult;
}

//-------------------------------------------------------------------------------------------------
// numoplist
//
int numoplist(binop *fun, double dVal1, nodelist &nlVal2, plist &vRes) {
    int iResult = 0;
    nodelist::const_iterator it2 = nlVal2.begin();
            
    while (it2 != nlVal2.end()) {
        double dVal = fun(dVal1, it2->second);
        vRes.push_back(std::pair< long, double>(it2->first, dVal));
        it2++;
    }
    return iResult;
}

//-------------------------------------------------------------------------------------------------
// main
//  arguments: 
//    <QMap1> <OpName> <QMap2> <QmapOut>
//  or
//    <QMap1> <OpName> <number> <QmapOut>
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sInput1[LONG_INPUT];
    char sInput2[LONG_INPUT];
    char sOutput[LONG_INPUT];
    char sOp[64];

    nodelist nlVal1;
    double   dVal1 = dNaN;
    nodelist nlVal2;
    double dVal2 = dNaN;
    
    if (iArgC > 4) {
        strcpy(sInput1, apArgV[1]);
        strcpy(sInput2, apArgV[3]);
        strcpy(sOutput, apArgV[4]);
        strcpy(sOp, apArgV[2]);


        binop *fun = NULL;
        for (int i =0; (fun == NULL) && (i < NUM_OP); i++) {
            if (strcmp(sOp, sOps[i]) == 0) {
                fun = aops[i];
                iResult = 0;
                printf("Selected fun #%d\n", i);
            }
        }

        double   dMax;
        double   dMin;

        bool bPreSel=false;
        char sIcoFile[256];
        *sIcoFile = '\0';

        if (iResult == 0) {

            // check if sInput1 is a filename; if it is, read it
            iResult = NodeLister::createList(sInput1, nlVal1, &dMin, &dMax);
            if (iResult < 0) {
                iResult = 0;
                // it was not a file, so it should be a number
                if (strToNum(sInput1, &dVal1)) {
                    iResult = 0;
                } else if (strcmp(sInput1, "nan") == 0) {
                    dVal1 = dNaN;
                } else if (strcmp(sInput1, "inf") == 0) {
                    dVal1 = dPosInf;
                } else if (strcmp(sInput1, "+inf") == 0) {
                    dVal1 = dPosInf;
                } else if (strcmp(sInput1, "-inf") == 0) {
                    dVal1 = dNegInf;
                } else {
                    // error
                    printf("[%s] is neither a snap file nor a valid number\n", sInput1);
                    iResult = -1;
                }
                if (iResult == 0) {
                    printf("  -->Yes: %f\n", dVal1);
                }
 
            } else {
                iResult = -1;
                printf("find ico file in [%s] if there\n", sInput1);
                LineReader *pLR = LineReader_std::createInstance(sInput1, "rt");
                if (pLR != NULL) {
                    SnapHeader *pSH = new SnapHeader();
                    iResult = pSH->read(pLR, 0xffffffff);
                    if (iResult == 0) {
                        strcpy(sIcoFile, pSH->m_sIcoFile);
                        bPreSel=pSH->m_bPreSel;
                        printf("got icofile  [%s]\n", sIcoFile);
                    } else {
                        printf("Error reading snap header from [%s]\n", sInput1);
                        printf("   %s\n", pSH->getErrMess());
                    }
                } else {
                    printf("A couldn't open [%s] for reading\n", sInput1);
                }

            }
        }
            
        if (iResult == 0) {
            printf("Is [%s] a snapfile?\n", sInput2);
            // check if sInput2 is a filename; if it is, read it
            iResult = NodeLister::createList(sInput2, nlVal2, &dMin, &dMax);
            if (iResult < 0) {
                printf("  --> no; is it a number?\n");
            
                iResult = 0;

                // it was not a file, so it should be a number
                if (strToNum(sInput2, &dVal2)) {
                    iResult = 0;
                } else if (strcmp(sInput2, "nan") == 0) {
                    dVal2 = dNaN;
                } else if (strcmp(sInput2, "inf") == 0) {
                    dVal2 = dPosInf;
                } else if (strcmp(sInput2, "+inf") == 0) {
                    dVal2 = dPosInf;
                } else if (strcmp(sInput2, "-inf") == 0) {
                    dVal2 = dNegInf;
                } else {
                    printf("[%s] is neither a snap file nor a valid number\n", sInput2);
                    iResult = -1;
                }
                if (iResult == 0) {
                    printf("  -->Yes: %f\n", dVal2);
                }
            } else {
                printf("  --> yes;\n");
                printf("find ico file in [%s] if there\n", sInput2);
                LineReader *pLR = LineReader_std::createInstance(sInput2, "rt");
                if (pLR != NULL) {
                    // try snap
                    SnapHeader *pSH = new SnapHeader();
                    iResult = pSH->read(pLR, 0xffffffff);
                    if (iResult == 0) {
                        if (*sIcoFile == '\0') {
                            strcpy(sIcoFile, pSH->m_sIcoFile);
                            bPreSel=pSH->m_bPreSel;
                        } else {
                            if ((strcmp(sIcoFile,  pSH->m_sIcoFile) != 0) || (bPreSel != pSH->m_bPreSel)) {
                                printf("WARNING!! Mismatched Ico files and/or PreSel setting\n");
                            }
                        }
                    }
                }
            }
        }
    
        // do the calculation
        if (iResult == 0) {
            printf("operating....\n");
            plist vRes;
            if (isnan(dVal1) && isnan(dVal2)) {
                printf("li1 size: %zd\n", nlVal1.size());
                iResult = listoplist(fun, nlVal1, nlVal2, vRes);
            } else if (isnan(dVal1) && !isnan(dVal2)) {
                iResult = listopnum(fun, nlVal1, dVal2, vRes);
            } else if (!isnan(dVal1) && isnan(dVal2)) {
                iResult = numoplist(fun, dVal1, nlVal2, vRes);
            } else {
                printf("Use your head to combine two numbers!\n");
                iResult = -1;
            }
        

        
            // save the result
            if (iResult == 0) {
                printf("Do the save to [%s]\n", sOutput);
                SnapHeader *pSH = new SnapHeader(MAGIC_SNAP, COORD_NODE,0, 0, "ld",  "()", bPreSel, -1, "COMB", 0, NULL);
                FILE *fOut = fopen(sOutput, "wb");
                if (fOut != NULL) {
                    pSH->write(fOut, true);
                    
                    int itemsize = sizeof(long) + sizeof(double);;
                    unsigned char *pBuf = new unsigned char[NUMITS*itemsize];
                    int iC = 0;
                    unsigned char *pCur = pBuf;
                    // copy snap header from one of the files
                    // write it to file 
                    int iW = 0;
                    for (unsigned int i = 0; (iResult == 0) && (i < vRes.size()); i++) {
                        if (iC == NUMITS) {
                            iW = fwrite(pBuf, itemsize, iC, fOut);
                            if (iW != iC) {
                                iResult = -1;
                            } else {
                                pCur = pBuf;
                                iC = 0;
                            }
                        }
                        pCur = putMem(pCur, &vRes[i].first, sizeof(gridtype));
                        pCur = putMem(pCur, &vRes[i].second, sizeof(double));
                        iC++;
                        //                    printf("%ld -> %f\n", vRes[i].first, vRes[i].second);
                    }
                    if ((iResult == 0) && (iC > 0)) {
                        iW = fwrite(pBuf, itemsize, iC, fOut);
                        if (iW != iC) {
                            iResult = -1;
                        }
                    }
                    fclose(fOut);
                } else {
                    printf("Couldn't open [%s] for writing\n", sOutput);
                }
            } else {

                printf("Huh?\n");
            }
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
