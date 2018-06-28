#include <stdio.h>
#include <string.h>
#include <map>
#include <set>
#include <omp.h>

#include "types.h"
#include "strutils.h"
#include "colors.h"
#include "BufReader.h"

#include "AncestorNode.h"
#include "AncGraphBase.h"
#include "RWAncGraph.h"
#include "AGOracle.h"

#define MAXBUFSIZE 100000000


//----------------------------------------------------------------------------
// createInstance
//
AGOracle *AGOracle::createInstance(const char *pAGFile, uint iBlockSize) {
    AGOracle *pAGO = new AGOracle();
    int iResult = pAGO->init(pAGFile, iBlockSize);
    if (iResult != 0) {
        delete pAGO;
        pAGO = NULL;
    }
    return pAGO;
}

//----------------------------------------------------------------------------
// constructor
//
AGOracle::AGOracle() 

    : m_fIn(NULL),
      m_pBR(NULL),
      m_iNumNodes(0),
      m_iBlockSize(0),
      
      m_iNumThreads(0),
      m_iNumBlocks(0),
      m_asBlockIDs(NULL),
      m_afIn(NULL),
      m_apAG(NULL),
      m_aiIndexes(NULL) {
}

//----------------------------------------------------------------------------
// destructor
//
AGOracle::~AGOracle() {
    if (m_pBR != NULL) {
        delete m_pBR;
    }
    if (m_fIn != NULL) {
        fclose(m_fIn);
    }

    for (int i = 0; i < m_iNumThreads; ++i) {
        fclose(m_afIn[i]);
        delete m_apAG[i];
        delete m_apBR[i];
    }
    delete[] m_afIn;
    delete[] m_apAG;
    delete[] m_apBR;
    delete[] m_asBlockIDs;
    delete[] m_aiIndexes;
    delete[] m_aiCounts;
}


//----------------------------------------------------------------------------
// init
//
int AGOracle::init(const char *pAGFile, uint iBlockSize) {
    int iResult = -1;
    
    uint iBufSize = iBlockSize;
    if (iBufSize > MAXBUFSIZE) {
        iBufSize = MAXBUFSIZE;
    }
    printf("[AGOracle] BlockSize %u\n", iBlockSize);

    // open file, check magic, read number of nodes
    m_fIn = fopen(pAGFile, "rb");
    if (m_fIn != NULL) {

        m_pBR = BufReader::createInstance(m_fIn, iBufSize);
        if (m_pBR != NULL) {
            // try to read header...
            char sMagic[3];
            // read and check magic
            iResult = m_pBR->getBlock(sMagic, 4);
            if ((iResult == 0) && (strncmp(sMagic, "AGRB", 4) == 0)) {
                long lListOffset=0;
                iResult = m_pBR->getBlock((char*)&lListOffset, sizeof(long));
                if (iResult == 0) {
                    iResult = m_pBR->getBlock((char*)&m_iNumNodes, sizeof(long));
                    if (iResult == 0) {
                        //                        printf("[AGOracle] found num nodes: %d\n", m_iNumNodes);
                        // move past the magic, the listoffset, and the number of nodes
                        fseek(m_fIn, 4+2*sizeof(long), SEEK_SET);
                    } else {
                        printf("%sCouldn't read number of nodes\n", RED);
                    }
                } else {
                    printf("%sCouldn't read list offset\n", RED);
                }
            } else {
                printf("%sCouldn't read file, or bad magic\n", RED);
            }
        } else {
            printf("%sCouldn't create BufReader for file [%s]\n", RED, pAGFile);
        }
    } else {
        printf("%sCouldn't open file [%s]\n", RED, pAGFile);
    }

    // scan through nodes and mark position of every N-th entry
    if (iResult == 0) {
        m_iBlockSize = iBlockSize;
        //        printf("[AGOracle] has %d blocks, mostly with size: %d\n", m_iNumNodes/m_iBlockSize, m_iBlockSize);

        int iC = 1; 
        idtype iID = -1;
        idtype iLastID = -1;
        // entries start after the magic, the listoffset, and the number of nodes
        long lPrevPos = 4+2*sizeof(long);
        for (uint i = 0; (iResult == 0) && (i < m_iNumNodes); i++) {
            iID = scanNextNode();
            if (iID > 0) {
                if ((iC > 1) && (((iC) % m_iBlockSize) == 0)) {
                    long lCurPos =  ftell(m_fIn);
                    m_mList[iID] = ancrange(lPrevPos, lCurPos);
                    //                    printf("Mark ID %d -> [%ld, %ld]\n", iID, lPrevPos, lCurPos);
                    lPrevPos = lCurPos;
                }
		if (iLastID >= iID) {
		    printf("Last: %ld, Cur: %ld\n", iLastID, iID);
		}
                iLastID = iID;
                ++iC;
            } else {
                printf("%sCouldn't read node\n", RED);
                iResult = -1;
            }
        }
        // last ID to mark the end
        if (iResult == 0) {
            long lCurPos =  ftell(m_fIn);
            m_mList[iLastID] = ancrange(lPrevPos, lCurPos);
            //            printf("Mark ID %d -> [%ld, %ld]\n", iID, lPrevPos, lCurPos);
        }
    }
        
    if (iResult == 0) {
        m_iNumThreads = omp_get_max_threads();
        m_afIn = new FILE*[m_iNumThreads];
        m_apAG = new RWAncGraph*[m_iNumThreads];
        m_apBR = new BufReader*[m_iNumThreads];

        m_iNumBlocks = m_mList.size();
        m_asBlockIDs = new blockset[m_iNumBlocks];
        ancinfo_cit ita;
        int iC = 0;
        for (ita = m_mList.begin(); ita != m_mList.end(); ++ita) {
            m_asBlockIDs[iC++].iIDFirst = ita->first;
        }
        m_aiIndexes = new int[m_iNumThreads];
        m_aiCounts  = new int[m_iNumThreads];

#pragma omp parallel
        {
            int iT = omp_get_thread_num();
            m_afIn[iT] = fopen(pAGFile, "rb");
            m_apAG[iT] = new RWAncGraph();
            m_apBR[iT] = BufReader::createInstance(m_afIn[iT], iBufSize);
            m_aiIndexes[iT] = iT;
            m_aiCounts[iT] = 0;
        }
    } 

    return iResult;
}


//----------------------------------------------------------------------------
// loadNodes
//  Load the ancestor nodes for the specified set.
//
int AGOracle::loadNodes(AncGraphBase *pAGB, idset sNodeIDs) {
    int iResult = 0;
    
    // move past the magic, the listoffset, and the number of nodes
    fseek(m_fIn, 4+2*sizeof(long), SEEK_SET);

    // find blocks containing node IDs
    idset sBlocks;
    idset_it its;
    for (its = sNodeIDs.begin(); its != sNodeIDs.end(); ++its) {
        ancinfo_cit itm = m_mList.lower_bound(*its);
        if (itm != m_mList.end()) {
            sBlocks.insert(itm->first);
        } else {
            printf("%sno upper bound found for id %ld\n", RED, *its);
            iResult = -1;
        }
    }

    int iCount = 0;
    its = sNodeIDs.begin();
    idset_cit itB = sBlocks.begin();
    while ((iResult == 0) && (itB != sBlocks.end())) {

        long lMin = m_mList[*itB].first;
        long lMax = m_mList[*itB].second;
        long lCur = lMin;
        
        // load block
        fseek(m_fIn, lMin, SEEK_SET);
        m_pBR->loadFromCurrentPos();
        // search for ID
        while ((iResult == 0) && (its != sNodeIDs.end()) && (lCur < lMax)) {

            iResult = pAGB->readNodeIf(m_pBR, *its);
            if (iResult == 0) {
                //found it - go for next ID
               ++its;
               ++iCount;
            } else {
                // not finding is not a real error
                iResult = 0;
            }
            // in any case, skip to next record
            lCur = lMin+m_pBR->getCurPos();
        }
        // block done - go for next block
        ++itB;
    }
    // went through all blocks - all IDs should have been loaded
    if (its != sNodeIDs.end()) {
        printf("%sCouldn't find all IDs: %ld\n", RED, *its);
        iResult = -1;
    }

    if (iResult != 0) {
        printf("Loaded %d/%zd nodes\n", iCount, sNodeIDs.size());
    }
    return iResult;
}



//----------------------------------------------------------------------------
// loadNodesPar
//  Load the ancestor nodes for the specified set.
//
int AGOracle::loadNodesPar(AncGraphBase *pAGB, idset sNodeIDs) {
    int iResult = 0;
    
    // move past the magic, the listoffset, and the number of nodes
    fseek(m_fIn, 4+2*sizeof(long), SEEK_SET);

    // find blocks containing node IDs
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < m_iNumBlocks; ++i) {
        idset_it its;
        for (its = sNodeIDs.begin(); its != sNodeIDs.end(); ++its) {
            ancinfo_cit itm = m_mList.lower_bound(*its);
            if (itm != m_mList.end()) {
                if (itm->first == m_asBlockIDs[i].iIDFirst) {
                    m_asBlockIDs[i].sLocalIDs.insert(*its);
                }
            } else {
                printf("%sno upper bound found for id %ld\n", RED, *its);
                iResult = -1;
            }
        }
    }

    // get node data and create nodes
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < m_iNumBlocks; ++i) {
        int iRes = 0;
        int iT = omp_get_thread_num();
        idtype iIDFirst = m_asBlockIDs[i].iIDFirst;
        long lMin = m_mList[iIDFirst].first;
        idset_it its = m_asBlockIDs[i].sLocalIDs.begin();
              
        fseek(m_afIn[iT], lMin, SEEK_SET);
        m_apBR[iT]->loadFromCurrentPos();

        while ((iRes == 0) && (its != m_asBlockIDs[i].sLocalIDs.end()) && (*its <= iIDFirst)) {

            iRes = m_apAG[iT]->readNodeIf(m_apBR[iT], *its);
            if (iRes == 0) {
               ++its;
               ++m_aiCounts[iT];
            } else {
                // not finding is not a real error
                iRes = 0;
            }
        }

        // is everything ok?
        if (its != m_asBlockIDs[i].sLocalIDs.end()) {
            printf("%sT%d ,%d: Couldn't find all IDs: %ld\n", RED, iT, i, *its);
            iRes = -1;
        }

        // clean local set for next round        
        m_asBlockIDs[i].sLocalIDs.clear();

        // set main result code if necessary
        if (iRes != 0) {
            iResult = iRes;
        }
    }

    // merge the mnaps created by the threads into the main map:
    // successively merge pairs of maps in parallel;
    // merge final one with main map
    if (iResult == 0) {
        int iD = 1;
        int iNumMaps = m_iNumThreads;
        while (iNumMaps > 1) {
            //            printf("Next stage, num maps %d\n", iNumMaps);
            // merge in parallel
#pragma omp parallel for
            for (int j = 0; j < iNumMaps/2; ++j) {
                int k1=m_aiIndexes[2*j*iD];
                int k2=m_aiIndexes[(2*j+1)*iD];
                m_apAG[k1]->getModifiableMap().insert(m_apAG[k2]->getMap().begin(),m_apAG[k2]->getMap().end()); 
            }
            iNumMaps = (iNumMaps+1)/2;
            iD *= 2;
        }
        // now merge with main
        pAGB->getModifiableMap().insert(m_apAG[0]->getMap().begin(),m_apAG[0]->getMap().end());
    }
    
    
#pragma omp parallel 
    {
        m_apAG[omp_get_thread_num()]->clear(false);
    }
    
    if (iResult != 0) {
        int iCount = 0;
        for (int i = 0; i < m_iNumThreads; ++i) {
            iCount += m_aiCounts[i];
            m_aiCounts[i] = 0;
        }
        
        printf("Loaded %d/%zd nodes\n", iCount, sNodeIDs.size());
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// scanNextNode
//
idtype AGOracle::scanNextNode() {
    idtype iID = -1;
    ANodeHeader aN;

    //    printf("At position %ld: ", ftell(m_fIn));
    size_t iRead = fread((char *)&aN, sizeof(ANodeHeader), 1, m_fIn);
    if (iRead == 1) {
        iID = aN.iID;
        if (iID > 0) {
            //            printf("%d [%d,%d] %d\n", aData[0], aData[1], aData[2], aData[4]);
            int iNC = aN.iNumChildren;
            fseek(m_fIn, iNC*sizeof(idtype), SEEK_CUR);
        } else {
            //            printf("()\n");
        }
    } else {
        printf("%sCouldn't read node at pos %ld\n", RED, ftell(m_fIn));
    }
    return iID;
}

//----------------------------------------------------------------------------
// scanNextNode
//
int AGOracle::loadBlock(AncGraphBase *pAGB, idtype iNodeID, idtype *piCurMinID, idtype *piCurMaxID) {
    int iResult = 0;
    idtype iBlock = -1;
    *piCurMinID = -1;
    *piCurMaxID = -1;

    /*
    // move past the magic, the listoffset, and the number of nodes
    fseek(m_fIn, 4+sizeof(long)+sizeof(int), SEEK_SET);
    */
    ancinfo_cit itm = m_mList.lower_bound(iNodeID);
    if (itm != m_mList.end()) {
        iBlock = itm->first;
        long lMin = m_mList[iBlock].first;

        uint iC = 0;
        fseek(m_fIn, lMin, SEEK_SET);
        m_pBR->loadFromCurrentPos();
        while ((iResult == 0) && (iC < m_iBlockSize) && (iC < m_iNumNodes)) {
            AncestorNode *pAN = pAGB->readNode(m_pBR);
            if (pAN != NULL) {
                *piCurMaxID = pAN->m_iID;
                if (*piCurMinID < 0) {
                    *piCurMinID = pAN->m_iID;
                }
                iC++;

            } else {
                iResult = -1;
                printf("%sError while reading node\n", RED);
            }
        }

    } else {
        printf("%sno upper bound found for id %ld\n", RED, iNodeID);
        iResult = -1;
    }
    return iResult;
}
