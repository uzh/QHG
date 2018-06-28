#ifndef __QUADFACE_H__
#define __QUADFACE_H__

#include "PolyFace.h"
#include "SimpleFace.h"


class QuadFace : public SimpleFace<4> {
public:
    static QuadFace *createFace(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, Vec3D *pV4);
    static QuadFace *createFace(Vec3D **ppV);

    QuadFace();
    virtual ~QuadFace();
    
    void setVerts(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, Vec3D *pV4) { m_apVerts[0] = pV1; m_apVerts[1] = pV2; m_apVerts[2] = pV3; m_apVerts[3] = pV4;};
    void setIds(gridtype lID1, gridtype lID2, gridtype lID3, gridtype lID4) { m_alIDs[0] = lID1; m_alIDs[1] = lID2; m_alIDs[2] = lID3;m_alIDs[3] = lID4;};

    virtual PolyFace *contains(Vec3D *pP);

protected:
    int init(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3, Vec3D *pV4);


};


#endif
