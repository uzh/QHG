
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> //gtpid
#include <unistd.h> //sleep, getpid
#include <time.h>
#include "math.h"
#include <mpi.h>

#include "strutils.h"
#include "EQTileLinks.h"
#include "MPIMulti.h"


//-----------------------------------------------------------------------------
// setSendBuffers
//   fill sendbuffers with data taken from pEQT (CellIDs)
//
int setSendBuffers(int iTile, MPIMulti *pMM, EQTileLinks *pEQT) {

    const tTileCommUnits &mTCB = pEQT->getBorderUnits(iTile);
    tTileCommUnits::const_iterator it;
    printf("[%d] Filling %zd buffers\n", iTile, mTCB.size());
    int iResult = 0;
    int iTargetIndex = 0;

    // loop through the map of ID sets
    for (it = mTCB.begin(); (iResult == 0) && (it != mTCB.end()); ++it) {

        // create a buffer of the required size
        int iSendBufSize = it->second.size()*sizeof(int);
        uchar *pSendBuf = pMM->createSendBuf(iTargetIndex, iSendBufSize);
        printf("[%d] %zd ints -> size[%d] %d\n", iTile, it->second.size(), iTargetIndex, iSendBufSize);

        // now fill it with the elements of the set
        uchar *p = pSendBuf;
        intset_cit iti;
        for (iti = it->second.begin(); iti != it->second.end(); ++iti) {
            int iCur = *iti;
            printf("[%d]   adding %d\n", iTile, iCur);fflush(stdout);
            p = putMem(p, &iCur, sizeof(int));
        }

        iTargetIndex++;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// displaySendBuffers
//   display all sendbuffers for this tile
//
void displaySendBuffers(int iTile, MPIMulti *pMM) {
    printf("[%d] num targets %d\n", iTile, pMM->getNumTargets());
    for (int iTargetIndex = 0; iTargetIndex < pMM->getNumTargets(); iTargetIndex++) {
        int iSize = 0;
        const uchar *pBuf = pMM->getSendBuf(iTargetIndex, &iSize);
        if ((pBuf != NULL) && (iSize > 0)) {
            printf("[%d] SendBuf[%d]->%d: ", iTile, iTargetIndex, pMM->getTarget(iTargetIndex));
            int iNumInt = iSize/sizeof(int);
            const uchar *p = pBuf;
            for(int j = 0; j < iNumInt; j++) {
                int iN;
                p = getMem(&iN, p, sizeof(int));
                printf(" %d", iN);
            }
            printf("\n");
        } else {
            printf("[%d] target #%d: size %d, o %p\n", iTile, iTargetIndex, iSize, pBuf);fflush(stdout);
        }
    }
}


//-----------------------------------------------------------------------------
// displayRecvBuffers
//   display all recvbuffers for this tile
//
void displayRecvBuffers(int iTile, MPIMulti *pMM) {
    printf("[%d] num sources %d\n", iTile, pMM->getNumSources());
    for (int iSourceIndex = 0; iSourceIndex < pMM->getNumSources(); iSourceIndex++) {
        int iSize = 0;
        const uchar *pBuf = pMM->getRecvBuf(iSourceIndex, &iSize);
        if ((pBuf != NULL) && (iSize > 0)) {
            printf("[%d] RecvBuf[%d]<-%d: ", iTile, iSourceIndex, pMM->getSource(iSourceIndex));
            int iNumInt = iSize/sizeof(int);
            const uchar *p = pBuf;
            for(int j = 0; j < iNumInt; j++) {
                int iN;
                p = getMem(&iN, p, sizeof(int));
                printf(" %d", iN);
            }
            printf("\n");
        } else {
            printf("[%d] source #%d: size %d, o %p\n", iTile, iSourceIndex, iSize, pBuf);fflush(stdout);
        }
    }
}

//-----------------------------------------------------------------------------
// extractLinkInfo
//
int extractLinkInfo(EQTileLinks *pEQT, int iTile, linkinfo &vBorderInfo, linkinfo &vHaloInfo) {
    int iResult = 0;

    // create the linkinfo vectors from the EQTiling
    tTileCommUnits::const_iterator it;

    const tTileCommUnits &mTCB = pEQT->getBorderUnits(iTile);
    for (it = mTCB.begin(); it != mTCB.end(); ++it) {
        vBorderInfo.push_back(it->first);
    }

    const tTileCommUnits &mTCH = pEQT->getHaloUnits(iTile);
    for (it = mTCH.begin(); it != mTCH.end(); ++it) {
        vHaloInfo.push_back(it->first);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// main
//
int main(int iArgC,char **apArgV) {
    int iResult = 0;

    int          iTile;
    int          iNumProcs;
   
    if (iArgC > 1) {
        printf("MPI start\n"); fflush(stdout);
        // MPI Initialisation
        MPI_Init(&iArgC, &apArgV);
            
        // Get the number of MPI tasks and the taskid of this task
        MPI_Comm_rank(MPI_COMM_WORLD, &iTile);
        MPI_Comm_size(MPI_COMM_WORLD, &iNumProcs);

        printf("MPI: rank %d, num %d, proc %d\n", iTile, iNumProcs, getpid()); fflush(stdout);
        
        EQTileLinks *pEQT = EQTileLinks::createInstance(apArgV[1]);
        if (pEQT != NULL) {

            linkinfo vBorderInfo;
            linkinfo vHaloInfo;

            extractLinkInfo(pEQT, iTile, vBorderInfo, vHaloInfo);
       
            
            // create a MPIMulti object
            MPIMulti *pMM = MPIMulti::createInstance(iTile,  vBorderInfo, vHaloInfo);
            if (pMM != NULL) {
                
                
                // set data (using pMM->createSendBuf())
                setSendBuffers(iTile, pMM, pEQT);

                // show state before (using pMM->getSendBuf(), pMM->getNumTargets(), pMM->getTarget())
                displaySendBuffers(iTile, pMM);
           
                // Communication.
                printf("[%d] exchanges\n", iTile);fflush(stdout);
                iResult = pMM->exchangeData();
                
                // show state after (using pMM->getRecvBuf(), pMM->getNumSources(), pMM->getSource()
                displayRecvBuffers(iTile, pMM);
                
                delete pMM;
            }
        
            delete pEQT;
       
        } else {
            printf("Couldn't creqte EQTileLinks from [%s]\n", apArgV[1]);
        }
        
        printf("[%d] Ending MPI\n", iTile);
        // MPI finalisation.
        MPI_Finalize();
        
        printf("ciao\n");
    } else {
        printf("Usage: %s <EQTileLinkFile>\n", apArgV[0]);

        printf("To compile\n");
        printf("    mpic++ -Wall -g mpitest.cpp MPIMulti.cpp EQTileLinks.cpp -lmpi_cxx -I../common -I../icosa -L../icosa -lIcosa -L../common -lCommon -lz\n");

        printf("To run:\n");
        printf("    mpirun -np 8 ./a.out lalalank_8\n");
    }
    return iResult;
}

