#include <stdio.h>
#include <string.h>
#include <math.h>

#include "strutils.h"
#include "ParamReader.h"
#include "LineReader.h"
#include "GeneUtils.h"
#include "GridSampler.h"


typedef std::map<std::string, std::vector<idtype> > named_ids;
typedef std::map<idtype, std::pair<double,double> > id_locs;
typedef std::map<std::string, std::pair<double,double> > named_locs;


//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("  %s - testing GridSampler\n", pApp);
    printf("Usage:\n");
    printf("  %s -g <GeoQDF> -m <StatQDF> -p <PopQDF> -s  <SpeciesName>\n", pApp);
    printf("     -r <DeltaLon>:<DeltaLat>:<Dist>\n");
    printf("     -o <output>\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// readStandardAsc
//   read a ASCII genome file as created by GeneWriter (via QDFSampler)
//
const char **readStandardAsc(const char *pGenomeFile, int *piGenomeSize, int *piNumGenes,  
                             named_ids &mvIDs, std::map<int,int> &mNodeIDs, id_locs &mIdLocs, named_locs &mNamedLocs) {
     const char **pGenomes = NULL;
     int iResult = 0;
     int iGOffset = 0;
     bool bFull = false;
     std::vector<std::string> vG;
     // we assume that the genomesize is less than 16384
     LineReader *pLR = LineReader_std::createInstance(pGenomeFile, "rt", 16384+256);
     std::string sCur = "(single)";

     if (pLR != NULL) {
         // header line
         char *pLine = pLR->getNextLine(GNL_IGNORE_BLANKS);
         char sMode[256];
         int iRead = sscanf(pLine, "# GENES %s G-OFFSET %d", sMode, &iGOffset);
         if (iRead == 2) {
             bFull = (strcmp(sMode, "full") == 0);
             printf("We have %s; g-offset %d\n", bFull?"full":"reduced", iGOffset);
         } else {
             printf("Bad header line [%s]\n", pLine);
             iResult = -1;
         }
         pLine = pLR->getNextLine(GNL_IGNORE_BLANKS);
         while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
             
             if (pLine[strlen(pLine)-1] == '\n') {
                 pLine[strlen(pLine)-1] = '\0';
             }
             // does a new group start?
             if (strstr(pLine, "# GROUP") == pLine) {
                 char sName[256];
                 char sName1[256];
                     
                 double dLon = 0;
                 double dLat  = 0; 
                 char *p1 = strchr(pLine, '[');
                 if (p1 != NULL) {
                     *p1 = '\0';
                     p1++;
                 }
                 char *p2 = strrchr(p1, ']');
                 if (p2 != NULL) {
                     *p2 = '\0';
                     p2++;
                 }
                 // get some stuff from group header
                 int iRead = sscanf(pLine, "# GROUP %s", sName1);
                     if (iRead == 1) {

                     sprintf(sName, "%s[%s]", sName1, p1);
                     iRead = sscanf(p2, " (%lf, %lf)", &dLon, &dLat);
                     if (iRead == 2) {
                         sCur = sName;
                         mNamedLocs[sCur] = std::pair<double, double>(dLon, dLat);
                         pLine = pLR->getNextLine(GNL_IGNORE_BLANKS);
                         //@@                         printf("reading for group [%s]\n", sCur.c_str()); fflush(stdout);
                     }
                 }
             } else {
                 // it's a data lie
                 long iID;
                 int iMomID  = 0;
                 int iDadID  = 0;
                 int iGender = 0;
                 int iNodeID = 0;
                 double dLon = 0;
                 double dLat = 0;
                 int iRequired = 0;
                 int iRead = 0;
                 if (bFull) {
                     iRequired = 7;
                     iRead = sscanf(pLine, "%ld %d %d %d %d %lf %lf", &iID, &iMomID, &iDadID, &iGender, &iNodeID, &dLon, &dLat);
                 } else {
                     iRequired = 4;
                     iRead = sscanf(pLine, "%ld %d %lf %lf", &iID, &iNodeID, &dLon, &dLat);
                 }
                 if (iRead == iRequired) {
                     mvIDs[sCur].push_back(iID);
                     mNodeIDs[iID] = iNodeID;
                     mIdLocs[iID] = std::pair<double,double>(dLon, dLat);
                     char *sG = new char[strlen(pLine)+1];
                     memset(sG, 0, strlen(pLine)+1);
                     char *pS = sG;
                     // asc standard format: genomes begin at position iGOffset
                     pLine += iGOffset;

                     int iC1 = 0;
                     while (*pLine != '\0') {
                         if ((*pLine == 'A') || (*pLine == 'C') || (*pLine == 'G') || (*pLine == 'T')) {
                             *pS = *pLine;
                             pS++;
                             iC1++;
                         }
                         pLine++;
                     }
                     vG.push_back(sG);
                     delete[] sG;
                     pLine = pLR->getNextLine(GNL_IGNORE_BLANKS);
                     
                 } else {
                     printf("Couldn't read ID [%s]\n", pLine);
                     iResult =-1;
                 }
             }
         }
         delete pLR;
     } else {
         printf("Couldn't open [%s] for reading\n", pGenomeFile);
         iResult =-1;
     }
      
     if (iResult == 0) {
         *piNumGenes = vG.size();
         *piGenomeSize = -1;
         // check whether all genomes have same size
         for (uint i = 0; (iResult == 0) && (i < vG.size()); i++) {
             int iS = vG[i].size()/2;
             if (*piGenomeSize < 0) {
                 *piGenomeSize = iS;
             } else {
                 if (*piGenomeSize != iS) {
                     printf("Size Mismatch for genome #%d (%d != %d)\n", i, *piGenomeSize, iS);
                 }
             }
         }
  
         // allocate array and copy strings
         if (iResult == 0) {
             pGenomes = new const char*[*piNumGenes];
             for (int i = 0; i < *piNumGenes; i++) {
                 pGenomes[i] = strdup(vG[i].c_str());
             }
         }
        

         if (mvIDs.size() > 1) {
             named_ids::iterator it = mvIDs.find("(single)");
             if (it != mvIDs.end()) {
                 mvIDs.erase(it);
             }
         }
     }

     
     return pGenomes;    
}



//----------------------------------------------------------------------------
// readStandardBin
//   read a binary genome file as created by GeneWriter (via QDFSampler)
//   
ulong **readStandardBin(const char *pGenomeFile, int *piGenomeSize, int *piNumGenes, 
                        named_ids &mvIDs, std::map<int,int> & mNodeIDs, id_locs &mIdLocs, named_locs &mNamedLocs) {
     ulong **pGenomes = NULL;
     int iResult = 0;
     int iNumLocs = 0;
     bool bFull = false;
     FILE *fIn = fopen(pGenomeFile, "rb");
     if (fIn != NULL) {
         int iHeaderLen = 4*sizeof(char)+3*sizeof(int)+sizeof(bool);
         char *pH = new char[iHeaderLen];
         int iRead = fread(pH, iHeaderLen, 1, fIn);
         if (iRead == 1) {
             
             if (memcmp(pH, "GENS", 4) == 0) {
                 char *p = pH+4;
                 p = getMem(piGenomeSize, p, sizeof(int));
                 p = getMem(piNumGenes,   p, sizeof(int));
                 p = getMem(&iNumLocs,    p, sizeof(int));
                 p = getMem(&bFull,       p, sizeof(bool));

                 printf("We have %s\n", bFull?"full":"reduced");
                 pGenomes = new ulong*[*piNumGenes];
                 memset(pGenomes, 0, *piNumGenes*sizeof(ulong));
             

                 //                 printf("Header: GenomeSize %d, NumGenes %d. numLocs %d\n", *piGenomeSize, *piNumGenes, iNumLocs); 
             } else {
                 printf("Bad magic number\n");
                 iResult = -1;
             }
             
         } else {
             printf("Couldn't read header\n");
             iResult = -1;
         }
         delete[] pH;
         
         if (iResult == 0) {
             int iC = 0;
             char *pSpecial = NULL;
             int iSpecial = 0;
             int iNumBlocks = GeneUtils::numNucs2Blocks(*piGenomeSize);
             for (int i = 0; (iResult == 0) && (i < iNumLocs); i++) {
                 int iNameLen = 0;
                 // header is OK
                 int iRead = fread(&iNameLen, sizeof(int), 1, fIn);
                 if (iRead == 1) {
                     int iFullLength = iNameLen+sizeof(int)+3*sizeof(double);
                     if (iFullLength > iSpecial) {
                         if (pSpecial != NULL) {
                             delete[] pSpecial;
                         }
                         iSpecial = iFullLength;
                         pSpecial = new char[iSpecial];
                     }
                     iRead = fread(pSpecial, iFullLength, 1, fIn);
                     if (iRead == 1) {
                         double dLon=-1;
                         double dLat=-1;
                         double dDist=-1;
                         int iNumSubGenomes=0;
                         //                         printf("Loc [%s]\n", pSpecial);
                         // skip the name
                         char *p = pSpecial+iNameLen;
                         p = getMem(&dLon,  p, sizeof(double));
                         p = getMem(&dLat,  p, sizeof(double));
                         p = getMem(&dDist, p, sizeof(double));
                         p = getMem(&iNumSubGenomes, p, sizeof(int));
                         mNamedLocs[pSpecial] = std::pair<double,double>(dLon, dLat);
                         //                         printf("LocationHeader: dLon %f, dLat %f, dDist %f, iNumSubGenomes %d\n", dLon, dLat, dDist, iNumSubGenomes);
                         // todo: store dLon, dLat, dDist in a locdef

                         std::vector<idtype> vIDs;
                         for (int k = 0; (iResult == 0) && (k < iNumSubGenomes); k++) {
                             idtype iID     = 0;
                             idtype iMomID  = 0;
                             idtype iDadID  = 0;
                             int iGender = 0;
                             int iNodeID = 0;
                             double dLon = -1;
                             double dLat = -1;
                             int iDataLen =  sizeof(idtype)+sizeof(int)+2*sizeof(double);
                             if (bFull) {
                                 iDataLen += 2*sizeof(idtype)+sizeof(int);
                             }
                             char *pH = new char[iDataLen];
                             
                             iRead = fread(pH, iDataLen, 1, fIn);
                             if (iRead == 1) {
                                 p = pH;
                                 p = getMem(&iID,     p, sizeof(idtype));
                                 if (bFull) {
                                     p = getMem(&iMomID,  p, sizeof(idtype));
                                     p = getMem(&iDadID,  p, sizeof(idtype));
                                     p = getMem(&iGender, p, sizeof(int));
                                 }
                                 p = getMem(&iNodeID, p, sizeof(int));
                                 p = getMem(&dLon,    p, sizeof(double));
                                 p = getMem(&dLat,    p, sizeof(double));
                                 vIDs.push_back(iID);
                                 mNodeIDs[iID] = iNodeID;
                                 mIdLocs[iID]=std::pair<double, double>(dLon, dLat);

                                 ulong *pG =  new ulong[2*iNumBlocks];
                                 iRead = fread(pG, sizeof(ulong), 2*iNumBlocks, fIn);
                                 if (iRead == 2*iNumBlocks) {
                                     pGenomes[iC++] = pG;
                                 } else {
                                     printf("Couldn't read genomes\n");
                                     iResult = -1;
                                 }
                             } else {
                                 printf("COuldn't read genome header\n");
                                 iResult = -1;
                             }
                             delete[] pH;
                         }
                         // pSpecial contains the name with terminating 0
                         mvIDs[pSpecial] = vIDs;
                     } else {
                         printf("Couldn't readsubheader\n");
                         iResult = -1;
                     }

                 } else {
                     printf("Couldn't read name len\n");
                     iResult = -1;
                 }


             }
             printf("Saved %d genomes\n", iC);
             delete[] pSpecial;
         }
    
         // in case of error delete the array
         if (iResult != 0) {
             if (pGenomes != NULL) {
                 for (int i = 0; i < *piNumGenes; i++) {
                     delete[] pGenomes[i];
                 }
                 delete[] pGenomes;
             }
             pGenomes = NULL;
         }

         fclose(fIn);
     } else {
         printf("Couldn't open [%s] for reading\n", pGenomeFile);
         iResult = -1;
     }
     

     return pGenomes;
}


//----------------------------------------------------------------------------
// readGenomes
//   try to read given file a s binary; if it fails try as ASCII
//
ulong **readGenomes(const char *pGeneFile, int *piGenomeSize, int *piNumGenes) {

    std::map<int,int> mNodeIDs;
    named_ids mvIDs;
    id_locs mIdLocs;
    named_locs mNamedLocs;

    printf("Trying binary...\n");
    ulong **pGenomes = readStandardBin(pGeneFile, piGenomeSize, piNumGenes, mvIDs, mNodeIDs, mIdLocs, mNamedLocs);
    if (pGenomes == NULL) {
        printf("Trying asc...\n");
        const char **pGenomeStrings = readStandardAsc(pGeneFile, piGenomeSize, piNumGenes, mvIDs, mNodeIDs, mIdLocs, mNamedLocs);
        if (pGenomeStrings != NULL) {
            pGenomes = new ulong*[*piNumGenes];
            for (int i = 0; i < *piNumGenes; i++) {
                pGenomes[i] = GeneUtils::translateGenome(*piGenomeSize, pGenomeStrings[i]);
            }

            // free the genome strings after use
            for (int i = 0; i < *piNumGenes; i++) {
                free((void *)pGenomeStrings[i]);
            }
            delete[] pGenomeStrings;
        }
    }
    if (pGenomes == NULL) {
        printf("Couldn't read genome file [%s]\n", pGeneFile);
    }
    return pGenomes;
}


//----------------------------------------------------------------------------
// splitRaster
//
int splitRaster(char *pRasterDef, double *pdDLon, double *pdDLat, double *pdDist) {
    int iResult = -1;
    char *p = strtok(pRasterDef, ":");
    if (p != NULL) {
        if (strToNum(p, pdDLon)) {

            p = strtok(NULL, ":");
            if (p != NULL) {
                if (strToNum(p, pdDLat)) {
                    p = strtok(NULL, ":");
                    if (p != NULL) {
                        if (strToNum(p, pdDist)) {
                            iResult = 0;
                        } else {
                            printf("Bad number for Dist [%s]\n", p);
                        }
                    } else {
                        printf("Expected ':' after Lat\n");
                   }
                } else {
                    printf("Bad number for Lat [%s]\n", p);
                }
            } else {
                printf("Expected ':' after Lon\n");
            }
        } else {
            printf("Bad number for Lon [%s]\n", p);
        }
    } else {
        printf("Bad RasterDef:  [%s]\n", pRasterDef);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    
    char *sQDFGeo    = NULL;
    char *sQDFStats  = NULL;
    char *sQDFPop    = NULL;
    char *sSpecies   = NULL;
    char *sRasterDef = NULL;
    char *sRefFile   = NULL;
    char *sOutput    = NULL;

    int iNSel = 0;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(8,
                                   "-g:S!",  &sQDFGeo,
                                   "-m:S!",  &sQDFStats,
                                   "-p:S!",  &sQDFPop,
                                   "-s:S!",  &sSpecies,
                                   "-d:S!",  &sRasterDef,
                                   "-r:S!",  &sRefFile,
                                   "-o:S!",  &sOutput,
                                   "-n:i!",  &iNSel);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                double dDLon = 0;
                double dDLat = 0;
                double dDist = 0;
                
                if (splitRaster(sRasterDef, &dDLon, &dDLat, &dDist)==0) {
                    // dDLon *= M_PI/180;
                    // dDLat *= M_PI/180;
                    GridSampler *pGS = GridSampler::createInstance(sQDFGeo, sQDFStats, sQDFPop, sSpecies);
                    if (pGS != NULL) {
                        
                        iResult = pGS->findCandidates(dDLon, dDLat, dDist);
                        if (iResult == 0) {
                            iResult = pGS->selectAtGrids(iNSel);
                            if (iResult == 0) {
                                ulong **pRefGenome = NULL;
                                int iNumGenesRef = 0;
                                    
                                iResult = pGS->loadGenomes();
                                if (iResult == 0) {
                                    int iGenomeSizeR;
                                    pRefGenome = readGenomes(sRefFile, &iGenomeSizeR, &iNumGenesRef);
                                    if (pRefGenome != NULL) {
                                        printf("Reference genome read ok\n");
                                        iResult = -1;
                                        if (iGenomeSizeR == pGS->getGenomeSize()) {
                                            iResult = 0;
                                        } else {
                                            printf("Reference genome size does not match\n");
                                        }
                                    } else {
                                        printf("Couldn't read reference file\n");
                                    }
                                    
                                    if (iResult == 0) {
                                        iResult = pGS->calcGeoGenomeDists(pRefGenome[0]);
                                        if (iResult == 0) {
                                            iResult = pGS->writeFile(sOutput);
                                            if (iResult == 0) {
                                                printf("Finished writing\n");
                                            }  
                                        }
                                    }
                                } else {
                                    printf("Couldn't load genomes\n");
                                }
 
                                // clean up
                                if (pRefGenome != NULL) {
                                    for (int i = 0; i < iNumGenesRef; i++) {
                                        delete[] pRefGenome[i];
                                    }
                                    delete[] pRefGenome;
                                }
                                
                                printf(" +++ success +++\n");
                                 
                            } else {
                                printf("Couldn't do selection\n");
                            }
                        } else {
                            printf("Couldn't collect candidates\n");
                        }
                        delete pGS;
                    }

                } else {
                    iResult = -1;
                    printf("Culdn't split raster def [%s]\n", sRasterDef);
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
