#ifndef __EQTRIANGLE_H__
#define __EQTRIANGLE_H__

#include "Vec3D.h"
#include "icoutil.h"

typedef struct {
    gridtype lID;
    Vec3D v;
} node;

typedef struct {
    int aiIdx[3];
} triangle;

class Quat;

class EQTriangle {

public: 
    static EQTriangle *createInstance(int iSubDiv, bool bTegmark);
    static EQTriangle *createEmpty();
    ~EQTriangle();

    int saveNodes(const char * pNodeFile, bool bWithTriangles);
    void displayNodes(bool bWithTriangles);

    int save(FILE *fOut);
    long load(FILE *fIn, long lStartPos);

    void applyQuat(Quat *pQ);
    EQTriangle *copy();

    Vec3D *getCorner(int i);
    void shift(Vec3D *pvShift);
    void scale(double dScale);
    void normalize();

    gridtype getNumNodes() {return m_iNumNodes;};
    gridtype getNumTriangles() {return m_iNumTris;};
    triangle *getTriangles() { return m_pTriangles;};
    node *getNodes() { return m_pNodes;};
    void deleteTriangles();
    //    gridtype findNode(Vec3D *pV, gridtype *piIdx);
    int findNeighbors(Vec3D *pVPlane, gridtype *aiNeighborIDs);
    gridtype findFaceID(Vec3D *pVPlane);
protected:
    EQTriangle();
    int init(int iSubDiv, bool bTegmark);
    void transformTriangle();

    int createSubdividedTriangle(double dSide, int iSubDiv);
    gridtype *nodesInSeedTriangle(gridtype *piSelected);

    void transformSeedNodes();
    void mirrorNodesX();
    void rotateNodes120();

public:
    void shearScale(double dX, double dY, Vec3D *pV);
    void invShearScale(double dX, double dY, Vec3D *pV);
protected:
    long readHeader(FILE *fIn, long lStartPos);

    int        m_iN;  // = num subdivs + 1 = number of segments on edge
    gridtype   m_iNumNodes;
    node      *m_pNodes;
    gridtype   m_iNumTris;
    triangle  *m_pTriangles;
    bool m_bMirr;

    bool m_bTegmark;
};


#endif
