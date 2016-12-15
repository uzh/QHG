#include <stdio.h>
#include <string.h>

#include <vector>

#include <hdf5.h>

#include "strutils.h"
#include "ParamReader.h"

#include "QDFUtils.h"

#include "SCellGrid.h"
#include "Geography.h"

#include "GridReader.h"
#include "GridWriter.h"
#include "GeoReader.h"
#include "GeoWriter.h"

#include "GeoInfo.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"
#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "shpUtils.h"
#include "shpHeader.h"
#include "shpRecord.h"
#include "dbfReader.h"

typedef std::vector<vecdoubledouble> vecvecdoubledouble;

const double dDefUseVal = 1.0;


//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - converting polyline vector data to QDF\n", pApp);
    printf("Usage:\n");
    printf("  %s -s <surf_file> (-i <ign_file> | -q <input_qdf>)\n", pApp);
    printf("        -v <shp_file> -d <dbf_file> -f <field_name>[:<match_val>[:<use_val>]]\n");
    printf("        -o <output_qdf\n");
    printf("where\n");
    printf("  surf_file    a surface description file (.ico, .ieq, ...)\n");
    printf("  ign_file     ico grid node file corresponding to <surf_file>\n");
    printf("  shp_file     SHP file containig vector data\n");
    printf("  dbf_file     DBF file corresponding to <shp_file>\n");
    printf("  field_name   name of field in <dbf_file> to extract.\n");
    printf("               To see all possible field names, call with '-d <dbf_file> only\n");
    printf("  matchval     target value to select indexes\n");
    printf("  useval       value to use instead of matchval\n");
    printf("\n");
    printf("<surf_file> and <ign_file> are needed to create a cell grid with geography\n");
    printf("all points contained in the poly lines are extracted and the values for <field_name>\n");
    printf("are used as entries in the QDF files \"Water\" array corresponding to the points' coordinates\n");
    printf("If <match_val> is specified, only polylines whose <field_name> value equals <match_val> are used,\n");
    printf("and the array is set to <use_val> (or 1.0) in the corresponding places\n");
    printf("\n");
    printf("Examples\n");
    printf("List the field names:\n");
    printf("  %s -d ~/rivers/ne_10m_rivers.dbf\n", pApp);
    printf("\n");
    printf("Convert (using CellGrid and Geo from qdf file):\n");
    printf("  %s -s eq256.ieq  -q GridSG_ieq_256.qdf -v ~/rivers/ne_10m_rivers.shp -d ~/rivers/ne_10m_rivers.dbf -f strokeweig -o rivers_1a_256\n",pApp);
    printf("\n");
    printf("Convert (creating CellGrid from ign file):\n");
    printf("  %s -s eq256.ieq  -i Grid_ieq_256_000.ign -v ~/rivers/ne_10m_rivers.shp -d ~/rivers/ne_10m_rivers.dbf -f strokeweig -o rivers_2a_256\n", pApp);
    printf("\n");
}


//----------------------------------------------------------------------------
// readShapeFile
//
int readShapeFile(const char *pShapeFile, vecvecdoubledouble &vRecs) {
    int iResult = 0;

    FILE *fIn = fopen(pShapeFile, "rb");
    if (fIn != NULL) {
        shpHeader *pShapeHeader = new shpHeader(fIn);
        iResult = pShapeHeader->read();
        if (iResult == 0) {
            //      pShapeHeader->display(pShapeFile);
            while ((iResult == 0) && !feof(fIn)) {
                vecdoubledouble  vCoords;
                shpRecord *pShapeRecord = new shpRecord(fIn);
                iResult = pShapeRecord->read(vCoords);
                if (iResult == 0) {
                    //  pShapeRecord->display("");
                    vRecs.push_back(vCoords);
                } else {
                    if (iResult < 0) {
                        printf("Shape read error\n");
                    }
                }
            }   
        } else {
            printf("Reading of shape file failed\n");
        }
        delete pShapeHeader;
    } else {
        printf("Couldn't open [%s]\n", pShapeFile);
    }
    if (iResult == 1) {
        iResult = 0;
    }
    printf("Finished shapefile, res: %d\n", iResult);
    return iResult;
}

//----------------------------------------------------------------------------
// listDBFFields
//
void listDBFFields(const char *pDBFFile) {
    FILE *fIn = fopen(pDBFFile, "rb");
    if (fIn != NULL) { 
        std::vector<double> vVals;
        dbfReader *pDBFReader = new dbfReader(fIn);
        int  iResult = pDBFReader->read(NULL, vVals);
        if (iResult == 0) {
            const nameoffsets &no = pDBFReader->getOffsets();
            nameoffsets::const_iterator it;
            printf("Names of numerical fields in [%s]\n", pDBFFile);
            for (it = no.begin(); it != no.end(); ++it) {
                printf(" %s\n", it->first.c_str());
            }
        } else {
            printf("Reading of dbf file failed\n");
        }
        delete pDBFReader;
    } else {
        printf("Couldn't open [%s]\n", pDBFFile);
    }
    
}


//----------------------------------------------------------------------------
// readDBFFile
//
int readDBFFile(const char *pDBFFile, const char *pFieldName, std::vector<double> &vVals) {
    int iResult = 0;

    FILE *fIn = fopen(pDBFFile, "rb");
    if (fIn != NULL) {
        dbfReader *pDBFReader = new dbfReader(fIn);
        iResult = pDBFReader->read(pFieldName, vVals);
        if (iResult == 0) {
            
        } else {
            printf("Reading of dbf file failed\n");
            iResult = -1;
        }
        delete pDBFReader;
    } else {
        printf("Couldn't open [%s]\n", pDBFFile);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// collectData
//
int collectData(const char *pShapeFile,
                const char *pDBFFile,
                const char *pFieldName,
                vecvecdoubledouble &vRecs,
                vecdouble &vVals) {
    int iResult =-1;

    iResult = readShapeFile(pShapeFile, vRecs);
    if (iResult == 0) {
        iResult = readDBFFile(pDBFFile, pFieldName, vVals);
        if (iResult == 0) {
            if (vRecs.size() == vVals.size()) {
                printf("Have %zd records\n", vRecs.size());
            } else {
                printf("size mismatch: shp [%zd], dbf [%zd]\n", vRecs.size(), vVals.size()); 
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// insertRiverData
//
int insertRiverData(vecvecdoubledouble &vRecs, vecdouble &vVals, SCellGrid *pCG, Surface *pSurf, const double *pMatchVal, const double *pUseVal) {
    int iResult = 0;

    Geography *pGeo = pCG->m_pGeography;
    printf("Looping over %zd recs\n", vRecs.size());
    for (uint i = 0; i < vRecs.size(); ++i) {
        vecdoubledouble &vdd = vRecs[i];

        for (uint j = 0; j < vdd.size(); ++j) {

            double dLon = vdd[j].first;
            double dLat = vdd[j].second;
            bool bDeg2Rad = true;
            
            /*
            // for LINEAR projections do not transform to radians
            if (pCG->m_smSurfaceData[SURF_TYPE].compare(SURF_LATTICE) == 0) {
                iResult = -1;
                char sPT[256];
                strcpy(sPT, pCG->m_smSurfaceData[SURF_LTC_PROJ_TYPE].c_str());
                char *p = strtok(sPT, " ");
                if (p != NULL) {
                    char *pEnd;
                    int iPT = strtol(p, &pEnd, 10);
                    if (*pEnd == '\0') {
                        iResult = 0;
                        if (iPT == PR_LINEAR) {
                            // printf("have LINEAR\n");
                            bDeg2Rad = false;
                        }
                    }
                }
            }
            */
            if (iResult == 0) {
                if (bDeg2Rad) {
                    /*
                    dLon *= M_PI/180;
                    dLat *= M_PI/180;
                    */
                }
                gridtype lNode = pSurf->findNode(dLon, dLat);
                int iIndex = pCG->m_mIDIndexes[lNode];
                if (pMatchVal != NULL) {
                    if (*pMatchVal == vVals[i]) {
                        pGeo->m_adWater[iIndex] = *pUseVal;
                    } else {
                        // do nothing
                    }
                } else {
                    pGeo->m_adWater[iIndex] = vVals[i];
                }

                //printf("Path %d, segment %d:(%f,%f)->%d:  %f(%f)\n", i, j, dLon*180/M_PI, dLat*180/M_PI, iIndex, dVal, vVals[i]);
                
            }
            
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// initializeGeography
//  create Geography, set
//   Longitude
//   Latitude
//   Distances
//   Area
//
int initializeGeography(SCellGrid *pCG, IcoGridNodes *pIGN) {

    int iResult = 0;
    
    bool bDeg2Rad = true;
    // rectangular grids with linear "projection" should not 
    // have their coordinates modified
    printf("Testing type of IGN surface:[%s]\n", pCG->m_smSurfaceData[SURF_TYPE].c_str());
    if (pCG->m_smSurfaceData[SURF_TYPE].compare(SURF_LATTICE) == 0) {
        printf("  --> is lattice\n");
        iResult = -1;
        char sPT[256];
        strcpy(sPT, pCG->m_smSurfaceData[SURF_LTC_PROJ_TYPE].c_str());
        printf("PROJ type  --> [%s]\n", sPT);

        char *p = strtok(sPT, " ");
        if (p != NULL) {
            char *pEnd;
            int iPT = (int)strtol(p, &pEnd, 10);
            printf("First word [%s]\n", p);
            if (*pEnd == '\0') {
                iResult = 0;
                if (iPT == PR_LINEAR) {
                    printf("have LINEAR\n");
            
                    bDeg2Rad = false;
                }
            }
        }
    }
    
    if (iResult == 0) {
        Geography *pGeo = pCG->m_pGeography;

        for (uint i=0; i< pCG->m_iNumCells; ++i) {
            gridtype iIndex = pCG->m_aCells[i].m_iGlobalID;  // for each cell find its ID
            IcoNode* pIN = pIGN->m_mNodes[iIndex];           // get the corresponding iconode in pIGN

       
            if(pIN != NULL) {
                pGeo->m_adAltitude[iIndex] = 0;

                pGeo->m_adLatitude[iIndex]  =  pIN->m_dLat;
                pGeo->m_adLongitude[iIndex] =  pIN->m_dLon;
                if (bDeg2Rad) {
                    pGeo->m_adLatitude[iIndex]  *=  180/M_PI;
                    pGeo->m_adLongitude[iIndex] *=  180/M_PI;
                }
                // the neighbor arrays are arranged sequentially into a big 1-d array
                int i0 = pGeo->m_iMaxNeighbors*iIndex;
                for (int j = 0; j < pIN->m_iNumLinks; j++) {
                    pGeo->m_adDistances[i0+j] = pIN->m_adDists[j];
                }
                pGeo->m_adWater[iIndex] = 0;
                pGeo->m_adArea[iIndex] = pIN->m_dArea;
                pGeo->m_abIce[iIndex] = false;

            } else {
                fprintf(stderr,"[GridFactory::setGeography] node of index %d not found\n",iIndex);
                iResult = -1;
            }
        }
    } else {
        fprintf(stderr,"[GridFactory::setGeography] couldn't read projection details\n");
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// createCells
//   create cells
//   link cells
//
int createCells(IcoGridNodes *pIGN, SCellGrid *pCG) { // THIS IS FOR ICOSAHEDRON GRID
    
    
    
    int iC = 0;
    pCG->m_aCells = new SCell[pCG->m_iNumCells];
    std::map<gridtype, IcoNode*>::const_iterator it;
    for (it = pIGN->m_mNodes.begin(); it != pIGN->m_mNodes.end(); ++it) {
        pCG->m_mIDIndexes[it->first]=iC;
        pCG->m_aCells[iC].m_iGlobalID    = it->first;
        pCG->m_aCells[iC].m_iNumNeighbors = (uchar)it->second->m_iNumLinks;
        //        pCF->setGeography(m_pGeography, iC, it->second);
        iC++;
    }

 
    // linking and distances
    for (uint i =0; i < pCG->m_iNumCells; ++i) {
        // get link info from IcCell
        IcoNode *pIN = pIGN->m_mNodes[pCG->m_aCells[i].m_iGlobalID];
        for (int j = 0; j < pIN->m_iNumLinks; ++j) {
            pCG->m_aCells[i].m_aNeighbors[j] = pCG->m_mIDIndexes[pIN->m_aiLinks[j]];
        }
        for (int j = pIN->m_iNumLinks; j < MAX_NEIGH; ++j) {
            pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char *sSurfFile   = NULL;
    char *sIGNFile    = NULL;
    char *sSHPFile    = NULL;
    char *sDBFFile    = NULL;
    char *sFieldName  = NULL;
    char *sOutputQDF  = NULL;
    char *sInputQDF   = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(7,
                                   "-s:S",           &sSurfFile,
                                   "-i:S",           &sIGNFile,
                                   "-q:S",           &sInputQDF,
                                   "-v:S",           &sSHPFile,
                                   "-d:S!",          &sDBFFile,
                                   "-f:S",           &sFieldName,
                                   "-o:S",           &sOutputQDF);
 

        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {

                int iNumCells =0;

                if (sFieldName == NULL) {
                    listDBFFields(sDBFFile);
                } else if (((sIGNFile != NULL) || (sInputQDF != NULL)) &&
                           (sSurfFile != NULL) && (sSHPFile != NULL) && (sOutputQDF!= NULL)){
                    SCellGrid *pCG = NULL;
                    // get surface
                    Surface *pSurf = NULL;
                    
                    Lattice *pLat = new Lattice();
                    iResult = pLat->load(sSurfFile);
                    if (iResult == 0) {
                        pSurf = pLat;
                        printf("Have Lattice\n");
                        iNumCells = pLat->getLinkage()->getNumVertices();
                    } else {
                        EQsahedron *pEQ = EQsahedron::createEmpty();
                        iResult = pEQ->load(sSurfFile);
                        if (iResult == 0) {
                            
                            pEQ->relink();
                            pSurf = pEQ;
                            printf("Have EQsahedron\n");
                            iNumCells = pEQ->getLinkage()->getNumVertices();
                        } else {
                            Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
                            pIco->setStrict(true);
                            bool bPreSel = false;
                            pIco->setPreSel(bPreSel);
                            iResult = pIco->load(sSurfFile);
                            printf("Have Icosahedron\n");
                            
                        }
                    }
                    // get cell grid from ign file
                    if (pSurf != NULL) {
                        iResult = 0;
                    } else {
                        printf("Couldn't create surface file\n");
                    }



                    if (iResult == 0) {

                        if (sInputQDF != NULL) {
                            iResult = -1;
                            hid_t hFile = qdf_openFile(sInputQDF);
                            if (hFile > 0) {
                                GridReader *pGR = GridReader::createGridReader(hFile);
                                if (pGR != NULL) {
                                    stringmap smSurfData;
                                    iResult = pGR->readAttributes(&iNumCells, smSurfData);
                                    if (iResult == 0) {
                                        pCG = new SCellGrid(0, iNumCells, smSurfData);
                                        pCG->m_aCells = new SCell[iNumCells];
                                        
                                        iResult = pGR->readCellData(pCG);
                                        if (iResult == 0) {
                                            GeoReader *pGeoR = GeoReader::createGeoReader(hFile);
                                            if (pGeoR != NULL) {
                                                int iMaxNeighbors;
                                                uint iNumCells2;
                                                double dRadius;
                                                double dSeaLevel;
                                                iResult = pGeoR->readAttributes(&iNumCells2, &iMaxNeighbors, &dRadius, &dSeaLevel);
                                                if (iResult == 0) {
                                                    Geography *pGeo = new Geography(iNumCells2, iMaxNeighbors, dRadius);
                                                
                                                    iResult = pGeoR->readData(pGeo);
                                                    if (iResult == 0) {
                                                        pCG->setGeography(pGeo);
                                                    } else {
                                                        printf("Couldn't read geo data from [%s]\n", sInputQDF);
                                                    }
                                                    } else {
                                                        printf("Couldn't read geo data from [%s]\n", sInputQDF);
                                                    }
                                                delete pGeoR;
                                            } else {
                                                printf("Couldn't create GeoReader for QDF file [%s]\n", sInputQDF);
                                            }
                                        } else {
                                            printf("Couldn't read geo attributes from [%s]\n", sInputQDF);
                                        }
                                    } else {
                                        printf("Couldn't get number of cells from [%s]\n", sInputQDF);
                                    }
                                    delete pGR;
                                } else {
                                    printf("Couldn't create GridReader for QDF file [%s]\n", sInputQDF);
                                }
                            } else {
                                printf("Couldn't open QDF file [%s]\n", sInputQDF);
                            }
                            
                        } else {
                            IcoGridNodes *pIGN = new IcoGridNodes();
                            iResult = pIGN->read(sIGNFile);
                            if (iResult == 0) {
                                iNumCells = pIGN->m_mNodes.size();
                                
                                int iC = 0;
                                pCG = new SCellGrid(0, iNumCells, pIGN->getData());
                                
                                createCells(pIGN,pCG);
                                
                                
                                // if qdf file exists, get the gography array from there, otherwise create new one
                                Geography *pGeo = NULL;
                                pGeo = new Geography(iNumCells, 6, 6371.0);  // create geography
                                memset(pGeo->m_adWater, 0, pCG->m_iNumCells*sizeof(double));
                                pCG->setGeography(pGeo);
                                initializeGeography(pCG, pIGN);
                                
                                
                            } else {
                                printf("Couldn't read from [%s]\n", apArgV[2]);
                            }
                            delete pIGN;
                        }
                    }
                    

                    const double *pMatchVal = NULL;
                    const double *pUseVal   = NULL;
                    double  dVal = 0;
                    if (iResult ==0) {
                        char *pMatchValStr = strchr(sFieldName, ':');
                        if (pMatchValStr != NULL) {
                            *pMatchValStr = '\0';
                            pMatchValStr++;
                            pUseVal =&dDefUseVal;
                                    
                            char *pUseValStr = strchr(sFieldName, ':');
                            if (pUseValStr != NULL) {
                                *pUseValStr = '\0';
                                pUseValStr++;
                                if (strToNum(pUseValStr, &dVal)) {
                                    pUseVal =  &dVal;
                                } else {
                                    printf("Bad use value: [%s]\n", pUseValStr);
                                    iResult = -1;
                                }
                            }
                            if (strToNum(pMatchValStr, &dVal)) {
                                pMatchVal =  &dVal;

                            } else {
                                printf("Bad match value: [%s]\n", pMatchValStr);
                                iResult = -1;
                            }
                        }
                    }

                    if (iResult ==0) {
                        vecvecdoubledouble vRecs;
                        vecdouble vVals;
                        printf("collecting data\n");
                        iResult = collectData(sSHPFile, sDBFFile, sFieldName, vRecs, vVals);
                        if (iResult == 0) {
                       

                            printf("inserting rivers\n");
                            iResult = insertRiverData(vRecs, vVals, pCG, pSurf, pMatchVal, pUseVal); 
            

                            printf("result before writing to [%s]: %d\n", sOutputQDF, iResult);
                            // dummy time value
                            float fTime = -1;
                            /*
                            hid_t hFile = -1;
                            
                            // create output file
                            if (sInputQDF != NULL) {
                                hFile = qdf_createFile(sInputQDF, fTime);
                            } else {
                                hFile = qdf_createFile(sOutputQDF, fTime);
                            }
                            */
                            hid_t hFile = qdf_createFile(sOutputQDF, fTime);
                            
                            if (hFile >= 0) {
                                GridWriter *pGridW = new GridWriter(pCG, NULL);
                                pGridW->write(hFile);
                                GeoWriter *pGeoW = new GeoWriter(pCG->m_pGeography);
                                pGeoW->write(hFile);
                                qdf_closeFile(hFile);
                                printf("Written to QDF file [%s]\n", sOutputQDF);
                    
                            } else {
                                printf("Couldn't create QDF file [%s]\n", sOutputQDF);
                            }
                        }
                    }
                    delete pSurf;
                    delete pCG;
        
                } else {
                    usage(apArgV[0]);
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
