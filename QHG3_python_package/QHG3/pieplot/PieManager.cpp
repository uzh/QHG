#include <stdio.h>
#include <stdlib.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkCellData.h>
#include <vtkSmartPointer.h>
#include <vtkDataSetAttributes.h>
#include <vtkGenericCell.h>
#include <vtkDoubleArray.h>

#include <GL/gl.h>

#include "PieAttributes.h"
#include "PieManager.h"

#define ARR_NORMALS "Normals"
#define ARR_CENTERS "Centers"


const float MAT_DARK[8][4]  = {
    { 0.5, 0.0, 0.0, 1.0 },
    { 0.0, 0.5, 0.0, 1.0 },
    { 0.0, 0.0, 0.5, 1.0 },
    { 0.5, 0.5, 0.0, 1.0 },
    { 0.5, 0.0, 0.5, 1.0 },
    { 0.0, 0.5, 0.5, 1.0 },
    { 0.5, 0.5, 0.5, 1.0 },
    { 0.0, 0.0, 0.0, 1.0 },
};


const float MAT_BRIGHT[8][4]  = {
    { 1.0, 0.0, 0.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0 },
    { 0.0, 0.0, 1.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
    { 1.0, 0.0, 1.0, 1.0 },
    { 0.0, 1.0, 1.0, 1.0 },
    { 1.0, 1.0, 1.0, 1.0 },
    { 0.0, 0.0, 0.0, 1.0 },
};

//----------------------------------------------------------------------------
// createInstance
//  - determine dimensionality of data
//  - check if point data or cell data
//
PieManager *PieManager::createInstance(const PieAttributes *pAtt,  vtkDataSet *pDataSet) {
    PieManager *pPieManager = NULL;
    std::cout << "[PieManager::createInstance]: dataset" << endl;
    // pDataSet->Print(std::cout); 
    
    bool bCellData = false;
    int iNArrays = 0;
    int iNDims = 0;

    vtkSmartPointer<vtkPointData> pd = pDataSet->GetPointData();
    iNArrays = pd->GetNumberOfArrays();
    if (iNArrays > 0) {
        bCellData = false;
    } else {
        vtkSmartPointer<vtkCellData> cd = pDataSet->GetCellData();
        iNArrays = cd->GetNumberOfArrays();
        if (iNArrays > 0) {
            bCellData = true;
        } else {
            std::cout << "[PieManager::createInstance]: couldn't find a data set" << endl;
        }
    }
    

    if (iNArrays > 0) {
        pPieManager = new PieManager(bCellData, NUM_TRI);
        int iResult = pPieManager->buildPies(pAtt, pDataSet);
        if (iResult != 0) {
            delete pPieManager;
            pPieManager = NULL;
        }
    }
    

    return pPieManager;
}

//----------------------------------------------------------------------------
// constructor
//
 PieManager::PieManager(bool bCellData, int iNumTri) 
     : m_bCellData(bCellData),
       m_iNumDims(0),
       m_iNumTri(iNumTri),
       m_pPieCreator(NULL),
       m_sCurMode(MODE_NONE),
       m_vValues(NULL),
       m_vPoints(NULL),
       m_vCenters(NULL) {
     uglstart = glGenLists(4);
 }


//----------------------------------------------------------------------------
// buildPies
//
int PieManager::buildPies(const PieAttributes *pAtt,  vtkDataSet *pDataSet) {
    int iResult = -1;
    /*
    vtkSmartPointer<vtkPolyData> pDataSetC = 
        vtkSmartPointer<vtkPolyData>::New();
    PieManager::createPointCube(pDataSetC, 10);
    std::cout << "pdatasetc: " << pDataSetC << endl;
    pDataSetC->Print(std::cout);
    */
    vtkSmartPointer<vtkDataSetAttributes> vdsa ;
    if (m_bCellData) {
        addCellCenters(pDataSet, ARR_CENTERS);
        vdsa = pDataSet->GetCellData(); //actually make new dataset from cell centers; use with normals
    } else {
        std::cout << "about to get point data..." << endl;
        vdsa = pDataSet->GetPointData();
        std::cout << "have point data :" << vdsa << endl;
    } 
    int iNArrays = vdsa->GetNumberOfArrays();
    std::cout << "[PieManager::buildPies]: we have a " << (m_bCellData?"Cell":"Point") << " dataset" << endl;
    const char *pSelected = NULL;
    for (int i = 0; i < iNArrays; i++) {
        const char *p = vdsa->GetArrayName(i);
        if ((strcmp(p, ARR_NORMALS) == 0) || (m_bCellData && (strcmp(p, ARR_CENTERS) == 0))) {
        } else {
            if (pSelected == NULL) {
                pSelected = p;
            }
            std::cout << "  " << p << endl;
        }
    }
    if (pSelected != NULL) {
        if (m_bCellData) {
            // calculate centers of cells and create new pointset from it
            // m_vPoints = ...        
            // std::cout << "[PieManager::buildPies]: cell data sets currently not supported" << endl;
            m_vPoints = pDataSet;
            m_vCenters = vdsa->GetArray(ARR_CENTERS);
        } else {
            m_vPoints = pDataSet;
        }

        if (m_vPoints != NULL) {
            std::cout << "[PieManager::buildPies]: selected [" << pSelected << "]" << endl;
            m_vValues = vdsa->GetArray(pSelected);
            
            m_iNumDims = m_vValues->GetNumberOfComponents();
            int iSize = m_vValues->GetNumberOfTuples();
            vtkIdType iNumPoints = 0;
            if (m_bCellData) {
                iNumPoints = m_vCenters->GetNumberOfTuples();
            } else {
                iNumPoints =  m_vPoints->GetNumberOfPoints();
            }

            if (iNumPoints == iSize) {
                
                std::cout << "[PieManager::createInstance]: Dimension: " << m_iNumDims << endl;
                std::cout << "[PieManager::createInstance]: Size: " << iSize << endl;
                std::cout << "[PieManager::createInstance]: number of points : " << iNumPoints << endl;
                
                m_pPieCreator = PieCreator::createInstance(m_iNumDims, NUM_TRI);
                if (m_pPieCreator != NULL) {
                    iResult = makeLists(pAtt); 
                    std::cout << "[PieManager::createInstance]: result afterMakeList: " << iResult << endl;
                } else {
                    std::cout << "[PieManager::createInstance]: couldn't create PieCreator" << endl;
                    iResult = -1;
                }
                
            } else {
                std::cout << "[PieManager::createInstance]: problem: umber of points [" << iNumPoints << " is different from data size [" << iSize << "]" << endl;
            }
        } // m_vPoints != NULL
    } else {
        std::cout << "[PieManager::createInstance]: nothing selected" << endl;
    }
    std::cout << endl;

    return iResult;
}

//----------------------------------------------------------------------------
// makeLists
//
int PieManager::makeLists(const PieAttributes *pAtt) {
    int iResult = 0;
    // get necessary stuff from attributes
    PieAttributes::GlyphStyle gs = pAtt->GetIGlyphStyle();
    switch (gs) {
    case  PieAttributes::STYLE_PIE: 
        m_sCurMode = MODE_PIE;
        break;
    case  PieAttributes::STYLE_STAR: 
        m_sCurMode = MODE_STAR;
        break;
    case  PieAttributes::STYLE_BARS: 
        m_sCurMode = MODE_BARS;
        break;
    default:
        m_sCurMode = MODE_NONE;
    }

    if (iResult == 0) {
        glNewList(uglstart+MODE_PIE, GL_COMPILE);
        iResult = makePie(pAtt, MODE_PIE);
        glEndList();
    }
   
    if (iResult == 0) {
        glNewList(uglstart+MODE_STAR, GL_COMPILE);
        iResult = makePie(pAtt, MODE_STAR);
        glEndList();
    }

    if (iResult == 0) {
        glNewList(uglstart+MODE_BARS, GL_COMPILE);
        iResult = makePie(pAtt, MODE_BARS);
        glEndList();
    }
 
    std::cout << "Supermax: " << m_pPieCreator->getSuperMax() << endl;
    return iResult;
}


//----------------------------------------------------------------------------
// makePie
//
int PieManager::makePie(const PieAttributes *pAtt, ShapeMode sMode) {
    int iResult = -1;
    if (m_bCellData) {
        iResult = makePieCells(pAtt, sMode);
    } else {
        iResult = makePiePoints(pAtt, sMode);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// makePiePoints
//
int PieManager::makePiePoints(const PieAttributes *pAtt, ShapeMode sMode) {
    int iResult = 0;

    int iNumPoints = m_vPoints->GetNumberOfPoints();

    float fScale = pAtt->GetFScale1();
    bool bFramed = pAtt->GetBFramed();

    double *adVal = new double[m_iNumDims];
    double aPos[3];
    for (int j = 0; (iResult == 0) && (j < iNumPoints); j++) {
        m_vValues->GetTuple(j, adVal);
        m_vPoints->GetPoint(j, aPos);
        // distance criterion for testset only; inorder to have surface points only
        double d = sqrt(aPos[0]*aPos[0]+aPos[1]*aPos[1]+aPos[2]*aPos[2]); 
        if (d > 9) {
            // z-random for testset only
            //    adVal[2] += 1-((2.0*rand())/RAND_MAX);
            iResult = m_pPieCreator->makeTangentTriangles(aPos, aPos, adVal, sMode, fScale, bFramed);
            if (iResult == 0) {
                iResult = addTriangles();
            }
        }
    }

    delete[] adVal;
    return iResult;

}

//----------------------------------------------------------------------------
// makePiePoints
//
int PieManager::makePieCells(const PieAttributes *pAtt, ShapeMode sMode) {
    int iResult = 0;

    int iNumPoints = m_vPoints->GetNumberOfPoints();

    float fScale = pAtt->GetFScale1();
    bool bFramed = pAtt->GetBFramed();

    double *adVal = new double[m_iNumDims];
    double aPos[3];
    for (int j = 0; (iResult == 0) && (j < iNumPoints); j++) {
        m_vValues->GetTuple(j, adVal);
        m_vCenters->GetTuple(j, aPos);
        // distance criterion for testset only; inorder to have surface points only
        double d = sqrt(aPos[0]*aPos[0]+aPos[1]*aPos[1]+aPos[2]*aPos[2]); 
        if (d > 9) {
            // z-random for testset only
            adVal[2] += 1-((2.0*rand())/RAND_MAX);
            iResult = m_pPieCreator->makeTangentTriangles(aPos, aPos, adVal, sMode, fScale, bFramed);
            if (iResult == 0) {
                iResult = addTriangles();
            }
        }
    }

    delete[] adVal;
    return iResult;

}

//----------------------------------------------------------------------------
// addTriangles
//
int PieManager::calcColor(int iIndex, float aCol[4]) {
    aCol[0]=1.0;
    aCol[1]=1.0;
    aCol[2]=1.0;
    aCol[3] = 1.0;
    /*
    int k = (m_iNumDims+1)/2;
    if (iIndex < m_iNumDims/2) {
        iIndex *=2;
    } else {
        iIndex = (iIndex- k)*2 +1;
    }
    */
    int k = m_iNumDims/2;
    if ((m_iNumDims % 2) == 0) {
        iIndex = (iIndex*(k-1)) % m_iNumDims;

    } else {
        iIndex = (iIndex*k) % m_iNumDims;
    }
    double dValue = (1.0*iIndex)/m_iNumDims;
    if (6*dValue < 1) {
        double z = 6*dValue;
        aCol[0]  = 1.0; 
        aCol[1]  = z; //z*(2.0-z);
        aCol[2]  = 0.0;

    } else if (6*dValue < 2) {
        double z = 6*dValue - 1;
        aCol[0]  = 1.0 - z; //1.0-z*z; 
        aCol[1]  = 1.0;
        aCol[2]  = 0.0;
            
    } else if (6*dValue < 3) {
        double z = 6*dValue - 2;
        aCol[0]  = 0.0;
        aCol[1]  = 1.0;
        aCol[2]  = z; //z*(2.0-z);

    } else if (6*dValue < 4) {
        double z = 6*dValue - 3;
        aCol[0]  = 0.0;
        aCol[1]  = 1.0 -z; //1.0-z*z;
        aCol[2]  = 1.0;

    } else if (6*dValue < 5) {
        double z = 6*dValue - 4;
        aCol[0]  = z; //z*(2.0-z);
        aCol[1]  = 0.0;
        aCol[2]  = 1.0;

    } else if (6*dValue < 6) {
        double z = 6*dValue - 5;
        aCol[0]  = 1.0;
        aCol[1]  = 0.0;
        aCol[2]  = 1.0-z; //1.0-z*z;
    }
    //    printf("%d -> %f- >[%.3f,%.3f,%.3f,%.3f]\n", iIndex, dValue, aCol[0], aCol[1], aCol[2], aCol[3]);
    return 0;
}


//----------------------------------------------------------------------------
// addTriangles
//
int PieManager::addTriangles() {
    int iResult = 0;
    trianglemap &tm = m_pPieCreator->getTriangles();
    int iCol = 0;
    float fLift = 1.001;

    glBegin(GL_TRIANGLES);

    trianglemap::const_iterator it;
    // front side bight
    for (it = tm.begin(); it != tm.end(); ++it) {
        float aCol[4];
        calcColor(iCol, aCol);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, aCol /*MAT_BRIGHT[iCol%8]*/);
        iCol++;
        for (uint j = 0; j < it->second.size(); ++j) {
            const triangle &t = it->second[j];
            for (int k = 0; k < 3; k++) {
                //                printf("Adding triangle(%7.3f,%7.3f,%7.3f)\n", fLift*t.avVert[k].m_fX, fLift*t.avVert[k].m_fY, fLift*t.avVert[k].m_fZ);
                glVertex3f(fLift*t.avVert[k].m_fX, fLift*t.avVert[k].m_fY, fLift*t.avVert[k].m_fZ);
            }
        }
    }
    
    /*
    iCol = 0;
    // backside dark
    for (it = tm.begin(); it != tm.end(); ++it) {
        glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, MAT_DARK[iCol%8]);
        iCol++;
        for (uint j = 0; j < it->second.size(); ++j) {
            const triangle &t = it->second[j];

            for (int k = 2; k >= 0; k--) {
                glVertex3f(t.avVert[k].m_fX, t.avVert[k].m_fY, t.avVert[k].m_fZ);
            }

        }
    }
    */
    float aBGCol[4] = {0.3, 0.3, 0.3, 1.0};
    trianglevec &tv = m_pPieCreator->getBGTriangles();
    trianglevec::const_iterator itv;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, aBGCol);
    for (itv = tv.begin(); itv != tv.end(); itv++) {
        for (int k = 0; k < 3; k++) {
            glVertex3f(fLift*itv->avVert[k].m_fX, fLift*itv->avVert[k].m_fY, fLift*itv->avVert[k].m_fZ);
        }
        for (int k = 2; k >= 0; k--) {
            glVertex3f(fLift*itv->avVert[k].m_fX, fLift*itv->avVert[k].m_fY, fLift*itv->avVert[k].m_fZ);
        }
    }
    

    glEnd();

    return iResult;
}

//----------------------------------------------------------------------------
// drawPies
//
void PieManager::drawPies() {
    glCallList(uglstart+m_sCurMode);
}

//----------------------------------------------------------------------------
// changeAttrs
//
void PieManager::changeAttrs(const PieAttributes *pAtt) {   
    ShapeMode sNew = MODE_NONE;
    PieAttributes::GlyphStyle gs = pAtt->GetIGlyphStyle();
    switch (gs) {
    case  PieAttributes::STYLE_PIE: 
        sNew = MODE_PIE;
        break;
    case  PieAttributes::STYLE_STAR: 
        sNew = MODE_STAR;
        break;
    case  PieAttributes::STYLE_BARS: 
        sNew = MODE_BARS;
        break;
    default:
        sNew = MODE_NONE;
    }

    if ((sNew != MODE_NONE) && (m_sCurMode != sNew)) {
        m_sCurMode = sNew;
    }
    glCallList(uglstart+m_sCurMode);
}

//----------------------------------------------------------------------------
// addCellCenters
//
int PieManager::addCellCenters(vtkDataSet *pDataSet, const char *pName) {
    int iResult = 0;
    std::cout << "adding cell centers ..." << endl;
 
    vtkCellData *pcdata = pDataSet->GetCellData();
    int iNumCells = pDataSet->GetNumberOfCells();
 
    vtkSmartPointer<vtkDoubleArray> vda = vtkSmartPointer<vtkDoubleArray>::New();
    vda->SetName(pName);
    vda->SetNumberOfTuples(iNumCells);

    std::cout << "looping (num cells: " << iNumCells << ") ..." << endl;

    for (int i = 0; i < iNumCells; i++) {
        vtkCell *c = pDataSet->GetCell(i);
        
        int iNumPoints = c->GetNumberOfPoints();

        double *dWeights = new double[iNumPoints];
        double pc[3];
        double pos[3];
        int subid = 0;

        c->GetParametricCenter(pc);
        c->EvaluateLocation(subid, pc, pos, dWeights);
        if (i < 20) {
            std::cout << "point #" << i <<": " << pos[0] << " " << pos[1] << " " << pos[2] << endl;
        }
        vda->SetTuple(i, pos);
        delete[] dWeights;
    }
    std::cout << "adding array ..." << endl;
    pcdata->AddArray(vda);


    return iResult;
}


const double aaPoints[8][3] = {
    { 1,  1,  1},
    { 1,  1, -1},
    { 1, -1,  1},
    { 1, -1, -1},
    {-1,  1,  1},
    {-1,  1, -1},
    {-1, -1,  1},
    {-1, -1, -1},
};

const double aaPointData[8][7] = {
    {0.3, 0.1, 0.4, 0.1, 0.5, 0.9, 0.2},
    {0.2, 0.7, 0.1, 0.8, 0.2, 0.8, 0.1},
    {0.1, 0.4, 0.1, 0.4, 0.2, 0.1, 0.3},
    {0.0, 0.8, 0.6, 0.6, 0.0, 0.2, 0.5},
    {1.0, 0.6, 0.1, 0.8, 0.0, 0.3, 0.3},
    {0.1, 0.9, 0.4, 0.5, 0.9, 0.1, 0.0},
    {0.8, 0.7, 0.7, 0.4, 0.9, 0.6, 0.4},
    {2.0, 0.0, 0.8, 0.0, 0.0, 0.8, 0.3},
};

vtkPolyData *PieManager::createPointCube(vtkPolyData *polyData, double dSize) {
    vtkSmartPointer<vtkPoints> points =
        vtkSmartPointer<vtkPoints>::New();

    double x[3];
    for (int i = 0; i < 8; i++) {
        memcpy(x, aaPoints[i], 3 * sizeof(double));
        for (int i = 0; i < 3; i++) {
            x[i] *= dSize;
        }
        points->InsertNextPoint(x);
    }

    /*
    // Create a polydata to store everything in
    vtkSmartPointer<vtkPolyData> polyData = 
        vtkSmartPointer<vtkPolyData>::New();
    */

    // Add the points to the dataset
    polyData->SetPoints(points);

    vtkSmartPointer<vtkPointData> pointData = polyData->GetPointData();
    vtkSmartPointer<vtkDoubleArray> vda = 
        vtkSmartPointer<vtkDoubleArray>::New();
    vda->SetNumberOfComponents(7); 
    vda->SetNumberOfTuples(8);
    vda->SetName("PointCube");
    for (int i = 0; i < 8; i++) {
        vda->SetTuple(i, aaPointData[i]);
    }
    pointData->AddArray(vda);

    vtkSmartPointer<vtkDoubleArray> normalsArray = 
        vtkSmartPointer<vtkDoubleArray>::New();
    normalsArray->SetNumberOfComponents(3); //3d normals (ie x,y,z)
    normalsArray->SetNumberOfTuples(8);
    normalsArray->SetName(ARR_NORMALS);
    for (int i = 0; i < 8; i++) {
        normalsArray->SetTuple(i, aaPoints[i]);
    }
    pointData->SetNormals(normalsArray);

    vtkDataArray *vda3 = pointData->GetNormals();
    std::cout << "orig : " << normalsArray << "; vda3 : " << vda3 << endl;
    if (vda3 != NULL) {
        std::cout << "vda3 numtup: " << vda3->GetNumberOfTuples() << " x " <<vda3->GetNumberOfComponents() << endl;
    }
    //    polyData->Print(std::cout);

    vtkSmartPointer<vtkPointData> pointData2 =   polyData->GetPointData();
    std::cout << "pointdata: " << pointData2 << endl;

    vtkPolyDataWriter *pdw = vtkPolyDataWriter::New();
    pdw->SetFileName("pointCube.vtk");
    pdw->SetInputData(polyData);
    pdw->Write();

    return polyData;    
}

vtkPolyData *PieManager::createCellCube(vtkPolyData *polyData, double dSize) {
    vtkSmartPointer<vtkPoints> points =
        vtkSmartPointer<vtkPoints>::New();

    double x[3];
    for (int i = 0; i < 8; i++) {
        memcpy(x, aaPoints[i], 3 * sizeof(double));
        for (int i = 0; i < 3; i++) {
            x[i] *= dSize;
        }
        points->InsertNextPoint(x);
    }

    /*
    // Create a polydata to store everything in
    vtkSmartPointer<vtkPolyData> polyData = 
        vtkSmartPointer<vtkPolyData>::New();
    */

    // Add the points to the dataset
    polyData->SetPoints(points);


    vtkSmartPointer<vtkCellData> cellData = polyData->GetCellData();
    vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> vda = 
        vtkSmartPointer<vtkDoubleArray>::New();
    vda->SetNumberOfComponents(7); 
    vda->SetNumberOfTuples(8);
    vda->SetName("PointCube");
    for (int i = 0; i < 8; i++) {
        vda->SetTuple(i, aaPointData[i]);
    }
    cellData->AddArray(vda);

    vtkSmartPointer<vtkDoubleArray> normalsArray = 
        vtkSmartPointer<vtkDoubleArray>::New();
    normalsArray->SetNumberOfComponents(3); //3d normals (ie x,y,z)
    normalsArray->SetNumberOfTuples(8);
    normalsArray->SetName(ARR_NORMALS);
    for (int i = 0; i < 8; i++) {
        normalsArray->SetTuple(i, aaPoints[i]);
    }
    cellData->SetNormals(normalsArray);

    vtkDataArray *vda3 = cellData->GetNormals();
    std::cout << "orig : " << normalsArray << "; vda3 : " << vda3 << endl;
    if (vda3 != NULL) {
        std::cout << "vda3 numtup: " << vda3->GetNumberOfTuples() << " x " <<vda3->GetNumberOfComponents() << endl;
    }
    //    polyData->Print(std::cout);

    vtkSmartPointer<vtkPointData> pointData2 =   polyData->GetPointData();
    std::cout << "pointdata: " << pointData2 << endl;

    return polyData;    
}
