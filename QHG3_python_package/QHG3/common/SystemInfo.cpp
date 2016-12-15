#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/times.h>
#include <sys/vtimes.h>
#include "SystemInfo.h"

#include "strutils.h"

/***********************
 * basic functionality from Stackoverflow:
 *  
 * http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
 *
 * However, the CPU-related stuff seems not to work
 ***********************/


//----------------------------------------------------------------------------
// constructor
//
SystemInfo::SystemInfo() {
    memset(&m_sisCurrent, 0, sizeof(sysinfstruct));
}

//----------------------------------------------------------------------------
// destructor
//
SystemInfo::~SystemInfo() {

}


//----------------------------------------------------------------------------
// instance
//
SystemInfo *SystemInfo::createInstance() {
    SystemInfo *pSystemInfo = new SystemInfo();
    if (pSystemInfo != NULL) {
        pSystemInfo->init();
    }
    
    return pSystemInfo;
}




//----------------------------------------------------------------------------
// init
//
void SystemInfo::init() {
    /*
    FILE* fIn1 = fopen("/proc/stat", "r");
    fscanf(fIn1, "cpu %ld %ld %ld %ld", 
           &m_ulLastTotalUser, 
           &m_ulLastTotalUserLow,
           &m_ulLastTotalSys,
           &m_ulLastTotalIdle);
    fclose(fIn1);
    */
    
    /*
    FILE* fIn2;
    struct tms timeSample;
    char sLine[128];

    m_lLastCPU = times(&timeSample);
    m_lLastSysCPU = timeSample.tms_stime;
    m_lLastUserCPU = timeSample.tms_utime;
    

    fIn2 = fopen("/proc/cpuinfo", "r");
    m_iNumProcessors = 0;
    while (fgets(sLine, 128, fIn2) != NULL){
        if (strncmp(sLine, "processor", 9) == 0) m_iNumProcessors++;
    }
    fclose(fIn2);
    */
}

    
//----------------------------------------------------------------------------
// collectInfo
//
int SystemInfo::collectInfo() {
    int iResult = -1;
    
    struct sysinfo memInfo;
    iResult = sysinfo (&memInfo);
    
    if (iResult == 0) {
        _getNamePid();
        m_sisCurrent.ulTotalVirt    = _getTotalVirtualMemory(memInfo);
        m_sisCurrent.ulUsedVirt     = _getUsedVirtualMemory(memInfo);
        m_sisCurrent.ulProcUsedVirt = _getProcUsedVirtualMemory();

        m_sisCurrent.ulTotalRAM     = _getTotalRAM(memInfo);
        m_sisCurrent.ulUsedRAM      = _getUsedRAM(memInfo);
        m_sisCurrent.ulProcUsedRAM  = _getProcUsedRAM();
        m_sisCurrent.ulPeakVM       = _getPeakVM();
        /*
        m_sisCurrent.dUsedCPU       = _getUsedCPU();
        m_sisCurrent.dProcUsedCPU   = _getProcUsedCPU();
        */
    } else {
        printf("Couldn't so sysinfo()\n");
    }

    return iResult;
}

//----------------------------------------------------------------------------
// getInfo
//
int SystemInfo::getInfo(sysinfstruct *psis) {
    int iResult = -1;
    iResult = collectInfo();
    if (iResult == 0) {
        psis->ulTotalVirt    = m_sisCurrent.ulTotalVirt;      
        psis->ulUsedVirt     = m_sisCurrent.ulUsedVirt;    
        psis->ulProcUsedVirt = m_sisCurrent.ulProcUsedVirt;
        
        psis->ulTotalRAM     = m_sisCurrent.ulTotalRAM;    
        psis->ulUsedRAM      = m_sisCurrent.ulUsedRAM;     
        psis->ulProcUsedRAM  = m_sisCurrent.ulProcUsedRAM; 
        psis->ulPeakVM       = m_sisCurrent.ulPeakVM;
        /*
        psis->dUsedCPU       = m_sisCurrent.dUsedCPU;      
        psis->dProcUsedCPU   = m_sisCurrent.dProcUsedCPU;  
        */
    } else {
        printf("Couldn't get info\n");
    }
    return iResult;
}

//----------------------------------------------------------------------------
// getTotalVirtualMemory
//
ulong SystemInfo::getTotalVirtualMemory() {
    collectInfo();
    return m_sisCurrent.ulTotalVirt;
}


//----------------------------------------------------------------------------
// getUsedVirtualmemory
//
ulong SystemInfo::getUsedVirtualMemory() {
    collectInfo();
    return m_sisCurrent.ulUsedVirt;
}


//----------------------------------------------------------------------------
// getProcUsedVirtualMemory
//
ulong SystemInfo::getProcUsedVirtualMemory() {
    collectInfo();
    return m_sisCurrent.ulProcUsedVirt;
}


//----------------------------------------------------------------------------
// getTotalRAM
//
ulong SystemInfo::getTotalRAM() {
    collectInfo();
    return m_sisCurrent.ulTotalRAM;
}


//----------------------------------------------------------------------------
// getUsedRAM
//
ulong SystemInfo::getUsedRAM() {
    collectInfo();
    return m_sisCurrent.ulUsedRAM;
}

//----------------------------------------------------------------------------
// getProcUsedRAM
//
ulong SystemInfo::getProcUsedRAM() {
    collectInfo();
    return m_sisCurrent.ulProcUsedRAM;
}

//----------------------------------------------------------------------------
// getPeakVM
//
ulong SystemInfo::getPeakVM() {
    collectInfo();
    return m_sisCurrent.ulPeakVM;
}

//----------------------------------------------------------------------------
// getPeakVM
//
void SystemInfo::showCurrent() {
    collectInfo();
    printf("----- Process [%s] PID %d -----\n", m_sName, m_iPid);
    printf("Total Virtual Memory:     %ld\n", m_sisCurrent.ulTotalVirt);
    printf("Used  Virtual Memory:     %ld\n", m_sisCurrent.ulUsedVirt);
    printf("Proc Used Virtual Memory: %ld\n", m_sisCurrent.ulProcUsedVirt);
    printf("Peak VM:                  %ld\n", m_sisCurrent.ulPeakVM);
    printf("Total RAM:                %ld\n", m_sisCurrent.ulTotalRAM);
    printf("Used  RAM:                %ld\n", m_sisCurrent.ulUsedRAM);
    printf("Proc Used RAM:            %ld\n", m_sisCurrent.ulProcUsedRAM);
    printf("-----\n");
}


//----------------------------------------------------------------------------
// _getTotalVirtualMemory
//
ulong SystemInfo::_getTotalVirtualMemory(struct sysinfo &memInfo) {
    /*
    printf("totalram:  %ld\n", memInfo.totalram);
    printf("totalswap: %ld\n", memInfo.totalswap);
    printf("mem_unit:  %u\n",  memInfo.mem_unit);
    */
    ulong ulTotalVirtualMem = memInfo.totalram;
    //Add other values in next statement to avoid int overflow on right hand side...
    ulTotalVirtualMem      += memInfo.totalswap;
    ulTotalVirtualMem      *= memInfo.mem_unit;
    
    return ulTotalVirtualMem;
}


//----------------------------------------------------------------------------
// _getUsedVirtualmemory
//
ulong SystemInfo::_getUsedVirtualMemory(struct sysinfo &memInfo) {

  ulong ulVirtualMemUsed  = memInfo.totalram - memInfo.freeram;
  ulVirtualMemUsed       += memInfo.totalswap - memInfo.freeswap;
  ulVirtualMemUsed       *= memInfo.mem_unit;

  return ulVirtualMemUsed;
}


//----------------------------------------------------------------------------
// parseLine
//   get first number found in line
// 
int parseLine(char* pLine){
    int i = strlen(pLine);
    while (*pLine < '0' || *pLine > '9') pLine++;
    char *p = pLine;
    while (*p >= '0' && *p <= '9') p++;
    *p = '\0';
    i = atoi(pLine);
    return i;
}

//----------------------------------------------------------------------------
// _getNamePid
//   get process name and id from proc/self/status
//
int SystemInfo::_getNamePid() {
    int iResult = -1;
    FILE* fIn = fopen("/proc/self/status", "r");
    if (fIn != NULL) {
        iResult = 0;
        char sLine[128];
    
        while (fgets(sLine, 128, fIn) != NULL) {
            if (strncmp(sLine, "Name:", 5) == 0) {
                char *p = sLine+5;
                strcpy(m_sName, trim(p));
            } else   if (strncmp(sLine, "Pid:", 4) == 0) {  
                m_iPid = parseLine(sLine); 
            }
        }
        
        fclose(fIn);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// _getProcUsedVirtualMemory
//  get virtual memory used by this process from /proc/self/status
//
ulong SystemInfo::_getProcUsedVirtualMemory() {
    ulong ulResult = 0;
    
    FILE* fIn = fopen("/proc/self/status", "r");
    char sLine[128];
    
    while (fgets(sLine, 128, fIn) != NULL) {
        if (strncmp(sLine, "VmSize:", 7) == 0) {
            ulResult = parseLine(sLine); // result in KB
            ulResult *= 1024;
            break;
        }
    }
    fclose(fIn);
    return ulResult;
    
}


//----------------------------------------------------------------------------
// _getTotalRAM
//
ulong SystemInfo::_getTotalRAM(struct sysinfo &memInfo) {
    
    ulong ulTotalRAM = memInfo.totalram;
    ulTotalRAM *= memInfo.mem_unit;
    return ulTotalRAM;
}


//----------------------------------------------------------------------------
// _getUsedRAM
//
ulong SystemInfo::_getUsedRAM(struct sysinfo &memInfo) {
    
    ulong ulUsedRAM = memInfo.totalram - memInfo.freeram;
    ulUsedRAM *= memInfo.mem_unit;
    return ulUsedRAM;
}


//----------------------------------------------------------------------------
// _getProcUsedRAM
//  get RAM used by process from /proc/self/status
//
ulong SystemInfo::_getProcUsedRAM() {
    ulong ulResult = 0;

    FILE* fIn = fopen("/proc/self/status", "r");
    char sLine[128];
    
    while (fgets(sLine, 128, fIn) != NULL) {
        if (strncmp(sLine, "VmRSS:", 6) == 0) {
            ulResult = parseLine(sLine);  //result in KB
            ulResult *= 1024; 
            break;
        }
    }
    fclose(fIn);
    return ulResult;
}

//----------------------------------------------------------------------------
// _getPeakVM
//  get peak virtual memory usage from /proc/self/status
//
ulong SystemInfo::_getPeakVM() {
    ulong ulResult = 0;
    //    ulong ulProgSize = 0;
    FILE* fIn = fopen("/proc/self/status", "r");
    char sLine[128];
    
    int iToDo = 1;
    while ((iToDo > 0) && (fgets(sLine, 128, fIn) != NULL)) {
        if (strncmp(sLine, "VmPeak:", 7) == 0) {
            ulResult = parseLine(sLine);  //result in KB
            ulResult *= 1024; 
            iToDo--;
        }

    }

    fclose(fIn);
    return ulResult;
}

/*
//----------------------------------------------------------------------------
// _getUsedCPU
//
double SystemInfo::_getUsedCPU() {
    double dPercent;
    ulong totalUser;
    ulong totalUserLow;
    ulong totalSys;
    ulong totalIdle;
   
    FILE *fIn = fopen("/proc/stat", "r");
    fscanf(fIn, "cpu %ld %ld %ld %ld", &totalUser, &totalUserLow,
           &totalSys, &totalIdle);
    fclose(fIn);
    

    if (totalUser < m_ulLastTotalUser || totalUserLow < m_ulLastTotalUserLow ||
        totalSys < m_ulLastTotalSys || totalIdle < m_ulLastTotalIdle){
        //Overflow detection. Just skip this value.
        dPercent = -1.0;
    }
        else{
            ulong total = (totalUser    - m_ulLastTotalUser) + 
                          (totalUserLow - m_ulLastTotalUserLow) +
                          (totalSys     - m_ulLastTotalSys);
            dPercent = total;
            total += (totalIdle - m_ulLastTotalIdle);
            dPercent /= total;
            dPercent *= 100;
        }
    

        m_ulLastTotalUser    = totalUser;
        m_ulLastTotalUserLow = totalUserLow;
        m_ulLastTotalSys     = totalSys;
        m_ulLastTotalIdle    = totalIdle;
    

        return dPercent;
}

//----------------------------------------------------------------------------
// _getProcUsedCPU
//
double SystemInfo::_getProcUsedCPU() {
    struct tms timeSample;
    clock_t cNow;
    double dPercent;
    
    
    cNow = times(&timeSample);
    if (cNow <= m_lLastCPU || timeSample.tms_stime < m_lLastSysCPU ||
        timeSample.tms_utime < m_lLastUserCPU){
        //Overflow detection. Just skip this value.
        dPercent = -1.0;
    }
    else{
        dPercent = (timeSample.tms_stime - m_lLastSysCPU) +
            (timeSample.tms_utime - m_lLastUserCPU);
        dPercent /= (cNow - m_lLastCPU);
        dPercent /= m_iNumProcessors;
        dPercent *= 100;
    }
    m_lLastCPU = cNow;
    m_lLastSysCPU  = timeSample.tms_stime;
    m_lLastUserCPU = timeSample.tms_utime;
    
    
    return dPercent;
}
*/
