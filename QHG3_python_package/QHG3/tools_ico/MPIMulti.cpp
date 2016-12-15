#include <stdio.h>
#include <limits.h>  // INT_MAX
#include <mpi.h>

#include "types.h"
#include "MPIMulti.h"

#define INITIAL_SIZE 16


//----------------------------------------------------------------------------
// createInstance
//   iTile:       ID of tile (MPI rank)
//   vOutLinks:   array of tile IDs to send data to
//   vInLinks:    array of tile IDs to receive data from
// In this form, the index to be used for receive buffers is determined
// automatically (m_mRecvIDToIdx)
//
MPIMulti *MPIMulti::createInstance(int iTile, linkinfo &vOutLinks, linkinfo &vInLinks) {
    linkinfo vTags; // empty
    return createInstance(iTile, vOutLinks, vInLinks, vTags);
}


//----------------------------------------------------------------------------
// createInstance
//   iTile:       ID of tile (MPI rank)
//   vOutLinks:   array of tile IDs to send data to
//   vInLinks:    array of tile IDs to receive data from
//   vTags:       array of tags to be used for send 
//                (may be omitted if vInLinks contains no multiple entries)
//
MPIMulti *MPIMulti::createInstance(int iTile, linkinfo &vOutLinks, linkinfo &vInLinks, linkinfo &vTags) {
    MPIMulti *pMM = new MPIMulti(iTile);
    int iResult = pMM->init(vOutLinks, vInLinks, vTags);
    if (iResult != 0) {
        delete pMM;
        pMM = NULL;
    }
    return pMM;
}


//----------------------------------------------------------------------------
// constructor
//
MPIMulti::MPIMulti(int iTile)
    : m_iTile(iTile),
      m_iNumTargets(0),
      m_iNumSources(0),
      m_aSendRequests(NULL),
      m_aRecvRequests(NULL),
      m_aSendBufSizes(NULL),
      m_aRecvBufSizes(NULL),
      m_aSendSizes(NULL),
      m_aRecvSizes(NULL),
      m_aSendBufs(NULL),
      m_aRecvBufs(NULL),
      m_aTargets(NULL),
      m_aSources(NULL),
      m_iInitialSize(INITIAL_SIZE) {
}


//----------------------------------------------------------------------------
// destructor
//
MPIMulti::~MPIMulti() {
    deleteBufs();
}


//----------------------------------------------------------------------------
// init
//   create and initialize buffers
//
int MPIMulti::init(linkinfo &vOutLinks, linkinfo &vInLinks, linkinfo &vTags) {
    int iResult = 0;

    iResult = createBufs(vOutLinks.size(), vInLinks.size());
    if (iResult == 0) {
        iResult = initializeBufs(vOutLinks, vInLinks, vTags);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createBufs
//   once we know the number of tiles we communicate with,
//   we can allocate most arrays
//
int MPIMulti::createBufs(int iNumTargets, int iNumSources) {
    int iResult = 0;
    printf("[%d]createBufs(%d, %d)\n", m_iTile, iNumTargets, iNumSources);
    m_iNumTargets = iNumSources;
    m_iNumSources = iNumTargets;
    
    // buffer sizes
    m_aSendBufSizes = new int[m_iNumTargets];
    m_aRecvBufSizes = new int[m_iNumSources];

    // current send sizes
    m_aSendSizes = new int[m_iNumTargets];
    m_aRecvSizes = new int[m_iNumSources];
    
    // ids of communication partners
    m_aTargets = new int [m_iNumTargets];
    m_aSources = new int [m_iNumSources];
    
    // request buffers
    m_aSendRequests = new MPI_Request[m_iNumTargets];
    m_aRecvRequests = new MPI_Request[m_iNumSources];
        
    // send buffers
    m_aSendBufs = new uchar*[m_iNumTargets];
    memset(m_aSendBufs, 0, m_iNumTargets*sizeof(uchar *));

    // recv buffers
    m_aRecvBufs = new uchar*[m_iNumSources];
    memset(m_aRecvBufs, 0, m_iNumSources*sizeof(uchar *));

    // tags
    m_aTags = new int[m_iNumSources];
    for (int i = 0; i < m_iNumSources; i++) {
        m_aTags[i] = INT_MAX; // no negative tags allowed, so we use another special number
    }

    return iResult;
}


//----------------------------------------------------------------------------
// deleteBufs
//
int MPIMulti::deleteBufs() {

    if (m_aSendBufSizes != NULL) {
        delete[] m_aSendBufSizes;
    }

    if (m_aRecvBufSizes != NULL) {
        delete[] m_aRecvBufSizes;
    }

    if (m_aSendSizes != NULL) {
        delete[] m_aSendSizes;
    }

    if (m_aRecvSizes != NULL) {
        delete[] m_aRecvSizes;
    }

    if (m_aTargets != NULL) {
        delete[] m_aTargets;
    }

    if (m_aSources != NULL) {
        delete[] m_aSources;
    }

    if (m_aSendRequests != NULL) {
        delete[] m_aSendRequests;
    }

    if (m_aRecvRequests != NULL) {
        delete[] m_aRecvRequests;
    }

    if (m_aSendBufs != NULL) {
        for (int i = 0; i < m_iNumTargets; i++) {
            if (m_aSendBufs[i] != NULL) {
                delete[] m_aSendBufs[i];
            }
        }  
        delete[] m_aSendBufs;
    }

    if (m_aRecvBufs != NULL) {
        for (int i = 0; i < m_iNumSources; i++) {
            if (m_aRecvBufs[i] != NULL) {
                delete[] m_aRecvBufs[i];
            }
        }  
        delete[] m_aRecvBufs;
    }

    if (m_aTags != NULL) {
        delete[] m_aTags;
    }

    return 0;
}


//----------------------------------------------------------------------------
// initializeBufs
//   initialize target and source arrays based on link info; collect tags
//
int MPIMulti::initializeBufs(linkinfo &vOutLinks, linkinfo &vInLinks, linkinfo &vTags) {
    
    int iResult = 0;
    
    // initialize all send buffers
    for (uint iC = 0; iC < vOutLinks.size(); ++iC) {
        m_aTargets[iC] = vOutLinks[iC];

        // dummy 1-byte buffers
        m_aSendBufSizes[iC] = m_iInitialSize;
        m_aSendSizes[iC] = 0;
        m_aSendBufs[iC] = new uchar[m_aSendBufSizes[iC]];
        // initialize to 0xff
        memset(m_aSendBufs[iC], 0xff, m_aSendBufSizes[iC]*sizeof(uchar));
    }

    // initialize all recv buffers
    for (uint iC = 0; iC < vInLinks.size(); ++iC) {
        m_aSources[iC] =  vInLinks[iC];

        // create buffers with some initial size
        m_aRecvBufSizes[iC] = m_iInitialSize;
        m_aRecvSizes[iC] = 0;
        m_aRecvBufs[iC] = new uchar[m_aRecvBufSizes[iC]];
        // initialize to 0xff
        memset(m_aRecvBufs[iC], 0xff, m_aRecvBufSizes[iC]*sizeof(uchar));

        m_mRecvIDToIdx[vInLinks[iC]] = iC;
    }
    

    // collect tags
    if ((int)vTags.size() == m_iNumTargets) {
        for (int i = 0; i < m_iNumTargets; i++) {
            m_aTags[i] = vTags[i];
        }
    } else {
        if (vTags.size() > 0) {
            iResult = -1;
            printf("Wrong number of tags provided\n");
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createSendBuf
//  return specified buffer (to be filled by the caller);
//  if necessary, the buffer is resized
// 
uchar *MPIMulti::createSendBuf(int iTargetIndex, int iSize) {
    uchar *pOut = NULL;

    if ((iTargetIndex >= 0) && (iTargetIndex < m_iNumTargets)) {
        if (iSize >= m_aSendBufSizes[iTargetIndex]) {
            // must increase buffer
            delete[] m_aSendBufs[iTargetIndex];
            m_aSendBufSizes[iTargetIndex] = iSize;
            m_aSendBufs[iTargetIndex] = new uchar[m_aSendBufSizes[iTargetIndex]];
        }
        m_aSendSizes[iTargetIndex] = iSize;
        pOut = m_aSendBufs[iTargetIndex];
    } else {
        printf("No send buffer for target index %d (ID %d)\n", iTargetIndex, m_aTargets[iTargetIndex]);
    }
    return pOut;
}


//----------------------------------------------------------------------------
// getSendBuf
//   returns the specified send buffer; *piSize is set to current message size
//
const uchar *MPIMulti::getSendBuf(int iTarget, int *piSize) {
    uchar *pOut = NULL;
    
    *piSize = m_aSendSizes[iTarget];
    pOut    = m_aSendBufs[iTarget];

    return pOut;
}


//----------------------------------------------------------------------------
// getRecvBuf
//   returns the specified recv buffer; *piSize is set to current message size
//
const uchar *MPIMulti::getRecvBuf(int iSource, int *piSize) {
    uchar *pOut = NULL;
  
    *piSize = m_aRecvSizes[iSource];
    pOut    = m_aRecvBufs[iSource];
  
  return pOut;
}


//----------------------------------------------------------------------------
// exchangeData
//  asynchronous exchange of data
//  * send all outgoing buffers with MPI_Isend
//  * loop until all receives are done
//  *   detect incoming data (MPI_IProbe)
//  *   if data present
//  *     get data size
//  *     if data size > current buffersize
//  *       resize recv buffer
//  *     endif
//  *     receive the data
//  *   endif
//  * endloop
//
int MPIMulti::exchangeData() {
    int iResult = 0;
    int iErr = MPI_SUCCESS;
    
    // send of all outgoing data
    for (int i = 0; (iErr == MPI_SUCCESS) && (i < m_iNumTargets); i++) {
        //        printf("ISending %d bytes to ID %d,tag %d (idx %d)\n",  m_aSendSizes[i], m_aTargets[i], m_aTags[i], i);fflush(stdout);
        iErr=MPI_Isend(m_aSendBufs[i], m_aSendSizes[i], MPI_BYTE,
                       m_aTargets[i], m_aTags[i], MPI_COMM_WORLD, &(m_aSendRequests[i]));   
    }

    // we expect m_iNumSources messages to come in
    int iHangingRecvs = m_iNumSources;
    for (int i = 0;  (iErr == MPI_SUCCESS) && (iHangingRecvs > 0); i++) {
       
        int iFlag=0;
        MPI_Status status;
        // check if something has arrived
        iErr = MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &iFlag, &status);
        if (iFlag > 0) {
            // something has arrived!
            int iSource = status.MPI_SOURCE;
            int iTag    = status.MPI_TAG;
            int iNum = 0;
            // get data size of arrival
            MPI_Get_count(&status, MPI_BYTE, &iNum);

            // we need the source index to use the correct buffers
            int iSourceIndex = -1;
            if (iTag == INT_MAX) {
                // find index for source id:
                // this only works if all targets are unique
                std::map<int,int>::const_iterator it = m_mRecvIDToIdx.find(iSource);
                if (it != m_mRecvIDToIdx.end()) {
                    iSourceIndex = it->second;
                }
            } else {
                // in case of non-unique targets, tags should be provided
                iSourceIndex = iTag;
            }

            if (iSourceIndex >= 0) {
                // if the buffer's size is too small, increase it (i.e. make new one)
                if (iNum >= m_aRecvBufSizes[iSourceIndex]) {
                    int iNewSize = 1*iNum;
                    //@@  printf("[%d] Resizing Recvbuf[%d] (%p) from %d to %d\n", m_iTile, iSourceIndex,  m_aRecvBufs[iSourceIndex], m_aRecvBufSizes[iSourceIndex], iNewSize);fflush(stdout);
                    delete[] m_aRecvBufs[iSourceIndex];
                    m_aRecvBufs[iSourceIndex] = new uchar[iNewSize];
                    m_aRecvBufSizes[iSourceIndex] = iNewSize;
                }
                m_aRecvSizes[iSourceIndex] = iNum;

                // actually receive the data
                //@@  printf("Receiving %d bytes from ID %d (idx %d, tag %d)\n",  iNum, iSource, iSourceIndex, iTag);
                iErr=MPI_Recv((m_aRecvBufs[iSourceIndex]), iNum, MPI_BYTE,
                              iSource,  iTag, MPI_COMM_WORLD, &status);

                iHangingRecvs--;
            } else {
                printf("[%d] found no index for [%d]\n", m_iTile, iSource);fflush(stdout);
                iErr = -1;
            }
        }
    }

    return iResult;
}



