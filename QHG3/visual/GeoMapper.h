/** ***************************************************************************\
* GeoMapper.h
*
* Convert an array with content projected by projection P0 into an array
* with the same content projected with projection P1.
* Every element (x1,y1) of M1 is projected with P1^-1 to get longitude and 
* latitude values theta and phi. Theta and phi are projected using P1 yielding
* coordinates (x0,y0) in M0. The value at (x0,y0) is used as value
* for (x1,y1) in M1
*** ***************************************************************************/
#ifndef __GEOMAPPER_H__
#define __GEOMAPPER_H__

#include "GeoInfo.h"
#include "GridProjection.h"

template<class T>
class GeoMapper {
public:
    GeoMapper(const ProjGrid *ppg0, Projector *pProj0, T **pptData0,
              const ProjGrid *ppg1, Projector *pProj1, T **pptData1);

    virtual ~GeoMapper();
    
    void setClosure(bool bCloseX, bool bCloseY);
    void map(T tUndef);
 
private:
    GridProjection *m_pGP0;
    GridProjection *m_pGP1;
    T             **m_pptData0;
    T             **m_pptData1;
    const ProjGrid       *m_ppg0;
    const ProjGrid       *m_ppg1;
    bool            m_bCloseX;
    bool            m_bCloseY;
};

#endif


