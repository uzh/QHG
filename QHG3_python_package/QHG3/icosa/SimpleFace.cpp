#include <stdio.h>
#include "Vec3D.h"
#include "SimpleFace.h"


template<int N>
SimpleFace<N>::SimpleFace()
    : m_pNormal(NULL) {
}
template<int N>
SimpleFace<N>::~SimpleFace() {
    delete m_pNormal;
}


//-----------------------------------------------------------------------------
// closestVertex
//
template<int N>
Vec3D *SimpleFace<N>::closestVertex(Vec3D *pP) {
    double dMinDist = 1e6;
    Vec3D *pClosest = NULL;
    for (int i = 0; i < N; i++) {
        double dDist = m_apVerts[i]->dist(pP);
        if (dDist < dMinDist) {
            dMinDist = dDist;
            pClosest = m_apVerts[i];
        }
    }
    return pClosest;
}

//-----------------------------------------------------------------------------
// closestVertexID
//
template<int N>
gridtype SimpleFace<N>::closestVertexID(Vec3D *pP) {
    int idxClosest = -1;

    double dMinDist = 1e6;
    for (int i = 0; i < N; i++) {
        double dDist = m_apVerts[i]->dist(pP);
       if (dDist < dMinDist) {
           dMinDist = dDist;
           idxClosest = i;
       }
   }
   return m_alIDs[idxClosest];
}
