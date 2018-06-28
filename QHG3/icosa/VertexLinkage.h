#ifndef __VERTEXLINKAGE_H__
#define __VERTEXLINKAGE_H__

#include <stdio.h>

#include <map>
#include <vector>
#include <set>

#include "types.h"
#include "Vec3D.h"
#include "icoutil.h"

class IcoFace;
class PolyFace;

struct classcomp {
  bool operator() (const Vec3D* lhs, const Vec3D* rhs) const
    {return (*lhs)<(*rhs);}
};

#define ADDTYPE_NONE  -1
#define ADDTYPE_FACE   0
#define ADDTYPE_FACEID 1

#define MODE_PMESH 0

typedef std::map<gridtype, std::set<gridtype> > vlinks;

typedef  std::pair<Vec3D *, Vec3D *> edge;
typedef std::map<edge, Vec3D* , bool(*)(edge, edge)> edgemap;

typedef std::map<Vec3D *, std::set<gridtype> > vdualedgemap;


class VertexLinkage {
public:
    VertexLinkage();
    ~VertexLinkage();
    int addFace(IcoFace *pF);
    int addFace(IcoFace *pF, gridtype *aiID);
    int addPolyFace(int iNumVerts, Vec3D **apVerts, int *aiID);
    int addPolyFace2(int iNumVerts, PolyFace *pFace, int *aiID);

    void display(FILE *fOut=NULL);
    long long getNumVertices() { return m_mI2V.size();};

    Vec3D *getVertex(gridtype lID); 
    //    gridtype getVertexID(Vec3D * pv);

    std::set<gridtype> &getLinksFor(gridtype lID) { return m_vLinks[lID];};
    std::map<gridtype, Vec3D *> m_mI2V;

    void destroyVertices();
    int merge(VertexLinkage *pVL);
    int getNumFaces() { return m_iNumFaces;};
    int collectNeighborIDs(gridtype iID, int iDist, std::set<gridtype> & sIds);

    std::map<gridtype, std::set<std::pair< double, gridtype> > > m_mI2H;
    Vec3D *getCenter(gridtype lID);

    std::map<Vec3D, std::set<std::pair< double, Vec3D*> > > m_mI2S;

    double calcArea(gridtype lNode);

    std::vector<Vec3D *> m_vDualCenters;

protected:

    int insertCenter(gridtype lNode, Vec3D *pvCenter);
    int insertFlatCenter(gridtype lNode, Vec3D *pvCenter);

    vlinks m_vLinks;
    gridtype m_iCurID;
    std::map<Vec3D *, gridtype, classcomp> m_mV2I;
    std::map<Vec3D *, gridtype, classcomp> m_mC2I;
    std::vector<Vec3D *> m_vCenters;

    int m_iAddType; // ADDTYPE_FACE: addFace(Face*), ADDTYPE_FACEID: addFace(Face*, int)
    int m_iNumFaces;
    
};


#endif
