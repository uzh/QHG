#include <stdio.h>
#include <math.h>
#include <string.h>
#include <map>
#include <string>


#include "utils.h"
#include "strutils.h"
#include "LineReader.h"
#include "BufReader.h"
#include "icoutil.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"

#define NUM_BLOCKS   10000
#define SINGLE_BLOCK  65536

#define IGN_MAGIC      "IGN3"
#define IGN_MAGIC2     "IGN4"
#define IGN_MAX_LINKS  "MAXLINKS"
#define IGN_SINGLE     "SINGLE"
#define IGN_TILED      "TILED"
#define IGN_HEADER_END "HEADER_END"
 
#define MAX_LINKS 6

//-----------------------------------------------------------------------------
// destructor
//
IcoGridNodes::~IcoGridNodes() {
    
    std::map<gridtype, IcoNode*>::iterator it;
    for (it = m_mNodes.begin(); it != m_mNodes.end(); ++it){
        if (it->second != NULL) {
            delete it->second;
        }
    }
}

    
//-----------------------------------------------------------------------------
// constructor
//
IcoGridNodes::IcoGridNodes() {

}

//-----------------------------------------------------------------------------
// write
//
int IcoGridNodes::write(const char *pOutput, int iMaxLinks, bool bAddTilingInfo, stringmap &mAdditionalHeaderLines) {
    int iResult = -1;
    FILE *fOut = fopen(pOutput, "wb");
    if (fOut != NULL) {
        fprintf(fOut, "%s\n",IGN_MAGIC2);
        fprintf(fOut, "%s %d\n", IGN_MAX_LINKS, iMaxLinks);
        fprintf(fOut, "%s\n", bAddTilingInfo?IGN_TILED:IGN_SINGLE);
        
        bool bVerbose = false;
        if (bVerbose) {
            printf("%s\n",IGN_MAGIC2);
            printf("%s %d\n", IGN_MAX_LINKS, iMaxLinks);
            printf("%s\n", bAddTilingInfo?IGN_TILED:IGN_SINGLE);
        }

        for (stringmap::const_iterator it = mAdditionalHeaderLines.begin(); it != mAdditionalHeaderLines.end(); ++it) {
            fprintf(fOut, "%s:%s\n", it->first.c_str(), it->second.c_str());
        }
        fprintf(fOut, "%s\n",IGN_HEADER_END);

        if ((iMaxLinks > 0) && !bAddTilingInfo) {
            iResult = blockWrite(fOut, MAX_LINKS/*iMaxLinks*/);
        } else {
            iResult = sequentialWrite(fOut, bAddTilingInfo);
        }
        fclose(fOut);
    } else {
        printf(" Couldn't open [%s] for writing\n", pOutput);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// blockWrite
//
int IcoGridNodes::blockWrite(FILE *fOut, int iMaxLinks) {
    // printf("BlockWrite\n");
    int   iResult = 0;
    ulong iCSize     = (iMaxLinks+2)*sizeof(gridtype) + (iMaxLinks+3)*sizeof(double) + 3*sizeof(int);
    ulong iNumCells  = m_mNodes.size();
    ulong iNumBlocks = (iNumCells>NUM_BLOCKS)?NUM_BLOCKS:iNumCells;
    unsigned char *pBuf = new unsigned char[iNumBlocks*iCSize];
    memset(pBuf, 0, iNumBlocks*iCSize);
    bool bDisplay = false;
    gridtype iEmpty = -1;
    double dEmpty = -1.0;
    ulong iTotal  = 0;
    ulong iBlocks = 0;
    unsigned char *p = pBuf;
    std::map<gridtype, IcoNode*>::const_iterator it = m_mNodes.begin();
    while ((iResult == 0) && (iTotal < iNumCells) && (it != m_mNodes.end())) {
        IcoNode *pN = it->second;
        if (pN->m_iNumLinks <= iMaxLinks) {
            p = putMem(p, &(pN->m_lID),       sizeof(gridtype));
            p = putMem(p, &(pN->m_lTID),      sizeof(gridtype));
            p = putMem(p, &(pN->m_dLon),      sizeof(double));
            p = putMem(p, &(pN->m_dLat),      sizeof(double));
            p = putMem(p, &(pN->m_dArea),     sizeof(double));
            p = putMem(p, &(pN->m_iZone),     sizeof(int));
            p = putMem(p, &(pN->m_iRegionID), sizeof(int));
            p = putMem(p, &(pN->m_iNumLinks), sizeof(int));

            if (bDisplay) {
                printf("\e[1;34m%d\e[0m ", pN->m_lID);
                printf("%f ", pN->m_dLon*180/M_PI);
                printf("%f\n", pN->m_dLat*180/M_PI);
                printf("  %d links:", pN->m_iNumLinks);
            }
            int D = iMaxLinks - pN->m_iNumLinks;
            memcpy(p, pN->m_aiLinks,  pN->m_iNumLinks*sizeof(gridtype));
            // jump to next location
            p +=  pN->m_iNumLinks*sizeof(gridtype);
            // fill up holes
            for (int i = 0; i < D; i++) {
                p = putMem(p, &iEmpty, sizeof(gridtype));
            }
            // copy the distances
            memcpy(p, pN->m_adDists,  pN->m_iNumLinks*sizeof(double));
            // jump to next location
            p += pN->m_iNumLinks*sizeof(double);
            // fill up holes 
            for (int i = 0; i < D; i++) {
                p = putMem(p, &dEmpty, sizeof(double));
            }
            if (bDisplay) {
                for (int i =0; i < pN->m_iNumLinks; i++) {
                    printf("%d; %f ", pN->m_aiLinks[i], pN->m_adDists[i]);
                }
                printf("\n");
            }
        } else {
            printf("Cell# %ld[ID %d] has %d links (MaxLinks given as %d)\n", iTotal,  pN->m_lID, pN->m_iNumLinks, iMaxLinks);
            iResult = -1;
        }
        if (iResult == 0) {
            iTotal++;
            if (iTotal%1000 == 0) {
                printf("\r                                 \rTotal Blocks %ld", iTotal);
            }
            iBlocks++;
            it++;
            if (iBlocks == iNumBlocks) {
                ulong iWritten = fwrite(pBuf, iCSize, iBlocks, fOut);  
                if (iWritten == iBlocks) {
                    p = pBuf;
                    iBlocks = 0;
                } else {
                    printf("Error: only written %ld blocks instead of %ld\n", iWritten, iBlocks);
                }
            }
        }
        
    }
    if (iResult == 0) {
        if (iBlocks > 0) {
            ulong iWritten = fwrite(pBuf, iCSize, iBlocks, fOut);  
            if (iWritten != iBlocks) {
                printf("Error: only written %ld blocks instead of %ld\n", iWritten, iBlocks);
            }   
        }
    }
    if (iTotal > 1000) {
        printf("*\n");
    }
    delete[] pBuf;
    return iResult;
}

//-----------------------------------------------------------------------------
// sequentialWrite
//
int IcoGridNodes::sequentialWrite(FILE *fOut, bool bAddTilingInfo) {
    int iResult = 0;
    unsigned char *pBuf = new unsigned char[SINGLE_BLOCK];
    printf("[IcoGridNodes::sequentialWrite] %s\n", bAddTilingInfo?"Tile":"Single");
    bool bDisplay = false;

    std::map<gridtype, IcoNode*>::const_iterator it = m_mNodes.begin();
    while ((iResult == 0) && (it != m_mNodes.end())) {
        unsigned char *p = pBuf;
        IcoNode *pN = it->second;
        p = putMem(p, &(pN->m_lID),       sizeof(gridtype));
        p = putMem(p, &(pN->m_lTID),      sizeof(gridtype));
        p = putMem(p, &(pN->m_dLon),      sizeof(double));
        p = putMem(p, &(pN->m_dLat),      sizeof(double));
        p = putMem(p, &(pN->m_dArea),     sizeof(double));
        p = putMem(p, &(pN->m_iZone),     sizeof(int));
        p = putMem(p, &(pN->m_iRegionID), sizeof(int));
        p = putMem(p, &(pN->m_iNumLinks), sizeof(int));
        // copy the links
        p = putMem(p, pN->m_aiLinks, pN->m_iNumLinks*sizeof(gridtype));
        // copy the distances
        p = putMem(p, pN->m_adDists, pN->m_iNumLinks*sizeof(double));
        
            if (bDisplay) {
                printf("%d ", pN->m_lID);
                printf("%f ", pN->m_dLon);
                printf("%f\n", pN->m_dLat);
                printf("  %d links:", pN->m_iNumLinks);
                for (int i =0; i < pN->m_iNumLinks; i++) {
                    printf("%d; %f ", pN->m_aiLinks[i], pN->m_adDists[i]);
                }
                printf("\n");
                if (bAddTilingInfo) {
                    /*
                    printf("  type %d, region %d, dests:", pN->m_iType, pN->m_iRegionID);
                    std::set<gridtype>::const_iterator it;
                    for(it = pN->m_sDests.begin(); it != pN->m_sDests.end(); it++) {
                        printf("  %d", *it);
                    }
                    printf("\n");
                    */
                }

            }

        if (bAddTilingInfo) {
            /*
            p = putMem(p, &(pN->m_iType),         sizeof(unsigned int));
            p = putMem(p, &(pN->m_iRegionID),     sizeof(int));
            size_t iDestSize = pN->m_sDests.size();
            p = putMem(p, &(iDestSize), sizeof(size_t));
            std::set<gridtype>::const_iterator its;
            for (its = pN->m_sDests.begin(); its != pN->m_sDests.end(); its++) {
                gridtype lll = *its;
                p = putMem(p, &(lll), sizeof(gridtype));
            }
            */
        }
        ulong iNumBytes = p - pBuf;
        ulong iWritten = fwrite(pBuf, 1, iNumBytes, fOut);  
        if (iWritten != iNumBytes) {
            printf("Error: only written %ld bytes instead of %ld\n", iWritten, iNumBytes);
        }
        it++;
     }

    delete[] pBuf;
    return iResult;
}


//-----------------------------------------------------------------------------
// readHeader 
//
int IcoGridNodes::readHeader(FILE *fIn, int  *piMaxLinks, bool *pbAddTilingInfo, stringmap *pAdditionalHeader, bool *pbVer2) {
    int iResult = -1;
 
    //@@    printf("Before starting at %lu\n", ftell(fIn));
    LineReader *pLR = LineReader_std::createInstance(fIn);
    if (pLR != NULL) {
        char *p = pLR->getNextLine();
        if (p != NULL) {
            //@@            printf("After first line at %lu\n", ftell(fIn));
            if ((strcmp(trim(p), IGN_MAGIC) == 0) || (strcmp(trim(p), IGN_MAGIC2) == 0)) {
                *pbVer2 = (strcmp(trim(p), IGN_MAGIC2) == 0);
                p = pLR->getNextLine();
                if (p != NULL) {
                    //@@                    printf("After second line at %lu\n", ftell(fIn));
                    char sTag[32];
                    int iMaxLinks;
                    int iRead = sscanf(p, "%s %d", sTag, &iMaxLinks);
                    if (iRead == 2) {
                        p = pLR->getNextLine();
                        if (p != NULL) {
                    //@@        printf("After third line at %lu\n", ftell(fIn));
                            char sTag2[32];
                            iRead = sscanf(p, "%s", sTag2);
                            if (iRead == 1) {
                                iResult = 0;
                                bool bAddTilingInfo = false;
                                if (strcmp(sTag, IGN_SINGLE) == 0) {
                                    bAddTilingInfo = false;
                                } else if (strcmp(sTag2, IGN_TILED) == 0) {
                                    bAddTilingInfo = true;
                                } else {
                                    printf("unknown tag [%s]\n", p);
                                    iResult = -1;
                                }
                                if (iResult == 0) {
                                    // save header data if requires
                                    if (piMaxLinks != NULL) {
                                        *piMaxLinks = iMaxLinks;
                                    }
                                    if (pbAddTilingInfo != NULL) {
                                        *pbAddTilingInfo = bAddTilingInfo;
                                    }
                                 //@@   int iCC = 0;
                                    // now continue to read lines until header ends
                                    bool bGoOn = true;
                                    p = pLR->getNextLine();
                        //@@            printf("starting loop\n");
                                    while ((p != NULL) && (iResult == 0) && bGoOn) {
                            //@@            printf("After %dth line at %lu\n", iCC+4, ftell(fIn));iCC++;
                                //@@        printf("Have p=[%s]\n", p);
                                        if (strcmp(p, IGN_HEADER_END) == 0) {
                                            bGoOn = false;
                                        } else {
                                            if (pAdditionalHeader != NULL) {
                                                char *p1 = strchr(p, ':');
                                                if (p1 != NULL) {
                                                    *p1='\0';
                                                    p1++;
                                    //@@                printf("put %s -> %s\n", trim(p), trim(p1));
                                                    (*pAdditionalHeader)[trim(p)] = trim(p1);
                                                } else {
                                                    printf("Missing ':'' in additional header line [%s]\n", p);
                                                    iResult = -1;
                                                }
                                            }
                                            p = pLR->getNextLine();
                                        }
                                    }
                                    if (bGoOn) {
                                        iResult = -1;
                                        printf("No end of header found SINGLE\n");
                                    }
                                }
                            } else {
                                printf("No valid SINGLE/TILED line [%s]\n", p);
                            }
                        } else {
                            printf("No SINGLE/TILED line\n");
                        }
                    } else {
                        printf("No valid MAXLINKS line [%s]\n", p);
                    }
                } else {
                    printf("No MAXLINKS line\n");
                }
            } else {
                printf("No IGN Header [%s]\n", p);
            }
        } else {
            printf("No header\n");
        }
        delete pLR;
    } else {
        printf("empty file?\n");
    }
    return iResult;
}

 
//-----------------------------------------------------------------------------
// read
//
int IcoGridNodes::read(const char *pInput) {
    int iResult = -1;
    int iMaxLinks = -1;
    bool bAddTilingInfo = false;
    bool bVer2 = true;

    m_smData.clear();

    FILE *fIn = fopen(pInput, "rb");
    if (fIn  != NULL) {

        iResult = readHeader(fIn, &iMaxLinks, &bAddTilingInfo, &m_smData, &bVer2);
        //@@unsigned long pos1 = ftell(fIn);
        //@@        printf("After header at position %lu\n", pos1);
        if (iResult == 0) {
            // read data
            if ((iMaxLinks > 0) && !bAddTilingInfo) {
                // read blocks
                blockRead(fIn, MAX_LINKS/*iMaxLinks*/, bVer2);
            } else {
                sequentialRead(fIn, bAddTilingInfo, bVer2);
            }
        } 
        fclose(fIn);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// blockRead
//
int IcoGridNodes::blockRead(FILE *fIn, int iMaxLinks, bool bVer2) {
//    printf("new blokkread\n");
    int iResult = 0;
    int iCount = 0;
    bool bExitOK = false;
    ulong iCSize = (iMaxLinks+2)*sizeof(gridtype)+(iMaxLinks+3)*sizeof(double)+3*sizeof(int);
    BufReader *pBR = BufReader::createInstance(fIn, (uint)(NUM_BLOCKS*iCSize));
    if (pBR != NULL) {
        int iC = 0;
        while (iResult == 0) {
            gridtype lID   = -1;
            gridtype lTID  = -1;
            double dLon  = dNaN;
            double dLat  = dNaN;
            double dArea = dNaN;
            int iNumLinks = -1;
            IcoNode *pIN;
            char pX[64];
            iResult = pBR->getBlock(pX, sizeof(gridtype));
            if (iResult == 0) {
                getMem(&lID, pX, sizeof(gridtype));
                iResult = pBR->getBlock(pX, sizeof(gridtype));
                if (iResult == 0) {
                    getMem(&lTID, pX, sizeof(gridtype));
                    iResult = pBR->getBlock(pX, sizeof(double));
                    if (iResult == 0) {
                        getMem(&dLon, pX, sizeof(double));
                        iResult = pBR->getBlock(pX, sizeof(double));
                        if (iResult == 0) {
                            getMem(&dLat, pX, sizeof(double));
                            iResult = pBR->getBlock(pX, sizeof(double));
                            if (iResult == 0) {
                                getMem(&dArea, pX, sizeof(double));
                                pIN = new IcoNode(lID, lTID, dLon, dLat, dArea);

                                if (bVer2) {
                                    iResult = pBR->getBlock(pX, sizeof(int));
                                    if (iResult == 0) {
                                        getMem(&(pIN->m_iZone), pX, sizeof(int));
                                        iResult = pBR->getBlock(pX, sizeof(int));
                                        if (iResult == 0) {
                                            getMem(&(pIN->m_iRegionID), pX, sizeof(int));
                                            
                                        } else {
                                            iResult = -1;
                                            printf("Corrupted file - break after Region ID\n");
                                        }
                                    } else {
                                        iResult = -1;
                                        printf("Corrupted file - break after Area\n");
                                    }
                                }

                                iResult = pBR->getBlock(pX, sizeof(int));
                                if (iResult == 0) {
                                    getMem(&iNumLinks, pX, sizeof(int));
                                    
                                    std::vector<gridtype> vIds;
                                    
                                    iCount++;
                                    // we must pick up iMaxLinks ids
                                    for (int i = 0; (iResult == 0) && (i < iMaxLinks); i++) {
                                        iResult = pBR->getBlock(pX, sizeof(gridtype));
                                        // but only add the links of the first iNumLinks
                                        if (iResult == 0)  {
                                            if (i < iNumLinks) {
                                                gridtype iL;
                                                getMem(&iL, pX, sizeof(gridtype));
                                                vIds.push_back(iL);
                                            }
                                        } else {
                                            iResult = -1;
                                            printf("Corrupted file - break in link list (#%d)\n", iC);
                                        }
                                    }
                                    if (iResult == 0) {
                                        // we must pick up iMaxLinks distances
                                        for (int i = 0; (iResult == 0) && (i < iMaxLinks); i++) {
                                            iResult = pBR->getBlock(pX, sizeof(double));
                                            // but only add the links of the first iNumLinks
                                            if (iResult == 0)  {
                                                if (i < iNumLinks) {
                                                    double dDist;
                                                    getMem(&dDist, pX, sizeof(double));
                                                    pIN->addLink(vIds[i], dDist);
                                                }
                                            } else {
                                                iResult = -1;
                                                printf("Corrupted file - break in dist list\n");
                                            }
                                        }
                                    }
                                    

                                } else {
                                    iResult = -1;
                                    printf("Corrupted file - break after Area (count %d) id %d, tid %d, (%f,%f) A %f\n", iC, lID, lTID, dLon, dLat, dArea);
                                }
                            } else {
                                iResult = -1;
                                printf("Corrupted file - break after Lat\n");
                            }
                        } else {
                            iResult = -1;
                            printf("Corrupted file - break after Lon\n");
                        }
                    } else {
                        iResult = -1;
                        printf("Corrupted file - break after TID\n");
                    }
                } else {
                    iResult = -1;
                    printf("Corrupted file - break after ID\n");
                }
                
            } else {
//                printf("End of data?\n");
                bExitOK = true;
            }    
            if (iResult == 0) {
                m_mNodes[pIN->m_lID] = pIN;
            }
            iC++;
        }
        
        delete pBR;
    } else {
        iResult = -1;
        printf("Couldn't create read buffer\n");
    }
    
    if (bExitOK) {
        iResult = 0;
    }
//    printf("Loaded %d items; res %d\n", iCount, iResult);
    return iResult;
}



//-----------------------------------------------------------------------------
// sequentialRead
//
int IcoGridNodes::sequentialRead(FILE *fIn, bool bAddTilingInfo, bool bVer2) {
    int iResult = 0;
    bool bExitOK = false;
    BufReader *pBR = BufReader::createInstance(fIn, SINGLE_BLOCK);
    if (pBR != NULL) {
        while (iResult == 0)  {
            gridtype lID = -1;
            gridtype lTID = -1;
            double dLon  = dNaN;
            double dLat  = dNaN;
            double dArea = dNaN;
            int iNumLinks = -1;
            IcoNode *pIN;
            char pX[64];
            iResult = pBR->getBlock(pX, sizeof(gridtype));
            if (iResult == 0) {
                getMem(&lID, pX, sizeof(gridtype));
                iResult = pBR->getBlock(pX, sizeof(gridtype));
                if (iResult == 0) {
                    getMem(&lTID, pX, sizeof(gridtype));
                    iResult = pBR->getBlock(pX, sizeof(double));
                    if (iResult == 0) {
                        getMem(&dLon, pX, sizeof(double));
                        iResult = pBR->getBlock(pX, sizeof(double));
                        if (iResult == 0) {
                            getMem(&dLat, pX, sizeof(double));
                            iResult = pBR->getBlock(pX, sizeof(double));
                            if (iResult == 0) {
                                getMem(&dArea, pX, sizeof(double));
                                pIN = new IcoNode(lID, lTID, dLon, dLat, dArea);
                                
                                if (bVer2) {
                                    iResult = pBR->getBlock(pX, sizeof(int));
                                    if (iResult == 0) {
                                        getMem(&(pIN->m_iZone), pX, sizeof(int));
                                        iResult = pBR->getBlock(pX, sizeof(int));
                                        if (iResult == 0) {
                                            getMem(&(pIN->m_iRegionID), pX, sizeof(int));
                                        } else {
                                            iResult = -1;
                                            printf("Corrupted file - break after Region ID\n");
                                        }
                                    } else {
                                        iResult = -1;
                                        printf("Corrupted file - break after Area\n");
                                    }
                                }

                                iResult = pBR->getBlock(pX, sizeof(int));
                                if (iResult == 0) {
                                    getMem(&iNumLinks, pX, sizeof(int));
                                    std::vector<gridtype> vIds;
                                    for (int i = 0; (iResult == 0) && (i < iNumLinks); i++) {
                                        
                                        iResult = pBR->getBlock(pX, sizeof(gridtype));
                                        if (iResult == 0) {
                                            gridtype iL;
                                            getMem(&iL, pX, sizeof(gridtype));
                                            vIds.push_back(iL);
                                        } else {
                                            iResult = -1;
                                            printf("Corrupted file - break in link list\n");
                                        }
                                    }
                                    if (iResult == 0) {
                                        for (int i = 0; (iResult == 0) && (i < iNumLinks); i++) {
                                            
                                            iResult = pBR->getBlock(pX, sizeof(double));
                                            if (iResult == 0) {
                                                double dDist;
                                                getMem(&dDist, pX, sizeof(double));
                                                pIN->addLink(vIds[i], dDist);
                                            } else {
                                                iResult = -1;
                                                printf("Corrupted file - break in link list\n");
                                            }
                                        }
                                    }
                                } else {
                                    iResult = -1;
                                    printf("Corrupted file - break after Area\n");
                                }
                            } else {
                                iResult = -1;
                                printf("Corrupted file - break after Lat\n");
                            }
                        } else {
                            iResult = -1;
                            printf("Corrupted file - break after Lon\n");
                        }
                    } else {
                        iResult = -1;
                        printf("Corrupted file - break after TID\n");
                    }
                } else {
                    iResult = -1;
                    printf("Corrupted file - break after ID\n");
                }
            } else {
                bExitOK = true;
            }        
        
            if (iResult == 0) {
                if (bAddTilingInfo) {
                    /*

                    iResult = pBR->getBlock(pX, sizeof(unsigned int));
                    if (iResult == 0) {
                        getMem(&(pIN->m_iType), pX, sizeof(unsigned int));
                    
                        iResult = pBR->getBlock(pX, sizeof(int));
                        if (iResult == 0) {
                            getMem(&(pIN->m_iRegionID), pX, sizeof(int));
                            iResult = pBR->getBlock(pX, sizeof(size_t));
                            if (iResult == 0) {
                                size_t iDests;
                                getMem(&iDests, pX, sizeof(size_t));
                                for (unsigned int i = 0; (iResult == 0) && (i < iDests); i++) {
                                    iResult = pBR->getBlock(pX, sizeof(gridtype));
                                    if (iResult == 0) {
                                        gridtype iR;
                                        getMem(&iR, pX, sizeof(gridtype));
                                        pIN->m_sDests.insert(iR);
                                    } else {
                                        iResult = -1;
                                        printf("Corrupted file - break in Destination IDs\n");
                                    }
                                }
                            
                            } else {
                                iResult = -1;
                                printf("Corrupted file - break before num dests\n");
                            }
                        } else {
                            iResult = -1;
                            printf("Corrupted file - break before region ID\n");
                        }
                    } else {
                        iResult = -1;
                        printf("Corrupted file - break before type\n");
                    }
                
                    */
                }
            }
            if (iResult == 0) {
                m_mNodes[pIN->m_lID] = pIN;
            }
        }


        delete pBR;
    } else {
        iResult = -1;
        printf("Couldn't create read buffer\n");
    }
    if (bExitOK) {
        iResult = 0;
    }
    
    return iResult;
}



//-----------------------------------------------------------------------------
// setTiledIDs
//
int IcoGridNodes::setTiledIDs( std::map<gridtype,gridtype> &mID2T) {
    int iResult = 0;
    std::map<gridtype, IcoNode*>::const_iterator it;
    for (it = m_mNodes.begin(); (iResult == 0) && (it != m_mNodes.end()); it++) {
        std::map<gridtype, gridtype>::const_iterator it2 = mID2T.find(it->first);
        if (it2 != mID2T.end()) {
            it->second->m_lTID = it2->second;
        } else {
            printf("Unmatched entry ID: %d can't be found in translation map\n", it->first);
            iResult = -1;
        }
    }
    return iResult;
}
