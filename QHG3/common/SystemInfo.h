#ifndef __SYSTEMINFO_H__
#define __SYSTEMINFO_H__

#include <sys/sysinfo.h>
#include "types.h"


/***********************
 * basic functionality from Stackoverflow:
 *  
 * http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
 *
 * However, the CPU-related stuff seems not to work
 ***********************/

typedef struct {
    ulong ulTotalVirt;
    ulong ulUsedVirt;
    ulong ulProcUsedVirt;
    ulong ulTotalRAM;
    ulong ulUsedRAM;
    ulong ulProcUsedRAM;
    ulong ulPeakVM;
    /*
    double dUsedCPU;
    double dProcUsedCPU;
    */
} sysinfstruct;

class SystemInfo {
public:
    static SystemInfo *createInstance();
    
    ~SystemInfo();
    
    ulong getMaxVirtualMemory();
    
    ulong getTotalVirtualMemory();
    ulong getUsedVirtualMemory();
    ulong getProcUsedVirtualMemory();
    ulong getPeakVM();
    
    ulong getTotalRAM();
    ulong getUsedRAM();
    ulong getProcUsedRAM();
    /*
      double getUsedCPU();
      double getProcUsedCPU();
    */
    
    int getInfo(sysinfstruct *psis);
    
    void showCurrent();
private:
    sysinfstruct m_sisCurrent;

    SystemInfo();
    void init();
    int collectInfo();
    ulong _getTotalVirtualMemory(struct sysinfo &meminfo);
    ulong _getUsedVirtualMemory(struct sysinfo &meminfo);
    ulong _getProcUsedVirtualMemory();
    ulong _getPeakVM();
    
    ulong _getTotalRAM(struct sysinfo &meminfo);
    ulong _getUsedRAM(struct sysinfo &meminfo);
    ulong _getProcUsedRAM();

    int   _getNamePid();
   
    char m_sName[256];
    int  m_iPid;
    /*
    double _getUsedCPU();
    double _getProcUsedCPU();
    */
    
    /*
    ulong m_ulLastTotalUser;
    ulong m_ulLastTotalUserLow;
    ulong m_ulLastTotalSys;
    ulong m_ulLastTotalIdle;
       
    clock_t m_lLastCPU;
    clock_t m_lLastSysCPU;
    clock_t m_lLastUserCPU;
    
    int m_iNumProcessors;
    */
};



#endif
