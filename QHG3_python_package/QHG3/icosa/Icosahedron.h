#ifndef __ICOSAHEDRON_H__
#define __ICOSAHEDRON_H__

#include <vector>
#include "types.h"
#include "icoutil.h"
#include "IcoLoc.h"
#include "PolyFace.h"
#include "IcoFace.h"
#include "VertexLinkage.h"
#include "Surface.h"

class Vec3D;
class PolyFace;

class ValReader;

class BufWriter;
class symbuf;

class IcoGridNodes;


typedef int (VertexLinkage::*ADDFACEMETHOD) (IcoFace *);

class Icosahedron :public Surface, public IcoLoc  {
public:
    static Icosahedron *create(double dRadius, int iPolyType);
    virtual ~Icosahedron();
    void subdivide(int iNumLevels);
    void merge(int iNumLevels);
    void subdivideLand(ValReader *pVR, float fMinAlt, int iNumLevels);
   
    void variableSubDiv2(int iMaxLevel, 
                        double dLonMin, double dLonMax, double  dDLon, 
                        double dLatMin, double dLatMax, double dDLat);
    
    void variableSubDiv(int iMaxLevel, 
                        double dLonMin, double dLonMax, double  dDLon, 
                        double dLatMin, double dLatMax, double dDLat);
    
    void variableSubDivOMP(int iMaxLevel, tbox &tBox, double  dDLon, double dDLat);
    void variableSubDivOMPB(int iMaxLevel, tbox &tBox, double  dDLon, double dDLat);

    void variableSubDivOMP2(int iMaxLevel, 
                          double dLonMin, double dLonMax, double  dDLon, 
                          double dLatMin, double dLatMax, double dDLat);

    void relink();
    void relink(bool bDual);
    PolyFace *getFace(int i) { PolyFace *pF = m_apMainFaces[i]; return pF;};
    VertexLinkage *getLinkage() { return m_pVL;};
    int write(FILE *fOut);
    //    void display();
    void facedisplay(IcoFace *pF, const char *pIndent, bool bAll);
    PolyFace *getFirstFace();
    PolyFace *getNextFace();
    int getNumFaces() { return (int)m_vFaceList.size();}
    int getSubLevel() { return m_iSubLevel;};
    void setSubLevel(int i) { m_iSubLevel = i;};

    void setBox(double dLonMin, double dLonMax,
                double dLatMin, double dLatMax) {
        m_curBox.dLonMin = dLonMin;
        m_curBox.dLonMax = dLonMax;
        m_curBox.dLatMin = dLatMin;
        m_curBox.dLatMax = dLatMax;
    };
    void setBox(tbox &tBox) {
        m_curBox.dLonMin = tBox.dLonMin;
        m_curBox.dLonMax = tBox.dLonMax;
        m_curBox.dLatMin = tBox.dLatMin;
        m_curBox.dLatMax = tBox.dLatMax;
    };


    // from Surface
    virtual int save(const char *pFile);
    // from Surface
    virtual int load(const char *pFile);
    // from Surface
    virtual PolyFace *findFace(double dLon, double dLat);
    // from Surface
    virtual gridtype findNode(Vec3D *pv);
    // from Surface
    tbox *getBox() {return &m_curBox;};
    // from Surface
    virtual void display();
    // from Surface
    virtual Vec3D* getVertex(gridtype lID) { return m_pVL->getVertex(lID);};
    // from Surface
    virtual int collectNeighborIDs(gridtype lID, int iDist, std::set<gridtype> & sIds) { return m_pVL->collectNeighborIDs(lID, iDist, sIds);};

    // from Surface/IcoLoc
    virtual gridtype findNode(double dLon, double dLat);
    // from IcoLoc
    virtual bool findCoords(int iNodeID,double *pdLon, double *pdLat);

    bool getPreSel() { return m_bPreSel;};
    void setPreSel(bool bPreSel) {m_bPreSel = bPreSel;};
    void setStrict(bool bStrict) {m_bStrict = bStrict;};




protected:
    int writeFace(BufWriter *pBW, IcoFace *pF);

    Icosahedron(double dRadius);
    int init(int iPolyType);
    int initIco();
    int initOct();
    int initTet();
    double calcSinAngle();
    void relinkR(IcoFace *pF);
  
    int parseSubFace(symbuf *psm, IcoFace *pF);

    double m_dR;
    Vec3D   **m_apMainVertices;
    IcoFace **m_apMainFaces;
    VertexLinkage *m_pVL;


    //    std::vector<IcoFace *> *m_avFaceLists;

    int m_iNumMainFaces;
    int m_iNumMainVerts;
    int m_iPolyType;

    std::vector<IcoFace *>m_vFaceList;
    unsigned int   m_iCurFace;

    int m_iSubLevel;

    tbox m_curBox;
    bool m_bPreSel;
    bool m_bStrict;

    ADDFACEMETHOD m_fFaceAddVL;

public:
   
};

#endif

