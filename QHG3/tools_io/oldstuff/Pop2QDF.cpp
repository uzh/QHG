#include <stdio.h>
#include <string.h>

#include <hdf5.h>

#include "GeoInfo.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"
#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "PopBase.h"
#include "SPopulation.h"
#include "PopulationFactory.h"
#include "SCellGrid.h"
#include "IDGen.h"

#include "QDFUtils.h"
#include "PopWriter.h"


//-----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - export population files to a QDF file\n", pApp);
    printf("usage:\n");
    printf("  %s <surffile> <ignfile> (<clsfile>:<datafile>)* <output-QDF-name>\n", pApp);
    printf("where\n");
    printf("  surffile         name of surface file (icosahedron [ico], eqsahedron [ieq] or lattice [ltc])\n");
    printf("  ignfile          name of grid node file based on surface\n");
    printf("  clsfile          class file for population\n");
    printf("  datafile         text file containing coordinates and agent data (1 agent per line)\n");
    printf("  output-QDF-name  name for output QDF file\n");
    printf("\n");
    printf("The surface file is used to convert the coordinates in the pop files to node ids for the specified surface.\n");
    printf("The grid node file is used to assign correct cell indexes to agents\n");
    printf("Each line of the data file must start with longitude, latitude and a colon, followed by specific data\n");
    printf("\n");
    printf("Exmple:\n");
    printf("  %s eq128.ieq eq128_002.ign clsAltMoverPop.def:AltMoverData.txt clsGeneticPop.def:GeneticData.txt new.qdf\n", pApp);
    printf("\n");
}


//-----------------------------------------------------------------------------
// readAgentData
//   we expect lines containing longitude and latitude followed by a colon ':'
//   and  specific agent data
//
int readAgentData(PopBase *pB, const char *pAgentDataFile, SCellGrid *pCG, Surface *pSurf) {
    int iResult = 0;
    
    LineReader *pLR = LineReader_std::createInstance(pAgentDataFile, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
            if (pLine != NULL) {

                double dLon;
                double dLat;
                
                int iNum = sscanf(pLine, "%lf %lf", &dLon, &dLat);
                if (iNum == 2) {
                    bool bDeg2Rad = true;

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

                    if (iResult == 0) {
                        if (bDeg2Rad) {
                            dLon *= M_PI/180;
                            dLat *= M_PI/180;
                        }
                        gridtype lNode = pSurf->findNode(dLon, dLat);
//                        printf("%f %f -> %d\n", dLon, dLat, lNode);
                        
                        char *pData = strchr(pLine, ':');
                        if (pData != NULL) {
                            *pData = '\0';
                            pData++;
                            // here we assume the index is equal to the ID
                            int iIndex = pCG->m_mIDIndexes[lNode];
                            iResult = pB->addAgent(iIndex, pData);
                        } else {
                            printf("No colon ':' in line:[%s]\n", pLine);
                            iResult = -1;
                        }
                    } // iresult == 2
                } else {
                    printf("Wasn't able to extract Lo, Lat from [%s]\n", pLine);
                }
            } // pLine != NULL
        } //while
        
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pAgentDataFile);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    if (iArgC > 3) {
        int iNumCells =0;

        // get surface
        Surface *pSurf = NULL;

        Lattice *pLat = new Lattice();
        iResult = pLat->load(apArgV[1]);
        if (iResult == 0) {
            pSurf = pLat;
            printf("Have Lattice\n");
            iNumCells = pLat->getLinkage()->getNumVertices();
        } else {
            EQsahedron *pEQ = EQsahedron::createEmpty();
            iResult = pEQ->load(apArgV[1]);
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
                iResult = pIco->load(apArgV[1]);
                printf("Have Icosahedron\n");

            }
        }
        
        // get cell grid from ign file
        SCellGrid *pCG = NULL;
        if (pSurf != NULL) {
            printf("Surface OK\n");
            IcoGridNodes *pIGN = new IcoGridNodes();
            iResult = pIGN->read(apArgV[2]);
            if (iResult == 0) {
                iNumCells = pIGN->m_mNodes.size();
            
                int iC = 0;
                pCG = new SCellGrid(0, iNumCells, pIGN->getData());
                pCG->m_pGeography = new Geography();
                pCG->m_aCells = new SCell[iNumCells];
                std::map<gridtype, IcoNode*>::const_iterator it;
                for (it = pIGN->m_mNodes.begin(); it != pIGN->m_mNodes.end(); ++it) {
                    pCG->m_mIDIndexes[it->first]=iC;
                    pCG->m_aCells[iC].m_iGlobalID    = it->first;
                    pCG->m_aCells[iC].m_iNumNeighbors = it->second->m_iNumLinks;
                    iC++;
                }
            } else {
                printf("Couldn't read from [%s]\n", apArgV[2]);
            }
            delete pIGN;
        }
        
        if (iResult ==0) {
            IDGen **apIDG = new IDGen*[1];
            apIDG[0] = new IDGen(0, 0, 1);
            // create a dummy arrray needed by Pop factory for pop generation
            uint32_t aulDummyState[STATE_SIZE];
            memset(aulDummyState, 0, STATE_SIZE*sizeof(uint32_t));
            printf("Cellgrid OK\n");
            printf("Creating PopulationFactory\n");
            PopulationFactory *pPF = new PopulationFactory(pCG, NULL, 1024, apIDG, aulDummyState);
            std::vector<PopBase *> vPops;
            for (int i = 3; (iResult == 0) && (i < iArgC-1); i++) {
                char *pClsDataFile = apArgV[i];
                char *pAgentDataFile = strchr(pClsDataFile, ':');
                if (pAgentDataFile != NULL) {
                    *pAgentDataFile = '\0';
                    pAgentDataFile++;
                    printf("reading [%s]\n",  pClsDataFile);
                    PopBase *pB = pPF->readPopulation(pClsDataFile);
                    if (pB != NULL) {
                        vPops.push_back(pB);
                        printf("doing species [%s]\n", pB->getSpeciesName());
                        // read agent data
                        iResult = readAgentData(pB, pAgentDataFile, pCG, pSurf);
                        
                        printf(" species [%s] done: %ld agents\n", pB->getSpeciesName(), pB->getNumAgentsTotal());
                    } else {
                        printf("Couldn't create data from [%s]\n", pClsDataFile);
                    }
                } else {
                    printf("Expected <clsfile>:<datafile>: [%s]\n", apArgV[1]);
                }
                     
            }
            
            char *pOutput  = apArgV[iArgC-1];
            printf("result before writing to [%s]: %d\n", pOutput, iResult);
            // dummy time value
            float fTime = -1;
            // create output file
            hid_t hFile = qdf_createFile(pOutput, fTime);
            if (hFile >= 0) {
                PopWriter *pPW = new PopWriter(vPops);
                pPW->write(hFile);
                qdf_closeFile(hFile);
                printf("Written to QDF file [%s]\n", pOutput);
               
            } else {
                printf("Couldn't create QDF file [%s]\n", pOutput);
            }
        }
        delete pSurf;
        delete pCG;
        
    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
