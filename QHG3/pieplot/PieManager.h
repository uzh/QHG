#ifndef __PIEMANAGER_H__
#define __PIEMANAGER_H__

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointSet.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include "PieAttributes.h"
#include "PieCreator.h"

#define NUM_TRI 64

#define PIE   1
#define STAR  2
#define BARS  3

class PieManager {
public:
    static PieManager *createInstance(const PieAttributes *pAtt,  vtkDataSet *pDataSet);
    
    void drawPies();
    
    virtual ~PieManager(){};
    void changeAttrs(const PieAttributes *pAtt);

    static vtkPolyData *createPointCube(vtkPolyData *pPoly, double dSize);
    static vtkPolyData *createCellCube(vtkPolyData *pPoly, double dSize);
protected:
    PieManager(bool bCellData, int iNumTri);
    int buildPies(const PieAttributes *pAtt,  vtkDataSet *pDataSet);
    int makeLists(const PieAttributes *pAtt);
    int makePie(const PieAttributes *pAtt, ShapeMode sMode);
    int makePiePoints(const PieAttributes *pAtt, ShapeMode sMode);
    int makePieCells(const PieAttributes *pAtt, ShapeMode sMode);
    int addTriangles();
    int addCellCenters(vtkDataSet *pDataSet, const char *pName);
    int calcColor(int iIndex, float aCol[4]);

    bool m_bCellData;
    int m_iNumDims;
    int m_iNumTri;
    PieCreator *m_pPieCreator;
    ShapeMode m_sCurMode;
    vtkSmartPointer<vtkDataArray> m_vValues; 
    vtkSmartPointer<vtkDataSet>   m_vPoints;
    vtkSmartPointer<vtkDataArray> m_vCenters; 
    uint uglstart;
};


#endif

