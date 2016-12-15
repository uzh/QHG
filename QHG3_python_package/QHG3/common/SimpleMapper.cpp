#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include "strutils.h"
#include "LineReader.h"
#include "ValueMapper.h"
#include "SimpleMapper.h"

//----------------------------------------------------------------------------
// constructor
//
template<class T> SimpleMapper<T>::SimpleMapper(TRANS_MAP &mapTrans, T tDefVal)
    : m_mapTrans(mapTrans),
      m_tDefVal(tDefVal) {
}

//----------------------------------------------------------------------------
// constructor
//
template<class T> SimpleMapper<T>::SimpleMapper(const char *pTranslationFile, T tDefVal) 
    : m_tDefVal(tDefVal) {

    bool bOK = false;
    LineReader *pLR = LineReader_std::createInstance(pTranslationFile, "rb");
    if (pLR != NULL) {
        if (pLR->isReady()) {
            bOK = true;
            char *pLine;
            char *pCtx;
            char *pEnd;
            while (bOK && !pLR->isEoF()) {
                pLine = pLR->getNextLine();
                if (pLine != NULL) {
                    // chop off comments beginning in line
                    char *pC = strchr(pLine, '#');
                    if (pC != NULL) {
                        *pC = '\0';
                    }
                    char *p = strtok_r(pLine, " ,;\t\n", &pCtx);
                    if (p != NULL) {
                        int iFromVal = strtol(trim(p), &pEnd, 10);
                        if (*pEnd == '\0') {
                            p = strtok_r(NULL, " ,;\t\n", &pCtx); 
                            if (p != NULL) {
                                int iToVal = strtol(trim(p), &pEnd, 10);
                                if (*pEnd == '\0') {
                                    //                                    printf("Map:%hu->%hu\n", iFromVal, iToVal);
                                    m_mapTrans[iFromVal] = iToVal;
                                } else {
                                    bOK = false;
                                    printf("bad number format: %s\n", pLine);
                                }
                            } else {
                                bOK = false;
                                printf("bad format: %s\n", pLine);
                            }
                        } else {
                            bOK = false;
                            printf("bad number format: %s\n", pLine);
                        }   
                    } else {
                        bOK = false;
                        printf("Bad format: %s\n", pLine);
                    }
                }
            }
        } else {
            printf("Couldn't open translation file %s\n", pTranslationFile);
        }
        delete pLR;
    }
 }

//----------------------------------------------------------------------------
// mapValue
//
template<class T> T SimpleMapper<T>::mapValue(T t) {
    T tOut = m_tDefVal;

    typename TRANS_MAP::const_iterator cit = m_mapTrans.find(t);
    if (cit != m_mapTrans.end()) {
        tOut = cit->second;
    }
    
    return tOut;
}

