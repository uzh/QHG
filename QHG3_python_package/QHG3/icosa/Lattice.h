#ifndef __LATTICE_H__
#define __LATTICE_H__

#include <vector>

#include "IcoLoc.h"
#include "Surface.h"
#include "VertexLinkage.h"

class Vec3D;
class GridProjection;
class PolyFace;
class VertexLinkage;

class Lattice : public Surface, public IcoLoc {
public:

    Lattice();
    virtual ~Lattice();
    int create(int iNumLinks, GridProjection *pGP);
    int create(int iNumLinks, const char *pProjType, const char *pProjGrid);

    
    // from Surface
    int save(const char *pFile);
    // from Surface
    int load(const char *pFile);
    // from Surface
    virtual PolyFace *findFace(double dLon, double dLat);
    // from Surface
    virtual gridtype findNode(Vec3D *pv);
    // from Surface
    virtual Vec3D* getVertex(gridtype lID) { return m_pVL->getVertex(lID);};
    // from Surface
    virtual int collectNeighborIDs(gridtype lID, int iDist, std::set<gridtype> & sIds) { return m_pVL->collectNeighborIDs(lID, iDist, sIds);};
    // from Surface
    virtual tbox *getBox() { return &m_curBox;};

    // from Surface
    virtual void display();

    // from Surface/IcoLoc
    virtual gridtype findNode(double dLon, double dLat);
    // from Icoloc:
    virtual bool findCoords(int iNodeID,double *pdLon, double *pdLat);


    GridProjection *getGridProjection() { return m_pGP;};
    VertexLinkage *getLinkage() { return m_pVL;};



    int getNumLinks() { return m_iNumLinks;};
    int getNumPolys() { return m_iNumPolys;};
    PolyFace **getFaces() { return m_apFaces;};
    int getNumX() { return m_iNumX;};
    int getNumY() { return m_iNumY;};
protected:
    int create(int iNumLinks);

    void link4();
    void link6();

    int createRectVL();

    int m_iNumX;
    int m_iNumY;
    int m_iNumLinks;
    GridProjection *m_pGP;
    bool m_bDeleteGP;
    int m_iNumV;
    Vec3D **m_apV;

    VertexLinkage *m_pVL;
    int m_iNumPolys;
    PolyFace ** m_apFaces;
    tbox m_curBox;
public:


};

#endif
