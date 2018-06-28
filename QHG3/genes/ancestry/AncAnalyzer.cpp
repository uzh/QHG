#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>

#include "AncAnalyzer.h"


//----------------------------------------------------------------------------
// constructor
// 
AncAnalyzer::AncAnalyzer() 
    : m_fIn(NULL),
      m_lFileSize(0),
      m_iNumBlocks(0),
      m_pMinParents(NULL),
      m_pLimitIds(NULL),
      m_iBufSize(0),
      m_pReadBuf(NULL), 
      m_iStep(0),
      m_iLocalPos(0),
      m_iRealPos(0),
      m_iCurSegment(0) {
}

//----------------------------------------------------------------------------
// destructor
// 
AncAnalyzer::~AncAnalyzer() {
    if (m_fIn != NULL) {
        fclose(m_fIn);
    }

    if (m_pMinParents != NULL) {
        delete[] m_pMinParents;
    }

    if (m_pLimitIds != NULL) {
        delete[] m_pLimitIds;
    }


    if (m_pReadBuf != NULL) {
        delete[] m_pReadBuf;
    }
}

//----------------------------------------------------------------------------
// createInstance
// 
AncAnalyzer *AncAnalyzer::createInstance(const char *pAncFile, int iBufSize) {
    AncAnalyzer *pAA = new AncAnalyzer();
    int iResult = pAA->init(pAncFile, iBufSize);
    if (iResult != 0) {
        delete pAA;
        pAA = NULL;
    }
    return pAA;
}
    

//----------------------------------------------------------------------------
// init
// 
int AncAnalyzer::init(const char *pAncFile, int iBufSize) {
    int iResult = -1;
    
    m_fIn = fopen(pAncFile, "rb");
    if (m_fIn != NULL) {
        fseek(m_fIn, 0, SEEK_END);
        m_lFileSize = ftell(m_fIn);
        fseek(m_fIn, 0, SEEK_SET);


        if (iBufSize > 0) {
            m_iBufSize = 3*(1+(iBufSize-1)/3);
            printf("Set buffer size to [%d]\n", m_iBufSize);
            m_pReadBuf = new int[m_iBufSize];
            iResult = 0;
        } else {
            printf("Invalid buffer size [%d]\n", iBufSize);
        }

    } else {
        printf("Couldn't open anc file [%s]\n", pAncFile);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// processChunk
//   process iSize
int AncAnalyzer::processChunk(int iSize) {
    int iResult = 0;

    int iBuf = 0;
    while (iBuf < iSize) {
        int iID  = m_pReadBuf[iBuf];
        int iMomPos = m_mIdPos[m_pReadBuf[iBuf+1]];
        int iDadPos = m_mIdPos[m_pReadBuf[iBuf+2]];

        // is Mom minimal?
	if ((m_pMinParents[m_iCurSegment] < 0) || 
            (iMomPos < m_pMinParents[m_iCurSegment])) { 
            m_pMinParents[m_iCurSegment] = iMomPos;
        }
        // is Dad minimal?
        if ((m_pMinParents[m_iCurSegment] < 0) || 
            (iDadPos < m_pMinParents[m_iCurSegment])) {
            m_pMinParents[m_iCurSegment] = iDadPos;
        }
        // time for next segment?
        if (m_iLocalPos == m_iStep) {
            //	    printf("New sector (%d) for abs %d (cur %d) id %d, minPar %d \n", m_iCurSector, m_iRealPos, m_iLocalPos, iID, m_pMinParents[m_iCurSector]);fflush(stdout);
            m_pLimitIds[m_iCurSegment] = iID;
            m_iCurSegment++;
	    m_iLocalPos = 0;
        }
        // next step
        iBuf += 3;
        m_iLocalPos++;            
	m_iRealPos++;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// reduceSegments
// 
int AncAnalyzer::reduceSegments() {
    int iResult = 0;

    int iPrev = -1;
    int iTemp = 0;
    printf("reducing %d segments\n", m_iNumBlocks);

    for (unsigned int i = 0; i < m_iNumBlocks; i++) {
        printf("\r%d", i);
        iTemp =  i;
        for (unsigned int j = i; j < m_iNumBlocks; j++) {
            if (m_pMinParents[j] < m_pMinParents[iTemp]) {
                iTemp = j;
            }
        }

        //        printf("    temp %zd -> %d (v %d; p %d)\n", m_vSegments.size(), iTemp, m_pMinParents[iTemp], iPrev);
        // only accept new segment border if it is greater than the previous one
        if  (m_pMinParents[iTemp] > iPrev) {
            m_vSegments.push_back(iTemp);
            iPrev = m_pMinParents[iTemp];
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// analyze
// 
int AncAnalyzer::analyze(int iNumRecs, int iStep) {
    int iResult = -1;
    m_iNumRecs = iNumRecs;
    if (m_iNumRecs <= 0) {
        m_iNumRecs = m_lFileSize/12; // 1 rec = 3 ints
        printf("setting numrecs to %ld/12 = %d\n", m_lFileSize, m_iNumRecs);
    }
    iResult = getPositions();
    if (iResult == 0) {


        // delete old arrays
        m_iStep = iStep;
        m_iNumBlocks = m_iNumRecs/m_iStep;
    
        m_pMinParents = new int[m_iNumBlocks+1];
        m_pLimitIds   = new int[m_iNumBlocks];
        memset(m_pLimitIds, 0, m_iNumBlocks*sizeof(int));
        for (unsigned int i = 0; i < m_iNumBlocks+1; i++) {
            m_pMinParents[i] = -1;
        }

        m_iLocalPos = 0;
        m_iRealPos = 0;
        m_iCurSegment = 0;

        iResult = 0;
        int iTodo = 3*m_iNumRecs;
        int idLast = 0;
        int iDone = 0; //number of ints
        while ((iResult == 0) && (m_iRealPos < m_iNumRecs) && !feof(m_fIn) ) {
            int iSize = (iTodo>m_iBufSize)?m_iBufSize:iTodo;
            iTodo -= iSize;
            int iRead = fread(m_pReadBuf, sizeof(int), iSize, m_fIn);
            if (iRead == iSize)  {
                printf("\r%d", m_iRealPos);
                /*
                  for (int i = 0; (i < 51) && (i < iRead); i+= 3) {
                  printf("%d ", m_pReadBuf[i]);
                  }
                  printf("\n");
                */
                idLast = m_pReadBuf[iRead-3];
                //            printf("idlast (p. %d): %d\n",  sizeof(int)*(iRead-3+iDone),idLast);
                //            printf("Doing %d; reakpos %d, numecs %d, curSec %d\n", iRead, m_iRealPos, iNumRecs, m_iCurSegment);
                iResult = processChunk(iRead);
                if (iResult == 0) {
                    //                        printf("Num %d; cursegment %d; abscount %d\n", m_iNumBlocks, m_iCurSegment, m_iRealPos);
                } else {
                    printf("error in firstRun\n");
                }
                iDone   += iRead; 
            } else {
                printf("Couldn't read from file\n");
            }
        }
        printf("\n");
        if (iResult == 0) {
            iResult = reduceSegments();
        }
        printf("iRes: %d; realPos %d (nr %d); feof %s\n", iResult, m_iRealPos, m_iNumRecs, feof(m_fIn)?"true":"false");
        m_pLimitIds[m_iNumBlocks-1] = idLast;
        if ((m_iCurSegment < m_iNumBlocks-1) && (m_pMinParents[m_iCurSegment] >= 0)) {
            m_iCurSegment++;
            printf("incincincincnicincincincincinc\n");
        }

        printf("numblocks %d, cursegment: %d\n", m_iNumBlocks, m_iCurSegment);
    } else {
        printf("Couldn't read positions\n");
    }
    m_mIdPos.clear();
    return iResult;
}


//----------------------------------------------------------------------------
// show
// 
void AncAnalyzer::show() {
    printf("NumBlocks: %d\n", m_iNumBlocks);
    printf("MinPars: ");
    for (unsigned int i  = 0; i < m_iNumBlocks; i++) {
        printf(" %d", m_pMinParents[i]);
    }
    printf("\n");
    /*
    printf("LimitIds: ");
    for (unsigned int i  = 0; i < m_iNumBlocks; i++) {
        printf(" %d", m_pLimitIds[i]);
    }
    printf("\n");
    */
    printf("Segments: ");
    for (unsigned int i = 0; i < m_vSegments.size(); i++) {
        printf(" %d", m_vSegments[i]);
    }
    printf("\n");
}

//----------------------------------------------------------------------------
// getPositions
// 
int AncAnalyzer::getPositions() {
    int iResult = 0;
    printf("getting positions\n");
    m_mIdPos.clear();
    m_mIdPos[0] = -1;
    int iTodo = 3*m_iNumRecs;
    fseek(m_fIn, 0, SEEK_SET);
    while ((iResult == 0) && (m_iRealPos < m_iNumRecs) && !feof(m_fIn) ) {
        int iSize = (iTodo>m_iBufSize)?m_iBufSize:iTodo;
        iTodo -= iSize;
        int iRead = fread(m_pReadBuf, sizeof(int), iSize, m_fIn);
        if (iRead == iSize)  {

            int iBuf = 0;
            while (iBuf < iSize) {
                m_mIdPos[m_pReadBuf[iBuf]] = m_iRealPos;

                iBuf +=3;
                m_iRealPos++;
            }
            
        } else {
            printf("Couldn't read from file\n");
            iResult = -1;
        }
    }
    printf("ended loop with res %d, realpos %d numrec %d, feof %s\n", iResult, m_iRealPos, m_iNumRecs, feof(m_fIn)?"true":"false");
    fseek(m_fIn, 0, SEEK_SET);
    m_iRealPos = 0;
    return iResult;

}
