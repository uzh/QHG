/*============================================================================
| Pattern
| 
|  Represents a pattern including wild cards for use by FileList, with
|  a match() function to match it to a string
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#include <string.h>
#include <vector>
#include <algorithm>

#include "utils.h"
#include "Pattern.h"

Pattern::Pattern() {
}

Pattern::~Pattern() {
}

//------------------------------------------------------------------------------
// clear
//
void Pattern::clear() {
    m_vvPatternParts.clear();
    m_vvPatternModes.clear();
}

//------------------------------------------------------------------------------
// addPattern
//
void Pattern::addPattern(const char *pFilePattern) {
     char *pCopy = new char[strlen(pFilePattern)+1];
    strcpy(pCopy, pFilePattern);
    preparePattern(pCopy);
    delete[] pCopy;
}

//------------------------------------------------------------------------------
// min
//   return the smaller of the 2 pointers (NULL == infinity)
//
char *min(char *p1, char *p2) {
    char *p = NULL;
    if (p1 == NULL) {
      p = p2;
    } else if (p2 == NULL) {
      p = p1;
    } else {
      p = (p1<p2)?p1:p2;
    }
    return p;    
}

//------------------------------------------------------------------------------
// preparePattern
//
void Pattern::preparePattern(char *pPattern) {
  int iIndex = 0;
  int bGoOn  = 1;
  char *p;
  char *p1;
  char *p2;
  char *p3;
  char *p4;
  char *pCur;

  VEC_CHARS   vModes;
  VEC_STRINGS vParts;

  if (pPattern != NULL) {
      switch (*pPattern) {
        case '?':
        case '*':
          vModes.push_back(*pPattern);
          pPattern++;
          break;
        default:
          vModes.push_back('\0');
      }

      pCur = pPattern;
      while ((*pCur != '\0') && (iIndex < MAX_NUMPARTS)) {
        // find the first wildcard starting from current position in string
        p1 = strchr(pCur, '?');
        p2 = strchr(pCur, '*');
        p3 = strchr(pCur, '#');
        p4 = strchr(pCur, '&');
        p = min(min(min(p1, p2), p3), p4);

        if (p != NULL) {
          vModes.push_back(*p);
          *p = '\0';
        } else {
          vModes.push_back('\0');
          // make sure loop will stop after next part
          p = pCur +strlen(pCur)-1;
        }

        vParts.push_back(pCur);
        if (bGoOn) {
          pCur = p+1;
        }
        iIndex++;
      }
    } else {
    }

    m_vvPatternParts.push_back(vParts);
    m_vvPatternModes.push_back(vModes);
}


//------------------------------------------------------------------------------
// subMatch
//   This recursive method tries to match the part #iPart of pattern #iPat to the
//   current string part.
//   In case of mode '*', all possible matches are tried, and a sub match
//   with the next pattern and the substring starting at the end of the match
//   is tried until a complete match is found.
//   is found.
//   In case of mode '?', the current string loses its first character, and
//   a match with the pattern is tried; if OK, a sub match with the next pattern
//   and the substring starting at the end of the match is tried
//   In case of mode '\0', a match with the pattern is tried; if OK, a sub match
//   with the next pattern and the substring starting at the end of the match
//   is tried
//
//   The recursion stops if a '?' or '\0' match fails, or if there are no more
//   patterns or if the substring is empty
//
bool Pattern::subMatch(unsigned int iPat, unsigned int iPart,const char *pString, VEC_STRINGS &vMatches) {
    bool bOK=false;
    const char *p;
    char sMatch[MAX_PATH];

    if (iPart < m_vvPatternParts[iPat].size()) {

        switch (m_vvPatternModes[iPat][iPart]) {
        case '*':
            p = strstr(pString, m_vvPatternParts[iPat][iPart].c_str());
            if (p != NULL) {

                strncpy(sMatch, pString, p-pString);
                sMatch[p-pString] = '\0';
                vMatches.push_back(sMatch); 

              bOK = false;
              while ((!bOK) && (p != NULL)) {
                  bOK = subMatch(iPat, iPart+1, p + strlen(m_vvPatternParts[iPat][iPart].c_str()), vMatches);
                  p = strstr(p+1, m_vvPatternParts[iPat][iPart].c_str());
              }

            } else {
              bOK = false;
            }
            break;

        case '#':
            if (isdigit(*pString)) {
                const char *p0 = pString;
                do {
                  pString++;
                } while (isdigit(*pString));
                p = strstr(pString, m_vvPatternParts[iPat][iPart].c_str());
                if (p == pString) {

                    strncpy(sMatch, p0, p-p0);
                    sMatch[p-p0] = '\0';
                    vMatches.push_back(sMatch); 
                    bOK = subMatch(iPat, iPart+1, p+strlen(m_vvPatternParts[iPat][iPart].c_str()), vMatches);

                }
            } else {
                bOK = false;
            }
            break;

       case '&':
            if (isxdigit(*pString)) {
                const char *p0 = pString;
                do {
                  pString++;
                } while (isxdigit(*pString));
                p = strstr(pString, m_vvPatternParts[iPat][iPart].c_str());
                if (p == pString) {

                    strncpy(sMatch, p0, p-p0);
                    sMatch[p-p0] = '\0';
                    vMatches.push_back(sMatch); 

                    bOK = subMatch(iPat, iPart+1, p+strlen(m_vvPatternParts[iPat][iPart].c_str()), vMatches);
                }
            } else {
                bOK = false;
            }
            break;

        case '?':
            sprintf(sMatch, "%c", *pString);
            pString++;

            p = strstr(pString, m_vvPatternParts[iPat][iPart].c_str());
            if (p == pString) {
                vMatches.push_back(sMatch); 
                bOK = subMatch(iPat, iPart+1, p + strlen(m_vvPatternParts[iPat][iPart].c_str()), vMatches);
            }
            break;

        case '\0':
            p = strstr(pString, m_vvPatternParts[iPat][iPart].c_str());
            if (p == pString) {
                //  m_pvCurMatches->push_back(""); 
                bOK = subMatch(iPat, iPart+1, p + strlen(m_vvPatternParts[iPat][iPart].c_str()), vMatches);
            }
            break;
        }
    } else {
        if (*pString == '\0') {
            bOK = (m_vvPatternModes[iPat][iPart] != '?') && 
                  (m_vvPatternModes[iPat][iPart] != '#') &&
                  (m_vvPatternModes[iPat][iPart] != '&');
            if (bOK) {
                vMatches.push_back(""); 
            }
        } else {
            bOK = false;
            switch (m_vvPatternModes[iPat][iPart]) {
            case '*':
                vMatches.push_back(pString); 
                bOK = true;
                break;
            case '\0':
                bOK = (strlen(pString) == 0);
                vMatches.push_back(""); 
                break;
            case '?':
                if (strlen(pString) == 1) {
                    bOK = true;
                    sprintf(sMatch, "%c", *pString);
                    vMatches.push_back(sMatch); 
                }
                break;
            case '#':
                if (isdigit(*pString)) {
                    const char *p0 = pString;
                    do {
                        pString++;
                    } while (isdigit(*pString));
                    if (*pString == '\0') {
                        bOK = true;
                        strncpy(sMatch, p0, pString-p0);
                        sMatch[pString-p0] = '\0';
                        vMatches.push_back(sMatch); 
                    }
                }
                
                break;
            
            case '&':
                if (isxdigit(*pString)) {
                    const char *p0 = pString;
                    do {
                        pString++;
                    } while (isxdigit(*pString));
                    if (*pString == '\0') {
                        bOK = true;
                        strncpy(sMatch, p0, pString-p0);
                        sMatch[pString-p0] = '\0';
                        vMatches.push_back(sMatch); 
                    }
                }
                
                break;
            }
        }
    }
    return bOK;
}


//------------------------------------------------------------------------------
// matchPattern
//
bool Pattern::matchPattern(int iPat, const char *pString, VEC_STRINGS &vMatches) {
    return  subMatch(iPat, 0, pString, vMatches);
}
