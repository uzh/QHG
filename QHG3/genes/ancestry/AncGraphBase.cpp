#include <stdio.h>

#include "types.h"
#include "colors.h"
#include "BufWriter.h"
#include "BufReader.h"
#include "AncestorNode.h"
#include "AncGraphBase.h"


//----------------------------------------------------------------------------
// findAncestorNode
// 
AncestorNode *AncGraphBase::findAncestorNode(idtype iID) {
    AncestorNode *pFN = NULL;
    ancnodelist::const_iterator it = m_mIndex.find(iID);
    if (it != m_mIndex.end()) {
        pFN = it->second;
    }
    return pFN;
}


//----------------------------------------------------------------------------
// saveNode
// 
int AncGraphBase::saveNode(BufWriter *pBW, AncestorNode *pAN) {
    ANodeHeader sNodeBuf;

    sNodeBuf.iID = pAN->m_iID;
    sNodeBuf.iMomID = pAN->m_aParents[MOM];
    sNodeBuf.iDadID = pAN->m_aParents[DAD];
    sNodeBuf.iGender = pAN->m_iGender;
    sNodeBuf.iNumChildren =  (int) pAN->m_sChildren.size();
    
    pBW->addChars((char *)&sNodeBuf, sizeof(ANodeHeader));        
    idset::const_iterator it2;
    for (it2 = pAN->m_sChildren.begin(); it2 != pAN->m_sChildren.end(); ++it2) {
        idtype iC = *it2;
        pBW->addChars((char *) & iC, sizeof(idtype));
    }
    
    return 0;
}

//----------------------------------------------------------------------------
// readNode
// 
AncestorNode* AncGraphBase::readNode(BufReader *pBR) {
    int iResult = -1;
    ANodeHeader sNodeBuf;
    
    AncestorNode *pAN = NULL;
    iResult = pBR->getBlock((char *)&sNodeBuf, sizeof(ANodeHeader));
    if (iResult == 0) {
        idtype iID = sNodeBuf.iID;
        pAN = findAncestorNode(iID);
        if (pAN == NULL) {
            pAN = new AncestorNode(iID);
            m_mIndex[iID] = pAN;
        }
   
        pAN->setMom(sNodeBuf.iMomID);
        pAN->setDad(sNodeBuf.iDadID);
        pAN->m_iGender = sNodeBuf.iGender;
        int iNC = sNodeBuf.iNumChildren;
                      
        for (int j = 0; (iResult == 0) && (j < iNC); j++) {
            idtype iCC = 0;
            iResult = pBR->getBlock((char *)&iCC, sizeof(idtype));
            if (iResult == 0) {
                pAN->addChild(iCC);
            } else {
                printf("%sCouldn't read child #%d of %ld\n", RED, j, iID);
            }
        }
    } else {
        printf("%sCouldn't read node header\n", RED);
    }

    return pAN;
}

//----------------------------------------------------------------------------
// readNodeIf
//  - try to read header (5 ints: id, mom, dad, gender, numchildren)
//  - if ID matches, create and add a node, then read and children IDs  
//  - otherwise, skip over the children
//
int AncGraphBase::readNodeIf(BufReader *pBR, idtype iWantedID) {
    int iResult = -1;
    ANodeHeader sNodeBuf;
    
    AncestorNode *pAN = NULL;
    iResult = pBR->getBlock((char *)&sNodeBuf, sizeof(ANodeHeader));
    if (iResult == 0) {
        idtype iID = sNodeBuf.iID;

        int iNumChildren = sNodeBuf.iNumChildren;
        if (iID == iWantedID) {
            pAN = findAncestorNode(iID);
            if (pAN == NULL) {
                pAN = new AncestorNode(iID);
                m_mIndex[iID] = pAN;
            }
        
            pAN->setMom(sNodeBuf.iMomID);
            pAN->setDad(sNodeBuf.iDadID);
            pAN->m_iGender = sNodeBuf.iGender;
                      
            for (int j = 0; (iResult == 0) && (j < iNumChildren); j++) {
                int iCC = 0;
                iResult = pBR->getBlock((char *)&iCC, sizeof(idtype));
                if (iResult == 0) {
                    pAN->addChild(iCC);
                } else {
                    iResult = -1;
                    printf("%sCouldn't read child #%d of %ld\n", RED, j, iID);
                }
            }
        } else {
            iResult = -1;
            pBR->skip((uint)(iNumChildren*sizeof(idtype)));
        }
    } else {
        printf("%sCouldn't read node header\n", RED);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// readNodeIf
//  - try to read header (5 ints: id, mom, dad, gender, numchildren)
//  - if ID matches, create and add a node, then read and children IDs  
//  - otherwise, skip over the children
//
int AncGraphBase::readNodeIf2(BufReader *pBR, idtype iWantedID, bool bShow) {
    int iResult = -1;
    ANodeHeader sNodeBuf;
    
    AncestorNode *pAN = NULL;
    iResult = pBR->getBlock((char *)&sNodeBuf, sizeof(ANodeHeader));
    if (iResult == 0) {
        idtype iID = sNodeBuf.iID;
        if (bShow) printf("-- %ld\n", iID);
        int iNumChildren = sNodeBuf.iNumChildren;
        if (iID == iWantedID) {
            pAN = findAncestorNode(iID);
            if (pAN == NULL) {
                pAN = new AncestorNode(iID);
                m_mIndex[iID] = pAN;
            }
        
            pAN->setMom(sNodeBuf.iMomID);
            pAN->setDad(sNodeBuf.iDadID);
            pAN->m_iGender = sNodeBuf.iGender;
                      
            for (int j = 0; (iResult == 0) && (j < iNumChildren); j++) {
                int iCC = 0;
                iResult = pBR->getBlock((char *)&iCC, sizeof(idtype));
                if (iResult == 0) {
                    pAN->addChild(iCC);
                } else {
                    iResult = -1;
                    printf("%sCouldn't read child #%d of %ld\n", RED, j, iID);
                }
            }
        } else {
            iResult = -1;
            pBR->skip((uint)(iNumChildren*sizeof(idtype)));
        }
    } else {
        printf("%sCouldn't read node header\n", RED);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// writeListOffset
// 
int AncGraphBase::writeListOffset(const char *pFileName, long lListOffset) {
    int iResult = -1;
    FILE *fIn = fopen(pFileName, "r+");
    if (fIn != NULL) {
        fseek(fIn, 4*sizeof(char), SEEK_SET);
        ulong iWritten = fwrite(&lListOffset, sizeof(long), 1, fIn);
        if (iWritten == 1) {
            iResult = 0;
        } else {
            printf("%sCouldn't modify listoffset\n", RED);
        }
        fclose(fIn);
    } else {
        printf("%sCouldn't open outputfile [%s]\n", RED, pFileName);
    } 
    return iResult;
}

//----------------------------------------------------------------------------
// getListOffset
// 
long AncGraphBase::getListOffset(const char *pFileName) {
    long lListOffset = -1;

    FILE * fIn = fopen(pFileName, "rb");
    if (fIn != NULL) {
        fseek(fIn, 4*sizeof(char), SEEK_SET);
        ulong iRead = fread(&lListOffset, sizeof(long), 1, fIn);
        if (iRead == 1) {
            // OK
        } else {
            lListOffset = -1;
            printf("%sCouldn't read listoffset\n", RED);
        }
        
        fclose(fIn);
    } else {
        printf("%sCouldn't open agfile [%s]\n", RED, pFileName);
    }

    return lListOffset;
}


//----------------------------------------------------------------------------
// readIntSetBin
//   read int set from binary file (represented by BufReader)
//
int AncGraphBase::readIntSetBin(BufReader *pBR, intset &sData) {
    int iResult = -1;

    // read number of progenitors
    sData.clear();
    int iNumElems = -1;
    iResult = pBR->getBlock((char *)&iNumElems, sizeof(int));
    if (iResult == 0) {
        // read progenitors
        for (int j = 0; (iResult == 0) && (j < iNumElems); j++) {
            int iElem = 0;
            iResult = pBR->getBlock((char *)&iElem, sizeof(int));
            if (iResult == 0) {
                sData.insert(iElem);
            } else {
                printf("%sCouldn't read node id\n", RED);
            }
        }
    } else {
        printf("%sCouldn't read number of elements\n", RED);
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// readIDSetBin
//   read id set from binary file (represented by BufReader)
//
int AncGraphBase::readIDSetBin(BufReader *pBR, idset &sData) {
    int iResult = -1;

    // read number of progenitors
    sData.clear();
    int iNumElems = -1;
    iResult = pBR->getBlock((char *)&iNumElems, sizeof(int));
    if (iResult == 0) {
        // read progenitors
        for (int j = 0; (iResult == 0) && (j < iNumElems); j++) {
            idtype iElem = 0;
            iResult = pBR->getBlock((char *)&iElem, sizeof(idtype));
            if (iResult == 0) {
                sData.insert(iElem);
            } else {
                printf("%sCouldn't read node id #%d\n", RED, j);
            }
        }
    } else {
        printf("%sCouldn't read number of elements\n", RED);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// writeIDSetBin
//   read id set from binary file (represented by BufReader)
//
int AncGraphBase::writeIDSetBin(BufWriter *pBW, idset &sData) {
    int iResult = 0;
    idset_cit it2;
    int iNumElems = (int)sData.size();
    pBW->addChars((char *) &iNumElems, sizeof(int));
    for (it2 = sData.begin(); (iResult == 0) && (it2 != sData.end()); ++it2) {
        idtype iP = *it2;
        iResult = pBW->addChars((char *) &iP, sizeof(idtype));
    }
    return iResult;
}


//----------------------------------------------------------------------------
// clear
//   clean up
//
void AncGraphBase::clear(bool bDelete) {
    if (bDelete) {
        ancnodelist::const_iterator it;
        for (it = m_mIndex.begin(); it != m_mIndex.end(); ++it) {
            delete it->second;
        }
    }
    m_mIndex.clear();
    m_sSelected.clear();
    m_sRoots.clear();
}
