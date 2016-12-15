#ifndef __RECTSURF_H__
#define __RECTSURF_H__

#include "Surface.h"
#include "icoutil.h"

class RectSurf : public Surface {
public:
    RectSurf(int iW, int iH);
    ~RectSurf();
    static RectSurf *createRectSurf(char *pFormat);
    virtual gridtype findNode(double dLon, double dLat);
    int getNumVertices() { return m_iW*m_iH;};
private:
    virtual gridtype findNode(Vec3D *pv);
    virtual PolyFace *findFace(double dLon, double dLat);
    virtual Vec3D *getVertex(gridtype lID); 
    virtual int collectNeighborIDs(gridtype iID, int iDist, std::set<gridtype> & sIds);
    virtual tbox *getBox();
    virtual int            load(const char *pFileName);
    virtual int            save(const char *pFileName);
 
    virtual void display();

protected:
    int m_iW;
    int m_iH;
};


#endif

