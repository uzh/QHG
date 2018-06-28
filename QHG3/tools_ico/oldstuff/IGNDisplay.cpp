#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "IcoGridNodes.h"
#include "IcoNode.h"


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 1) {
        double fFactor = 1;
        char sIGNFile[1024];
        *sIGNFile = '\0';
        if (iArgC > 2) {
            if (strcmp(apArgV[1], "-d") == 0) {
                strcpy(sIGNFile, apArgV[2]);
                fFactor = 180.0/M_PI;
            }
        } else {
            strcpy(sIGNFile, apArgV[1]);
        }
        IcoGridNodes *pIGN2 = new IcoGridNodes();
        iResult  = pIGN2->read(sIGNFile);
        if (iResult == 0) {
            
            std::map<gridtype, IcoNode*>::reverse_iterator rit = pIGN2->m_mNodes.rbegin();
            int iL = ceil((2.0+log2(rit->first))/4.0);
            iL = 2*((iL+1)/2);
            printf("iL is %d\n", iL);
            std::map<gridtype, IcoNode*>::const_iterator it;
 
            for (it = pIGN2->m_mNodes.begin();it != pIGN2->m_mNodes.end();it++) {
                IcoNode *pIN = it->second;
                printf("Node #%0*llx (T%0*llx) (%f,%f) A:%f: %d links\n", iL, pIN->m_lID, iL, pIN->m_lTID, pIN->m_dLon*fFactor, pIN->m_dLat*fFactor, pIN->m_dArea, pIN->m_iNumLinks);
                for (int i = 0; i < pIN->m_iNumLinks; i++) {
                    printf(" %0*llx (%f)",  iL, pIN->m_aiLinks[i], pIN->m_adDists[i]); 
                }
                printf("\n");
            }

        } else {
            printf("--- couldn't read [%s]\n", sIGNFile);
        }
        delete pIGN2;
    } else {
        printf("Usage: %s <ignfile>\n", apArgV[0]);
    }
    return iResult;
}


