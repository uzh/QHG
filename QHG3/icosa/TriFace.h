#ifndef __TRIFACE_H__
#define __TRIFACE_H__

#include "PolyFace.h"
#include "SimpleFace.h"


class TriFace : public SimpleFace<3> {
public:
    static TriFace *createFace(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3);
    static TriFace *createFace(Vec3D **ppV);

    TriFace();
    virtual ~TriFace();
    
    void setVerts(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3) { m_apVerts[0] = pV1; m_apVerts[1] = pV2; m_apVerts[2] = pV3;};
    void setIds(gridtype lID1, gridtype lID2, gridtype lID3) { m_alIDs[0] = lID1; m_alIDs[1] = lID2; m_alIDs[2] = lID3;};

    virtual PolyFace *contains(Vec3D *pP);
 
protected:
    int init(Vec3D *pV1, Vec3D *pV2, Vec3D *pV3);

 

};


#endif
