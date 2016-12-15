#ifndef __MPIMULTI_H__
#define __MPIMULTI_H__

#include <map>
#include <vector>
#include <mpi.h>

typedef std::vector<gridtype> linkinfo;

class MPIMulti {
public:
    static MPIMulti *createInstance(int iTile, linkinfo &vOutLinks, linkinfo &vInLinks);
    static MPIMulti *createInstance(int iTile, linkinfo &vOutLinks, linkinfo &vInLinks, linkinfo &vTags);
    ~MPIMulti();
    int exchangeData();

    uchar       *createSendBuf(int iTarget, int iSize);
    const uchar *getRecvBuf(int iSource, int *piSize);
    const uchar *getSendBuf(int iTarget, int *piSize);


    int getNumTargets() { return m_iNumTargets;};
    int getNumSources() { return m_iNumSources;};
    int getTarget(int iTIndex) { return m_aTargets[iTIndex];};
    int getSource(int iSIndex) { return m_aSources[iSIndex];};

protected:
    MPIMulti(int iTile);
    int init(linkinfo &vOutLinks, linkinfo &vInLinks, linkinfo &vTags);

    int createBufs(int iNumSends, int iNumRecvs);
    int deleteBufs();
    int initializeBufs(linkinfo &vOutLinks, linkinfo &vInLinks, linkinfo &vTags);
    
    std::map<int, int> m_mRecvIDToIdx;

    int          m_iTile;
    int          m_iNumTargets;
    int          m_iNumSources;
    MPI_Request	*m_aSendRequests;
    MPI_Request *m_aRecvRequests;
    int	        *m_aSendBufSizes;
    int	        *m_aRecvBufSizes;
    int	        *m_aSendSizes;
    int	        *m_aRecvSizes;
    uchar      **m_aSendBufs;
    uchar      **m_aRecvBufs;
    int         *m_aTargets;
    int         *m_aSources;
    int         *m_aTags;
    int          m_iInitialSize;
};

#endif
