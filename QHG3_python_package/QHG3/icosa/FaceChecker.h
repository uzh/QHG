#ifndef __FACECHECKER_H__
#define __FACECHECKER_H__

class IcoFace;
class ValReader;

class FaceChecker {
public:
    virtual ~FaceChecker(){};
    virtual bool check(Vec3D **pVerts) = 0; 
};

class FaceCheckerTrue : public virtual FaceChecker {
public:
    FaceCheckerTrue() {};
    virtual bool check(Vec3D **pVertsF) {return true;}; 
};

class FaceCheckerAlt : public virtual FaceChecker {
public:
    FaceCheckerAlt(ValReader *pVR, float fMinAlt);
    virtual bool check(Vec3D **pVerts); 
protected:
    ValReader *m_pVR;
    float m_fMinAlt;
};

class FaceCheckerRect : public virtual FaceChecker {
public:
    FaceCheckerRect(tbox *pbox);
    virtual bool check(Vec3D **pVerts); 
protected:
    double m_dLonMin; 
    double m_dLonMax;
    double m_dLatMin;
    double m_dLatMax;
    double m_dOffset;
};

class FaceCheckerAltRect : public FaceCheckerAlt, public FaceCheckerRect {
public:
    FaceCheckerAltRect(ValReader *pVR, float fMinAlt, tbox *pbox);
    virtual bool check(Vec3D **pVerts); 
};

#endif
