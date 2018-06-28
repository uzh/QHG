#include <stdio.h>
#include <hdf5.h>
#include <math.h>

#include "Vec3D.h"
#include "SCell.h"
#include "QDFUtils.h"
#include "Angulator.h"


const double RAD = M_PI/180.0;
const double EPS = 1e-6;
const int    MAX_NEIGHBORS = 6;

//----------------------------------------------------------------------------
// createInstance
Angulator *Angulator::createInstance(const char *pFile) {
    Angulator *pAng = NULL;
    hid_t hFile = qdf_openFile(pFile, true);
    if (hFile > 0) {
        pAng = new Angulator(hFile);
    }
    return pAng;
}


//----------------------------------------------------------------------------
// createInstance
Angulator *Angulator::createInstance(hid_t hFile) {
    return new Angulator(hFile);
}

//----------------------------------------------------------------------------
// constructor
//
Angulator::Angulator(hid_t hFile)
    : m_hFile(hFile),
      m_hGridGroup(H5P_DEFAULT),
      m_hGeoGroup(H5P_DEFAULT),
      m_iNumCells(0),
      m_pdLons(NULL),
      m_pdLats(NULL),
      m_pdAngles(NULL),
      m_pdAngles2(NULL),
      m_pdDirs(NULL),
      m_pdDirs2(NULL),
      m_aCells(NULL) {
    // nothing to do
}


//----------------------------------------------------------------------------
// destructor
//
Angulator::~Angulator() {
    closeGroups();
    destroyArrays();
}

//----- geometric utils

//----------------------------------------------------------------------------
// polar2cart
//
void Angulator::polar2Cart(double dTheta, double dPhi, Vec3D *pVec) {
    double dX = cos(dTheta)*cos(dPhi);    
    double dY = sin(dTheta)*cos(dPhi);    
    double dZ = sin(dPhi);    
    
    pVec->set(dX, dY, dZ);
}


//----------------------------------------------------------------------------
// getNorth
//   returns north pointing tangential vector at (dTheta, dPhi) (radians)
//   returns (0,0,0) at poles
//
void Angulator::getNorth(double dTheta, double dPhi, Vec3D *pvNorth) {
    double dCP = cos(dPhi);
    double dSP = sin(dPhi);

    if (fabs(dCP) > (1 - EPS)) {
        pvNorth->set(0,0,0);
    } else {
        pvNorth->set(-dSP*cos(dTheta), -dSP*sin(dTheta), dCP);
    }
}


//----------------------------------------------------------------------------
// calcAngle
//   - project the dir vector to the tangential plane at pvVert
//   - determine angle to pvNorth
//   - determine east or west
//   negative angles are toeards the east
//
double Angulator::calcAngle(Vec3D *pvNorth, Vec3D *pvVert, Vec3D *pvDir) {

    // project the direction vector onto the tangenital plane
    Vec3D vVertCopy(pvVert);
    double d1 = vVertCopy.dotProduct(pvDir);
    vVertCopy.scale(d1);
    pvDir->subtract(&vVertCopy);
    
    // angle in tangential plane
    double dA = pvNorth->getAngle(pvDir);
    
    // now find out if it is towards east (=> neg)  or towards west (=> pos)
    Vec3D *pVTemp = pvNorth->crossProduct(pvDir);
    double d2 = pVTemp->dotProduct(pvVert);
    if (d2 < 0) {
        dA = -dA;
    }
    if (isnan(dA)) {
        printf("Have nan for angle\n");
    }
    delete pVTemp;
    return dA;
}

//----- HDF5 utilities

//----------------------------------------------------------------------------
// createCellDataType
//
hid_t Angulator::createCellDataType() {
    hid_t hCellDataType = H5Tcreate (H5T_COMPOUND, sizeof(SCell));
    H5Tinsert(hCellDataType, GRID_DS_CELL_ID,    HOFFSET(SCell, m_iGlobalID),      H5T_NATIVE_INT);
    H5Tinsert(hCellDataType, GRID_DS_NUM_NEIGH,  HOFFSET(SCell, m_iNumNeighbors),  H5T_NATIVE_UCHAR);
    hsize_t dims = MAX_NEIGH;
    hid_t hAttrArr = H5Tarray_create2(H5T_NATIVE_INT, 1, &dims);
    H5Tinsert(hCellDataType, GRID_DS_NEIGHBORS,  HOFFSET(SCell, m_aNeighbors), hAttrArr);


    return hCellDataType;
}


//----------------------------------------------------------------------------
// readCellData
//
int Angulator::readCellData() {
    int iResult = 0;
    hid_t hCellType = createCellDataType();
    hid_t hDataSet   = H5Dopen(m_hGridGroup, CELL_DATASET_NAME, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);
    
    hsize_t dimsm   = m_iNumCells;
    hid_t hMemSpace = H5Screate_simple (1, &dimsm, NULL); 
    herr_t status   = H5Dread(hDataSet, hCellType, hMemSpace, hDataSpace, H5P_DEFAULT, m_aCells);
    
    iResult = (status >= 0)?0:-1;
    return iResult;
}



//----------------------------------------------------------------------------
// openGroups
//
int Angulator::openGroups() {
    int iResult = -1;

    m_hGridGroup = qdf_openGroup(m_hFile, GRIDGROUP_NAME, true);
    if (m_hGridGroup > 0) {

        iResult = qdf_extractAttribute(m_hGridGroup, GRID_ATTR_NUM_CELLS, 1, &m_iNumCells); 
        if (iResult == 0) {

            m_hGeoGroup =  qdf_openGroup(m_hFile, GEOGROUP_NAME, true);
            if (m_hGeoGroup > 0) {
                iResult = 0;
            } else {
                qdf_closeGroup(m_hGridGroup);
                printf("Couldn't open geo group\n");
            }

        } else {
            printf("Couldn't extract attribute [%s]\n", GRID_ATTR_NUM_CELLS);
        }


    } else {
        printf("Couldn't open grid group\n");
    }

    return iResult;
}


//----------------------------------------------------------------------------
// closeGroups
//
void Angulator::closeGroups() {
    if (m_hGridGroup != H5P_DEFAULT) {
        qdf_closeGroup(m_hGridGroup);
        m_hGridGroup = H5P_DEFAULT;
    }
    if (m_hGeoGroup != H5P_DEFAULT) {
        qdf_closeGroup(m_hGeoGroup);
        m_hGeoGroup = H5P_DEFAULT;
    }
    if (m_hFile != H5P_DEFAULT) {
        qdf_closeFile(m_hFile);
        m_hFile = H5P_DEFAULT;
    }
}


//----- array utilities


//----------------------------------------------------------------------------
// prepareArrays
//
int Angulator::prepareArrays(bool bAngles) {
    int iResult = 0;

    m_pdLons   = new double[m_iNumCells];
    m_pdLats   = new double[m_iNumCells];
    if (bAngles) {
        m_pdAngles = new double[m_iNumCells*MAX_NEIGHBORS];
        m_pdAngles2 = new double[m_iNumCells*MAX_NEIGHBORS];
    }
    m_pdDirs  = new double[3*m_iNumCells*MAX_NEIGHBORS];
    m_pdDirs2 = new double[3*m_iNumCells*MAX_NEIGHBORS];
    m_aCells  = new SCell[m_iNumCells];

    iResult = qdf_readArray(m_hGeoGroup, GEO_DS_LONGITUDE, m_iNumCells, m_pdLons);
    if (iResult == 0) {
        iResult = qdf_readArray(m_hGeoGroup, GEO_DS_LATITUDE, m_iNumCells, m_pdLats);
        if (iResult == 0) {
#pragma omp parallel for
            for (int i = 0; i < m_iNumCells; i++) {
                m_pdLons[i] *= RAD;
                m_pdLats[i] *= RAD;
            }

            iResult = readCellData();
        } else {
            printf("Couldn't read array [%s]\n", GEO_DS_LATITUDE); 
        }
    } else {
        printf("Couldn't read array [%s]\n", GEO_DS_LONGITUDE); 
    }
            
    return iResult;    
}


//----------------------------------------------------------------------------
// destroyArrays
//
void Angulator::destroyArrays() {
    if (m_pdLons != NULL) {
        delete[] m_pdLons;
        m_pdLons = NULL;
    }
    if (m_pdLats != NULL) {
        delete[] m_pdLats;
        m_pdLats = NULL;
    }
    if (m_pdAngles != NULL) {
        delete[] m_pdAngles;
        m_pdAngles = NULL;
    }
    if (m_pdAngles2 != NULL) {
        delete[] m_pdAngles2;
        m_pdAngles2 = NULL;
    }
    if (m_pdDirs != NULL) {
        delete[] m_pdDirs;
        m_pdDirs = NULL;
    }
    if (m_pdDirs2 != NULL) {
        delete[] m_pdDirs2;
        m_pdDirs2 = NULL;
    }
    if (m_aCells != NULL) {
        delete[] m_aCells;
        m_aCells = NULL;
    }
}



//----------------------------------------------------------------------------
// calcAngles
//
int Angulator::calcAngles(double *pdAngles) {
    int iResult = 0;


    if (iResult == 0) {

        // loop through cell indexes
        Vec3D *pvVert  = new Vec3D(0,0,0);
        Vec3D *pvDir   = new Vec3D(0,0,0);
        Vec3D *pvNorth = new Vec3D(0,0,0);
 
#pragma omp parallel for
        for (int iC = 0; iC < m_iNumCells; ++iC) {
            double dTheta = m_pdLons[iC];
            double dPhi   = m_pdLats[iC];

            polar2Cart(dTheta, dPhi, pvVert);

            getNorth(dTheta, dPhi, pvNorth);

            int iBase = iC*MAX_NEIGHBORS;
            for (int i = 0; i < MAX_NEIGHBORS; i++) {
                int iN = m_aCells[iC].m_aNeighbors[i];
                if (iN >= 0) {
                    
                    double dTheta2 = m_pdLons[iN];
                    double dPhi2   = m_pdLats[iN];

                    polar2Cart(dTheta2, dPhi2, pvDir);

                    pvDir->subtract(pvVert);
                    
                    pdAngles[iBase++] = calcAngle(pvNorth, pvVert, pvDir);
                } else {
                    pdAngles[iBase++] = 0;
                }
            }
            
        }
        delete pvNorth;
        delete pvVert;
        delete pvDir;
    }
    
    return iResult;

}


//----------------------------------------------------------------------------
// calcDirs
//
int Angulator::calcDirs(double *pdDirs) {
    int iResult = 0;
    Vec3D *pvVert  = new Vec3D(0,0,0);
    Vec3D *pvDir   = new Vec3D(0,0,0);

    double dLength = 0.03;
    if (iResult == 0) {

 
#pragma omp parallel for
        for (int iC = 0; iC < m_iNumCells; ++iC) {
            double dTheta = m_pdLons[iC];
            double dPhi   = m_pdLats[iC];

            polar2Cart(dTheta, dPhi, pvVert);


            int iBase = iC*3*MAX_NEIGHBORS;
            for (int i = 0; i < MAX_NEIGHBORS; i++) {
                int iN = m_aCells[iC].m_aNeighbors[i];
                if (iN >= 0) {
                    
                    double dTheta2 = m_pdLons[iN];
                    double dPhi2   = m_pdLats[iN];

                    polar2Cart(dTheta2, dPhi2, pvDir);

                    pvDir->subtract(pvVert);
                    pvDir->normalize();
                    pvDir->scale(dLength);
                    pdDirs[iBase++] = pvDir->m_fX;
                    pdDirs[iBase++] = pvDir->m_fY;
                    pdDirs[iBase++] = pvDir->m_fZ;
                } else {
                    pdDirs[iBase++] = 0;
                    pdDirs[iBase++] = 0;
                    pdDirs[iBase++] = 0;
                }
            }
            
        }
        delete pvVert;
        delete pvDir;
    }
    
    return iResult;

}



//----------------------------------------------------------------------------
// saveAngles
//
int Angulator::saveAngles() {
    return qdf_replaceArray(m_hGeoGroup, GEO_DS_ANGLES, m_iNumCells*MAX_NEIGHBORS, m_pdAngles);
}


//----------------------------------------------------------------------------
// saveDirs
//
int Angulator::saveDirs() {
    return qdf_replaceArray(m_hGeoGroup, GEO_DS_DIRS, m_iNumCells*MAX_NEIGHBORS*3, m_pdDirs);
}


//----------------------------------------------------------------------------
// doAngles
//
int Angulator::doAngles() {
    int iResult = 0;

    printf("opening groups...\n");
    if (iResult == 0) {
        iResult = openGroups();
    }

    printf("preparing arrays...\n");
    if (iResult == 0) {
        iResult = prepareArrays(true);
    }

    printf("calculating angles...\n");
    if (iResult == 0) {
        iResult = calcAngles(m_pdAngles);
    }

    printf("saving angles...\n");
    if (iResult == 0) {
        iResult = saveAngles();
    }

    printf("calculating dirs...\n");
    if (iResult == 0) {
        iResult = calcDirs(m_pdDirs);
    }

    printf("saving dirs...\n");
    if (iResult == 0) {
        iResult = saveDirs();
    }


    if (iResult != 0) {
        printf("error when saving\n");
    }
    printf("cleaning up...\n");

    destroyArrays();
    closeGroups();
    return iResult;
}

//----------------------------------------------------------------------------
// qdf_readArraySlabT
//   double
//
int qdf_readArraySlabT(hid_t hGroup, const char *pName, int iSize, int iOffset, int iStride, double *pData) {
    herr_t status = H5P_DEFAULT;
    
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hsize_t iSCount = iSize;  

        hsize_t iSOffset = iOffset;

        hsize_t iSStride = iStride;
   
        hsize_t dims = iSize;

        
        hsize_t iSOffsetM = 0;
        hsize_t iSCountM  = dims;
        
        hid_t hMemSpace  = H5Screate_simple (1, &dims, NULL); 
        status = H5Sselect_hyperslab(hMemSpace, H5S_SELECT_SET, 
                                     &iSOffsetM, NULL, &iSCountM, NULL);

        hid_t hDataSet   = H5Dopen2(hGroup, pName, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iSOffset, &iSStride, &iSCount, NULL);
        status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData);
     
        qdf_closeDataSet(hDataSet);
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSpace(hMemSpace);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}

//----------------------------------------------------------------------------
// qdf_readArraySlabS
//   double
//
int qdf_readArraySlabS(hid_t hGroup, const char *pName, int iCount, int iOffset, int iStride, int iBlock, double *pData) {
    herr_t status = H5P_DEFAULT;
    
    if (H5Lexists(hGroup, pName, H5P_DEFAULT)) {
        hsize_t iSCount = iCount;  

        hsize_t iSOffset = iBlock * iOffset;

        hsize_t iSStride = iBlock * iStride;

        hsize_t iSBlock  = iBlock;


   
        hsize_t dims = iCount;

        
        hsize_t iMOffset = 0;
        hsize_t iMCount  = iBlock*iCount;
        
        hid_t hMemSpace  = H5Screate_simple (1, &iMCount, NULL); 
        status = H5Sselect_hyperslab(hMemSpace, H5S_SELECT_SET, 
                                     &iMOffset, NULL, &iMCount, NULL);
        //        printf("H5Sselect_hyperslab(hMemSpace, H5S_SELECT_SET, %lld, NULL, %lld, NULL)\n", iMOffset, iMCount);
        hid_t hDataSet   = H5Dopen2(hGroup, pName, H5P_DEFAULT);
        hid_t hDataSpace = H5Dget_space(hDataSet);

        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iSOffset, &iSStride, &iSCount, &iSBlock);
        //        printf("H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, %lld, %lld, %lld, %lld)\n", iSOffset, iSStride, iSCount, iSBlock);
        status = H5Dread(hDataSet, H5T_NATIVE_DOUBLE, hMemSpace,
                         hDataSpace, H5P_DEFAULT, pData);
     
        qdf_closeDataSet(hDataSet);
        qdf_closeDataSpace(hDataSpace);
        qdf_closeDataSpace(hMemSpace);
    } else {
        printf("Dataset [%s] does not exist in group\n", pName);
    }
    return (status >= 0)?0:-1;
}



//----------------------------------------------------------------------------
// check
//
int Angulator::check(const char *pFile) {
    int iResult = 0;
    m_hFile = qdf_openFile(pFile);

    printf("opening groups...\n");
    if (iResult == 0) {
        iResult = openGroups();
    }

    printf("preparing arrays...\n");
    if (iResult == 0) {
        iResult = prepareArrays(true);
    }

    printf("getting the angl data...\n");
    // get the data 

    iResult =  qdf_readArray(m_hGeoGroup, GEO_DS_ANGLES, m_iNumCells, m_pdAngles);
        
    char sName[128];
    double *pData = m_pdAngles2;
    for (int i = 0; i < MAX_NEIGHBORS; i++) {
        sprintf(sName, "%s_%d", GEO_DS_ANGLES, i);
        iResult =  qdf_readArraySlabS(m_hGeoGroup, GEO_DS_ANGLES, m_iNumCells, i, MAX_NEIGHBORS, 1, pData);
        //iResult =  qdf_readArraySlabT(m_hGeoGroup, GEO_DS_ANGLES, m_iNumCells, i, MAX_NEIGHBORS, pData);
        pData += m_iNumCells;
    }

    printf("comparing it...\n");
    // compare
    bool bMatch = true;
    for (int iC = 0; bMatch && (iC < m_iNumCells); ++iC) {
        for (int iN = 0; bMatch && (iN < MAX_NEIGHBORS); ++iN) {
            bool bMatch1 = (m_pdAngles2[iN*m_iNumCells+iC] == m_pdAngles[iC*MAX_NEIGHBORS+iN]);
            if (!bMatch1) {
                printf("Mismatch for iC:%d, iN:%d - m_pdAngles2[%d]:%f, m_pdAngles[%d]:%f\n", iC,iN, iN*m_iNumCells+iC, m_pdAngles2[iN*m_iNumCells+iC],iC*MAX_NEIGHBORS+iN,m_pdAngles[iC*MAX_NEIGHBORS+iN]);
            } 
            bMatch = bMatch && bMatch1;
            
        }
    }

    printf("angle check was %sOK\n", bMatch?"":"not ");


    printf("getting the dirl data...\n");
    iResult =  qdf_readArray(m_hGeoGroup, GEO_DS_DIRS, m_iNumCells*MAX_NEIGHBORS*3, m_pdDirs);
    pData = m_pdDirs2;
    for (int i = 0; i < MAX_NEIGHBORS; i++) {
        sprintf(sName, "%s_%d", GEO_DS_ANGLES, i);
        iResult =  qdf_readArraySlabS(m_hGeoGroup, GEO_DS_DIRS, m_iNumCells, i, MAX_NEIGHBORS, 3, pData);
        pData += m_iNumCells*3;
    }
    
    printf("comparing it...\n");
    // compare
    bMatch = true;
    for (int iC = 0; bMatch && (iC < m_iNumCells); ++iC) {
        for (int iN = 0; bMatch && (iN < MAX_NEIGHBORS); ++iN) {
            for (int iV = 0; bMatch && (iV < 3); ++iV) {
                bool bMatch1 = (m_pdDirs2[iN*m_iNumCells*3+3*iC+iV] == m_pdDirs[iC*MAX_NEIGHBORS*3+iN*3+iV]);
                if (!bMatch1) {
                    printf("Mismatch for iC:%d, iN:%d, iV: %d - m_pdDir2[%d]:%e, m_pdDir[%d]:%e\n", iC,iN, iV, iN*m_iNumCells*3+3*iC+iV, m_pdDirs2[iN*m_iNumCells*3+3*iC+iV],iC*MAX_NEIGHBORS*3+iN*3+iV,m_pdDirs[iC*MAX_NEIGHBORS*3+iN*3+iV]);
                } 
                bMatch = bMatch && bMatch1;
            }
        }
    }


    printf("dir check was %sOK\n", bMatch?"":"not ");
        

    return iResult;
}

//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - at every node of an EQsahedron, calculate the directions to\n", pApp);
    printf("     the 6 (or 5) neighbors, and save their angles relative to north\n");
    printf("     as dataset \"Angles\" of the \"Geography\" group in th QDF file. \n");
    printf("     The VisIt-plugin will show them as\n");
    printf("       ANGLES_0, ANGLES_1, ANGLES_2, ANGLES_3, ANGLES_ç, ANGLES_5\n");
    printf("     where ANGLES_X consisits of the angles corresponding to neighbor #X\n");
    printf("usage:\n");
    printf("  %s <QDFgridgeo>\n", pApp);
    printf("where\n");
    printf("  QDFGridGep   a QDF containing a \"GRid\" and a \"Geograpy\" group\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    
    if (iArgC > 1) {

        Angulator *pAng = Angulator::createInstance(apArgV[1]);
        if (pAng != NULL) {
            
            iResult =  pAng->doAngles();
            if (iResult == 0) {
                iResult = pAng->check(apArgV[1]);
            }
            delete pAng;
        }
    } else {
        usage(apArgV[0]);
    }

    return iResult;

}
