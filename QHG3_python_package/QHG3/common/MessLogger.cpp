#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>

#include "MessLogger.h"
#include "colors.h"

#define HEADER_STATUS  "S:"
#define HEADER_WARNING "W:"
#define HEADER_ERROR   "E:"
#define HEADER_DISP    "D:"


#define COL_STATUS  GREEN
#define COL_WARNING BLUE
#define COL_ERROR   RED
#define COL_DISP    PURPLE
#define COL_STOP    OFF

#define STDOUT      "(stdout)"

MessLogger *MessLogger::s_pML=NULL;
char        MessLogger::s_sLogName[256];
int         MessLogger::s_iNumMess=0;
char        MessLogger::s_asFirstMessages[NUM_MESS][MAX_MESS];

MessLogger::MessLogger(FILE *fOut) 
    : m_fOut(fOut),
      m_iNumStatus(0),
      m_iNumWarnings(0),
      m_iNumErrors(0) {
}

MessLogger::~MessLogger() {
    if ((m_fOut != NULL) && (m_fOut != stdout)) {
        fclose(m_fOut);
    }
}

//----------------------------------------------------------------------------
// create
//   creates a MessLogger object writing to the specified file.
//   if NULL is passed for pName, output goes to stdout
//   
//   should be called somewhere at the start of the program
//
MessLogger *MessLogger::create(const char *pName) {
    if (s_pML == NULL) {
        FILE *fOut = NULL;
        if (pName != NULL) {
            fOut = fopen(pName, "wt");
        } else {
            fOut = stdout;
        }
        if (fOut != NULL) {
            strcpy(s_sLogName, (pName==NULL)?STDOUT:pName);
            printf("Log name: [%s]\n", s_sLogName);
            s_pML = new MessLogger((FILE *)fOut);
        } else {
            printf("£$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ couldn't open [%s]\n", (pName==NULL)?"stdout":pName);
        }
    }
    return s_pML;
}

//----------------------------------------------------------------------------
// free
//  deletes the MessLogger
//
//  should be called somewhere at the end of the program
//
void MessLogger::free() {
    if (s_pML != NULL) {
        delete s_pML;
    }
    s_pML = NULL;
}


//----------------------------------------------------------------------------
// write
//  writees the line to file
//
void MessLogger::write(const char *pLine, const char *pPre, const char *pPost) {
    char sLine[8192];
    *sLine = '\0';

    if (m_fOut == stdout) {
        sprintf(sLine, "%s%s", pPre, pLine);
    } else {
        strcpy(sLine, pLine);
    }

    if (sLine[strlen(sLine)-1] != '\n') {
        strcat(sLine, "\n");
    }

    // colors for stdout
    if (m_fOut == stdout) {
        strcat(sLine, pPost);
    }
    
    if (s_iNumMess < NUM_MESS) {
        strcpy(s_asFirstMessages[s_iNumMess], pLine);
        if (s_asFirstMessages[s_iNumMess][strlen(s_asFirstMessages[s_iNumMess])-1] == '\n') {
            s_asFirstMessages[s_iNumMess][strlen(s_asFirstMessages[s_iNumMess])-1] = '\0';
        }
        s_iNumMess++;
   }

    fputs(sLine, m_fOut);
    fflush(m_fOut);
}

//----------------------------------------------------------------------------
// mprintf
//  writes message in printf style
//
void MessLogger::mprintf(const char * pFormat, ...) {
#ifndef MUTE
    va_list vl;
  
    va_start(vl, pFormat);

    char sLine[8192];
    vsprintf(sLine, pFormat, vl);
    va_end(vl);

    if (s_pML != NULL) {
        s_pML->write(sLine, "", "");
    }
#endif
}

//----------------------------------------------------------------------------
// logStatus
//  writes status message in printf style
//
void MessLogger::logStatus(const char * pFormat, ...) {
#ifndef MUTE
    va_list vl;
    va_start(vl, pFormat);
    MessLogger::logSpecial(HEADER_STATUS, COL_STATUS, pFormat, vl);
    va_end(vl);
#endif
}

//----------------------------------------------------------------------------
// logWarning
//  writes warning message in printf style
//
void MessLogger::logWarning(const char * pFormat, ...) {
#ifndef MUTE
    va_list vl;
    va_start(vl, pFormat);
    MessLogger::logSpecial(HEADER_WARNING, COL_WARNING, pFormat, vl);
    va_end(vl);
#endif
}


//----------------------------------------------------------------------------
// logError
//  writes error message in printf style
//
void MessLogger::logError(const char * pFormat, ...) {
#ifndef MUTE
    va_list vl;
    va_start(vl, pFormat);
    MessLogger::logSpecial(HEADER_ERROR, COL_ERROR, pFormat, vl);
    va_end(vl);
#endif
}

//----------------------------------------------------------------------------
// logDisp
//  writes specially colored liner in printf style
//
void MessLogger::logDisp(const char * pFormat, ...) {
#ifndef MUTE
    va_list vl;
    va_start(vl, pFormat);
    MessLogger::logSpecial(HEADER_DISP, COL_DISP, pFormat, vl);
    va_end(vl);
#endif
}

//----------------------------------------------------------------------------
// logSpecial
//  
//
void MessLogger::logSpecial(const char *sHeader, const char *sColor, const char * pFormat, va_list vl) {
  #ifndef MUTE
  
   
    char sLine[8192];
    char sLine2[8192];
    *sLine = '\0';


    strcat(sLine, sHeader);

    vsprintf(sLine2, pFormat, vl);
    
    strcat(sLine, sLine2);


    if (s_pML != NULL) {
        s_pML->write(sLine, sColor, COL_STOP);
        s_pML->incNumError();
    }
#endif  
}

//----------------------------------------------------------------------------
// logIgnore
//  does nothing
//
void MessLogger::logIgnore(const char * pFormat, ...) {
    printf("ignore\n");
}


//----------------------------------------------------------------------------
// showLog
//  displays a colored log file
//  use a or-ed combination of SHOW_STATUS, SHOW_WARNING, and SHOW_ERROR,
//  or use SHOW_ALL to see all messages
//
void MessLogger::showLog(int iWhat) {
    showLog(s_sLogName, iWhat);
}

//----------------------------------------------------------------------------
// showLog
//  displays a colored log file
//  use a or-ed combination of SHOW_STATUS, SHOW_WARNING, and SHOW_ERROR,
//  or use SHOW_ALL to see all messages
//
void MessLogger::showLog(const char *pLogFile, int iWhat) {
    printf("\n\n%s------------------------------------------------%s\n", COL_DISP, COL_STOP);
    if (strcmp(pLogFile, STDOUT) != 0) {
        FILE *fIn = fopen(pLogFile, "rt");
        if (fIn != NULL) {
            printf("%sShowing contents of [%s]%s\n\n", COL_DISP, pLogFile, COL_STOP);
            char sLine[1024];
            while (!feof(fIn)) {
                char *p = fgets(sLine, 1024, fIn);
                coloredLine(p, iWhat);
            }
            fclose(fIn);
        } else {
            printf("%s[MessLogger::showLog] couldn't open  [%s] for reading%s\n", COL_ERROR, pLogFile, COL_STOP);
        }
    } else {
        printf("%sShowing contents of Buffer%s\n\n", COL_DISP, COL_STOP);

        for (int i = 0; i < s_iNumMess; i++) {
            coloredLine(s_asFirstMessages[i], iWhat);
        }
    }
    printf("%s------------------------------------------------%s\n\n", COL_DISP, COL_STOP);
}


//----------------------------------------------------------------------------
// coloredLine
//  print a line colored depending on the header (first 2 characters)
//
void  MessLogger::coloredLine(char *pLine, int iWhat) {
    char sPre[64];
    char sPost[64];
    strcpy(sPost, COL_STOP);
   
    bool bShow = (iWhat == SHOW_ALL);
    if (pLine != NULL) {
        if (strncmp(pLine, HEADER_STATUS, strlen(HEADER_STATUS)) == 0) {
            bShow = (iWhat & SHOW_STATUS);
            strcpy(sPre, COL_STATUS);
        } else if (strncmp(pLine, HEADER_WARNING, strlen(HEADER_WARNING)) == 0) {
            bShow = (iWhat & SHOW_WARNING);
            strcpy(sPre, COL_WARNING);
        } else if (strncmp(pLine, HEADER_ERROR, strlen(HEADER_ERROR)) == 0) {
            bShow = (iWhat & SHOW_ERROR);
            strcpy(sPre, COL_ERROR);
        } else if (strncmp(pLine, HEADER_DISP, strlen(HEADER_DISP)) == 0) {
            bShow = (iWhat & SHOW_DISP);
            strcpy(sPre, COL_DISP);
        } else {
            bShow = true;
            strcpy(sPre, BLINK);
        }
        if (bShow) {
            if (pLine[strlen(pLine)-1] == '\n') {
                pLine[strlen(pLine)-1] = '\0';
            }
            printf("%s%s%s\n", sPre, pLine, sPost);
        }
    }
}
