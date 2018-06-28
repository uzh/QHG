#include <stdio.h>
#include <string.h>

#include "LineReader.h"
#include "GeneUtils.h"
#include "GeneWriter2.h"
#include "IDSample.h"
#include "PlinkReader.h"

//----------------------------------------------------------------------------
// creatInstance
//
PlinkReader*PlinkReader::createInstance(const char *pPlinkFile, int iGenomeSize) {
    PlinkReader *pPR = new PlinkReader(iGenomeSize);
    int iResult = pPR->init(pPlinkFile);
    if (iResult != 0) {
        delete pPR;
        pPR = NULL;
    }
    return pPR;
}


//----------------------------------------------------------------------------
// constructor
//
PlinkReader::PlinkReader(int iGenomeSize)
    : m_iCurID(0),
      m_iGenomeSize(iGenomeSize),
      m_pLR(NULL),
      m_pIDSample(new IDSample){

    genomelist::const_iterator it;
    for (it = m_mGenomeList.begin();it != m_mGenomeList.end(); ++it) {
        delete[] it->second;
    }
}


//----------------------------------------------------------------------------
// destructor
//
PlinkReader::~PlinkReader() {
    if (m_pLR != NULL) {
        delete m_pLR;
    }
    if (m_pIDSample != NULL) {
        delete m_pIDSample;
    }

    genomelist::const_iterator it;
    for (it = m_mGenomeList.begin(); it != m_mGenomeList.end(); ++it) {
        delete[] it->second;
    }
    
}


//----------------------------------------------------------------------------
// init
//
int PlinkReader::init(const char *pPlinkFile) {
    int iResult = -1;
    m_pLR = LineReader_std::createInstance(pPlinkFile, "rt", 4*m_iGenomeSize+1024);
    if (m_pLR != NULL) {
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getSequence 
//   (from SequenceProvider)
//
const ulong *PlinkReader::getSequence(idtype iID) {
    ulong *pGenome = NULL;
    genomelist::const_iterator it = m_mGenomeList.find(iID);
    if (it != m_mGenomeList.end()) {
        pGenome = it->second;
    }
    return pGenome;
}

//----------------------------------------------------------------------------
// registerID 
//   if the the plink id is already registered return the corresponding qhg id
//   otherwise register it and return the new qhg id
//
idtype PlinkReader::registerID(std::string sPlinkID) {
    idtype iCurID = 0; 
    string2id::const_iterator it = m_mPlink2ID.find(sPlinkID);
    if (it != m_mPlink2ID.end()) {
        iCurID = it->second;
    } else {
        m_iCurID++;
        iCurID = m_iCurID;

        m_mID2Plink[iCurID]    = sPlinkID;
        m_mPlink2ID[sPlinkID]  = iCurID;
    }
    return iCurID;
}

//----------------------------------------------------------------------------
// readGenomes 
//
int PlinkReader::readGenomes() {
    int iResult = 0;


    char sFam[128];
    char sSamp[128];
    char sPID[128];
    char sMID[128];
    char sSex[128];
    char sAff[128];
    char *pRest = new char[2 * 2 *m_iGenomeSize]; //2 alleles, and spaces
    char *pNucs = new char[2*m_iGenomeSize]; //2 alleles, and spaces
    memset(pNucs,  0, 2*m_iGenomeSize);

    int iLineNo = 1;
    
    char *pLine = m_pLR->getNextLine(GNL_IGNORE_ALL);
    while ((iResult == 0) && (pLine != NULL) && (!m_pLR->isEoF())) {
        printf("doing line %d\n", iLineNo);
        int iRead =sscanf(pLine,"%s %s %s %s %s %s %[^\n]", sFam, sSamp, sPID, sMID, sSex, sAff, pRest);
        if (iRead == 7){
            int iCount = 0;
            char *pCur  = pRest;
            char *pG1   = pNucs;
            char *pG2   = pNucs+m_iGenomeSize;
            for (uint i = 0; i < strlen(pRest); i++) {
                if ((*pCur =='A') || (*pCur =='C') || (*pCur =='G') || (*pCur =='T')) {
                    
                    if ((iCount%2) == 0) {
                        *pG1++ = *pCur;
                    } else {
                        *pG2++ = *pCur;
                    }
                    //     printf("increasing count\n");
                    iCount++;
                }
                //        printf("increasing cur\n");
                pCur++;
            }

            
            ulong *pGenomes = GeneUtils::translateGenome(m_iGenomeSize, pNucs);
            idtype iID = registerID(sSamp);
            m_mGenomeList[iID] = pGenomes;
            m_mPlinkLocs[iID] = sFam;
            // now handle mother & father ids
            idtype iPatID = registerID(sPID);
            idtype iMatID = registerID(sMID);
            
            agdata *pad = new agdata();
            pad->iID = iID;
            pad->iMomID = iMatID;
            pad->iDadID = iPatID;
            pad->iGender = (strcmp(sSex, "2") == 0)?0:1;
            pad->iCellID = -1;
            pad->dLon = -1;
            pad->dLat = -1;
            pad->iArrayPos = iCount;
            m_pIDSample->addAgentData(sFam, 0, pad);

            locitem li;
            li.dLon  = -1;
            li.dLat  = -1;
            li.dDist =  0;
            li.iNum  =  1; 
            li.iPadding = 0;

            m_mLocData[sFam] = li;

            iLineNo++;
        } else {
            printf("Error scaning line #%d\n", iLineNo);
            iResult =-1;

        }
        pLine = m_pLR->getNextLine(GNL_IGNORE_ALL);
    }
    delete[] pRest;
    delete[] pNucs;
    return iResult;
}


//----------------------------------------------------------------------------
// writeBin 
//
int PlinkReader::writeBin(const char *pOutput) {
    int iResult = 0;
    
    iResult = GeneWriter2::writeSequence(FORMAT_BIN, this, pOutput,  m_mLocData,  m_pIDSample, true, false);
    return iResult;
        
        

}
