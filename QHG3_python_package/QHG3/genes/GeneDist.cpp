#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <map>
#include <vector>
#include <algorithm>

#include "ParamReader.h"
#include "LineReader.h"
#include "types.h"
#include "strutils.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "QDFArray.cpp"
#include "GeneUtils.h"
#include "DistMat.h"

typedef std::map<std::string, std::vector<int> > named_ids;
typedef std::map<int, std::pair<double,double> > id_locs;
typedef std::map<std::string, std::pair<double,double> > named_locs;
//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - genetic distances\n", pApp);
    printf("Usage;\n");
    printf("  %s -g <genomefile> -o <outputbody> [-n]\n", pApp);
    printf("      [-m <movestatqdf> [-G <gridqdf>] [-r <referencefile>]]\n");
    printf("where\n");
    printf("  genomefile     a file created by GeneSamples\n");
    printf("  outputbody     output file name body\n");
    printf("  movestatqdf    qdf file containing move statistics (for geo/gene dist calculations)\n");
    printf("  gridqdf        qdf file containing grid data (for geo/gene dist calculations)\n");
    printf("                 This file must be specified if movstatqdf contains no grid data\n");
    printf("                 the various outputs will append suffices tp it\n");
    printf("  referencefile  file containing one or more reference genomes, relative to which\n");
    printf("                 the genetic distances are calculated.\n");
    printf("                 If omitted, genetic distances are calculated relative to the \n");
    printf("                 sample group closest to the distance origin.\n");
    printf("  -n             omit headers in output files\n");
    printf("\n");
    printf("Outputs:\n");
    printf("  full distance matrix\n");
    printf("  grouped distance matrix\n");
    printf("\n");
}

#define IDBUFSIZE 4096


//----------------------------------------------------------------------------
// nodeIdsToIndexes
//   for each node ID, find its index in the CellGrid.
//   currently, th index is equal to thr ID, but in later versions 
//   with OpenMPI and spatial splitting this will not be the case anymore
//
int nodeIdsToIndexes(const char *pGridQDF, std::vector<int> &vNodeIDs, std::map<int, int> &mNodeIndexes) {
    int iResult = -1;
    
    //@@    printf("Reading indexes from [%s]\n", pGridQDF);
    QDFArray *pQA = QDFArray::create(pGridQDF);
    if (pQA != NULL) {

        iResult = pQA->openArray(GRIDGROUP_NAME, CELL_DATASET_NAME);
        if (iResult == 0) {

            intset sTemp;
            sTemp.insert(vNodeIDs.begin(), vNodeIDs.end());
            std::vector<int> vSorted(sTemp.size());
            std::copy(sTemp.begin(), sTemp.end(), vSorted.begin());
            int offset = 0;
            int *pBuf = new int[IDBUFSIZE];
            int iCount = pQA->getFirstSlab(pBuf, IDBUFSIZE, GRID_DS_CELL_ID);

            uint iCurIdx = 0;
            while ((iCurIdx < vSorted.size()) &&  (iCount > 0)) {
                int k = 0;
                while ((iCurIdx < vSorted.size()) && (k < iCount) && (pBuf[iCount-1] >= vSorted[iCurIdx])) {
                    if (pBuf[k] == vSorted[iCurIdx]) {
                        mNodeIndexes[vSorted[iCurIdx]] = offset+k;
                        iCurIdx++;
                    }
                    k++;
                }
                offset += iCount;
                iCount = pQA->getNextSlab(pBuf, IDBUFSIZE);
            }
            delete[] pBuf;

            pQA->closeArray();
        } else {
            printf("Couldn't open Dataset [%s/%s]\n", GRIDGROUP_NAME, CELL_DATASET_NAME);
        }
            
        delete pQA;
    } else {
        printf("Couldn't open QDF file [%s]\n", pGridQDF);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// nodeIdsToDistances
// 
int nodeIdsToDistances(const char *pStatQDF, std::map<int, int> &mNodeIndexes, std::map<int, float> &mNodeDistances) {
    int iResult = -1;
    
    //@@    printf("Reading distances from [%s]\n", pStatQDF);
    QDFArray *pQA = QDFArray::create(pStatQDF);
    if (pQA != NULL) {
        iResult = pQA->openArray(MSTATGROUP_NAME, MSTAT_DS_DIST);
        if (iResult == 0) {

            // we must find the distance values for the nodes
            // at the specified indexes

            std::vector<int> vIndexes;
            std::map<int, int>::const_iterator it;
            for (it = mNodeIndexes.begin(); it != mNodeIndexes.end(); ++it) {
                vIndexes.push_back(it->second);
            }
            std::sort(vIndexes.begin(), vIndexes.end());
            std::map<int, double> mIndexDistances;

            int offset = 0;
            double *pBuf = new double[IDBUFSIZE];

            // now loop through slabs of the distance values
            // to find those situated at our indexes
            int iCount = pQA->getFirstSlab(pBuf, IDBUFSIZE);

            uint iCurIdx = 0;
       
            while ((iCurIdx < vIndexes.size()) &&  (iCount > 0)) {
                int k = 0;
                while ((iCurIdx < vIndexes.size()) && (k < iCount) && ( offset+iCount >= vIndexes[iCurIdx])) {
                    if (k+offset == vIndexes[iCurIdx]) {
                        mIndexDistances[vIndexes[iCurIdx]] =pBuf[k];
                        iCurIdx++;
                    }
                    k++;
                }
                offset += iCount;
                iCount = pQA->getNextSlab(pBuf, IDBUFSIZE);
            }

            delete[] pBuf;
            //@@            printf("found %zd indexdists\n", mIndexDistances.size());
            
            // now we can assign distances to node IDs 
            for (it = mNodeIndexes.begin(); it != mNodeIndexes.end(); ++it) {
                mNodeDistances[it->first] = mIndexDistances[it->second];
            }
            //@@            printf("got %zd distances\n", mIndexDistances.size());
            
        } else {
            printf("Couldn't open Dataset [%s/%s]\n", MSTATGROUP_NAME, MSTAT_DS_DIST);
        }
        delete pQA;
    } else {
        printf("Couldn't open QDF file [%s]\n", pStatQDF);
    }

    return iResult;

}


//----------------------------------------------------------------------------
// readStandardAsc
//   read an ASCII genome file as created by GeneGraph or GeneGraphD
//
const char **readStandardAsc(const char *pGenomeFile, int *piGenomeSize, int *piNumGenes,  
                             named_ids &mvIDs, std::map<int,int> &mNodeIDs, id_locs &mIdLocs, named_locs &mNamedLocs) {
     const char **pGenomes = NULL;
     int iResult = 0;

     std::vector<std::string> vG;
     // we assume that the genomesize is less than 16384
     LineReader *pLR = LineReader_std::createInstance(pGenomeFile, "rt", 16384+256);
     std::string sCur = "(single)";

     if (pLR != NULL) {
         char *pLine = pLR->getNextLine(GNL_IGNORE_BLANKS);
         while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
             
             if (pLine[strlen(pLine)-1] == '\n') {
                 pLine[strlen(pLine)-1] = '\0';
             }
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
                 int iID;
                 int iMomID  = 0;
                 int iDadID  = 0;
                 int iGender = 0;
                 int iNodeID = 0;
                 double dLon = 0;
                 double dLat = 0;

                 int iC = sscanf(pLine, "%d %d %d %d %d %lf %lf", &iID, &iMomID, &iDadID, &iGender, &iNodeID, &dLon, &dLat);
                 if (iC == 7) {
                     mvIDs[sCur].push_back(iID);
                     mNodeIDs[iID] = iNodeID;
                     mIdLocs[iID] = std::pair<double,double>(dLon, dLat);
                     char *sG = new char[strlen(pLine)+1];
                     memset(sG, 0, strlen(pLine)+1);
                     char *pS = sG;
                     
                     // asc standard format: genomes begin at pos 70
                     pLine += 70;
                     
                     while (*pLine != '\0') {
                         if ((*pLine == 'A') || (*pLine == 'C') || (*pLine == 'G') || (*pLine == 'T')) {
                             *pS = *pLine;
                             pS++;
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
//   read a binary genome file as created by GeneGraph or GeneGraphD
//   
ulong **readStandardBin(const char *pGenomeFile, int *piGenomeSize, int *piNumGenes, 
                        named_ids &mvIDs, std::map<int,int> & mNodeIDs, id_locs &mIdLocs, named_locs &mNamedLocs) {
     ulong **pGenomes = NULL;
     int iResult = 0;
     int iNumLocs = 0;
     FILE *fIn = fopen(pGenomeFile, "rb");
     if (fIn != NULL) {
         char sH[4+3*sizeof(int)];
         int iRead = fread(sH, 4+3*sizeof(int), 1, fIn);
         if (iRead == 1) {
             
             if (memcmp(sH, "GENS", 4) == 0) {
                 char *p = sH+4;
                 p = getMem(piGenomeSize, p, sizeof(int));
                 p = getMem(piNumGenes,   p, sizeof(int));
                 p = getMem(&iNumLocs,    p, sizeof(int));

                 pGenomes = new ulong*[*piNumGenes];
                 memset(pGenomes, 0, *piNumGenes*sizeof(ulong));
             
             } else {
                 printf("Bad magic number\n");
                 iResult = -1;
             }
             
         } else {
             printf("Couldn't read header\n");
             iResult = -1;
         }
         
         if (iResult == 0) {

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
                         
                         // skip the name
                         char *p = pSpecial+iNameLen;
                         p = getMem(&dLon,  p, sizeof(double));
                         p = getMem(&dLat,  p, sizeof(double));
                         p = getMem(&dDist, p, sizeof(double));
                         p = getMem(&iNumSubGenomes, p, sizeof(int));
                         mNamedLocs[pSpecial] = std::pair<double,double>(dLon, dLat);
                         
                         // todo: store dLon, dLat, dDist in a locdef

                         std::vector<int> vIDs;
                         for (int k = 0; (iResult == 0) && (k < iNumSubGenomes); k++) {
                             int iID     = 0;
                             int iMomID  = 0;
                             int iDadID  = 0;
                             int iGender = 0;
                             int iNodeID = 0;
                             double dLon = -1;
                             double dLat = -1;
                             char pH[5*sizeof(int)+2*sizeof(double)];
                             
                             iRead = fread(pH, 5*sizeof(int)+2*sizeof(double), 1, fIn);
                             if (iRead == 1) {
                                 p = pH;
                                 p = getMem(&iID,     p, sizeof(int));
                                 p = getMem(&iMomID,  p, sizeof(int));
                                 p = getMem(&iDadID,  p, sizeof(int));
                                 p = getMem(&iGender, p, sizeof(int));
                                 p = getMem(&iNodeID, p, sizeof(int));
                                 p = getMem(&dLon,    p, sizeof(double));
                                 p = getMem(&dLat,    p, sizeof(double));
                                 vIDs.push_back(iID);
                                 mNodeIDs[iID] = iNodeID;
                                 mIdLocs[iID]=std::pair<double, double>(dLon, dLat);

                                 ulong *pG =  new ulong[2*iNumBlocks];
                                 iRead = fread(pG, sizeof(ulong), 2*iNumBlocks, fIn);
                                 if (iRead == 2*iNumBlocks) {
                                     pGenomes[k] = pG;
                                 } else {
                                     printf("Couldn't read genomes\n");
                                     iResult = -1;
                                 }
                             } else {
                                 printf("COuldn't read genome header\n");
                                 iResult = -1;
                             }
                             
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
ulong **readGenomes(const char *pGeneFile, int *piGenomeSize, int *piNumGenes, 
                    named_ids &mvIDs, std::map<int,int> &mNodeIDs, id_locs &mIdLocs, named_locs &mNamedLocs) {
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
// writeFullMatrixOld
//   
int writeFullMatrixOld(const char *pOutput, int **pM, int iNumGenes, named_ids &mvIDs, bool bHeaders) {
    int iResult = -1;
    
    FILE *fOut = NULL;

    fOut = fopen(pOutput, "wt");

    if (fOut != NULL) {
        iResult = 0;

        if(bHeaders) {
            // write header line            
            fprintf(fOut, "Location\tAgent_ID\t\t");
            named_ids::const_iterator it;
            for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                if (it->second.size() > 0) {
                    fprintf(fOut, "%s\t", it->first.c_str());
                    for (uint k = 1; k < it->second.size();k++) {
                        fprintf(fOut, "\t");
                    }
                }
            }
            fprintf(fOut, "\n");
        }

        // write distances with prepended location and agent ID
        int i = 0;
        named_ids::const_iterator it;
        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
            for (uint k = 0; k < it->second.size(); k++) {
                if (bHeaders) {
                    if (k == 0) {
                        fprintf(fOut, "%s\t", it->first.c_str());
                    } else {
                        fprintf(fOut, "\t");
                    }
                    fprintf(fOut, "%d\t\t", it->second[k]);
                }
                for (int j = 0; j < iNumGenes; j++) {
                    fprintf(fOut, "%d\t", pM[i][j]);
                }
                i++;
                fprintf(fOut, "\n");
            }
        }

        fclose(fOut);
        printf("Writing full distance matrix\n  [%s]\n", pOutput);
    } else {
        printf("Couldn't open output file [%s]\n", pOutput);
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// writeFullMatrix
//   
int writeFullMatrix(const char *pOutput, int **pM, int iNumGenes, named_ids &mvIDs, bool bHeaders) {
    int iResult = -1;
    
    FILE *fOut = NULL;

    fOut = fopen(pOutput, "wt");

    if (fOut != NULL) {
        iResult = 0;
        
        // write distances with prepended location and agent ID
        int i = 0;
        named_ids::const_iterator it;
        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
            int iC = 0;
            for (uint k = 0; k < it->second.size(); k++) {
                char sName[128];
                strcpy(sName, it->first.c_str());
                char *p = strchr(sName, '[');
                if (p != NULL) {
                    *p = '\0';
                }
                fprintf(fOut, "%s_%03d\t", sName, iC++);
                for (int j = 0; j < iNumGenes; j++) {
                    fprintf(fOut, "%d\t", pM[i][j]);
                }
                i++;
                fprintf(fOut, "\n");
            }
        }

        fclose(fOut);
        printf("Writing full distance matrix\n  [%s]\n", pOutput);
    } else {
        printf("Couldn't open output file [%s]\n", pOutput);
    }
    
    return iResult;
}



//----------------------------------------------------------------------------
// writeGroupedMatrix
//   write grouped averages of full matrix pM
//
int writeGroupedMatrix(const char *pOutput, int **pM, named_ids &mvIDs, bool bHeaders) {
    int iResult = -1;
    
    int iNumGroups = mvIDs.size();

    // create  a matrix
    float **pMG = new float*[iNumGroups];
    for (int i = 0; i < iNumGroups; i++) {
        pMG[i] = new float[iNumGroups];
        memset(pMG[i], 0, iNumGroups*sizeof(float));
    }

    std::vector<int> vGroupSizes;
    named_ids::const_iterator it;
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        vGroupSizes.push_back(it->second.size());
    }

    // calculate group averages
    int iY = 0;    
    for (int u = 0; u < iNumGroups; u++) {
        int iX = iY;    
        for (int v = u; v < iNumGroups; v++) {
            float s = 0;
            if ((vGroupSizes[u] == 0) ||(vGroupSizes[u] == 0)) {
                s = -1;
            } else {
                for (int i = 0; i < vGroupSizes[u]; i++) {
                    for (int j = 0; j < vGroupSizes[v]; j++) {
                        s += pM[iY+i][iX+j];
                    }
                }
                s /= (vGroupSizes[u]*vGroupSizes[u]);
                // diagonals have many 0s -
                // the average without them is 
                // N/(N-1) times the full average of the box
                if (u == v) {
                    s *= vGroupSizes[u]/(vGroupSizes[u]-1);
                }
            }
            pMG[u][v] = s;
            pMG[v][u] = s;
            
            iX += vGroupSizes[v];
        }
        iY += vGroupSizes[u];
    }
    

    // now write the group matrix
    FILE *fOut = fopen(pOutput, "wt");
    if (fOut != NULL) {

        if (bHeaders) {
            fprintf(fOut, "Location\t\t");
            named_ids::const_iterator it;
            for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                if (it->second.size() > 0) {
                    fprintf(fOut, "%s\t", it->first.c_str());
                }
            }
            fprintf(fOut, "\n");

        }
        int i = 0;
        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
            if (bHeaders) {
                fprintf(fOut, "%s\t\t", it->first.c_str());
            }
            for (int j = 0; j < iNumGroups; j++) {
                fprintf(fOut, "%.2f\t", pMG[i][j]);
            }
            i++;
            fprintf(fOut, "\n");
        }

        fclose(fOut);
        printf("Writing group matrix\n  [%s]\n", pOutput);
        
        for (int j = 0; j < iNumGroups; j++) {
            delete[] pMG[j];
        }
        delete[] pMG;

    } else {
        printf("Couldn't open [%s] for writing\n", pOutput);
        iResult = -1;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// distGeoGenesOld
//   creates 2 files associating geographic distances with genetic distances
//
//   sMin is the name of the group closest to the distance origin
//  
int distGeoGenesOld(named_ids &mvIDs, std::string &sMin, int iMinOffs, std::map<int, double> &mTravelDists, int **pM, const char *pOutput) {
    int iResult = 0;
    
    // average the distance of all "original" indivduals to the other individuals
    int iC = 0;
    int iC2 = 0;
    char sName[512];
    std::vector<int> vMin = mvIDs[sMin];
    named_ids::const_iterator it;
    printf("Writing %zd dist/geo files\n",2*mvIDs.size());
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        int i1 = it->first.find('[');
        sprintf(sName,  "%s.%s.dist", pOutput, it->first.substr(0, i1).c_str());
        FILE *fOut1 = fopen(sName, "wt");

        double dAvg2 = 0;
        double dVar2 = 0;
        double dDist = 0;

        for (uint k = 0; k < it->second.size(); k++) {
            // average all distances to Min group
            double dAvg = 0;
            double dVar = 0;
            for (uint j = 0; j < vMin.size(); j++) {
                // get distance to indivdual j of minimal group
                double dVal = pM[iC][iMinOffs+j];
                dAvg +=  dVal;
                dVar +=  dVal*dVal;
            }      
            dAvg /= vMin.size();
            dVar /= vMin.size();
            dVar  = dAvg*dAvg - dVar;
            fprintf(fOut1, "%d %f %f\n", it->second[k], mTravelDists[it->second[k]], dAvg);
            dDist += mTravelDists[it->second[k]];
            dAvg2 += dAvg;
            dVar2 += dAvg*dAvg;
            iC++;
        }
        fclose(fOut1);
        printf("  [%s]\n",  sName);

        sprintf(sName,  "%s.%s_2.dist", pOutput, it->first.substr(0, i1).c_str());
        FILE *fOut2 = fopen(sName, "wt");
        dDist /= it->second.size();
        dAvg2 /= it->second.size();
        dVar2 /= it->second.size();
        dVar2  = dVar2 - dAvg2*dAvg2;
        fprintf(fOut2, "%d %f %f %f\n", iC2, dDist, dAvg2, sqrt(dVar2));
        fclose(fOut2);
        printf("  [%s]\n",  sName);
        iC2++;
    }
              
    // write a gnuplot file to display the averaged travel distance to averaged genetic distance
    printf("Writing 2 gnuplot files for dist/geo data\n");
    sprintf(sName,  "%s.gpl", pOutput);
    FILE *fOut = fopen(sName, "wt");
    if (fOut != NULL) {
        fprintf(fOut, "set terminal png size 1200,900 enhanced\n");
        fprintf(fOut, "set output '%s.png'\n", pOutput);
        fprintf(fOut, "set yrange [0:1600]\n");
        fprintf(fOut, "set key right bottom\n");
        char s[4096];
        strcpy(s, "plot ");
        char sPart[128];
        char sComma[2] = "";
        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
            int i1 = it->first.find('[');
            sprintf(sPart, "%s \"%s.%s.dist\" using 2:3", sComma, pOutput, it->first.substr(0, i1).c_str());
            *sComma = ',';
            sComma[1] = '\0';
            strcat(s, sPart);
        }
        fprintf(fOut, "%s\n", s);
        fclose(fOut);
        printf("  [%s]\n",  sName);
    }

    // write a gnuplot file to display the averaged travel distance to averaged genetic distance
    sprintf(sName,  "%s_2.gpl", pOutput);
    FILE *fOut2 = fopen(sName, "wt");
    if (fOut2 != NULL) {
        fprintf(fOut2, "set terminal png size 1200,900 enhanced\n");
        fprintf(fOut2, "set output '%s_2.png'\n", pOutput);
        fprintf(fOut2, "set yrange [0:1600]\n");
        fprintf(fOut2, "set key right bottom\n");
        char s[4096];
        strcpy(s, "plot ");
        char sPart[128];
        char sComma[2] = "";
        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
            size_t i1 = it->first.find('[');
            size_t i2 = it->first.rfind(']');
            
            if (i1+1 == i2) {
                sprintf(sPart, "%s \"%s.%s_2.dist\" using 2:3:4 w errorbars t \"%s\"", sComma, pOutput, it->first.substr(0, i1).c_str(), it->first.substr(0, i1).c_str());
            } else {
                sprintf(sPart, "%s \"%s.%s_2.dist\" using 2:3:4 w errorbars t \"%s\"", sComma, pOutput, it->first.substr(0, i1).c_str(), it->first.substr(i1+1, i2-i1-1).c_str());
            }
            *sComma = ',';
            sComma[1] = '\0';
            strcat(s, sPart);
        }
        fprintf(fOut2, "%s\n", s);
        fclose(fOut2);
        printf("  [%s]\n",  sName);
    }

    return iResult;
}

//----------------------------------------------------------------------------
// distGeoGenes
//   creates 2 files associating geographic distances with genetic distances
//
//   sMin is the name of the group closest to the distance origin
//  
int distGeoGenes(named_ids &mvIDs, ulong **pRef, int iNumGenesRef, int iGenomeSize, ulong **pGenomes, std::map<int, double> &mTravelDists, const char *pOutput) {
    int iResult = 0;
    
    // average the distance of all "original" indivduals to the other individuals
    int iC = 0;
    int iC2 = 0;
    char sName[512];

    named_ids::const_iterator it;
    printf("Writing %zd dist/geo files\n",2*mvIDs.size());
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        int i1 = it->first.find('[');
        sprintf(sName,  "%s.%s.dist", pOutput, it->first.substr(0, i1).c_str());
        FILE *fOut1 = fopen(sName, "wt");

        double dAvg2 = 0;
        double dVar2 = 0;
        double dDist = 0;

        for (uint k = 0; k < it->second.size(); k++) {
            // average all distances to Min group
            double dAvg = 0;
            double dVar = 0;
            for (int j = 0; j < iNumGenesRef; j++) {
                // get distance to indivdual j of reference group

                double dVal = GeneUtils::calcDist(pRef[j], pGenomes[iC], iGenomeSize);;
                dAvg +=  dVal;
                dVar +=  dVal*dVal;
            }      
            dAvg /= iNumGenesRef;
            dVar /= iNumGenesRef;
            dVar  = dAvg*dAvg - dVar;
            fprintf(fOut1, "%d %f %f\n", it->second[k], mTravelDists[it->second[k]], dAvg);
            dDist += mTravelDists[it->second[k]];
            dAvg2 += dAvg;
            dVar2 += dAvg*dAvg;
            iC++;
        }
        fclose(fOut1);
        printf("  [%s]\n",  sName);

        sprintf(sName,  "%s.%s_2.dist", pOutput, it->first.substr(0, i1).c_str());
        FILE *fOut2 = fopen(sName, "wt");
        dDist /= it->second.size();
        dAvg2 /= it->second.size();
        dVar2 /= it->second.size();
        dVar2  = dVar2 - dAvg2*dAvg2;
        fprintf(fOut2, "%d %f %f %f\n", iC2, dDist, dAvg2, sqrt(dVar2));
        fclose(fOut2);
        printf("  [%s]\n",  sName);
        iC2++;
    }
              
    // write a gnuplot file to display the averaged travel distance to averaged genetic distance
    printf("Writing 2 gnuplot files for dist/geo data\n");
    sprintf(sName,  "%s.gpl", pOutput);
    FILE *fOut = fopen(sName, "wt");
    if (fOut != NULL) {
        fprintf(fOut, "set terminal png size 1200,900 enhanced\n");
        fprintf(fOut, "set output '%s.png'\n", pOutput);
        fprintf(fOut, "set yrange [0:]\n");
        fprintf(fOut, "set key right bottom\n");
        char s[4096];
        strcpy(s, "plot ");
        char sPart[128];
        char sComma[2] = "";
        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
            size_t i1 = it->first.find('[');
            size_t i2 = it->first.rfind(']');
            
            if (i1+1 == i2) {
                sprintf(sPart, "%s \"%s.%s.dist\" using 2:3 t \"%s\"", sComma, pOutput, it->first.substr(0, i1).c_str(), it->first.substr(0, i1).c_str());
            } else {
                sprintf(sPart, "%s \"%s.%s.dist\" using 2:3 t \"%s\"", sComma, pOutput, it->first.substr(0, i1).c_str(), it->first.substr(i1+1, i2-i1-1).c_str());
            }
            *sComma = ',';
            sComma[1] = '\0';
            strcat(s, sPart);
        }
        fprintf(fOut, "%s\n", s);
        fclose(fOut);
        printf("  [%s] -> writes image to %s.png\n", sName, pOutput);
    }

    // write a gnuplot file to display the averaged travel distance to averaged genetic distance
    sprintf(sName,  "%s_2.gpl", pOutput);
    FILE *fOut2 = fopen(sName, "wt");
    if (fOut2 != NULL) {
        fprintf(fOut2, "set terminal png size 1200,900 enhanced\n");
        fprintf(fOut2, "set output '%s_2.png'\n", pOutput);
        fprintf(fOut2, "set yrange [0:]\n");
        fprintf(fOut2, "set key right bottom\n");
        char s[4096];
        strcpy(s, "plot ");
        char sPart[128];
        char sComma[2] = "";
        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
            size_t i1 = it->first.find('[');
            size_t i2 = it->first.rfind(']');
            
            if (i1+1 == i2) {
                sprintf(sPart, "%s \"%s.%s_2.dist\" using 2:3:4 w errorbars t \"%s\"", sComma, pOutput, it->first.substr(0, i1).c_str(), it->first.substr(0, i1).c_str());
            } else {
                sprintf(sPart, "%s \"%s.%s_2.dist\" using 2:3:4 w errorbars t \"%s\"", sComma, pOutput, it->first.substr(0, i1).c_str(), it->first.substr(i1+1, i2-i1-1).c_str());
            }
            *sComma = ',';
            sComma[1] = '\0';
            strcat(s, sPart);
        }
        fprintf(fOut2, "%s\n", s);
        fclose(fOut2);
        printf("  [%s] -> writes image to %s_2.png\n", sName, pOutput);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    char *pGeneFile      = NULL;
    char *pOutput        = NULL;
    char *pMoveStatQDF   = NULL;
    char *pReferenceFile = NULL;
    bool bNoHeaders = false;
    char sGridQDF[128];
    *sGridQDF = '\0';

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(6,
                                   "-g:S!",   &pGeneFile,
                                   "-o:S!",   &pOutput,
                                   "-m:S",    &pMoveStatQDF,
                                   "-G:s",     sGridQDF,
                                   "-r:S",    &pReferenceFile,
                                   "-n:0",    &bNoHeaders);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                int iGenomeSize  = -1;
                int iNumGenes    = -1;
                int iNumGenesRef = -1;

                std::map<int,int> mNodeIDs;
                named_ids mvIDs;
                id_locs mIdLocs;
                named_locs mNamedLocs;
                ulong **pGenomes = readGenomes(pGeneFile, &iGenomeSize, &iNumGenes, mvIDs, mNodeIDs, mIdLocs, mNamedLocs);
    
                if (pGenomes != NULL) {
                    std::vector<int>  vIDs;
                    named_ids::const_iterator it;
                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                        vIDs.insert(vIDs.end(), it->second.begin(), it->second.end());
                    }

                    if (false) {
                        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                            printf("%s: %zd elements\n", it->first.c_str(), it->second.size());
                            int iShow = (it->second.size() > 10)?10: it->second.size();
                            for (int i = 0; i < iShow; i++) {
                                printf("  %d", it->second[i]);
                            }
                            if (it->second.size() > 10) {
                                printf(" ...");
                            }
                            printf("\n");
                        }
                    }
     
       
                    DistMat *pDM = DistMat::createDistMat(iGenomeSize, pGenomes, iNumGenes);
                    if (pDM != NULL) {
                        int **pM = pDM->createMatrix();
                        char sName1[512];
                        sprintf(sName1, "%s.full.mat", pOutput);
                        iResult = writeFullMatrix(sName1, pM, iNumGenes, mvIDs, !bNoHeaders);
                        if (iResult == 0) {
                            sprintf(sName1, "%s.group.mat", pOutput);
                            iResult = writeGroupedMatrix(sName1, pM, mvIDs, !bNoHeaders);
                        }
                
                        // if MoveStat qdf is present, to travel/genetic distance calcs
                        if (pMoveStatQDF != NULL) {
                            // iff no GridFile is specified, assume the MoveStat file 
                            // to contain grid data as well
                            //
                            if (*sGridQDF == '\0') {
                                strcpy(sGridQDF, pMoveStatQDF);
                            }

                            std::vector<int> vNodeIDs;
                            std::map<int, int>::const_iterator it1;
                            for (it1 = mNodeIDs.begin(); it1 != mNodeIDs.end(); it1++) {
                                vNodeIDs.push_back(it1->second);
                            }
               


                            std::map<int, int> mNodeIndexes;
                            iResult = nodeIdsToIndexes(sGridQDF, vNodeIDs, mNodeIndexes);
                            if (iResult == 0) {

                                std::map<int, float> mNodeDistances;
                                iResult = nodeIdsToDistances(pMoveStatQDF, mNodeIndexes, mNodeDistances);

                                ulong **pRef = NULL;
                                
                                if (pReferenceFile == NULL) {
                                    // find the population closest to the origin of the expansion
                                    double dMinDistance=1e99;
                                    std::string sMin;
                                    int iOffs = 0;
                                    int iMinOffs = 0;
                                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                                        //@@                                    printf("%s (%f,%f):\n", it->first.c_str(), mNamedLocs[it->first].first, mNamedLocs[it->first].second);
                                        for (uint k = 0; k < it->second.size(); k++) {
                                            int iID = it->second[k];
                                            double dDist = mNodeDistances[mNodeIndexes[mNodeIDs[iID]]];
                                            if (dDist < dMinDistance) {
                                                dMinDistance = dDist;
                                                sMin = it->first;
                                                iMinOffs = iOffs;
                                            }
                                            //@@                                        printf("  %d (%f, %f) -> %f\n", iID, mIdLocs[iID].first, mIdLocs[iID].second, dDist);
                                        }
                                        iOffs += it->second.size();
                                    }
                                    pRef = new ulong*[mvIDs[sMin].size()];
                                    for (uint k = 0; k < mvIDs[sMin].size(); k++) {
                                        int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
                                        pRef[k] = new ulong[2*iNumBlocks];
                                        memcpy(pRef[k], pGenomes[iMinOffs+k], 2*iNumBlocks*sizeof(ulong)); 
                                    }
                                    printf("Reference copied from min\n");
                                    iNumGenesRef = mvIDs[sMin].size();

                                } else {
                                    int iGenomeSizeR = -1;
                
                                    std::map<int,int> mNodeIDsR;
                                    named_ids mvIDsR;
                                    id_locs mIdLocsR;
                                    named_locs mNamedLocsR;
                                    pRef = readGenomes(pReferenceFile, &iGenomeSizeR, &iNumGenesRef, mvIDsR, mNodeIDsR, mIdLocsR, mNamedLocsR);
                                    if (pRef != NULL) {
                                        printf("Reference genome read ok\n");
                                        iResult = -1;
                                        if (iGenomeSizeR == iGenomeSize) {
                                            iResult = 0;
                                        } else {
                                            printf("Reference genome size does not match\n");
                                        }
                                    } else {
                                        printf("Couldn't read reference file\n");
                                    }
                                    if (iResult != 0) {
                                        for (int i = 0; i < iNumGenesRef; i++) {
                                            delete[] pRef[i];
                                        }
                                        delete[] pRef;
                                    }
                
                                }
                                
                                
                                std::map<int, double> mTravelDist;
                                for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                                    for (uint k = 0; k < it->second.size(); k++) {
                                        mTravelDist[it->second[k]] = mNodeDistances[mNodeIndexes[mNodeIDs[it->second[k]]]];
                                    }
                                }
                                iResult = distGeoGenes(mvIDs, pRef, iNumGenesRef, iGenomeSize, pGenomes, mTravelDist, pOutput);
                                // collect location centers coords 
                                // A: distances between origins and all area centers
                                // B: average distance between origin and all agents in an area
                    
                                if ((pRef != NULL) && (iNumGenesRef > 0)) {
                                    for (int i = 0; i < iNumGenesRef; i++) {
                                        delete[] pRef[i];
                                    }
                                    delete[] pRef;
                                }

                            
                            } else {
                                printf("reading grid data failed\n");
                            }
                        }

                        // clean up     

                        if (pGenomes != NULL) {
                            for (int i = 0; i < iNumGenes; i++) {
                                delete[] pGenomes[i];
                            }
                            delete[] pGenomes;
                        }
                
                        delete pDM;
                    }
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


/*


gnuplot> set xlabel "Geographic Distance"
gnuplot> set ylabel "Genetic Distance"
gnuplot> set yrange [0:1600]
gnuplot> set key right bottom
gnuplot> set terminal png size 1600,900 
gnuplot> set output 'DA4096_3498_s10.png'
gnuplot> plot "DAPop4096.EAfrica.dist" using 3:2 title "East Africa", \ 
              "DAPop4096.SAfrica.dist" using 3:2 title "South Africa", \
              "DAPop4096.NAfrica.dist" using 3:2 title "North Africa", \
              "DAPop4096.CentralEurope.dist" using 3:2 title "Central Europe", \
              "DAPop4096.India.dist" using 3:2 title "India", \
              "DAPop4096.Indonesia.dist" using 3:2 title "Indonesia", \
              "DAPop4096.Australia.dist" using 3:2 title "Australia", \
              "DAPop4096.Siberia.dist" using 3:2 title "Siberia", \
              "DAPop4096.NWAmerica.dist" using 3:2 title "North West America", \
              "DAPop4096.NEAmerica.dist" using 3:2 title "North East America", \
              "DAPop4096.Greenland.dist" using 3:2 title "Greenland", \
              "DAPop4096.Mexico.dist" using 3:2 title "Mexico", \
              "DAPop4096.Patagonia.dist" using 3:2 title "Patagonia"


*/
