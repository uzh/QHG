/*============================================================================
|  FileList
|
|   A FileList oject is capable of finding all files whose names match a set of 
|   patterns and providing access by means of an iterator (GetNextFile()).
|   Patterns may contain wildcards such as "*" (match any sequence of chars),
|   "#" (match any sequence of digits), and "?" (match a single char).
|   
\===========================================================================*/

#ifndef __FILELIST_H__
#define __FILELIST_H__

#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <map>
#include <string>
#include "types.h"
#include "utils.h"

#include "Pattern.h"


const int DIRLIST_OK  =  0;
const int DIRLIST_ERR = -1;


const bool FL_INCLUDE = true;
const bool FL_EXCLUDE = false;

typedef std::map<std::string, VEC_STRINGS *> MAP_STRING_LIST;


class FileList {
public:
    /** ***************************************************************************\
    *   \fn     FileList();
    *   \brief  constructor
    *
    *   The constructor initializes the membervariables.
    *** ***************************************************************************/
    FileList();
   ~FileList();

    /*@@@* ***************************************************************************\
    *   \fn     void clearPatterns();
    *   \brief  clear all patterns
    *
    *   Clear all patterns from the pattern list, but keep found files
    *   (useful for "or" searches)
    *** *************************************************************************** /
    void clearPatterns();
    */
    void clearMatches();

    /** ***************************************************************************\
    *   \fn     void addPatterns(char *pFilePattern, bool bInclude);
    *   \brief  add a new pattern
    *
    *   \param  pFilePattern    a pattern string, optionally containing wildcards
    *   \param  bInclude        include/exclude (use FL_INCLUDE, FL_EXCLUDE) 
    *   
    *   If bInclude is true, strings matching the pattern are included in the list;
    *   if false, strings matching pattern are excluded ("and not")
    *** ***************************************************************************/
    void addPattern(const char *pFilePattern, bool bInclude);

    /** ***************************************************************************\
    *   \fn     int initSearch(char *pDir);
    *   \brief  search directory for filenames matching the patterns in list
    *
    *   \param  pDir            Directory to search in 
    *   
    *   \return number of files in list
    *
    *   Find all file in directory whose names match the patterns and include in
    *   list (names who match a pattern with include flag false will not be included)
    *** ***************************************************************************/
    int initSearch(const char *pDir);   

    /** ***************************************************************************\
    *   \fn     int initSearch(char *pDir, char *pFilePattern);
    *   \brief  search directory for filenames matching the given pattern
    *
    *   \param  pDir            Directory to search in 
    *   \param  pFilePattern            Directory to search in 
    *   
    *   \return number of files in list
    *   
    *   Find all file in directory whose names match the given )
    *** ***************************************************************************/
    int initSearch(const char *pDir, const char *pFilePattern);   
    
    /** ***************************************************************************\
    *   \fn     int extendSearch(char *pDir);
    *   \brief  search directory for filenames matching the patterns in list
    *
    *   \param  pDir            Directory to search in 
    *   
    *   \return number of files in list
    *   
    *   Find all file in directory whose names match the patterns and include in
    *   list (names who match a pattern with include flag false will not be included)
    *   (same as InitSearch)
    *** ***************************************************************************/
    int extendSearch(const char *pDir);   

    
    /** ***************************************************************************\
    *   \fn     const char *getNextFile(void);
    *   \brief  Iterator function, gets next file name in list
    *
    *   \return  next file name in list, or null if end of list
    *   
    *   Moves the internal iterator to next position in list of file names before 
    *   returning file name
    *** ***************************************************************************/
    const char *getNextFile(void);

    /** ***************************************************************************\
    *   \fn     const char *getCurFile(void);
    *   \brief  Iterator function, gets current file name in list
    *
    *   \return  current file name in list, or null if end of list
    *   
    *   Does not move the internal iterator to next position in list of file names
    *   before returning file name. Used after GetNextFile(), this will return the
    *   the same string.
    *** ***************************************************************************/
    const char *getCurFile(void);

    /** ***************************************************************************\
    *   \fn     void resetIndex(void);
    *   \brief  resets file index to 0 (keeps file list)
    *
    *   Use this to get the first file in the list with the next call of
    *   getNextFile()
    *** ***************************************************************************/
    void resetIndex(void) {m_iCurIndex = 0;};

    /** ***************************************************************************\
    *   \fn     void reset(void);
    *   \brief  Clear all patterns and filename list
    *
    *   Use this to start a completely new search
    *** ***************************************************************************/
    void reset(void);

    /** ***************************************************************************\
    *   \fn     int getNumFiles(void);
    *   \brief  Get number of file names in list
    *
    *   \return number of file names in list
    *** ***************************************************************************/
    uint getNumFiles(void) const {return (uint)m_vData.size(); };

    /** ***************************************************************************\
    *   \fn     VEC_STRINGS getMatches(void);
    *   \brief  Get vector of matches
    *
    *   \return number of file names in list
    *   
    *   A vector of string where the i-th entry is the match for the i-th 
    *   wildcard symbol in the pattern.
    *** ***************************************************************************/
    VEC_STRINGS *getCurMatches(void);

    /** ***************************************************************************\
    *   \fn     int countWildcards(char *pString);
    *   \brief  Count wild card characters in pString
    *
    *   \return number of wildcard characters in pString
    *** ***************************************************************************/
    static int countWildcards(char *pString);

    /** ***************************************************************************\
    *   \fn     int replaceRefs(VEC_STRINGS vs, char *pOut, char *pOutR)
    *   \brief  replace place holders in pOut
    *
    *   \return if pOut contains place holders @0, ..@9, then
    *           each place holder will be replaced by the corresponding string in
    *           the vector vs.
    *** ***************************************************************************/
    static int replaceRefs(VEC_INTS &vOrder, VEC_STRINGS &vs, const char *pOut, char *pOutR);

    /** ***************************************************************************\
    *   \fn     int replaceRefsSimple(VEC_STRINGS vs, char *pOut, char *pOutR)
    *   \brief  replace wildcards in pOut
    *
    *   \return pOutR will be the result of replacing wildcard #n in pOut 
    *           with string #n in vs. If pOut has more wildcards than vs has strings
    *           pOutR will still contain wildcards.
    *** ***************************************************************************/
    static int replaceRefsSimple(VEC_STRINGS &vs, const char *pOut, char *pOutR);

protected:
    bool  m_bSearching;


    /** ***************************************************************************\
    *   \fn     char *findNextFile(void);
    *   \brief  find next file matching any of the patterns in the list
    *
    *   \return  next file name matching the pattern
    *   
    *   File names matching an "exclude" pattern are NOT returned
    *** ***************************************************************************/
    char *findNextFile(void);

 
    /** ***************************************************************************\
    *   \fn     int matchPattern(unsigned int iPat, char *pString);
    *   \brief  match pattern to given string
    *
    *   \param  iPat           index of pattern part to match with 
    *   \param  pString        string to match with 
    *   
    *   \return  true if match
    *
    *   Start of recursion: Calls SubMatch() with iPart = 0
    *** ***************************************************************************/
    int   matchPattern(int iPat, char *pString);
    

    char  m_sCurFile[MAX_PATH];
  
    VEC_BOOLS         m_vvPatternIncludes;
    
    Pattern *m_pPat;
    DIR              *m_hDir;
    struct dirent    *m_FileData;

    VEC_STRINGS      m_vData;
    MAP_STRING_LIST  m_mvMatches;
    VEC_STRINGS     *m_pvCurMatches;
    unsigned int     m_iCurIndex;
};

#endif

