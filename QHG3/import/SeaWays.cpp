#include <stdio.h>
#include <string.h>

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include <omp.h>
#include <hdf5.h>

#include "types.h"
#include "strutils.h"
#include "geomutils.h"
#include "ParamReader.h"

#include "SCell.h"
#include "SCellGrid.h"
#include "Geography.h"
#include "Navigation.h"

#include "GridGroupReader.h"
#include "GeoGroupReader.h"
#include "GridWriter.h"
#include "GeoWriter.h"
#include "NavWriter.h"

#include "QDFUtils.h"
#include "ComponentBuilder.h"
#include "CoastalDistances.h"
#include "SubComponentManager.h"

typedef std::vector<gridtype>        cellvec;
typedef std::set<gridtype>           cellset;
typedef std::map<gridtype, cellset>  cellrange;

typedef std::pair<gridtype, double>  endpoint;
//typedef std::vector<endpoint>        distlist;
typedef std::map<gridtype, double>   distlist;
typedef std::map<gridtype, distlist> distancemap;


typedef std::set<endpoint>           epset;
typedef std::map<gridtype, endpoint> endpoints;

typedef std::map<gridtype, int>      idindex;

bool endpointcomp (endpoint &lhs, endpoint &rhs) {return (lhs.second < rhs.second) || (lhs.first < rhs.first);};

typedef std::map<int, cellset>       compids;
typedef std::map<gridtype, compids>  connections;



int a1[] = {220915, 221102, 221290, 221479, 221669, 221860, 220914, 221101,
           221289, 221478, 221668, 221859, 221292, 221293, 221105, 220918,
           220545, 220546, 220547, 220548, 220550, 220734, 221863, 221673};

int a2[] = {221858, 221667, 221477, 221288, 221100, 220913, 221859, 221668, 221478, 221289, 
           221101, 220914, 221860, 221669, 221479, 221290, 221102, 220915,
           221671,221481, 221292, 221104, 220917, 221672, 221482, 221293, 221105, 220918, 
           220544};
int a[] = {241206, 241106, 241007,241205, 241105, 241304,  241203, 241103, 241405, 241303, 241202};

const char *s1[] = {"A", "B", "C", "D", "E", "F", "G", "H",
             "I", "J", "K", "L", "M", "N", "O", "P", 
                   "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "a", "b", "c"};
const char *s[] = {"i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s"};



static std::map<int, std::string> mnames;

//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - create links for overseas neighbors of coastal cells\n", pApp);
    printf("Usage:\n");
    printf("  %s -i <input_qdf> -d <distance> [-o <output_qdf>] [-b <Box>[,<Box>]*]\n", pApp);
    printf("where\n");
    printf("  input_qdf   qdf file with Geography (Altitude) in which to search for overseas neighbors\n");
    printf("  distance    max distance to sea (in km)\n");
    printf("  output_qdf  output file (if omitted, changes are made to <input_qdf>)\n");
    printf("  Box         longitude/latitude rectangle of the form\n");
    printf("                <LonMin>\":\"<LatMin>\":\"<LonMax>\":\"<LatMax>\n");
    printf("  LonMin,LonMax  minimum and maximum longitude of range to process\n");
    printf("  LatMin,LatMax  minimum and maximum latitude of range to process\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// splitBox
//
int splitBox(char *pBoxes, boxlist &vBoxes) {
    int iResult = 0;
    if (pBoxes != NULL) {
        char *pCtx0;
        char *pBox = strtok_r(pBoxes, ",", &pCtx0);
        while ((iResult == 0) && (pBox != NULL)) {
            
            iResult = -1;
            char *pCtx1;
            char *p = strtok_r(pBox, ":", &pCtx1);
            if (p != NULL) {
                double dLonMin;
                if (strToNum(p, &dLonMin)) {
                    p = strtok_r(NULL, ":", &pCtx1);
                    if (p != NULL) {
                        double dLatMin;
                        if (strToNum(p, &dLatMin)) {
                            p = strtok_r(NULL, ":", &pCtx1);
                            if (p != NULL) {
                                double dLonMax;
                                if (strToNum(p, &dLonMax)) {
                                    p = strtok_r(NULL, ":", &pCtx1);
                                    if (p != NULL) {
                                        double dLatMax;
                                        if (strToNum(p, &dLatMax)) {
                                            printf("Have range: [%f,%f] -- [%f,%f]\n", dLonMin, dLatMin, dLonMax, dLatMax);
                                            vBoxes.push_back(degbox(dLonMin, dLatMin, dLonMax, dLatMax));
                                            iResult = 0;
                                        } else {
                                            printf("invalid max latitude: [%s]\n", p);
                                        }
                                    } else {
                                        printf("expecetd max latitude: [%s]\n", p);
                                    }
                                } else {
                                    printf("invalid max longitude: [%s]\n", p);
                                }
                            } else {
                                printf("expecetd max longitude: [%s]\n", p);
                            }
                        } else {
                            printf("invalid min latitude: [%s]\n", p);
                        }
                    } else {
                        printf("expecetd min latitude: [%s]\n", p);
                    }
                } else {
                    printf("invalid min longitude: [%s]\n", p);
                }
            } else {
                printf("expecetd min longitude: [%s]\n", p);
            }
            pBox = strtok_r(NULL, ",", &pCtx0);
        }
    
    } else {
        vBoxes.push_back(degbox(-720, -180, 720, 180));
        printf("Have full range\n");
        iResult = 0;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createCellGrid
//
SCellGrid *createCellGrid(const char *pQDFFile) {
    SCellGrid *pCG = NULL;
    int iResult = -1;
    

    hid_t hFile = qdf_openFile(pQDFFile, true);
    if (hFile > 0) {
                printf("File opened\n");

        GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
        if (pGR != NULL) {
            //int iNumCells=0;
            GridAttributes gridatt;
            iResult = pGR->readAttributes(&gridatt);
            if (iResult == 0) {
                //                printf("num cells: %d\n", iNumCells);
                pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
                pCG->m_aCells = new SCell[gridatt.m_iNumCells];
                
                iResult = pGR->readData(pCG);
                if (iResult == 0) {
                    GeoGroupReader *pGeoR = GeoGroupReader::createGeoGroupReader(hFile);
                    if (pGeoR != NULL) {
                        GeoAttributes geoatt;
                        iResult = pGeoR->readAttributes(&geoatt);
                        if (iResult == 0) {
                            Geography *pGeo = new Geography(geoatt.m_iNumCells, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
                            
                            iResult = pGeoR->readData(pGeo);
                            if (iResult == 0) {
                                pCG->setGeography(pGeo);
                            } else {
                                printf("Couldn't read geo data from [%s]\n", pQDFFile);
                            }
                        } else {
                            printf("Couldn't read geo attributes from [%s]\n", pQDFFile);
                        }
                        delete pGeoR;
                    } else {
                        printf("Couldn't create GeoGroupReader for QDF file [%s]\n", pQDFFile);
                    }
                } else {
                    printf("Couldn't read geo attributes from [%s]\n", pQDFFile);
                }
            } else {
                printf("Couldn't get number of cells from [%s]\n", pQDFFile);
            }
            delete pGR;
        } else {
            printf("Couldn't create GridGroupReader for QDF file [%s]\n", pQDFFile);
        }
        qdf_closeFile(hFile);
    } else {
        printf("Couldn't open QDF file [%s]\n", pQDFFile);
    }
    
    if (iResult != 0) {
        delete pCG; 
        pCG = NULL;
    }


    return pCG;
}


//----------------------------------------------------------------------------
// showDistances
//
void showDistances(const distancemap &mDistances) {
    printf("Showing %zd distanecs\n", mDistances.size());
    distancemap::const_iterator it;

    for (it = mDistances.begin(); it != mDistances.end(); ++it) {
        printf("Orig %d(%s): (%zd entries)\n", it->first, mnames[it->first].c_str(), it->second.size());
        
        distlist::const_iterator itd;
        for (itd = it->second.begin(); itd != it->second.end(); ++itd) {
            printf("   %d(%s) : %f\n", itd->first, mnames[itd->first].c_str(), itd->second);
        }
        
    }
}


//----------------------------------------------------------------------------
// writeAllToQDF
//
int writeAllToQDF(SCellGrid *pCG, const char *sOutputQDF, cellset &sAll) {
    int iResult = 0;

    printf("total number of cells in range: %zd\n", sAll.size());
    cellset::const_iterator itc;
    for (itc=sAll.begin(); itc != sAll.end(); ++itc) {
        pCG->m_pGeography->m_adArea[*itc] = 999;
    }

    GridWriter *pGridW  = new GridWriter(pCG, &pCG->m_smSurfaceData);
    GeoWriter  *pGeoW   = new GeoWriter(pCG->m_pGeography);

    hid_t hFile = qdf_createFile(sOutputQDF, 0);
    iResult = pGridW->write(hFile);
    if (iResult == 0) {
        iResult = pGeoW->write(hFile);
        if (iResult == 0) {
            qdf_closeFile(hFile);  
        } else {
            printf("Couldn't write geography\n");
        }
    } else {
        printf("Couldn't write grid\n");
    }
    delete pGridW;
    delete pGeoW;
    return iResult;
}


//----------------------------------------------------------------------------
// writeAllToQDF
//
int writeAllToQDF(SCellGrid *pCG, const char *sOutputQDF, cellvec &vCoastCells, ComponentBuilder *pCB) {
    int iResult = 0;

    printf("writing components to area array\n"); fflush(stdout);

    for (uint i = 0; i < vCoastCells.size(); ++i) {
        int iComp = pCB->getComponentFor(vCoastCells[i]);

        pCG->m_pGeography->m_adArea[vCoastCells[i]] = iComp;
    }

    GridWriter *pGridW  = new GridWriter(pCG, &pCG->m_smSurfaceData);
    GeoWriter  *pGeoW   = new GeoWriter(pCG->m_pGeography);

    hid_t hFile = qdf_createFile(sOutputQDF, 0);
    iResult = pGridW->write(hFile);
    if (iResult == 0) {
        iResult = pGeoW->write(hFile);
        if (iResult == 0) {
            qdf_closeFile(hFile);  
        } else {
            printf("Couldn't write geography\n");
        }
    } else {
        printf("Couldn't write grid\n");
    }
    delete pGridW;
    delete pGeoW;
    return iResult;
}

//----------------------------------------------------------------------------
// writeAllToQDF
//
int writeAllToQDF(SCellGrid *pCG, const char *sOutputQDF, const distancemap &mFinalDistances, double dSampleDist) {
    int iResult = 0;
    
    printf("Write to [%s] (external)\n", sOutputQDF); fflush(stdout);

    Navigation *pNav    = new Navigation();
    pNav->setData(mFinalDistances, dSampleDist); 

    GridWriter *pGridW  = new GridWriter(pCG, &pCG->m_smSurfaceData);
    GeoWriter  *pGeoW   = new GeoWriter(pCG->m_pGeography);
    NavWriter  *pNavW   = new NavWriter(pNav);

    hid_t hFile = qdf_createFile(sOutputQDF, 0);
    iResult = pGridW->write(hFile);
    if (iResult == 0) {
        iResult = pGeoW->write(hFile);
        if (iResult == 0) {
            iResult = pNavW->write(hFile);
            if (iResult == 0) {
                printf("Written ok\n");
            } else {
                printf("Couldn't write navigation\n");
            }
        } else {
            printf("Couldn't write geography\n");
        }
    } else {
        printf("Couldn't write grid\n");
    }
    qdf_closeFile(hFile);  
    delete pNav;
    delete pNavW;
    delete pGeoW;
    delete pGridW;
    return iResult;
}

//----------------------------------------------------------------------------
// writeAllToQDFIn
//
int writeAllToQDFIn(SCellGrid *pCG, const char *sOutputQDF, const distancemap &mFinalDistances, double dSampleDist) {
    int iResult = 0;

    printf("Write to [%s] (internal)\n", sOutputQDF); fflush(stdout);

    Navigation *pNav    = new Navigation(); 
    pNav->setData(mFinalDistances, dSampleDist); 
    NavWriter  *pNavW   = new NavWriter(pNav);

    hid_t hFile = qdf_openFile(sOutputQDF, true);
    iResult = pNavW->write(hFile);
    if (iResult == 0) {
        qdf_closeFile(hFile);  
    } else {
        printf("Couldn't write navigation\n");
    }
    
    delete pNav;
    delete pNavW;
    return iResult;
}



//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {

    int    iResult = -1;
    char  *sInputQDF  = NULL;
    char  *sOutputQDF = NULL;
    char  *sBoxes     = NULL;
    double dDistance  = 0;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(4,
                                   "-i:S!",      &sInputQDF,
                                   "-d:d!",      &dDistance,
                                   "-o:S",       &sOutputQDF,
                                   "-b:S",       &sBoxes);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                for (uint i = 0; i < sizeof(a)/sizeof(int); i++) {
                    mnames[a[i]] = s[i];
                }
                printf("dDist:%f\n", dDistance);
                SCellGrid *pCG = createCellGrid(sInputQDF);
                if (pCG != NULL) {
                    
                    boxlist vBoxes;
                    iResult = splitBox(sBoxes, vBoxes);
                    if  (iResult == 0) {
                        CoastalDistances *pCD = CoastalDistances::createInstance(pCG, dDistance, vBoxes, false);
                        if (pCD != NULL) {
                            pCD->createNeighborList();
                            const neighborlist &mNeighbors = pCD->getNeighbors();
                            const distancemap &mReduced = pCD->getReduced();
                            //   showDistances(mReduced);

                              
                            cellset sAll;
                            SubComponentManager *pSCM = SubComponentManager::createInstance(pCG, mReduced, mNeighbors, false);
                            if (pSCM != NULL) {


                                distancemap mFinal;
                                pSCM->createDistanceMap(mFinal);
                                //showDistances(mFinal);

                                if (sOutputQDF != NULL) {
                                    writeAllToQDF(pCG, sOutputQDF, mFinal, dDistance);
                                } else {
                                    writeAllToQDFIn(pCG, sInputQDF, mFinal, dDistance);
                                }
                                /*
                                  const subcompmap &mSubComponents = pSCM->getSubComponents();
                            
                                  subcompmap::const_iterator it;
                                  for (it = mSubComponents.begin(); it != mSubComponents.end(); ++it) {
                                  const cellchain &vChain = it->second->getChain();
                                  cellchain::const_iterator itc;
                                  sAll.insert(vChain.begin(), vChain.end());
                                  }
                                  writeAllToQDF(pCG, sOutputQDF, sAll);
                                */
                                delete pSCM;
                            } else {
                                printf("Couldn't create SubComponentManager\n");
                            }
                        
                        
                            /*
                              cellset sAll;
                              distancemap::const_iterator it;
                              for (it = mReduced.begin(); it != mReduced.end(); ++it) {
                              distlist::const_iterator itd;
                              for (itd = it->second.begin(); itd != it->second.end(); ++itd) {
                              sAll.insert(itd->first);
                              }
                              }
                        
                              writeAllToQDF(pCG, sOutputQDF, sAll);
                            */  
                            delete pCD;
                        } else {
                            printf("Couldn't create CoastalDistances\n");
                        }
                    
                    
                        delete pCG->m_pGeography;
                        delete pCG;
                    } else {
                        printf("Bad Format for Box\n");
                    }
                } else {
                    printf("Couldn't create cellgrid from [%s]\n", sInputQDF);
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
