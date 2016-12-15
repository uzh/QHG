/* ****************************************************************************
|
| Extended General Purpose GetParams function
|
| <PlatformInfo>
|
|
| Module      :
|
| Project     :
|
| Description : The Function getParams has a variable Argument List which
|               looks like this:
|                   int argc, char *argv, int iNumOpts, 
|                   const char *pOptionDef0, void *pOptVariable0,
|                   const char *pOptionDef1, void *pOptVariable1,
|                                      :
|                   const char *pOptionDefN, void *pOptVariableN
|
|               The pOptionDef must have the following Form : 
|                   "-" <OptionLetter> ":" <OptionVariableType>]["!"][0]
|               or
|                   "--" <OptionString> ":" <OptionVariableType>]["!"][0]
|               where OptionVariableType is one of the following
|                   "c"  character
|                   "s"  char *
|                   "S"  char **    
|                   "h"  short
|                   "i"  integer
|                   "l"  long
|                   "f"  float
|                   "d"  double
|                   "b"  bool
|
|               The "!" means that the option is mandatory
|               The "0" means that the option needs no parameters. This 
|               implies that the corresponding variable must be boolean.
|
|               An 's' option must be followed by a char* pointing to an 
|               existing char array. It is filled by strcpy (not safe!).
|
|               An 'S' option  must be followed by a char** pointing to a 
|               char * variable. ParamReader allocates a string of correct
|               size and assigns it to the variable. This string will be
|               deleted in ParamReader's destructor.
|
|               GetParams returns true iff all the mandatory options were found
|
|               A subsequent call to getParams will not overwrite previously
|               set options, unless the bOverwrite param is set to true.
|               This feature is useful to get the name of a config file in 
|               a first call and then use the config file in a second call
|
|-------------------------------------------------------------------------------
| Created     : Mar 24
|
|
| Modified    :  Who    |  When    |  What
|               --------+----------+-------------------
|                ...    |  ...     |  ...
|                       |          |
|                       |          |
|                       |          |
|
|
\******************************************************************************/
#ifndef __PARAM_READER_H__
#define __PARAM_READER_H__

#include <stdio.h>
#include <vector>
#include <string>

#include "types.h"
class OptionList;

const int PARAMREADER_OK                    =  0;
const int PARAMREADER_ERR_MANDATORY_MISSING = -1;
const int PARAMREADER_ERR_MISSING_PARAM     = -2;
const int PARAMREADER_ERR_OPTION_SET        = -3;
const int PARAMREADER_ERR_BAD_CONFIG_FILE   = -4;
const int PARAMREADER_ERR_UNKNOWN_OPTION    =  1;
const int PARAMREADER_ERR_FREE_PARAMS       =  2;

const int STRICT_NONE = 0; // 
class ParamReader {
public:
    ParamReader();
   ~ParamReader();
   
    bool setOptions(int iNumOptions, ...);
    void setVerbose(bool bVerbose) {m_bVerbose = bVerbose;};
    
    int  getParams(int iArgC, char *apArgV[], bool bOverwrite=false);
    int  getParams(char *pConfigFile, bool bOverwrite=false);
    
    uint getMandatoryParams(std::vector<std::string> &vMand);
    uint getUnknownParams(std::vector<std::string> &vUnknown);
    uint getFreeParams(std::vector<std::string> &vFree);
    bool writeConfigFile(const char *pConfigFile, const char *pOmit);
    void collectOptions(std::vector<std::string> &vsOptions);

    void display();
    void display(FILE *fOut, bool bLines);

    std::string getErrorMessage(int iResult);
    std::string getBadArg() { return m_sBadArg;};
    std::string getBadVal() { return m_sBadVal;};
protected:
    int countWords(char *pConfigFile);
    OptionList *m_pOptionList;    
    bool m_bVerbose;
    std::string m_sBadArg;
    std::string m_sBadVal;
    std::vector<std::string> m_vFreeParams;
    std::string m_sErrorMessage;
    std::vector<std::string> m_vMissingManadatory;
};

#endif
