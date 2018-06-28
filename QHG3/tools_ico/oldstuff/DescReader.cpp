#include <stdio.h>
#include <stdlib.h>

#include "DescReader.h"
#include "strutils.h"


//-----------------------------------------------------------------------------
// constructor
//
DescReader::DescReader(const char *pDesc) 
    : m_pExp(NULL),
      m_iRepeat(-10),
      m_iPos(0) {
    
    initialize(pDesc);

    reset();

}

//-----------------------------------------------------------------------------
// destructor
//
DescReader::~DescReader() {
    if (m_pExp != NULL) {
        delete[] m_pExp;
    }
}

//-----------------------------------------------------------------------------
// reset
//
void DescReader::reset() {
    m_pCur    = m_pExp;
    m_iRepeat = -1;  // start condition
    m_iPos    = -1;
}


//-----------------------------------------------------------------------------
// getNextItem
//
char DescReader::getNextItem() {
    m_iPos++;
    return *m_pCur++;
}


void DescReader::initialize(const char *pDesc) {
    char *pEnd;
    m_iLen = 0;
    const char *p = pDesc;
    uint iMultiplicator = 1;

    while (*p != '\0') {
        uint iNew = (uint) strtol(p, &pEnd, 10);
        if (p != pEnd) {
            p = pEnd;
            iMultiplicator = iNew;
        } else {
            switch (*p) {
            case 'b':
            case 'c':
            case 's':
            case 'i':
            case 'l':
            case 'f':
            case 'd':
                m_iLen += iMultiplicator;
                break;
            }
            iMultiplicator = 1;
            p++;
        } 

    }

    
    m_pExp = new char[m_iLen+1];
    p = pDesc;
    m_iDataSize = 0;
    char *p0 = m_pExp;
    while (*p != '\0') {
        uint iNew = (uint)strtol(p, &pEnd, 10);
        if (p != pEnd) {
            p = pEnd;
            iMultiplicator = iNew;
        } else {
            int iSize = 0;
            switch (*p) {
            case 'b':
            case 'c':
                iSize = sizeof(char);
                break;
            case 's':
                iSize = sizeof(short int);
                break;
            case 'i':
                iSize = sizeof(int);
                break;
            case 'l':
                //iSize = sizeof(long);
                iSize = sizeof(long long);
                break;
            case 'f':
                iSize = sizeof(float);
                break;
            case 'd':
                iSize = sizeof(double);
                break;
                
            }   
            if (iSize > 0) {
                m_iDataSize += iMultiplicator*iSize;
                for (uint i = 0; i < iMultiplicator; i++) {
                    *p0++ = *p;
                }
            }

            iMultiplicator = 1;
            p++;
        } 

    }
    *p0 = '\0';
}



//-----------------------------------------------------------------------------
// calcDataSize
//
size_t DescReader::calcDataSize(char *pDataDesc) {
    DescReader *pDR = new DescReader(pDataDesc);
    size_t iSize = pDR->getDataSize();
    delete pDR;
    return iSize;
}

//-----------------------------------------------------------------------------
// data2AscFile
//
unsigned char *DescReader::data2AscFile(char *pDesc, FILE *fOut, unsigned char *p0) {
   
    char *pDesc0 = pDesc;
    int iSize = 0;
    char c;
    DescReader *pDR = new DescReader(pDesc);
    unumber val;

    do {
        c = pDR->getNextItem();
        if (c != '\0') {
            switch (c) {
            case 'b':
            case 'c':
                iSize = sizeof(char);
                p0 = getMem(&val, p0, iSize);
                fprintf(fOut, "%d ", val.c);
                break;
            case 's':
                iSize = sizeof(short int);
                p0 = getMem(&val, p0, iSize);
                fprintf(fOut, "%d ", val.s);
                break;
            case 'i':
                iSize = sizeof(int);
                p0 = getMem(&val, p0, iSize);
                fprintf(fOut, "%d ", val.i);
                break;
            case 'l':
                iSize = sizeof(long long);
                p0 = getMem(&val, p0, iSize);
                fprintf(fOut, "%lld ", val.l);
                break;
            case 'f':
                iSize = sizeof(float);
                p0 = getMem(&val, p0, iSize);
                fprintf(fOut, "%f ", val.f);
                break;
            case 'd':
                iSize = sizeof(double);
                p0 = getMem(&val, p0, iSize);
                fprintf(fOut, "%f ", val.d);
                break;
            
            default:
                p0 = NULL; // unknown desc 
                printf("[asc2Data] unknown desc [%c] in [%s]\n", *pDesc, pDesc0);
            }    

        }
    } while ((c != '\0') && (p0 != NULL));
    
    delete pDR;

    return p0;
}

//-----------------------------------------------------------------------------
// extractData
//
double *DescReader::extractData(const char *pDesc, int iFieldNo, int iNumItems, unsigned char *p0) {
    double *pData = new double[iNumItems];
    double *pData0 = pData;

   
    const char *pDesc0 = pDesc;
    int iSize = 0;
    int iLastItem=iFieldNo+iNumItems;
    char c;
    DescReader *pDR = new DescReader(pDesc);
    unumber val;
    int iC = 0;
    do {
        c = pDR->getNextItem();
        bool bSave=(iC >= iFieldNo) && (iC < iLastItem);
        if (c != '\0') {
            switch (c) {
            case 'b':
            case 'c':
                iSize = sizeof(char);
                p0 = getMem(&val, p0, iSize);
                if (bSave) {
                    *pData++ = (double)val.c;
                }
                break;
            case 's':
                iSize = sizeof(short int);
                p0 = getMem(&val, p0, iSize);
                if (bSave) {
                    *pData++ = (double)val.s;
         
                }
                break;
            case 'i':
                iSize = sizeof(int);
                p0 = getMem(&val, p0, iSize);
                if (bSave) {
                    *pData++ = (double)val.i;
                }         
                break;
            case 'l':
                iSize = sizeof(long long);
                p0 = getMem(&val, p0, iSize);
                if (bSave) {
                    *pData++ = (double)val.l; 
                }
                break;
            case 'f':
                iSize = sizeof(float);
                p0 = getMem(&val, p0, iSize);
                if (bSave) {
                    *pData++ = (double)val.f;
                }
                break;
            case 'd':
                iSize = sizeof(double);
                p0 = getMem(&val, p0, iSize);
                if (bSave) {
                    *pData++ = (double)val.d;
                }
                break;
                    
            default:
                p0 = NULL; // unknown desc 
                printf("[asc2Data] unknown desc [%c] in [%s]\n", *pDesc, pDesc0);
            }    
                
        }
        iC++;
    } while ((c != '\0') && (p0 != NULL));
    
    delete pDR;

 
    return pData0;
}


int DescReader::getValue(char *p, char cType, unumber *pval) {
    int iSize = -1;
    char *pEnd=NULL;

    switch (cType) {
    case 'b':
    case 'c':
        iSize = sizeof(char);
        pval->c = (char) strtol(p, &pEnd, 10);
        break;
    case 's':
        iSize = sizeof(short int);
        pval->s = (short int) strtol(p, &pEnd, 10);
        break;
    case 'i':
        iSize = sizeof(int);
        pval->i = (int) strtol(p, &pEnd, 10);
        break;
    case 'l':
        iSize = sizeof(long long);
        pval->l = (long long) strtol(p, &pEnd, 10);
        break;
    case 'f':
        iSize = sizeof(float);
        pval->f = (float) strtod(p, &pEnd);
        break;
    case 'd':
        iSize = sizeof(double);
        pval->d = (double) strtod(p, &pEnd);
        break;
    default:
        // unknown desc 
        iSize = -1;
    }    
    if ((pEnd != NULL) && (*pEnd != '\0')) {
        iSize = -1;
    }
    return iSize; 
}
