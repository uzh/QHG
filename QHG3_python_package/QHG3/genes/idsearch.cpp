#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <vector> 
#include <map>

#include "types.h"
#include "ParamReader.h"

#define BUFSIZE 1000000

//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - find a id in a binary file\n", pApp);
    printf("Usage:\n");
    printf("  %s --anc-file=<ancFile> --anc-size=<ancSize> --id=<id>[,<id>]* [--search-all]\n", pApp);
    printf("where\n");
    printf("  ancFile     binary anc file\n");
    printf("  ancSize     size of an anc record (number of longs)\n");
    printf("  id          id to search for (for hexadecimal ids:  precede with '0x')\n"); 
    printf("  search-all  search for all occurrences - don't stop at first\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s --anc-file=testAnc.anc4 --anc-size=4 --id=2099,0x2f9a,25282442,1\n", pApp);
    printf("\n");
}


//----------------------------------------------------------------------------
// searchBuf
//
int searchBuf(idtype *pBuf, int iAncSize, int iRead, idset &vIDs, bool bSearchAll, long lOffset, std::map<idtype, std::vector<long> > &vlPos) {
    int iResult = 0;

    for (idset::const_iterator it = vIDs.begin(); it != vIDs.end(); ++it) {
        idtype *p = pBuf;
        for (int i = 0; (i < iRead) && (bSearchAll || (vlPos[*it].size() == 0)); i++) {
            if (*p == *it) {
                vlPos[*it].push_back(i+lOffset);
            }
            p += iAncSize;
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char   *pAncFile = NULL;
    char   *pIDs     = NULL;
    int     iAncSize;
    bool    bSearchAll = false;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(4,
                                   "--anc-file:S!",  &pAncFile,
                                   "--id:S!",        &pIDs,
                                   "--anc-size:i",   &iAncSize,
                                   "--search-all:0", &bSearchAll);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                idset vIDs;
                char *p = strtok(pIDs,",|+");
                while ((p != NULL) && (iResult == 0)) {
                    char *pEnd;
                    idtype id = strtol(p, &pEnd, 10);
                    if (*pEnd == 'x') {
                        id = strtol(p, &pEnd, 16);
                    }
                    if (*pEnd == '\0') {
                        vIDs.insert(id);
                    } else {
                        printf("[%s] is neither a decimal nor a hex number\n",p);
                        iResult = -1;
                    }
                    p = strtok(NULL, ",|+");
                }
                
                if ((iResult == 0) && (vIDs.size() > 0)) {
                    if (iAncSize > 0) {
                        FILE *fIn = fopen(pAncFile, "rb");
                        if (fIn != NULL) {
                            iResult = 0;
                            
                            idtype *pBuf = new idtype[BUFSIZE*iAncSize];
                            int iRead = fread(pBuf, iAncSize, BUFSIZE, fIn);
                            std::map<idtype, std::vector<long> > vlPos;
                            long lOffset = 0;
                            bool bGoOn = true;
                            while ((iRead > 0) && (bSearchAll || bGoOn)) {
                                searchBuf(pBuf, iAncSize, iRead, vIDs, bSearchAll, lOffset, vlPos);
                                bGoOn = false;
                                for (idset::const_iterator it = vIDs.begin(); !bGoOn && (it != vIDs.end()); ++it) {
                                    if (vlPos[*it].size() == 0) {
                                        bGoOn = true;
                                    }
                                }
                                lOffset += iRead;
                                iRead = fread(pBuf, iAncSize, BUFSIZE, fIn);
                            }
                            delete[] pBuf;
                            fclose(fIn);
                            for (idset::const_iterator it = vIDs.begin(); it != vIDs.end(); ++it) {
                                idtype id = *it;
                                uint iNumFound = vlPos[id].size();
                                printf("Found %d occurrence%s of %ld [0x%lx] in [%s].", iNumFound, (iNumFound != 1)?"s":"", id, id, pAncFile);
                                if (iNumFound > 0) {
                                    printf(" Record%s at positions:\n", (iNumFound>0)?"s":"");
                                    for (uint i = 0; i < iNumFound; i++) {
                                        printf("  %ld [pos 0x%08x]\n", vlPos[id][i], (uint)(vlPos[id][i]*iAncSize*sizeof(idtype)));
                                    }
                                } else {
                                    printf("\n");
                                }
                            }
                        } else {
                            printf("Couldn't open [%s] for reading\n", pAncFile);
                            iResult = -1;
                        }
                    } else {
                        printf("Anc size has to be greater then 0\n");
                    iResult = -1;
                    }
                } else {
                    printf("ID has to be greater then 0\n");
                    iResult = -1;
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            printf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        printf("Couldn't create ParamReader\n");
    }

    return iResult;
}
