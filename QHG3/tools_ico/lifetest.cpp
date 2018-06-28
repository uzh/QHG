#include <stdio.h>
#include <sys/types.h> // getpid
#include <unistd.h>    // getpid

#include "strutils.h"
#include "ParamReader.h"
#include "LineReader.h"
#include "LifeBoard.h"


//-----------------------------------------------------------------------------
// copyTileTo
//   copy the data from tile outputfiles to the correct position in pTotal
//   
//   pTotal   target buffer
//   pTile    name of file to read data from
//   iW       width of tile (should be the same for all calls)
//   iH       height of tile (should be the same for all calls)
//   iNX      number of tiles in X direction
//   iNY      number of tiles in Y direction
//   iX       X 'coordinate' of tile
//   iY       Y 'coordinate' of tile
//
int copyTileTo(uchar *pTotal,  const char *pTile, int iW, int iH, int iNX, int iNY, int iX, int iY) {
    int iResult = -1;
    FILE *fIn = fopen(pTile, "rb");
    if (fIn != NULL) {
        int iSizes[2];
        int iRead = fread(iSizes, sizeof(int), 2, fIn);
        if (iRead == 2) {
            iResult = 0;
            for (int i = 0; (iResult == 0) && (i < iH); i++) {
                iRead =  fread(&(pTotal[(iY*iH+i)*iW*iNX+iX*iW]), sizeof(uchar), iW, fIn);
                if (iRead != iW) {
                    iResult = -1;
                    printf("Couldn't read line\n");
                }
            }
        } else {
            printf("Expected <W><H> as first entries\n"); 
            iResult = -1;
        }

        fclose(fIn);
        remove(pTile);
    } else {
        printf("Couldn't open [%s] for reading\n", pTile); 
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// saveTotal
//   save the big array as a binary file with name pOutput
//
int saveTotal(uchar *pTotal, int iTotalW, int iTotalH,  const char *pOutput) {
    int iResult = -1;

    FILE *fOut = fopen(pOutput, "wb");
    if (fOut != NULL) {
        /*
        fwrite(&iTotalW, sizeof(int), 1, fOut);
        fwrite(&iTotalH, sizeof(int), 1, fOut);
        */
        iResult = 0;
        for (int i = 0; (iResult == 0) && (i < iTotalH); i++) {
            int iWritten = fwrite(pTotal+i*iTotalW, sizeof(uchar), iTotalW, fOut);
            if (iWritten != iTotalW) {
                iResult = -1;
                printf("Couldn't write line\n");
            }
        }
        
        fclose(fOut);
        if (iResult != 0) {
            remove(pOutput);
        }
    } else {
        printf("Couldn't open [%s] for writing\n", pOutput);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// splitSizeExtString
//   splits a string of the form <W>x<H> or <W>x<H>:<A> into its parts
//   and returns them in piW, piH (and piAdd)
//
int splitSizeString(char *pSize, int *piW, int *piH, int *piAdd) {
    int iResult = 0;
    
    // check if there is a ':'
    char *pAdd = strchr(pSize, ':');
    if (pAdd != NULL) {
        *pAdd = '\0';
        pAdd++;
        if (piAdd != NULL) {
            if (strToNum(pAdd, piAdd)) {
                // ok
            } else {
                iResult = -1;
            }
        }
    }

     
    if (iResult == 0) {
        // now split the size into W and H
        char *pSep = strchr(pSize, 'x');
        if (pSep != NULL) {
            *pSep = '\0';
            pSep++;
            iResult = -1;
            if (strToNum(pSize, piW)) {
                if (strToNum(pSep, piH)) {
                    iResult = 0;
                }
            }
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// readData
//   read the coordinate pairs from the text file sData, apply offsets
//   and save in vData, if the (global) coordinates are inthis tile.
//   The offsets are passed along withe file name, separated by ':'
//
int readData(char *sData, int iTile, int iNX, int iNY, int iW, int iH, std::vector<std::pair<int,int> > &vData) {
    int iResult = 0;

    int iOffsetX = 0;
    int iOffsetY = 0;
    // get the offsets if specified
    char *pSep1 = strchr(sData, ':');
    if (pSep1 != NULL) {
        *pSep1 = '\0';
        ++pSep1;
        iResult = -1;
        char *pSep2 = strchr(pSep1, ':');
        if (pSep2 != NULL) {
            *pSep2 = '\0';
            ++pSep2;
            if (strToNum(pSep1, &iOffsetX)) {
                if (strToNum(pSep2, &iOffsetY)) {
                    iResult = 0;
                    printf("Have offsets (%d,%d)\n", iOffsetX, iOffsetY);
                } else {
                    printf("OfffsetY is not a number: [%s]\n", pSep2);
                }
            } else {
                printf("OfffsetX is not a number: [%s]\n", pSep1);
            }
            
        } else {
            // need second number
            printf("Must provide 2 offsets: <filename>:<offsX>:<offsY>\n");
        }
    }
        
    if (iResult == 0) {
        LineReader *pLR = LineReader_std::createInstance(sData, "rt");
        if (pLR != NULL) {
            char *p = pLR->getNextLine();
            while ((iResult == 0) && (p!= NULL)) {
                int iX = -1;
                int iY = -1;
                printf("[%d] have line [%s]\n", iTile, p);
                int iNum = sscanf(p, "%d %d", &iX, &iY);
                if (iNum == 2) {
                    // apply offsets
                    iX += iOffsetX;
                    iY += iOffsetY;
                    // find tile 'coordinates' and local coordinate of point
                    int itX = iX / iW;
                    int iX1 = iX - itX*iW;
                    int itY = iY / iH;
                    int iY1 = iY - itY*iH;
                    //                printf("tile (%d, %d): %d,%d\n", itX, itY, iX1, iY1);
                    if ((itX < iNX) && (itY < iNY)) {
                        if (itY*iNX+itX == iTile) {
                            printf("tile (%d, %d): %d,%d\n", itX, itY, iX1, iY1);
                            vData.push_back(std::pair<int, int>(iX1, iY1));
                        }
                        p = pLR->getNextLine();
                    } else {
                        iResult = -1;
                        printf("Coordinates (%d,%d) outside of field\n", iX, iY);
                    }
                } else {
                    iResult = -1;
                    printf("Bad line in data file [%s]\n", p);
                }
                
            }
            delete pLR;
        } else {
            iResult = -1;
            printf("Couldn't open data file [%s]\n", sData);
        }
    }
    return iResult;
}



//-----------------------------------------------------------------------------
// readAllData
//
int readAllData(char *sData, int iTile, int iNX, int iNY, int iW, int iH, std::vector<std::pair<int,int> > &vData) {
    int iResult = 0;
    char *p = strtok(sData, ",");
    
    while ((iResult == 0) && (p != NULL)) {
        iResult = readData(p, iTile, iNX, iNY, iW, iH, vData);
        p = strtok(NULL, ",");
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - Testing MPI Life\n", pApp);
    printf("Usage:\n");
    printf("  %s -s <NX>x<NY> -t <W>x<H>:<halo> -s <data> [-n <numsteps>]\n", pApp);
    printf("     [-i <saveinterval> -o <outputbody>\n");
    printf("where\n");
    printf("  NX        number of tiles in X direction\n");
    printf("  NY        number of tiles in Y direction\n");
    printf("  W         width of a tile\n");
    printf("  H         height of a tile\n");
    printf("  halo      halo size\n");
    printf("  data      data files and offsets\n");
    printf("            Format:\n");
    printf("            data ::= <dataItem>[,<dataItem>]*\n");
    printf("            dataItem ::= <file>[:<OffsetX>:<OffsetY>]\n");
    printf("            file     text file with a single coordinate pair on each line\n");
    printf("            OffsetX  amount to shift coordinates from file in x direction\n");
    printf("            OffsetY  amount to shift coordinates from file in Y direction\n");
    printf("  numsteps  number of steps\n");
    printf("  saveinterval  number of steps between saving data\n");
    printf("  outputbody    stem of output files - the step will be added to it:\n");
    printf("                 <outputbody>_T<step>.out\n");
    printf("\n");
    printf("Format of data file:\n");
    printf("  lines of the form \"X Y\"\n");
    printf("  (X,Y) coordinates of cell to set to 1 (local coordinates; can be shifted with offsets)\n");
    printf("\n");
}





//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult= 0;

    int          iTile;
    int          iNumProcs;

    char *sTiling   = NULL;
    char *sTileInfo = NULL;
    char *sData     = NULL;
    char *sOutput   = NULL;
    int iNumSteps   = 0;
    int iInterval   = 0;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(6,
                               "-s:S!",    &sTiling,
                               "-t:S!",    &sTileInfo,
                               "-d:S!",    &sData, 
                               "-n:i",     &iNumSteps,
                               "-o:S",     &sOutput,
                               "-i:i",     &iInterval);

    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
           
            int iNX   = -1;
            int iNY   = -1;
            int iW    = -1;
            int iH    = -1; 
            int iHalo = -1;

            if ((iInterval == 0) || (sOutput != NULL)) {
                iResult = splitSizeString(sTiling, &iNX, &iNY, NULL);
                if (iResult == 0) {
                    iResult = splitSizeString(sTileInfo, &iW, &iH, &iHalo);
                    if (iResult == 0) {
                        char sTemplate[256];
                        std::vector<std::pair<int,int> > vData;
                    
                    
                        printf("MPI start\n"); fflush(stdout);
                        // MPI Initialisation
                        MPI_Init(&iArgC, &apArgV);
                        
                        // Get the number of MPI tasks and the taskid of this task
                        MPI_Comm_rank(MPI_COMM_WORLD, &iTile);
                        MPI_Comm_size(MPI_COMM_WORLD, &iNumProcs);
                        
                        if (iNumProcs != iNX*iNY) {
                            printf("Number of procs should be %d\n", iNX*iNY);
                            iResult = -1;
                        }
                        
                        if (iResult == 0) {
                            iResult = readAllData(sData, iTile, iNX, iNY, iW, iH, vData);
                        }

                        if (iResult == 0) {
                            printf("MPI: rank %d, num %d, proc %d\n", iTile, iNumProcs, getpid()); fflush(stdout);
                            LifeBoard *pLB = LifeBoard::createInstance(iTile, iNX, iNY, iW, iH, iHalo);
                            if (pLB != NULL) {
                                iResult = pLB->setPattern(vData);
                                
                                pLB->showPattern(-1);
                                
                                for (int z = 0; z < iNumSteps; z++) {
                                    printf("--- step %d ---\n", z);
                                    pLB->doStep();
                                    
                                    pLB->showPattern(z);
                                    if ((iInterval > 0) && (z%iInterval == 0)) {
                                        sprintf(sTemplate, "%s_%d_%%d.out", sOutput, z);
                                        
                                        pLB->writeCurrent(sTemplate);
                                    }
                                }
                                
                                delete pLB;
                            }
                            
                            // now merge tilefiles
                            if (iTile == 0) {
                                uchar *pTotal = new uchar[iNX*iW * iNY*iH];
                                memset(pTotal, 0, iNX*iW*iNY*iH*sizeof(uchar)); 
                                for (int z = 0; z < iNumSteps; z++) {
                                    if ((iInterval > 0) && (z%iInterval == 0)) {
                                        sprintf(sTemplate, "%s_%d_%%d.out", sOutput, z);
                                        for (int i = 0; i < iNX*iNY; i++) {
                                            int iX = i % iNX;
                                            int iY = i / iNX;
                                            char sFile[256];
                                            sprintf(sFile, sTemplate, i);
                                            iResult = copyTileTo(pTotal, sFile, iW, iH, iNX, iNY, iX, iY);
                                        }
                                        char sOutFile[256];
                                        sprintf(sOutFile, "%s_T%d.out", sOutput, z);
                                        iResult = saveTotal(pTotal, iW*iNX, iH*iNY, sOutFile);
                                    }
                                }
                            }
                            
                        } else {
                            printf("Couldn't read data from [%s]\n", sData);
                        }
                        
                        MPI_Finalize();
                    } else {
                        printf("Couldn't split TileInfo [%s]\n", sTileInfo);
                    }
                } else {
                    printf("Couldn't split Tiling [%s]\n", sTiling);
                }
            } else {
                printf("If an interval is given, an output file bod must be specified\n");
            }
            
            
        } else {
            printf("%s\n", pPR->getErrorMessage(iResult).c_str());
            printf("-----\n");
            usage(apArgV[0]);
        }
        
    } else {
        printf("Error in setOptions\n");
    }
    delete pPR;
    

    return iResult;
}


