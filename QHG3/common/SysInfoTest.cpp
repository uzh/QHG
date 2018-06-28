#include <stdio.h>
#include <unistd.h>


#include "SystemInfo.h"

void showInfo(sysinfstruct &sis) {
    printf("-----\n");
    printf("Total Virtual Memory:     %ld\n", sis.ulTotalVirt);
    printf("Used  Virtual Memory:     %ld\n", sis.ulUsedVirt);
    printf("Proc Used Virtual Memory: %ld\n", sis.ulProcUsedVirt);
    printf("Total RAM:                %ld\n", sis.ulTotalRAM);
    printf("Used  RAM:                %ld\n", sis.ulUsedRAM);
    printf("Peak VM:                  %ld\n", sis.ulPeakVM);
    printf("Proc Used RAM:            %ld\n", sis.ulProcUsedRAM);
    /*
    printf("Used CPU:                 %f\n",  sis.dUsedCPU);
    printf("Proc Used CPU:            %f\n",  sis.dProcUsedCPU);
    */
    printf("-----\n");
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    SystemInfo *pSysInfo = SystemInfo::createInstance();
    sysinfstruct sis;

    printf("Init\n");
    pSysInfo->getInfo(&sis);
    showInfo(sis);
    
    char *p1 = new char[100000000];
    for (int i =0; i < 100000000; i++) {
        p1[i] = (3*i/2)%256;
    }
    pSysInfo->getInfo(&sis);
    printf("after new char 100000000\n");
    pSysInfo->getInfo(&sis);
    showInfo(sis);

    int *p2= new int[22222222];
    for (int i =0; i < 22222222; i++) {
        p2[i] = 2*i;
    }


    pSysInfo->getInfo(&sis);
    printf("after new int 22222222\n");
    pSysInfo->getInfo(&sis);
    showInfo(sis);

    delete p1;


    pSysInfo->getInfo(&sis);
    printf("after del p1\n");
    pSysInfo->getInfo(&sis);
    showInfo(sis);

    delete p2;
    
    usleep(1000);
    pSysInfo->getInfo(&sis);
    printf("after del p2\n");
    pSysInfo->getInfo(&sis);
    showInfo(sis);
    

    delete pSysInfo;
    return iResult;
}

