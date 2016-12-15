#ifndef __ANCANALYZER_H__
#define __ANCANALYZER_H__

#include <vector>
#include <map>

#define DEF_AABUF_SIZE 32768

class AncAnalyzer {
public:
    static AncAnalyzer *createInstance(const char *pAncFile, int iBufSize=DEF_AABUF_SIZE);
    ~AncAnalyzer();
    
    int analyze(int iNumRecs, int iStep);

    int getStep() { return m_iStep;};
    std::vector<int> &getSegments() { return m_vSegments;};

    int getSegment(unsigned int iIndex) { int iS = -1; if (iIndex < m_vSegments.size()) { iS = m_vSegments[iIndex];} return iS;};
    int getMinPar(unsigned int iIndex) { int iS = -1; if (iIndex < m_iNumBlocks) { iS = m_pMinParents[iIndex];} return iS;};
    int getLimit(unsigned int iIndex) { int iS = -1; if (iIndex < m_iNumBlocks) { iS = m_pLimitIds[iIndex];} return iS;};
    int getNumRecs() {return m_iNumRecs;};
    void show();
protected:
    AncAnalyzer();
    int init(const char *pAncFile, int iBufSize);

    int processChunk(int iSize);
    int reduceSegments();
    int getPositions();

    std::vector<int> m_vSegments;

    FILE *m_fIn;
    long m_lFileSize;

    unsigned int m_iNumBlocks;
    int *m_pMinParents;
    int *m_pLimitIds;
      
    int m_iBufSize;
    int *m_pReadBuf;
    
    int m_iNumRecs;
    int m_iStep;
    int m_iLocalPos;
    int m_iRealPos;
    unsigned int m_iCurSegment;

    std::map<int,int> m_mIdPos;  
};


#endif
