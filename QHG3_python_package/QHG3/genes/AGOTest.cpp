#include <stdio.h>
#include <stdlib.h>

#include "AncestorNode.h"
#include "AncGraphBase.h"
#include "RWAncGraph.h"
#include "AGOracle.h"


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 3) {
        int iNumBlocks = atoi(apArgV[2]);
        if (iNumBlocks > 0) {

            intset sIDs;
            // collect ids
            for (int c = 3; (iResult == 0) && (c < iArgC); ++c) {
                int h = atoi(apArgV[c]);
                if (h > 0) {
                    sIDs.insert(h);
                } else {
                    printf("IDs must be positive numbers\n");
                    iResult = -1;
                }
            }

            if (iResult == 0) {
                AGOracle *pAGO = AGOracle::createInstance(apArgV[1], iNumBlocks);
                if (pAGO != NULL) {
                    RWAncGraph *pAG = new RWAncGraph();
                    
                    iResult = pAGO->loadNodes(pAG, sIDs);
                    
                    if (iResult == 0) {
                        printf("AG now contains the nodes:\n");
                        const ancnodelist &mIndex = pAG->getMap();
                        ancnodelist::const_iterator it;
                        for (it = mIndex.begin(); it != mIndex.end(); ++it) {
                            AncestorNode *pAN = it->second;
                            printf("  Node %d: Parents [%d,%d], num children %zd\n", pAN->m_iID, pAN->getMom(), pAN->getDad(), pAN->m_sChildren.size());
                        }
                    } else {
                        printf("Loading of IDs failed\n");
                    }
                        
                    delete pAG;
                    delete pAGO;
                } else {
                    printf("Couldn't open AGOracle from [%s]\n", apArgV[1]);
                }
            }
        } else {
            printf("Numblocks must be a positive number\n");
        }
    } else {
        printf("usage: %s <agfile> <numblocks> <id>*\n",apArgV[0]);
        iResult = -1;
    }
    

    return iResult;
}
