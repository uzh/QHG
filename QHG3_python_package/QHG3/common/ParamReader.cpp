#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <stdarg.h>
#include <string.h>
#include "types.h"
#include "utils.h"
#include "ParamReader.h"

/*****************************************************************************\
 * OptionInfo
\*****************************************************************************/
class OptionInfo {
public:
    OptionInfo(std::string sOptionDef, void *pVariable);
    ~OptionInfo();
    const std::string getName() { return m_sName;};
    bool requiresArgument() { return m_bRequiresArgument;};
    bool isMandatory() { return m_bMandatory;};
    bool isCorrect() { return m_bCorrect;};
    bool isSet() { return m_bSet;};
    bool isLong() { return m_bLongOption;};
    char getType() { return m_cType;};
    bool setVal(const char *pNewVal);
    const std::string getVal() { return m_sValue;};
private:
    std::string m_sName;
    std::string m_sValue;
    void       *m_pVariable;
    char        m_cType;
    bool        m_bCorrect;
    bool        m_bMandatory;
    bool        m_bRequiresArgument;
    bool        m_bSet;
    bool        m_bLongOption;
    char       *m_pTemp;
};

//-----------------------------------------------------------------------------
// constructor
//
OptionInfo::OptionInfo(std::string sOptionDef, void *pVariable)
    :   m_pVariable(pVariable),
        m_cType('\0'),
        m_bCorrect(false),
        m_bMandatory(false),
        m_bRequiresArgument(true),
        m_bSet(false),
        m_bLongOption(false),
        m_pTemp(NULL) {
    
    std::string::size_type pos = sOptionDef.find(":",0);
    if (pos != std::string::npos) {
        m_sName = sOptionDef.substr(0, pos);
        std::string sRest = sOptionDef.substr(pos+1);
        switch (sRest[0]) {
        case 'c':
        case 's':
        case 'S':
        case 'h':
        case 'i':
        case 'l':
        case 'f':
        case 'd':
        case 'b':
            m_cType = sRest[0];
            m_bCorrect = true;
            m_bRequiresArgument = true;
            break;
        case '0':
            m_cType = 'b';
            m_bCorrect = true;
            m_bRequiresArgument = false;
            break;
        default:
            m_bCorrect = false;
            break;
        }

        pos = m_sName.find("--", 0);
        if (pos != std::string::npos) {
            m_bLongOption = true;
        }

        pos = sRest.find("!");
        if (pos != std::string::npos) {
            m_bMandatory = true;
        }
    } else {
        m_bCorrect = false;
    }
}

//-----------------------------------------------------------------------------
// destructor
//
OptionInfo::~OptionInfo() {
    if (m_pTemp != NULL) {
        delete[] m_pTemp;
    }
}

//-----------------------------------------------------------------------------
// setVal
//
bool OptionInfo::setVal(const char *pNewVal) { 
    m_bSet = true;
    char *pEnd;
    if (pNewVal != NULL) {
        m_sValue = pNewVal;
    }
    switch (m_cType) {
    case 'c' : {
        char *pChar = (char *) m_pVariable;
        *pChar = *pNewVal;
        break;
    }
    case 's' : {
        char *pString = (char *) m_pVariable;
        strcpy(pString, pNewVal);
        break;
    }
    case 'S' : {
        char **ppString = (char **) m_pVariable;
        m_pTemp = new char[strlen(pNewVal)+1];
        strcpy(m_pTemp, pNewVal);
        *ppString = m_pTemp;
        break;
    }
    case 'h' : {
        short int *pSInt = (short int *) m_pVariable;
        short int iDummy = (short int) strtol(pNewVal, &pEnd, 10);
        if (*pEnd =='\0') {
            *pSInt = iDummy;
        } else {
            m_bSet = false;
        }
        break;
    }
    case 'i' : {
        int *pInt = (int *) m_pVariable;
        int iDummy = (int) strtol(pNewVal, &pEnd, 10);
        if (*pEnd =='\0') {
            *pInt = iDummy;
        } else {
            m_bSet = false;
        }

        break;
    }
    case 'l' : {
        long *pLong = (long *) m_pVariable;
        long lDummy = strtol(pNewVal, &pEnd, 10);
        if (*pEnd =='\0') {
            *pLong = lDummy;
        } else {
            m_bSet = false;
        }
        break;
    }
    case 'd' : {
        double *pDouble = (double *) m_pVariable;
        double dDummy = strtod(pNewVal, &pEnd);
        if (*pEnd =='\0') {
            *pDouble = dDummy;
        } else {
            m_bSet = false;
        }
        break;
    }  
    case 'f' : {
        float *pFloat = (float *) m_pVariable;
        float fDummy = (float) strtod(pNewVal, &pEnd);
        if (*pEnd =='\0') {
            *pFloat = fDummy;
        } else {
            m_bSet = false;
        }
        *pFloat = (float) atof(pNewVal);
        break;
    }  
    case 'b' : {
        bool *pBool = (bool *) m_pVariable;
        if (m_bRequiresArgument) {
            *pBool =  ((strcasecmp(pNewVal, "yes") == 0) ||
                       (strcasecmp(pNewVal, "on") == 0) ||
                       (strcasecmp(pNewVal, "true") == 0) ||
                       (strcasecmp(pNewVal, "1") == 0));
        } else {
            *pBool = true;
        } 
      break;
    }  
    default:
        m_bSet = false;
        break; 
    }
    return m_bSet;
}

typedef std::map<std::string, OptionInfo *> OPTIONLIST;

/*****************************************************************************\
 * OptionList
\*****************************************************************************/
class OptionList {
public:
    OptionList();
   ~OptionList();
   
    bool addOption(std::string sDef, void *pVar);
    OptionInfo *getOptionInfo(std::string sOption); 
    bool allMandatorySet(std::vector<std::string> &vsMissing);
    bool optionOK(std::string &sOption);
    bool optionSet();
    bool requiresArgument();
    bool setValue(const char *sValue);
    bool writeSetOptions(FILE *fOut, bool bLines, char *pOmit);
    void collectOptions(std::vector<std::string> &vsOptions);
    uint numOptions() { return (uint)m_MapOptions.size();};
    void getMandatoryList(std::vector<std::string> &vMand);
    void getUnknownList(std::vector<std::string> &vUnknown);
private:
    OPTIONLIST m_MapOptions;
    OptionInfo *m_pCurOption;  
};

//-----------------------------------------------------------------------------
// constructor
//
OptionList::OptionList()
    :   m_pCurOption(NULL) {
}

//-----------------------------------------------------------------------------
// destructor
//
OptionList::~OptionList() {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {

        delete iter->second;
    }
}

//-----------------------------------------------------------------------------
// addOption
//
bool OptionList::addOption(std::string sDef, void *pVar) {
    bool bOK = false;
    OptionInfo *pOpt = new OptionInfo(sDef, pVar);
    if (pOpt->isCorrect()) {
        m_MapOptions[pOpt->getName()] = pOpt;
        bOK = true;
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// getOptionInfo
//
OptionInfo *OptionList::getOptionInfo(std::string sOption) {
    return m_MapOptions[sOption];
}

//-----------------------------------------------------------------------------
// setValue
//
bool OptionList::setValue(const char *pValue) {
    bool bOK = false;
    if (m_pCurOption != NULL) {
        bOK = m_pCurOption->setVal(pValue);
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// optionOK
//
bool OptionList::optionOK(std::string &sOption) {
    m_pCurOption = m_MapOptions[sOption];
    return m_pCurOption != NULL;
}

//-----------------------------------------------------------------------------
// optionSet
//
bool OptionList::optionSet() {
    bool bSet = false;
    if (m_pCurOption != NULL) {
        bSet = m_pCurOption->isSet();
    }
    return bSet;
}

//-----------------------------------------------------------------------------
// requiresArgument
//
bool OptionList::requiresArgument() {
    bool bRequires = false;
    if (m_pCurOption != NULL) {
        bRequires = m_pCurOption->requiresArgument();
    }
    return bRequires;
}

//-----------------------------------------------------------------------------
// allMandatorySet
//
bool OptionList::allMandatorySet(std::vector<std::string> &vsMissing) {
    bool bAllSet = true;
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
         
        if (iter->second != NULL) {
            if (iter->second->isMandatory() && !iter->second->isSet()) {
                vsMissing.push_back(iter->first);
                bAllSet = false;
            }
        }
    }
    return bAllSet;
}

//-----------------------------------------------------------------------------
// getMandatoryList
//
void OptionList::getMandatoryList(std::vector<std::string> &vMand) {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
        if (iter->second != NULL) {
            if (iter->second->isMandatory()) {
                vMand.push_back(iter->second->getName());
            }
        }
    }
}

//-----------------------------------------------------------------------------
// getUnknownList
//
void OptionList::getUnknownList(std::vector<std::string> &vUnknown) {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
        if (iter->second == NULL) {
            vUnknown.push_back(iter->first);
        }
    }
}

//-----------------------------------------------------------------------------
// writeSetOptions
//   bLines: write each option on a single line
//   pOmit:  comma-separated list of options to omit
//   If bLines is true, omitted options are written as comments
// 
bool OptionList::writeSetOptions(FILE *fOut, bool bLines, char *pOmit) {
    bool bOK = true;
    
    // create a map with options to omit
    std::map<std::string, bool> mOmit;
    if (pOmit != NULL) {
        char *pCtx;
        char *p = strtok_r(pOmit, " ,:;", &pCtx);
        while (p != NULL) {
            mOmit[p] = true;
            p = strtok_r(NULL, " ,::;", &pCtx);
        }
    }


    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
 
        
        if (!mOmit[iter->first] || bLines) {
            if (iter->second != NULL) {
                if (iter->second->isSet()) {
                    // if omitted in lines mode: comment
                    if (mOmit[iter->first]) {
                        fprintf(fOut, "#");
                    }
                    if (bLines) {
                        fprintf(fOut, "  ");
                    }
                    std::string sValue = iter->second->getVal();  
                    fprintf(fOut, "%s", iter->first.c_str());
                    if (iter->second->isLong()) {
                        if ((iter->second->getType() != '0') ||
                            (iter->second->requiresArgument())) {
                            fprintf(fOut, "=%s ", sValue.c_str()); fflush(fOut);
                        }
                    } else {
                        if ((iter->second->getType() != '0') ||
                            (iter->second->requiresArgument())) {
                            fprintf(fOut, " %s ", sValue.c_str()); fflush(fOut);
                        }    
                    }
                    if (bLines) {
                        fprintf(fOut, "\n");
                    }
                }
            }
        }        
    }   
    fprintf(fOut, "\n"); fflush(fOut);  
    return bOK;
}

//-----------------------------------------------------------------------------
// collectOptions
//
void OptionList::collectOptions(std::vector<std::string> &vsOptions) {
    OPTIONLIST::iterator iter;
    for (iter = m_MapOptions.begin();
         iter != m_MapOptions.end();
         ++iter) {
 
        if (iter->second != NULL) {
            if (iter->second->isSet()) {
                std::string sLine=iter->first;
                std::string sValue = iter->second->getVal();  
                if (iter->second->isLong()) {
                    if ((iter->second->getType() != '0') ||
                        (iter->second->requiresArgument())) {
                        sLine += "=" + sValue;
                    }
                } else {
                    if ((iter->second->getType() != '0') ||
                        (iter->second->requiresArgument())) {
                        sLine += " " + sValue;
                    }    
                }
                vsOptions.push_back(sLine);
            }
        }        
    }   
}



//-----------------------------------------------------------------------------
// constructor
//
ParamReader::ParamReader() 
    :   m_pOptionList(NULL),
        m_bVerbose(false),
        m_sBadArg(""),
        m_sBadVal("") {
}

//-----------------------------------------------------------------------------
// destructor
//
ParamReader::~ParamReader() {
    if (m_pOptionList != NULL) {
        delete m_pOptionList;
    }
}

//-----------------------------------------------------------------------------
// setOptions
//
bool ParamReader::setOptions(int iNumOptions, ...) {
    bool bOK = false;
    std::string sOptionString;
    void *pOptionVar;
    va_list vl;
  
    va_start(vl, iNumOptions);

    if (m_pOptionList != NULL) {
        delete m_pOptionList;
    }
    m_pOptionList = new OptionList();
    if (m_pOptionList != NULL) {
        bOK = true;
        for (int i = 0; bOK && (i < iNumOptions); i++) {
            sOptionString = va_arg(vl, char *);
            pOptionVar    = va_arg(vl, void *);
            if (m_pOptionList->addOption(sOptionString, pOptionVar)) {
            } else {
                bOK = false;
                printf("%p : Error at NewOption(%s, pOptionVar)\n", this,  sOptionString.c_str());
                break;
            }
        }
    }
    va_end(vl);
    return bOK;
}


//-----------------------------------------------------------------------------
// getParams
//
int ParamReader::getParams(int argc, char *argv[], bool bOverwrite) {
    int iResult = PARAMREADER_OK;

    for (int i = 1; (iResult >= 0) && (i < argc); i++) {

        bool bValPresent = false;
        std::string sVal;
        std::string sArg = argv[i];
        
        if (sArg[0] == '-') {
            std::string::size_type pos = sArg.find("=", 0);
            if (pos != std::string::npos) {
                sVal = sArg.substr(pos+1);
                sArg = sArg.substr(0, pos);
                bValPresent = true;
            }

            if (m_pOptionList->optionOK(sArg)) {
                if (!m_pOptionList->optionSet() || bOverwrite) {
                    bool bVarSet = false;
                    if (m_pOptionList->requiresArgument()) {
                        if (sArg[1] == '-') {
                            if (!bValPresent) {
                                if (m_bVerbose) {
                                    printf("Missing parameter for option %s\n", sArg.c_str());
                                }
                                iResult = PARAMREADER_ERR_MISSING_PARAM;
                                m_sBadArg = sArg;
                                m_sBadVal = "";
                            }
                        } else {
                            if (++i < argc) {
                                 sVal = argv[i];
                            } else {
                                if (m_bVerbose) {
                                    printf("Missing parameter for option %s\n", sArg.c_str());
                                }
                                iResult = PARAMREADER_ERR_MISSING_PARAM;
                                m_sBadArg = sArg;
                                m_sBadVal = "";

                            }
                        }
                        bVarSet = m_pOptionList->setValue(sVal.c_str());
                    } else  {
                        bVarSet = m_pOptionList->setValue(NULL);
                    }
                    if (!bVarSet) {
                        iResult = PARAMREADER_ERR_OPTION_SET;
                        m_sBadArg = sArg;
                        m_sBadVal = sVal;
                        if (m_bVerbose) {
                            printf("Error setting option %s to %s\n", sArg.c_str(), sVal.c_str());
                        }
                    }
                }
            } else {
                // unknown option
                if (m_bVerbose) {
                    printf("Unknown Option %s\n", sArg.c_str());
                }
                iResult |= PARAMREADER_ERR_UNKNOWN_OPTION;
                m_sBadArg = sArg;
                m_sBadVal = sVal;
            }

        } else {
            if (m_bVerbose) {
                printf("expected '-' (instead of %s)\n", sArg.c_str());
            }
            m_vFreeParams.push_back(sArg.c_str());
            iResult |= PARAMREADER_ERR_FREE_PARAMS;
        }
    }
  

    if (iResult >= 0) {
        if (!m_pOptionList->allMandatorySet(m_vMissingManadatory)) {
            if (m_bVerbose) {
                printf("Not all mandatory arguments set\n");
            }
            iResult = PARAMREADER_ERR_MANDATORY_MISSING;
        }
    }
  
  
    return iResult;
}
/*
//-----------------------------------------------------------------------------
// getParams
//
int ParamReader::getParams(char *pConfigFile) {
    int iResult = 0;
    char sLine[MAX_LINE];
    char sLine1[MAX_LINE];
 
    FILE *fIn = fopen(pConfigFile, "rt");
    if (fIn != NULL) {
        // count words in file
        int iNum = 0;
        char *p1 = fgets(sLine1, MAX_LINE, fIn);
        strcpy(sLine, sLine1);
 
        char *p11 = strtok(p1, " \t");
        while (p11 != NULL) {
            ++iNum;
            p11 = strtok(NULL, " \t");
        }

        char **ppArgs = new CHARP[iNum+1];
        if (ppArgs != NULL) {
            int iCount = 0;
            ppArgs[iCount++] = NULL;
            char *p = sLine;
            if (p != NULL) {
                char *pBuf;
                char *p0 = strtok_r(sLine, " \t", &pBuf);
                while (p0 != NULL) {
                    ppArgs[iCount++] = p0;
                    p0 = strtok_r(NULL, " \t", &pBuf);
                }
                iResult = getParams(iCount, ppArgs);
            } else {
                iResult = -1;
            }
            fclose(fIn);
        }
        delete[] ppArgs;
    } else {
        iResult = -1;
    }
    
    return iResult;
}
*/

//-----------------------------------------------------------------------------
// countWords
//
int ParamReader::countWords(char *pConfigFile) {
    int iNumWords = 0;
    char sLine[MAX_LINE];
 
    FILE *fIn = fopen(pConfigFile, "rt");
    if (fIn != NULL) {
        while (!feof(fIn)) {
            // count words in file
            char *p = fgets(sLine, MAX_LINE, fIn);
            if (p != NULL) {
                if (*p != '#') {
                    char *p11 = strtok(p, " \t\n");
                    while (p11 != NULL) {
                        ++iNumWords;
                        p11 = strtok(NULL, " \t\n");
                    }
                }
            }
        }
        fclose(fIn);
    }
    return iNumWords;
}

//-----------------------------------------------------------------------------
// getParams
//
int ParamReader::getParams(char *pConfigFile, bool bOverwrite) {
    int iResult = 0;
    char sLine[MAX_LINE];
    int iNumWords = countWords(pConfigFile);

    char **ppArgs = new char*[iNumWords+1];  // first word: appName
    if (ppArgs != NULL) {
        int iCount = 0;
        ppArgs[iCount++] = NULL;
 
        FILE *fIn = fopen(pConfigFile, "rt");
        if (fIn != NULL) {
            while (!feof(fIn)) {
                // count words in file
                char *p = fgets(sLine, MAX_LINE, fIn);
                if (p != NULL) {
                    if (*p != '#') {
                        char *pBuf;
                        char *p0 = strtok_r(p, " \t\n", &pBuf);
                        while (p0 != NULL) {
                            ppArgs[iCount++] = strdup(p0);
                            p0 = strtok_r(NULL, " \t\n", &pBuf);
                        }
                    }
                }
            }
            fclose(fIn);
           
            iResult = getParams(iCount, ppArgs, bOverwrite);

        } else {
            iResult = PARAMREADER_ERR_BAD_CONFIG_FILE;
            m_sBadVal = pConfigFile;
        }
        // free stuff
        for (int i = 0; i < iNumWords; ++i) {
            free(ppArgs[i+1]);
        }
        delete[] ppArgs;
    } else {
        iResult = -1;
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// getMandatoryParams
//
uint ParamReader::getMandatoryParams(std::vector<std::string> &vMand) {
    m_pOptionList->getMandatoryList(vMand);
    return (uint)vMand.size();
}

//-----------------------------------------------------------------------------
// getUnknownParams
//
uint ParamReader::getUnknownParams(std::vector<std::string> &vUnknown) {
    m_pOptionList->getUnknownList(vUnknown);
    return (uint)vUnknown.size();
}

//-----------------------------------------------------------------------------
// getFreeParams
//
uint ParamReader::getFreeParams(std::vector<std::string> &vFree) {
    vFree.clear();
    vFree.insert(vFree.end(), m_vFreeParams.begin(), m_vFreeParams.end());
    return (uint)vFree.size();
}

//-----------------------------------------------------------------------------
// writeConfigFile
//   pOmit:  comma-separated list of options to omit
//
bool ParamReader::writeConfigFile(const char *pConfigFile, const char *pOmit) {
    bool bOK = false;
    
    char *pOmit2 = strdup(pOmit);
    FILE * fOut = fopen(pConfigFile, "wt");
    if (fOut != NULL) {
        bOK = m_pOptionList->writeSetOptions(fOut, true, pOmit2);
        fclose(fOut);
    }
    free(pOmit2);
    return bOK;
}

//-----------------------------------------------------------------------------
// collectOptions
//
void ParamReader::collectOptions(std::vector<std::string> &vsOptions) {
    m_pOptionList->collectOptions(vsOptions);
}

//-----------------------------------------------------------------------------
// display
//
void ParamReader::display() {
    m_pOptionList->writeSetOptions(stdout, true, NULL);
}

//-----------------------------------------------------------------------------
// display
//
void ParamReader::display(FILE *fOut, bool bLines) {
    m_pOptionList->writeSetOptions(fOut, bLines, NULL);
}

//-----------------------------------------------------------------------------
// getErrorMessage
//
std::string ParamReader::getErrorMessage(int iResult) {
    int iNum = 0;
    char sNum[128];
    std::vector<std::string> vVals;
    m_sErrorMessage = "";

    if (iResult < 0) {
        
        switch (iResult) {
        case  PARAMREADER_ERR_MANDATORY_MISSING:
            if (m_sErrorMessage != "") {
                m_sErrorMessage += "\n";
            }
            iNum = m_vMissingManadatory.size();
            sprintf(sNum, "[ParamReader Error] %d mandatory option%s missing: ", iNum, (iNum != 1)?"s":"");
            m_sErrorMessage += sNum;
            for (int i = 0; i < iNum; ++i) {
                m_sErrorMessage += " "+m_vMissingManadatory[i];
            }

            break;
        case PARAMREADER_ERR_MISSING_PARAM:
            m_sErrorMessage = "[ParamReader Error] option parameter missing: " +m_sBadArg;
            break;
        case PARAMREADER_ERR_OPTION_SET:
            m_sErrorMessage = "[ParamReader Error] bad option value: " + m_sBadArg +" ("+m_sBadVal +")";
            break;
        case PARAMREADER_ERR_BAD_CONFIG_FILE:
            m_sErrorMessage = "[ParamReader Error] config file doesn't exist: " + m_sBadVal;
            break;
        default:
            sprintf(sNum, "[ParamReader Error] Unknown error (%d)", iResult);
            m_sErrorMessage = sNum;
        }
    } else {
        if (iResult == PARAMREADER_OK) {
            m_sErrorMessage = "[ParamReader OK]";
        } else {
            if ((iResult & PARAMREADER_ERR_UNKNOWN_OPTION) != 0) {
                if (m_sErrorMessage != "") {
                    m_sErrorMessage += "\n";
                }
                iNum =  getUnknownParams(vVals);
                sprintf(sNum, "[ParamReader Warning] %d unknown option%s: ", iNum, (iNum != 1)?"s":"");
                m_sErrorMessage += sNum;
                for (int i = 0; i < iNum; ++i) {
                    m_sErrorMessage = m_sErrorMessage + " "+vVals[i];
                }
            }
            if ((iResult & PARAMREADER_ERR_FREE_PARAMS) != 0) {
                if (m_sErrorMessage != "") {
                    m_sErrorMessage += "\n";
                }
                iNum =  getFreeParams(vVals);
                sprintf(sNum, "[ParamReader Warning] %d free param%s: ", iNum, (iNum != 1)?"s":"");
                m_sErrorMessage += sNum;
                for (int i = 0; i < iNum; ++i) {
                    m_sErrorMessage += " "+vVals[i];
                }
            }
        }
        if (m_sErrorMessage == "") { 
            sprintf(sNum, "[ParamReader Warnubg] Unknown warning (%d)", iResult);
            m_sErrorMessage = sNum;
        }
    }
    return m_sErrorMessage;
}



 
/* 
int main_test(int iArgC,char *apArgV[]) {
    int i;
    float f;
    bool b1=false;
    bool b2=false;
    char sDada[100];
/ *    
    OptionList *pOL = new OptionList();
    pOL->addOption("-a:i", &i);
    pOL->addOption("--asdasd:s", sDada);
    pOL->addOption("--fgh:f!", &f);
    pOL->addOption("--set-on:b", &b1);
    pOL->addOption("-h:0", &b2);

    pOL->setValue("--fgh", "3.7");
    pOL->setValue("-h", "3.7");
    pOL->setValue("--set-on", "false");
    pOL->setValue("--asdasd", "zutrew");
    pOL->setValue("-a", "4");
* /
    if (GetParamsExt(iArgC, apArgV, 5, 
                                    "-a:i", &i,
                                    "--aaa:i", &i,
                                    "--asdasd:s", sDada,
                                    "--fgh:f!", &f,
                                    "--set-on:b", &b1,
                                    "-h:0", &b2) >= 0) {
                                        
                                            
        printf("i : %d\n", i);
        printf("f : %f\n", f);
        printf("b1 : %d\n", b1);
        printf("b2 : %d\n", b2);
        printf("sDada : %s\n", sDada);
    } else {
        printf("babababa\n");
        
    }
}
*/
