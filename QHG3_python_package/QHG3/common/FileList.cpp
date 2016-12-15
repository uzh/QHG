#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include "FileList.h"
#include "utils.h"

//------------------------------------------------------------------------------
// constructor
//
FileList::FileList(void)
    : m_hDir(NULL),
      m_FileData(NULL),
      m_pvCurMatches(NULL) {

  
    memset(m_sCurFile, 0, MAX_PATH*sizeof(char));
    m_pPat = new Pattern();
    reset();

}

//------------------------------------------------------------------------------
// destructor
//
FileList::~FileList(void) {
    if (m_hDir != NULL) {
        closedir(m_hDir);
        m_hDir = NULL;
    }
    if (m_pvCurMatches != NULL) {
        delete m_pvCurMatches;
    }
    
    clearMatches();
    delete m_pPat;
}

//------------------------------------------------------------------------------
// addPattern
//
void FileList::addPattern(const char *pFilePattern, bool bInclude) {
    //    printf("Adding [%s]\n", pFilePattern);
    m_pPat->addPattern(pFilePattern);
    m_vvPatternIncludes.push_back(bInclude);

    //    int iNumP = m_pPat->numPatterns();
    //    printf("Now have %d pattern%s\n", iNumP, (iNumP!=1)?"s":"");
   
}

//------------------------------------------------------------------------------
// clearPatterns
//
void FileList::clearMatches() {
    
    m_vvPatternIncludes.clear();
  
    MAP_STRING_LIST::iterator iter;
        for (iter = m_mvMatches.begin(); iter != m_mvMatches.end(); ++iter) {
        if (iter->second != NULL) {
            delete iter->second;
        }
    }
    m_mvMatches.clear();
}

//------------------------------------------------------------------------------
// initSearch
//
int FileList::initSearch(const char *pDir) {
    int iResult = -1;

    
    m_hDir = opendir(pDir);
    if (m_hDir != NULL) {
        m_bSearching = true;
  
        const char *p = NULL;
        do {
            p = findNextFile();
        } while (p != NULL);
    
        sort(m_vData.begin(), m_vData.end());
        iResult = (int)m_vData.size();
        
        closedir(m_hDir);
        m_hDir = NULL;
    }
    return iResult;
}

//------------------------------------------------------------------------------
// initSearch
//
int FileList::initSearch(const char *pDir, const char *pFilePattern) {
    int iResult = -1;
    reset();
    m_pPat->addPattern(pFilePattern);
    
    m_vvPatternIncludes.push_back(FL_INCLUDE);
   
    m_hDir = opendir(pDir);
    if (m_hDir != NULL) {
        m_bSearching = true;
 
  
        const char *p = NULL;
        do {
            p = findNextFile();
        } while (p != NULL);
    
        sort(m_vData.begin(), m_vData.end());
        iResult = (int)m_vData.size();

        closedir(m_hDir);
        m_hDir = NULL;
        
    }
    return iResult;
}

//------------------------------------------------------------------------------
// extendSearch
//
int FileList::extendSearch(const char *pDir) {
    int iResult = -1;
    m_hDir = opendir(pDir);
    if (m_hDir != NULL) {
        m_bSearching = true;
  
        const char *p = NULL;
        do {
            p = findNextFile();
        } while (p != NULL);
    
        sort(m_vData.begin(), m_vData.end());
        iResult = (int)m_vData.size();
      
        closedir(m_hDir);
        m_hDir = NULL;
    }
    return iResult;
}

//------------------------------------------------------------------------------
// reset
//
void FileList::reset(void) {
    //@@@    clearPatterns();
    clearMatches();
    m_pPat->clear();
    m_vData.clear();
    m_iCurIndex = 0;
}

//------------------------------------------------------------------------------
// findNextFile
//
char *FileList::findNextFile(void) {
    char *pResult = NULL;

    if (m_bSearching) {
        bool bMatch = (m_vvPatternIncludes.size()==0);
        m_FileData = readdir(m_hDir);
        if (m_pvCurMatches != NULL) {
            //            delete m_pvCurMatches; //!!!!!!!!!!
        }
        m_pvCurMatches = new VEC_STRINGS();
        
        while (!bMatch  && (m_FileData != NULL)) {
           
            bMatch = true;
            for (unsigned int i = 0; i < m_pPat->numPatterns(); i++) {
                bool bTemp = m_pPat->matchPattern(i, m_FileData->d_name, *m_pvCurMatches);
                
                // dont use files matching an "exclude" pattern
                bMatch  &= (bTemp == m_vvPatternIncludes[i]);
                
            }
            // no match: check next file
            if (!bMatch) {       
                m_pvCurMatches->clear();
                m_FileData = readdir(m_hDir);
            }
        }
        // have match: save file name
        if (m_FileData != NULL) {
            if (bMatch) {
                strcpy(m_sCurFile, m_FileData->d_name);
                m_vData.push_back(m_FileData->d_name);
              
                m_mvMatches[m_FileData->d_name] = m_pvCurMatches;
            }
            
            pResult = m_sCurFile;
        } else {
            m_bSearching = false;
            pResult = NULL;
            delete m_pvCurMatches;
            m_pvCurMatches = NULL;
        }
        
    }
    return pResult;
}

//------------------------------------------------------------------------------
// getNextFile
//
const char *FileList::getNextFile(void) {
    const char *p = NULL;
    if  (m_iCurIndex < m_vData.size()) {
        p = m_vData[m_iCurIndex].c_str();
        ++m_iCurIndex;
    }
    return p;
    
}

//------------------------------------------------------------------------------
// getCurFile
//
const char *FileList::getCurFile(void) {
    const char *p = NULL;
    if (m_iCurIndex == 0) {
        p = m_vData[0].c_str();
    } else if ((0 < m_iCurIndex) &&(m_iCurIndex <= m_vData.size())) {
        p = m_vData[m_iCurIndex-1].c_str();
    }
    return p;
}

//------------------------------------------------------------------------------
// getCurMatches
//
VEC_STRINGS *FileList::getCurMatches(void) {
    VEC_STRINGS *pv = NULL;
    if (m_iCurIndex == 0) {
        pv = m_mvMatches[m_vData[0]];
    } else if ((0 < m_iCurIndex) &&(m_iCurIndex <= m_vData.size())) {
        pv = m_mvMatches[m_vData[m_iCurIndex-1]];
    }

    return pv;
}

//------------------------------------------------------------------------------
// matchPattern
//
int FileList::matchPattern(int iPat, char *pString) {
    int bOK = 0;
    m_pvCurMatches->clear();

    bOK = m_pPat->matchPattern(iPat, pString, *m_pvCurMatches);

    return bOK;
}

//*************** static methods *******************************//

//------------------------------------------------------------------------------
// countWildCards
//
int FileList::countWildcards(char *pString) {
    int iCount = 0;
    while (*pString != '\0') {
        switch (*pString) {
        case '*':
        case '#':
        case '&':
        case '?':
            iCount++;
            break;
        }
        ++pString;
    }
    return iCount;

}

//-----------------------------------------------------------------------------
// replaceRefs
//   replace placeholders @0 - @9 with corresponding fragments of vs
//   
int FileList::replaceRefs(VEC_INTS &vOrder, VEC_STRINGS &vs, const char *pOut, char *pOutR) {
    int iResult = 0;

    //    printf("ReplaceRefs [%s]\n", pOut);
    const char *p = pOut;
    char *q = pOutR;
    
    while ((iResult == 0) && (*p != '\0')) {
        if (*p == '@') {
            *q = '\0';
            //            printf("ReplaceRefs: encountered $ when pOutR[%s]\n", pOutR);
            p++;
            if (isdigit(*p)) {
                int iIndex = *p - '0';
                unsigned int iIndex2;
                bool bSearching = true;

                // find original index in pattern
                for (unsigned int i= 0; bSearching && (i < vOrder.size()); ++i) {
                    //@@printf("Comparing %d with %d\n", iIndex, vOrder[i]);
                    if (vOrder[i] == iIndex) {
                        iIndex2 = i;
                        bSearching = false;
                    }
                }
                if (!bSearching) {
                    if (iIndex2 < vs.size()) {
                    
                        //@@printf("ReplaceRefs: appending frag %d [%s]\n", iIndex, vs[iIndex].c_str());
                        strcat(q, vs[iIndex2].c_str());
                        q += vs[iIndex2].length();
                    } else {
                        printf("Invalid index : %d/%zd\n", iIndex2, vs.size());
                        iResult = -1;
                    }
                    p++;
                } else {
                    printf("Invalid index : %d\n", iIndex);
                    iResult = -1;
                }
            }
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
    return iResult;
}

//-----------------------------------------------------------------------------
// replaceRefsSimple
//   replace wildcards in pOut with fragments in vs in parallel
//   
int FileList::replaceRefsSimple(VEC_STRINGS &vs, const char *pOut, char *pOutR) {
    const char *p = pOut;
    char *q = pOutR;
    unsigned int iIndex = 0;
    while (*p != '\0') {
        if ((*p == '*') || (*p == '#') || (*p == '&')) {
            if (iIndex < vs.size()) {
                *q = '\0'; // so strcat will work 
                strcat(pOutR, vs[iIndex].c_str());
                q += vs[iIndex].length();
                iIndex++;
                p++;
            } else {
                *q++ = *p++;
            }
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
    return iIndex;
}
