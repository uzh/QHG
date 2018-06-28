#ifndef __PIECREATOR_H__
#define __PIECREATOR_H__

#include <vector>

#include "types.h"
#include "Vec3D.h"
#include "Quat.h"


#define PIE   1
#define STAR  2
#define BARS  3

typedef struct {
    Vec3D avVert[3];
} triangle;

typedef  std::vector<triangle>      trianglevec;
typedef  std::map<int, trianglevec> trianglemap;

enum ShapeMode { MODE_NONE, MODE_PIE, MODE_STAR, MODE_BARS};


class PieCreator {
public:


    static PieCreator *createInstance(uint iDim, uint iNumTri);
    PieCreator();
    ~PieCreator();

    int makeTangentTriangles(Vec3D *pvPos, Vec3D *pvNorm, double *dVals, ShapeMode sMode, float fScale, bool bFramed);
    int makeTangentTriangles(double *pfPos, double *pfNorm, double *dVals, ShapeMode sMode, float fScale, bool bFramed);
    trianglemap &getTriangles() { return m_mvTriangleList;};
    trianglevec &getBGTriangles() { return m_vBGTriangleList;};
    void setBG(bool bBG) { m_bBG = bBG;};
    double getSuperMax() { return m_dSuperMax;};

    void displayTriangles();
protected:
    int init(uint iDim, uint iNumTri);
    
    int createAllTrianglesPie();
    int createAllTrianglesStar();
    int createAllTrianglesBars();

    int createAllBGTrianglesPie();
    int createAllBGTrianglesStar();
    int createAllBGTrianglesBars();

    int calcPieSectors(double *dVals);
    int calcStarPoints(double *dVals);
    int calcBarHeights(double *dVals);;
    Quat *calcRotation(Vec3D *pvPos);

    void scaleTriangles();
    void translateTriangles(Vec3D *pvPos);
    void rotateTriangles(Vec3D *pvNorm);

    void scaleBGTriangles();
    void translateBGTriangles(Vec3D *pvPos);
    void rotateBGTriangles(Vec3D *pvNorm);


  

    // number of values
    uint m_iDim;
    uint m_iNumTri;
    // all triangles for the pie
    triangle *m_aTriangleListPie;
    // all triangles for the star
    triangle *m_aTriangleListStar;
    // all triangles for the bars
    triangle *m_aTriangleListBars;

    // all bg triangles for the pie
    triangle *m_aBGTriangleListPie;
    // all bg triangles for the star
    triangle *m_aBGTriangleListStar;
    // all bg triangles for the bars
    triangle *m_aBGTriangleListBars;


    // map of vectors of triangles belonging to a pie
    trianglemap m_mvTriangleList;
    // map of vectors of background triangles belonging to a pie
    trianglevec m_vBGTriangleList;

    double m_dScale;
    double m_dL;
    double m_dMax;
    double m_dSuperMax;

    double m_dStarMaxSpike;
    double m_dBarsRectWidth;
    double m_dBarsRectHeight;

    bool m_bBG;
};

#endif
