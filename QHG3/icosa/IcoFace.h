#ifndef __ICOFACE_H__
#define __ICOFACE_H__


#include "Vec3D.h"
#include "PolyFace.h"
#include "icoutil.h"

class ValReader;

class IcoFace : public PolyFace  {
public:
    static IcoFace *createFace(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, bool bCheckLength=true);
    static IcoFace *createFace(Vec3D **ppV);


    IcoFace();
    virtual ~IcoFace();
    
    void setVerts(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3) { m_apVerts[0] = pV1; m_apVerts[1] = pV2; m_apVerts[2] = pV3;};

    void setIDs(gridtype lID0, gridtype lID1, gridtype lID2) { m_alIDs[0] = lID0; m_alIDs[1] = lID1; m_alIDs[2] = lID2;};
    gridtype *getIDs() { return m_alIDs;};

    virtual int getNumVertices() { return 3;};
    virtual Vec3D *getVertex(int i) { return m_apVerts[i]; };
    virtual gridtype  getVertexID(int iIndex){ return m_alIDs[iIndex]; };

    virtual PolyFace *contains(Vec3D *pP);
    PolyFace *contains(Vec3D *pP, double dEps);
    virtual Vec3D *closestVertex(Vec3D *pP);
    virtual gridtype closestVertexID(Vec3D *pP);

    bool inBox(double dThetaMin, double dThetaMax,double dPhiMin, double dPhiMax);
    bool inBox(tbox &tBox);
    bool vertexInBox(tbox &tBox);
            
    void display();
    void displayLL();
    void displayRec(const char *pIndent);
    bool subdivideLand(ValReader *pVR, float fMinAlt, int iNumLevels);
    void subdivide(int iNumLevels);
    void merge(int iNumLevels);

    IcoFace  *getFirstSubFace();
    IcoFace  *getNextSubFace();
    float  getCosMaxAng() {return m_fCosMaxAng;};

    int    getNumFaces();
    bool   isSubdivided() { return m_bSubdivided;};
    virtual int    getLevel() { return m_iLevel;};
    void   setLevel(int iLevel) { m_iLevel = iLevel;};
    
    void setKeep(bool bKeep) {m_bKeep = bKeep;};
    int getCompletionLevels()  {return m_iCompletionLevels;};
    void setCompletionLevels(int iCL)  { m_iCompletionLevels = iCL;};
    int calcCompletion();
    virtual void    planify(Vec3D *pV);

    void calcBary(Vec3D v, double *pdL1, double *pdL2);
    virtual double getArea() { return m_dArea;};    
    virtual double *getLons() { return m_adLons; }; 
    virtual double *getLats() { return m_adLats; }  
    virtual Vec3D *getNormal() { return m_pNormal;};
protected:
    int init(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, bool bCheckLength);
    Vec3D *m_apVerts[3];
    IcoFace  *m_apSubFaces[4];
    Vec3D *m_pNormal;
    Vec3D *m_apBorderFaces[3];
    float  m_fCosMaxAng;

    void subdivide();
    void setNext(IcoFace *pF);
    bool m_bSubdivided;
    IcoFace *m_pNextSubFace;
    IcoFace *m_pFirstSubFace;
    int   m_iLevel;

    double m_adLons[3];
    double m_adLats[3];
    gridtype m_alIDs[3];

    bool m_bKeep;
    int  m_iCompletionLevels;

    double m_dArea;
    double m_dOrigDist;
};

#endif
