#include <stdio.h>
#include <string.h>

#include <vector>

#include "types.h"
#include "colors.h"
#include "BufWriter.h"
#include "BufReader.h"
#include "AncestorNode.h"
#include "RWAncGraph.h"
#include "AGFileMerger.h"


//-----------------------------------------------------------------------------
// constructor
//
AGFileMerger::AGFileMerger() {
}

//-----------------------------------------------------------------------------
// destructor
//
AGFileMerger::~AGFileMerger() {
    for (uint i = 0; i < m_vAGInfo.size(); ++i) {
        if (m_vAGInfo[i] != NULL) {
            if (m_vAGInfo[i]->m_pAG != NULL) {
                delete m_vAGInfo[i]->m_pAG;
            }
            if (m_vAGInfo[i]->m_pBR != NULL) {
                delete m_vAGInfo[i]->m_pBR;
            }
            delete m_vAGInfo[i];
        }
    }

}


//-----------------------------------------------------------------------------
// createInstance
//
AGFileMerger *AGFileMerger::createInstance(std::vector<std::string> &vAGFileNames, int iNumLoad) {
    AGFileMerger *pAGM = new AGFileMerger();
    int iResult = pAGM->init(vAGFileNames, iNumLoad);
    if (iResult != 0) {
        delete pAGM;
        pAGM = NULL;
    }
    return pAGM;
}

//-----------------------------------------------------------------------------
// openFirst
//
aginfo *AGFileMerger::openFirst(const char *pAGFile) {
    aginfo *pagi = NULL;
    int iResult = -1;
    BufReader *pBR = BufReader::createInstance(pAGFile, 2048*m_iNumLoad);
    if (pBR != NULL) {
        char sMagic[3];
        // read and check magic
        iResult = pBR->getBlock(sMagic, 4);
        if ((iResult == 0) && (strncmp(sMagic, "AGRB", 4) == 0)) {
            // read number of nodes
            long lListOffset;
            iResult = pBR->getBlock((char*)&lListOffset, sizeof(long));
            if (iResult == 0) {
                ulong iNumNodes;
                iResult = pBR->getBlock((char*)&iNumNodes, sizeof(long));
                if (iResult == 0) {
                    RWAncGraph *pAG = new RWAncGraph();
                    pagi = new aginfo(pAG, pBR, iNumNodes);
                } else {
                    printf("%s[AGFileMerger] couldn't read number of nodes from AG file [%s]\n", RED, pAGFile);
                }
            } else {
                printf("%s[AGFileMerger] couldn't listoffset from AG file [%s]\n", RED, pAGFile);
            }
        } else {
            iResult = -1;
            printf("%s[AGFileMerger] couldn't read from AG file [%s] or bad magic\n", RED, pAGFile);
        }
    } else {
        printf("%s[AGFileMerger] couldn't open AG file [%s]\n", RED, pAGFile);
    }

    return pagi;
}


//-----------------------------------------------------------------------------
// loadNodes
//
int AGFileMerger::loadNodes(aginfo *pagi) {
    int iResult = 0;

    int iNumLoad = m_iNumLoad;
    if (pagi->m_iNumToDo > 0) {
        if (iNumLoad > pagi->m_iNumToDo) {
            iNumLoad = pagi->m_iNumToDo;
        }

        for (int i = 0; (iResult == 0) && (i < iNumLoad); i++) {
            if (pagi->m_pAG->readNode(pagi->m_pBR) == NULL) {
                iResult = -1;
            }
        }
        if (iResult == 0) {
            pagi->m_iNumToDo -= iNumLoad;
            //            printf("successfully loaded %d AncestorNodes\n", iNumLoad);
        } else {
            printf("%serror reading node\n", RED);
            
        }
    } else {
        //        printf("nothing else to do; closing BR\n");
        
        // no more nodes: close BufReader
        delete pagi->m_pBR;
        pagi->m_pBR = NULL;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// init
//
int AGFileMerger::init(std::vector<std::string> &vAGFileNames, int iNumLoad) {
    int iResult = 0;
    m_iNumLoad = iNumLoad;
    m_iNumTotals = 0;
    for (uint i = 0; (iResult == 0) && (i < vAGFileNames.size()); ++i) {
        //        printf("Opening [%s]\n", vAGFileNames[i].c_str());
        aginfo *pagi = openFirst(vAGFileNames[i].c_str());
        if (pagi != NULL) {
            if (pagi->m_pAG != NULL) {
                m_iNumTotals += pagi->m_iNumToDo;
                iResult = loadNodes(pagi); 
                if (iResult == 0) {
                    m_vAGInfo.push_back(pagi);
                } else {
                    delete pagi->m_pAG;
                    delete pagi->m_pBR;
                    delete pagi;
                    pagi = NULL;
                }
            }
        } else {
            iResult = -1;
            printf("%scouldn't read ag info from [%s]\n", RED, vAGFileNames[i].c_str());
        }
    }
    if (iResult == 0) {
        printf("[AGFileMerger] will merge a total of %ld nodes\n", m_iNumTotals);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// merge
//  assumptions:
//    - the AGs are saved fragments of a larger AG 
//      i.e. a node may reference parents or children in another fragment
//    - no two AGs have any node id in common
//    - the set of roots of the larger AG is equal to its set of progenitors
//
int AGFileMerger::merge(const char *pOutputAG, int iBufSize, idset &sSelected, idset &sRoots) {
    int iResult = 0;

    BufWriter *pBW = BufWriter::createInstance(pOutputAG, iBufSize);
    if (pBW != NULL) {
        bool bHasMore = true;
        
        // get info for header & write it
        // here we use  the disjointness of the AGs
        pBW->addLine("AGRB");
        long lListOffset = 0;
        pBW->addChars((char *)&lListOffset, sizeof(long));        
        pBW->addChars((char *)&m_iNumTotals, sizeof(long));        
        idset sRoots1;
        int iSavedNodes = 0;
        // loop until all nodes are gone
        while (bHasMore && (iResult == 0)) {
            idtype iMinID = -1;
            uint iMinIndex = 0;

            // look for smallest node if
            for (uint i = 0; i < m_vAGInfo.size(); ++i) {
                if (!m_vAGInfo[i]->m_pAG->getMap().empty()) {
                    idtype iID = m_vAGInfo[i]->m_pAG->getMap().begin()->first;
                    if ((iMinID < 0) || (iID < iMinID)) {
                        iMinID = iID;
                        iMinIndex = i;
                    }
                }
            }
   
            if (iMinID >= 0) {
                // save node
                AncestorNode *pAN = m_vAGInfo[iMinIndex]->m_pAG->getMap().begin()->second;
                if ((pAN->getMom() == 0) && (pAN->getDad() == 0)) {
                    sRoots1.insert(iMinID);
                }
                iResult = RWAncGraph::saveNode(pBW, pAN);
                iSavedNodes++;

                // delete node
                delete pAN;
                m_vAGInfo[iMinIndex]->m_pAG->getModifiableMap().erase(m_vAGInfo[iMinIndex]->m_pAG->getModifiableMap().begin());

                // reload nodes if necessary
                if (m_vAGInfo[iMinIndex]->m_pAG->getMap().size() == 0) {
                    iResult = loadNodes(m_vAGInfo[iMinIndex]);
                    if (iResult == 0) {
                        // loaded OK
                    } else {
                        // loadNodes error
                    }
                }
            } else {
                // no MinID found -> all AGs empty
                bHasMore = false;
            }
        }
        lListOffset = pBW->getPos();
        printf("AGFileMerger: saved %d nodes, promised %lu\n", iSavedNodes,m_iNumTotals);
        
        // add roots
        printf("Adding %zd roots\n", sRoots.size());
        AncGraphBase::writeIDSetBin(pBW, sRoots);
        // "dummy" selected
        printf("Adding %zd dummy (selected)\n", sSelected.size());
        AncGraphBase::writeIDSetBin(pBW, sSelected);
        // progenitors
        printf("Adding %zd progenitors (roots)\n", sRoots.size());
        AncGraphBase::writeIDSetBin(pBW, sRoots);
        // leaves
        AncGraphBase::writeIDSetBin(pBW, sSelected);
        printf("Adding %zd leaves (selected)\n", sSelected.size());
        
        delete pBW;
        iResult = AncGraphBase::writeListOffset(pOutputAG, lListOffset);
    } else {
        iResult = -1;
        printf("%sCouldn't open output file [%s]\n", RED, pOutputAG);
    }
    
    return iResult;
}
