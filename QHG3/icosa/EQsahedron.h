#ifndef __EQSAHEDRON_H__
#define __EQSAHEDRON_H__

#include "utils.h"
#include "Quat.h"
#include "IcoFace.h"
#include "FaceChecker.h"
#include "Surface.h"
#include "IcoLoc.h"
#include "VertexLinkage.h"

class Vec3D;
class EQTriangle;
class ValReader;

#define ICOVERTS 12
#define ICOFACES 20
#define ICOEDGES 30


typedef struct {
    Vec3D  *vShift;
    Quat   *qRot;
    Quat   *qRotInv;
    double  dScale;
} transinfo;

class EQsahedron : public Surface, public IcoLoc {

public:
    //    static EQsahedron *createInstance(int iSubDivs);    
    static EQsahedron *createInstance(int iSubDivs, bool bTegmark, FaceChecker *pFC);
    static EQsahedron *createEmpty();
    ~EQsahedron();
 
    // from Surface
    int load(const char *pFile);
    // from Surface
    int save(const char *pFile);
    // from Surface
    virtual PolyFace *findFace(double dLon, double dLat);
    // slower version (brute force)
    virtual PolyFace *findFaceSlow(double dLon, double dLat);
    // from Surface
    virtual gridtype findNode(Vec3D *pV);
    // slower version
    virtual gridtype findNodeSlow(Vec3D *pv);
    // from Surface
    virtual gridtype findNode(double dLon, double dLat);
    // from Surface
    tbox *getBox() {return &m_curBox;};
    // from Surface
    virtual void display();
    // from Surface
    virtual Vec3D* getVertex(gridtype lID) { return m_pVL->getVertex(lID);};
    // from Surface
    virtual int collectNeighborIDs(gridtype lID, int iDist, std::set<gridtype> & sIds) { return m_pVL->collectNeighborIDs(lID, iDist, sIds);};
    // from IcoLoc
    virtual bool findCoords(int iNodeID,double *pdLon, double *pdLat);

    VertexLinkage *getLinkage() { return m_pVL;};

    PolyFace *getFirstFace();
    PolyFace *getNextFace();

    void relink();

    void setGlobalIDs(EQTriangle *pEQ, int iFaceNum);
    bool isPartial() { return m_bPartial;};
    /*
    void setLand(float fMinAlt) { m_fMinAlt = fMinAlt;};
    float getLand() { return m_fMinAlt;};
    */
    int getSubDivs() { return m_iSubDivs; };

    int convertTriangleToEQ(int iFaceNum, int iIndex);
    int convertEQToTriangle(int iEQID, int *piFaceNum);
    EQTriangle *getFaceTriangle(int iFaceNum);
    void show();
protected:
    EQsahedron();
    int init(int iSubDivs, bool bTegmark, FaceChecker *pFC);

    static double calcSinAngle(); 
    void calcIcoVerts();
    void createFaces();

    Quat *calcTriangleMapping(Vec3D *pA1,Vec3D *pA2,Vec3D *pA3,
                              Vec3D *pB1,Vec3D *pB2,Vec3D *pB3);

    void mapTriangle(EQTriangle *pEQ, int iFaceNum);

    void makeIcoFaces(FaceChecker *pFC);

    int findOrientedEdge(int iFaceNum, int iV0, int iV1, bool *pbReversed);

    Vec3D *icoToPlane(int iFace, Vec3D *pV);
    Vec3D *planeToIco(int iFace, Vec3D *pV);
 
    Vec3D *sphereToIco(Vec3D *pV, int *piFace);
 
 
    int     m_iSubDivs;
    bool    m_bTegmark;
    Vec3D *m_apMainVertices[ICOVERTS];
    EQTriangle *m_apEQFaces[ICOFACES];
    IcoFace    **m_apIcoFaces;
    VertexLinkage *m_pVL;
    long m_iNumIcoFaces;
    long m_iCurFace;

    tbox m_curBox;

    IcoFace *m_apMainFaces[ICOFACES];
    transinfo m_tiTrans[ICOFACES];
    Quat m_qGlobal;

    //    float m_fMinAlt;
    bool m_bPartial;

};
#endif
