/*============================================================================
| LineReader
| 
|  Baseclass for utilities to read lines from a file
|  - createInstance() will open the file for reading, or NULL on error
|  - getNextLine() returns pointer to next line ni file, or NULL if eof
|  
| LineReader_std
|   Reads from a normal text file
|
| LineReader_gz
|   Reads from a gzipped text file
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __LINEREADER_H__
#define __LINEREADER_H__

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "utils.h"
#include "zlib.h"

const int MAX_LINE2 = 8192;

const int GNL_IGNORE_NONE     =  0; // pass every line
const int GNL_IGNORE_BLANKS   =  1; // drop blank lines
const int GNL_IGNORE_COMMENTS =  2; // drop #-comment lines
const int GNL_IGNORE_MATCOMM   = 4; // drop %-comment lines
const int GNL_TRIM            =  8; // trim all lines
const int GNL_IGNORE_ALL      = 15; // pass non-blank, non-comment lines and trim them



class LineReader {
public:
    
    LineReader(int iLineLen);
    virtual ~LineReader();
    virtual char *getNextLine(int iMode=GNL_IGNORE_ALL)=0;
    virtual uint read(void *pBuf, uint iSize)=0;

    virtual int seek(long offset, int whence)=0;
    virtual long tell()=0;
    virtual bool isEoF()=0;
    virtual bool isReady()=0;
    char *getCurLine() { return m_pLine;};

protected:
    char *m_pLine;
    int   m_iLineLen;
    bool  m_bClose;
};
typedef LineReader *PLR;

class LineReader_std : public LineReader {
public:
    static LineReader *createInstance(const char *pName, const char *pMode, int iLineLen=MAX_LINE2);
    static LineReader *createInstance(FILE *fIn, int iLineLen=MAX_LINE2);
    LineReader_std(const char *pName, const char *pMode, int iLineLen=MAX_LINE2);
    LineReader_std(FILE *fIn, int iLineLen=MAX_LINE2);
    virtual ~LineReader_std();
    virtual char *getNextLine(int iMode=GNL_IGNORE_ALL);
    virtual uint read(void *pBuf, uint iSize);
    virtual int seek(long offset, int whence) {return fseek(m_fIn, offset, whence);};
    virtual long tell() {return ftell(m_fIn);};
    virtual bool isEoF() {return feof(m_fIn);};
    virtual bool isReady();

protected:
    FILE *m_fIn;
};

class LineReader_gz : public LineReader {
public:
    static LineReader *createInstance(const char *pName, const char *pMode, int iLineLen=MAX_LINE2);

    LineReader_gz(const char *pName, const char *pMode, int iLineLen=MAX_LINE2);
    LineReader_gz(gzFile fIn, int iLineLen=MAX_LINE2);
    virtual ~LineReader_gz();
    virtual char *getNextLine(int iMode=GNL_IGNORE_ALL);
    virtual uint read(void *pBuf, uint iSize);
    virtual int seek(long offset, int whence) {return (int)gzseek(m_fIn, offset, whence);};
    virtual long tell() {return gztell(m_fIn);};
    virtual bool isEoF() {return gzeof(m_fIn);};
    virtual bool isReady();

protected:
    gzFile m_fIn;
};

class LineReader_str : public LineReader {
public:
    static LineReader *createInstance(const char *pLines[], unsigned int iNum, int iLineLen=MAX_LINE2);
    static LineReader *createInstance(const char *pLines, int iLineLen=MAX_LINE2);

    LineReader_str(const char *pLines[], unsigned int iNum, int iLineLen=MAX_LINE2);
    LineReader_str(const char *pLines, int iLineLen=MAX_LINE2);
    virtual ~LineReader_str();
    virtual char *getNextLine(int iMode=GNL_IGNORE_ALL);
    virtual uint read(void *pBuf, uint iSize);
    virtual int seek(long offset, int whence);
    virtual long tell() {return m_iCurrentIndex;};
    virtual bool isEoF() {return m_iCurrentIndex >= m_vLines.size();};
    virtual bool isReady() {return m_vLines.size() > 0;};

protected:
    void addLines(const char *pLines);
    std::vector<char *> m_vLines;
    unsigned int  m_iCurrentIndex;
    std::vector<char *> m_vMem;
    
};


#endif
