#ifndef __MESSLOGGER_H__
#define __MESSLOGGER_H__


#include <stdio.h>
#include <stdarg.h>

/*
#ifndef MUTE
#define LOG_STATUS  MessLogger::logIgnore
#define LOG_WARNING MessLogger::logIgnore
#define LOG_ERROR   MessLogger::logIgnore
#else
#define LOG_STATUS  MessLogger::logStatus
#define LOG_WARNING MessLogger::logWarning
#define LOG_ERROR   MessLogger::logError
#endif
*/
#define LOG_STATUS  MessLogger::logStatus
#define LOG_WARNING MessLogger::logWarning
#define LOG_DISP    MessLogger::logDisp
#define LOG_ERROR   MessLogger::logError

#define NUM_MESS    32
#define MAX_MESS    1024
 
#define SHOW_NONE    0
#define SHOW_STATUS  1
#define SHOW_WARNING 2
#define SHOW_ERROR   4
#define SHOW_DISP    8
#define SHOW_ALL     (SHOW_STATUS | SHOW_WARNING | SHOW_ERROR | SHOW_DISP)

class MessLogger {
public:
    
    static MessLogger *create(const char *pName=NULL);
    static void free();
    static void logStatus(const char * pFormat, ...);
    static void logWarning(const char * pFormat, ...);
    static void logError(const char * pFormat, ...);
    static void logDisp(const char * pFormat, ...);
    static void logIgnore(const char * pFormat, ...);
    static void mprintf(const char * pFormat, ...);
    static void showLog(const char *pLogFile, int iWhat);
    static void showLog(int iWhat);
    
    static int getNumStatus()  { return s_pML->m_iNumStatus;};
    static int getNumWarning() { return s_pML->m_iNumWarnings;};
    static int getNumError()   { return s_pML->m_iNumErrors;};

    void incNumStatus()  { m_iNumStatus++;};
    void incNumWarning() { m_iNumWarnings++;};
    void incNumError()   { m_iNumErrors++;};

    static FILE *getOut() { return s_pML->m_fOut;};
    static const char *  getFile() { return s_sLogName;};

protected:
    MessLogger(FILE *fOut);
    ~MessLogger();
    void write(const char *pLine, const char *pPre, const char *pPost);
    static void coloredLine(char *pLine, int iWhat);
    static void logSpecial(const char *sHeader, const char *sColor, const char * pFormat, va_list vl);

    static char s_sLogName[256];
    FILE *m_fOut;

    static MessLogger *s_pML;

    int m_iNumStatus;
    int m_iNumWarnings;
    int m_iNumErrors;

    static int  s_iNumMess;
    static char s_asFirstMessages[NUM_MESS][MAX_MESS];
};


#endif

