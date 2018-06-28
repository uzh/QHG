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
#include "DistMat.h"
#include "BinPheneFile.h"

#include "DistMat.cpp"

typedef std::pair<std::string,double> loctime;
typedef std::map<loctime, std::vector<idtype> > tnamed_ids;
typedef std::map<std::string, std::vector<idtype> > named_ids;
typedef std::map<idtype, std::pair<double,double> > id_locs;
typedef std::map<std::string, std::pair<double,double> > named_locs;

//typedef std::map<idtype, ulong*> id_genomes;

//typedef std::map<idtype, float*> id_phenomes;


#define TEMPLATE_DIST_MAT  "%s.full.mat"
#define TEMPLATE_REF_MAT   "%s.ref.mat"
#define TEMPLATE_TABLE     "%s.tagdists"
#define TEMPLATE_GEO_PHEN  "%s.geophen"

#define IDBUFSIZE 4096

//----------------------------------------------------------------------------
// calcEuclideanDist
//   
static float calcEuclideanDist(float *p1, float *p2, int iN) {
    float dS = 0;
    for (int i = 0; i < iN; i++) {
        dS += (p1[i]-p2[i])*(p1[i]-p2[i]);
    }
    return sqrt(dS);
}
 
//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - phenetic distances\n", pApp);
    printf("Usage;\n");
    printf("  %s -g <phenomefile> -o <outputbody> [-n]\n", pApp);
    printf("      [-m <movestatqdf> [-G <gridqdf>] [-r <referencefile>]]\n");
    printf("where\n");
    printf("  phenomefile      a file created by QDFPhenSampler\n");
    printf("  outputbody       output file name body\n");
    printf("  movestatqdf      qdf file containing move statistics (for geo/gene dist calculations)\n");
    printf("  gridqdf          qdf file containing grid data (for geo/gene dist calculations)\n");
    printf("                   This file must be specified if movstatqdf contains no grid data\n");
    printf("                   the various outputs will append suffices tp it\n");
    printf("  referencefile    file containing one or more reference phenomes, relative to which\n");
    printf("                   the genetic distances are calculated.\n");
    printf("                   If \"auto\", genetic distances are calculated relative to the \n");
    printf("                   sample group closest to the distance origin.\n");
    printf("\n");
    printf("Outputs:\n");
    printf("always:\n");
    char s[512];
    sprintf(s, TEMPLATE_DIST_MAT, "XXX");
    printf("  '%s'  distance matrix\n", s);
    sprintf(s, TEMPLATE_TABLE, "XXX");
    printf("  '%s'  data table\n", s);
    printf("if reference file is given:\n");
    sprintf(s, TEMPLATE_REF_MAT, "XXX");
    printf("  '%s'   distance to reference\n", s);
    sprintf(s, TEMPLATE_GEO_PHEN, "XXX");
    printf("  '%s'    Geo-Phenetic distances\n", s);
    printf("\n");
}


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
// nodeIdsToStat
// 
int nodeIdsToStat(const char *pStatQDF, std::map<int, int> &mNodeIndexes, std::map<int, float> &mNodeStats, const char *pStatName) {
    int iResult = -1;
    
    //@@    printf("Reading distances from [%s]\n", pStatQDF);
    QDFArray *pQA = QDFArray::create(pStatQDF);
    if (pQA != NULL) {
        iResult = pQA->openArray(MSTATGROUP_NAME, pStatName);
        if (iResult == 0) {

            // we must find the distance values for the nodes
            // at the specified indexes

            std::vector<int> vIndexes;
            std::map<int, int>::const_iterator it;
            for (it = mNodeIndexes.begin(); it != mNodeIndexes.end(); ++it) {
                vIndexes.push_back(it->second);
            }
            std::sort(vIndexes.begin(), vIndexes.end());
            std::map<int, double> mIndexStats;

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
                        mIndexStats[vIndexes[iCurIdx]] =pBuf[k];
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
                mNodeStats[it->first] = mIndexStats[it->second];
            }
            //@@            printf("got %zd distances\n", mIndexDistances.size());
            
        } else {
            printf("Couldn't open Dataset [%s/%s]\n", MSTATGROUP_NAME, pStatName);
        }
        delete pQA;
    } else {
        printf("Couldn't open QDF file [%s]\n", pStatQDF);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// nodeIdsToDistances
// 
int nodeIdsToDistances(const char *pStatQDF, std::map<int, int> &mNodeIndexes, std::map<int, float> &mNodeDistances) {
    
    int iResult =  nodeIdsToStat(pStatQDF, mNodeIndexes, mNodeDistances, MSTAT_DS_DIST); 
    return iResult;
}


//----------------------------------------------------------------------------
// nodeIdsToTravelTimes
// 
int nodeIdsToTravelTimes(const char *pStatQDF, std::map<int, int> &mNodeIndexes, std::map<int, float> &mNodeTimes) {
    
    int iResult =  nodeIdsToStat(pStatQDF, mNodeIndexes, mNodeTimes, MSTAT_DS_TIME); 
    return iResult;
}


//----------------------------------------------------------------------------
// readPhenomes2
//   try to read given file a s binary
//
BinPheneFile *readPhenomes2(const char *pPheneFile) {
    int iNumPhenomes = -1;
    BinPheneFile *pBP = BinPheneFile::createInstance(pPheneFile);
    if (pBP != NULL) {
        iNumPhenomes = pBP->read();
        if (iNumPhenomes <= 0) {
            delete pBP;
            pBP = NULL;
        }
    }
    return pBP;
}   


//----------------------------------------------------------------------------
// reorderPhenes
//   make sure the genes are in the order given by mvIDs
//
float **reorderPhenes(id_phenomes &mIDPhen, tnamed_ids &mvIDs) {
    float **pAllGenomes = new float*[mIDPhen.size()];
    uint iC = 0;
    tnamed_ids::const_iterator it;
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        for (uint k = 0; k < it->second.size(); k++) {
            if (iC < mIDPhen.size()) {
                pAllGenomes[iC++] = mIDPhen[it->second[k]];
            }
        }
    }
    return pAllGenomes;
}


//----------------------------------------------------------------------------
// distGeoPhenes2
//   creates a file containing the following fields per individual:
//     ID
//     Longitude (or X)
//     Latitude  (or Y)
//     sampling time
//     travelled distance
//     travel time
//     location name
//     location id
//     region id (e.g. continents) 
//
//   the file name has the suffix '.tagdists'
//  
int distGeoPhenes2(id_phenomes &mIDPhen, tnamed_ids &mvIDs, id_locs &mIDLocs, int iPhenomeSize, 
                  std::map<int, double> &mTravelDists, 
                  std::map<int, double> &mTravelTimes,
                  const char *pOutput, bool bBitNucs = false) {
    int iResult = 0;
     
    // average the distance of all "original" indivduals to the other individuals
    int iC = 0;

    char sName[512];
    
    sprintf(sName,  TEMPLATE_TABLE, pOutput);
    FILE *fOut0 = fopen(sName, "wt");

    tnamed_ids::const_iterator it;
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        int i1 = it->first.first.find('[');
        std::string sLocName = it->first.first.substr(0, i1);
        int i2 = it->first.first.find(']');
        std::string sLocDefs = it->first.first.substr(i1+1, i2-i1-1);
        int iLocID = -1;
        int iRegID = -1;
        int i11 = sLocDefs.find(',');
        if (i11 >= 0) {
            std::string sLocID = sLocDefs.substr(0, i11);
            std::string sRegID = sLocDefs.substr(i11+1);
            if (strToNum(sLocID.c_str(), &iLocID)) {
                if (strToNum(sRegID.c_str(), &iRegID)) {
                    
                }
            }
        }

        for (uint k = 0; k < it->second.size(); k++) {
            idtype iID = it->second[k];
            fprintf(fOut0, "%ld %f %f %8.1f %f %f %s %d %d \n", iID, mIDLocs[iID].first, mIDLocs[iID].second, it->first.second, mTravelDists[it->second[k]],  mTravelTimes[it->second[k]], sLocName.c_str(), iLocID, iRegID);
            
            iC++;
            //@@            printf("Ref<->%s(%d): %f, %f\n", it->first.c_str(), k, dAvg, mTravelDists[it->second[k]]);
        }

    }
    fclose(fOut0);
    printf("Written data table (%dx%d)\n  [%s]\n", iC, 9, sName);
    return iResult;
}


//----------------------------------------------------------------------------
// writeFullMatrix
//   
int writeFullMatrix(const char *pOutput, float **pM, int iNumPhenes1, int iNumPhenes2) {
    int iResult = -1;
    
    FILE *fOut = NULL;

    fOut = fopen(pOutput, "wt");

    if (fOut != NULL) {
        iResult = 0;
        
       
        // write distances with prepended location and agent ID
        for (int i = 0; i < iNumPhenes2; i++) {
            for (int j = 0; j < iNumPhenes2; j++) {
                fprintf(fOut, "%f\t", pM[i][j]);
            }
            fprintf(fOut, "\n");
        }


        fclose(fOut);
        printf("Written distance matrix (%dx%d)\n  [%s]\n", iNumPhenes1, iNumPhenes2, pOutput);
    } else {
        fprintf(stderr, "Couldn't open output file [%s]\n", pOutput);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createAndWriteDistMat
//   
int createAndWriteDistMat(int iPhenomeSize, id_phenomes &mIDPhen, tnamed_ids &mvIDs, const char *pOutput) {
    float **pPhenomes = NULL;
    int iResult = -1;
    pPhenomes = reorderPhenes(mIDPhen, mvIDs);
    DistMat<float>::calcdist_t fcalcdist = calcEuclideanDist;
    DistMat<float> *pDM = DistMat<float>::createDistMat(iPhenomeSize, pPhenomes, mIDPhen.size(), fcalcdist);
    if (pDM != NULL) {
        float **pM = pDM->createMatrix();
        char sName1[512];
        sprintf(sName1, TEMPLATE_DIST_MAT, pOutput);
        iResult = writeFullMatrix(sName1, pM, mIDPhen.size(), mIDPhen.size()/*, mvIDs*/);
        delete pDM;
    }
    
    delete[] pPhenomes;

    return iResult;
}



//----------------------------------------------------------------------------
// createAndWriteDistMatRef
//   
int createAndWriteDistMatRef(int iPhenomeSize, 
                              id_phenomes &mIDPhen,    tnamed_ids &mvIDs, 
                              id_phenomes &mIDPhenRef, tnamed_ids &mvIDsRef, 
                              const char *pOutput) {
    float **pPhenomes1 = NULL;
    float **pPhenomes2 = NULL;
    int iResult = -1;
    pPhenomes1 = reorderPhenes(mIDPhen, mvIDs);
    pPhenomes2 = reorderPhenes(mIDPhenRef, mvIDsRef);
    DistMat<float>::calcdist_t fcalcdist = calcEuclideanDist;
    DistMat<float> *pDM = DistMat<float>::createDistMatRef(iPhenomeSize, pPhenomes1, mIDPhen.size(), pPhenomes2, mIDPhenRef.size(), fcalcdist);
    if (pDM != NULL) {
        float **pM = pDM->createMatrix();
        
        char sName1[512];
        sprintf(sName1, TEMPLATE_REF_MAT, pOutput);
        iResult = writeFullMatrix(sName1, pM, mIDPhen.size(), mIDPhenRef.size());

        delete pDM;
    }
    delete[] pPhenomes1;
    delete[] pPhenomes2;

    return iResult;
}


//----------------------------------------------------------------------------
// extractAndWriteGeoPhGenomeDists
//   
int extractAndWriteGeoPhenomeDists(const char *pOutput) {
    int iResult = -1;

    char sTable[512];
    sprintf(sTable, TEMPLATE_TABLE, pOutput);
    char sRefMat[512];
    sprintf(sRefMat, TEMPLATE_REF_MAT, pOutput);
    char sGGOut[512];
    sprintf(sGGOut, TEMPLATE_GEO_PHEN, pOutput);

    LineReader *pLRTable = LineReader_std::createInstance(sTable, "rt");
    if (pLRTable != NULL) {
        LineReader *pLRRefMat = LineReader_std::createInstance(sRefMat, "rt");
        if (pLRRefMat != NULL) {
            FILE * fOut = fopen(sGGOut, "wt");
            if (fOut != NULL) {
                iResult = 0;
                int iC = 0;
                int iW = 0;
                while ((iResult == 0) && (!pLRTable->isEoF()) && (!pLRRefMat->isEoF())) {
                    char *pLineTable = pLRTable->getNextLine();
                    char *pLineRefMat = pLRRefMat->getNextLine();
                    if ((pLineTable != NULL) && (pLineRefMat != NULL)) {
                        std::vector<std::string> vFields;
                        char *pWord = strtok(pLineTable, " \t\n");
                        while (pWord != NULL) {
                            vFields.push_back(pWord);
                            pWord = strtok(NULL, " \t\n");
                        }
                    
                        if (iW == 0) {
                            char *p = strtok(pLineRefMat, ",; \t\n");
                            while (p != NULL) {
                                iW++;
                                p = strtok(NULL, ",; \t\n");
                            }
                        }
                        if (vFields.size() > 4) {
                            fprintf(fOut, "%s %s\n", vFields[4].c_str(), pLineRefMat);
                            iC++;
                        } else {
                            printf("Not enough fields in [%s]\n", sTable);
                            iResult = -1;
                        }
                    } else {
                        if ((pLineTable != NULL) != (pLineRefMat != NULL)) {
                            printf("[%s] and [%s] have different number of lines\n", sTable, sRefMat);
                            iResult = -1;
                        }
                    }
                }
                fclose(fOut);
                printf("Written geo-genome distance matrix (%dx%d)\n  [%s]\n", iC, iW+1, sGGOut);
            } else {
                printf("Couldn't open [%s] for writing\n", sGGOut);
            }
            delete pLRRefMat;
        } else {
            printf("Couldn't open [%s] for reading\n", sRefMat);
        }

        delete pLRTable;
    } else {
        printf("Couldn't open [%s] for reading\n", sTable);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// selectFromClosest
//   
int selectFromClosest(tnamed_ids &mvIDs, 
                      std::map<int, float> &mNodeDistances, 
                      std::map<int, int>   &mNodeIndexes, 
                      std::map<idtype,int> &mNodeIDs, 
                      id_phenomes &mIDPhen,
                      id_phenomes &mIDPhenRef,
                      tnamed_ids &mvIDsRef) {
    // find the population closest to the origin of the expansion
    double dMinDistance=1e99;
    std::string sMin;
    double dTime = -1;
    idtype iMinID = -1;
    tnamed_ids::const_iterator it;
    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
        for (uint k = 0; k < it->second.size(); k++) {
            idtype iID = it->second[k];
            double dDist = mNodeDistances[mNodeIndexes[mNodeIDs[iID]]];
            if (dDist < dMinDistance) {
                dMinDistance = dDist;
                sMin = it->first.first;
                dTime = it->first.second;

                iMinID = iID;
            }
            
        }
    }
    
    loctime key(sMin, dTime);
    
    mIDPhenRef[iMinID] = mIDPhen[iMinID];
    mvIDsRef[key].push_back(iMinID);
    
    //@@    printf("Reference copied from min (%s), dist (%f)\n", sMin.c_str(), dMinDistance);
    int iNumPhenesRef = mvIDsRef[key].size();
    return iNumPhenesRef;
}


//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    char *pPheneFile     = NULL;
    char *pOutput        = NULL;
    char *pMoveStatQDF   = NULL;
    char *pReferenceFile = NULL;

    char sGridQDF[128];
    *sGridQDF = '\0';

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(5,
                                   "-g:S!",   &pPheneFile,
                                   "-o:S!",   &pOutput,
                                   "-m:S",    &pMoveStatQDF,
                                   "-G:s",     sGridQDF,
                                   "-r:S",    &pReferenceFile);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                int iPhenomeSize  = -1;
                int iNumPhenes    = -1;

                iResult = -1;
                std::map<idtype,int> mIDNodes;
                tnamed_ids mvIDs;
                id_locs mIdLocs;

                id_phenomes mIDPhen;

                BinPheneFile *pBP = readPhenomes2(pPheneFile);
                if (pBP != NULL) {
                    iNumPhenes = pBP->getNumPhenomes();
                    iPhenomeSize = pBP->getPhenomeSize();
                    mvIDs    = pBP->getvIDs();
                    mIDPhen  = pBP->getIDPhen();
                    mIDNodes = pBP->getIDNodes();
                    mIdLocs  = pBP->getIDLocs();

                    tnamed_ids::const_iterator it;
                    int iii = 0;
                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                        iii += it->second.size();
                    }
                    printf("after init: #idgen: %zd, #vIDs: %d\n", mIDPhen.size(), iii);

                    if (iNumPhenes > 0) {

                        mvIDs = pBP->getvIDs();
                        std::vector<int>  vIDs;
                        tnamed_ids::const_iterator it2;
                        for (it2 = mvIDs.begin(); it2 != mvIDs.end(); ++it2) {
                            vIDs.insert(vIDs.end(), it2->second.begin(), it2->second.end());
                        }

                        // if MoveStat qdf is present, to travel/genetic distance calcs
                        if (pMoveStatQDF != NULL) {
                            // iff no GridFile is specified, assume the MoveStat file 
                            // to contain grid data as well
                            //
                            if (*sGridQDF == '\0') {
                                strcpy(sGridQDF, pMoveStatQDF);
                            }
                            
                            mIDNodes = pBP->getIDNodes();
                            std::vector<int> vNodeIDs;
                            std::map<idtype, int>::const_iterator it1;
                            for (it1 = mIDNodes.begin(); it1 != mIDNodes.end(); it1++) {
                                vNodeIDs.push_back(it1->second);
                            }
                        
                        
                            std::map<int, int> mNodeIndexes;
                            iResult = nodeIdsToIndexes(sGridQDF, vNodeIDs, mNodeIndexes);
                            if (iResult == 0) {
                            
                                std::map<int, float> mNodeDistances;
                                iResult = nodeIdsToDistances(pMoveStatQDF, mNodeIndexes, mNodeDistances);
                                if (iResult == 0) {
                                    std::map<int, float> mNodeTimes;
                                    iResult = nodeIdsToTravelTimes(pMoveStatQDF, mNodeIndexes, mNodeTimes);
                                    if (iResult == 0) {
                            

                                        std::map<int, double> mTravelDist;
                                        std::map<int, double> mTravelTime;
                                        for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                                            for (uint k = 0; k < it->second.size(); k++) {
                                                mTravelDist[it->second[k]] = mNodeDistances[mNodeIndexes[mIDNodes[it->second[k]]]];
                                                mTravelTime[it->second[k]] = mNodeTimes[mNodeIndexes[mIDNodes[it->second[k]]]];
                                            }
                                        }
                                        
                                        // calculate and write distances
                                        iResult = distGeoPhenes2(mIDPhen, mvIDs, mIdLocs, iPhenomeSize, mTravelDist, mTravelTime, pOutput);
                                        //                                            iResult = distGeoGenes2(mIDGen, mvIDs, mIdLocs, mIDGenRef, iGenomeSize, mTravelDist, pOutput, bWriteGnuplot, bBitNucs);
                                        // distance matrix
                                        if (iResult == 0) {
                                            iResult = createAndWriteDistMat(iPhenomeSize, mIDPhen, mvIDs, pOutput);
                                        }
                                        
                                        
                                        // do reference stuff
                                        if ((iResult == 0) && (pReferenceFile != NULL)) {
                                            printf("Doing the reference bit\n");
                                            // try to get a reference file
                                            BinPheneFile *pBPRef = NULL;
                                            id_phenomes mIDPhenRef;
                                            tnamed_ids mvIDsRef;
                                            int iNumPhenesRef = 0;
                                            if (strcmp(pReferenceFile, "auto") == 0) {
                                                iNumPhenesRef = selectFromClosest(mvIDs, mNodeDistances, mNodeIndexes, mIDNodes, mIDPhen, mIDPhenRef, mvIDsRef);
                                                if (iNumPhenesRef <= 0) {
                                                    iResult= -1;
                                                }
                                                printf("'auto' resulted in %d phene%s\n", iNumPhenesRef, (iNumPhenesRef!=1)?"s":"");
                                            } else {
                                                
                                                pBPRef = readPhenomes2(pReferenceFile);
                                                if (pBPRef != NULL) {
                                                    mIDPhenRef = pBPRef->getIDPhen();
                                                    // make sure Genome szes match
                                                    int iPhenomeSizeR = pBPRef->getPhenomeSize();
                                                    if (iPhenomeSize != iPhenomeSizeR) {
                                                        printf("Reference phenome size %d does not equal to samples phenome size %d\n", iPhenomeSizeR, iPhenomeSize);
                                                        iResult = -1;
                                                    } 

                                                    delete pBPRef;
                                                }
                                            }
                                            if (iResult == 0) {
                                                iResult = createAndWriteDistMatRef(iPhenomeSize, mIDPhen, mvIDs, mIDPhenRef, mvIDsRef, pOutput);
                                                
                                                if (iResult == 0) {
                                                    iResult = extractAndWriteGeoPhenomeDists(pOutput);
                                                }
                                            }
                                        } 

                                        
                                    } else {
                                        fprintf(stderr, "couldn't extract travel times from stat file\n");
                                    }

                                } else {
                                    fprintf(stderr, "couldn't extract distances from stat file\n");
                                }

                            } else {
                                fprintf(stderr, "reading grid data failed\n");
                            }
                        } else {
                            // no grid/movestats
                            iResult = createAndWriteDistMat(iPhenomeSize, mIDPhen, mvIDs, pOutput);
                        }
                                    
                    } else {
                        fprintf(stderr, "Couldn't read phenomes\n");
                    }

                    delete pBP;
                } else {
                    // nothing to do
                    fprintf(stderr, "Problem reading the phenome file [%s]\n", pPheneFile);
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }
    if (iResult == 0) {
        printf("+++ success +++\n");
    }

    return iResult;
}

