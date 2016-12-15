#include <stdio.h>
#include <math.h>

#include <map>
#include <set>

#include "types.h"
#include "Vec3D.h"
#include "icoutil.h"
#include "VertexLinkage.h"

#include "IcoFace.h"
#include <math.h>



// compare function for edges
bool fncomp (edge el, edge er) {
    return (el.first < er.first) || ((el.first == er.first) && (el.second < er.second));}



//-----------------------------------------------------------------------------
// constructor
//
VertexLinkage::VertexLinkage():
    m_iCurID(0),
    m_iAddType(-1),
    m_iNumFaces(0) {

}

//-----------------------------------------------------------------------------
// destructor
//   remove centers
//
VertexLinkage::~VertexLinkage() {
    for (unsigned int i = 0; i < m_vCenters.size(); i++) {
        delete m_vCenters[i];
    }
    for (unsigned int i = 0; i < m_vDualCenters.size(); i++) {
        delete m_vDualCenters[i];
    }
}

//-----------------------------------------------------------------------------
// insertCenter
//   insert centerpoint of an edge
//
int VertexLinkage::insertCenter(gridtype lNode, Vec3D *pvCenter) {
    int iIndex=-1;
    double dAngle = 0;

    // find index of new center, or add it to list
    std::map<Vec3D *, gridtype, classcomp>::const_iterator it;
    it = m_mC2I.find(pvCenter);
    if (it != m_mC2I.end()) {
        iIndex = it->second;
    } else {
        m_vCenters.push_back(pvCenter);
        iIndex = (int)m_vCenters.size()-1;
        m_mC2I[pvCenter] = iIndex;
    }
    
    // calc angle to first entry
    if (m_mI2H[lNode].size() > 0) {
        Vec3D vDiff1(pvCenter);
        vDiff1.subtract(m_mI2V[lNode]);
        
        gridtype iFirst = m_mI2H[lNode].begin()->second;
        Vec3D *pFirst = m_vCenters[iFirst];
        Vec3D vDiff0(pFirst);
        vDiff0.subtract(m_mI2V[lNode]);
        
        double dNFD = vDiff0.calcNorm()*vDiff1.calcNorm();
        double dCosAng = vDiff0.dotProduct(&vDiff1)/dNFD;
        Vec3D *p1 = vDiff0.crossProduct(&vDiff1);
        p1->scale(1.0/dNFD);
        double dSinAng = p1->calcNorm();
        if (p1->dotProduct(m_mI2V[lNode]) < 0) {
            dSinAng = -dSinAng;
        }
        dAngle = atan2(dSinAng, dCosAng);
        if (dAngle < 0) {
            dAngle += 2*M_PI;
        }
        //        printf("... s %f, c %f, a %f\n", dSinAng, dCosAng, dAngle);
        delete p1;
    } else {
        dAngle = 0.0;
    }
    m_mI2H[lNode].insert(std::pair<double, int>(dAngle, iIndex));
  
    return 0;
}

//-----------------------------------------------------------------------------
// insertFlatCenter
//   insert centerpoint of an edge
//
int VertexLinkage::insertFlatCenter(gridtype lNode, Vec3D *pvCenter) {
    int iIndex=-1;
    double dAngle = 0;

    // find index of new center, or add it to list
    std::map<Vec3D *, gridtype, classcomp>::const_iterator it;
    it = m_mC2I.find(pvCenter);
    if (it != m_mC2I.end()) {
        iIndex = it->second;
    } else {
        m_vCenters.push_back(pvCenter);
        iIndex = (int)m_vCenters.size()-1;
        m_mC2I[pvCenter] = iIndex;
    }
    
    // absolute angle
    Vec3D vDiff1(pvCenter);
    vDiff1.subtract(m_mI2V[lNode]);
    dAngle = atan2(vDiff1.m_fY, vDiff1.m_fX);
    if (dAngle < 0) {
        dAngle += 2*M_PI;
    }

    m_mI2H[lNode].insert(std::pair<double, int>(dAngle, iIndex));
  
    return 0;
}



//-----------------------------------------------------------------------------
// addFace
//  if a vertex of the IcoFacec is not in the list, it receives a new ID and
//  is added to the list. Link information is created from the triangle.
//  The ids of the vertices are reported to the face (setIDs())  
//
int VertexLinkage::addFace(IcoFace *pF) {
    int iResult = 0;
    //    printf("AddFace\n");

    if (m_iAddType == ADDTYPE_FACEID) {
        // has started with other addFace method
        iResult = -1;

    } else {
        
        if (m_iAddType == ADDTYPE_NONE) {
            // first time: set type
            m_iAddType = ADDTYPE_FACE;
        }
  
        std::map<Vec3D *, gridtype, classcomp>::const_iterator it;
        // assign an ID to new vertices or find ID for existing vertices
        int tempID[3];
        Vec3D *pCenter = new Vec3D(0,0,0);
        for (int i = 0; i < 3; i++) {
            Vec3D *pCur = pF->getVertex(i);
            pCenter->add(pCur);
            it = m_mV2I.find(pCur);
            if (it == m_mV2I.end()) {
                // new vertex: use next higher ID
                tempID[i]    = m_iCurID;
                m_mV2I[pCur] = m_iCurID;
                m_mI2V[m_iCurID] = pCur;
                m_iCurID++;
            } else {
                // vertex already known
                tempID[i] = it->second;
            }
            
        }
        // let the face know the IDs of the vertices
        pF->setIDs(tempID[0], tempID[1], tempID[2]);

        pCenter->normalize();
        // in the new face every vertex is connected to all other vertices in the face
        for (int i = 0; i < 3; i++) {
            m_vLinks[tempID[i]].insert(tempID[(i+1)%3]);
            m_vLinks[tempID[i]].insert(tempID[(i+2)%3]);
            
            insertCenter(tempID[i], pCenter);
        }
        m_iNumFaces++;
     
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// addFace
//  add a face with user defined vertex IDs
//  if a vertex of the IcoFacec is not in the list, it is added to the list. 
//  Link information is created from the triangle.
//  The ids of the vertices must be (or have been) reported to the face
//  somewhere else!   
//
int VertexLinkage::addFace(IcoFace *pF, gridtype *aiID) {
    int iResult = 0;
    if (m_iAddType == ADDTYPE_FACE) {
        // has started with other addFace method
        iResult = -1;
    } else {
        if (m_iAddType == ADDTYPE_NONE) {
            // first time
            m_iAddType = ADDTYPE_FACEID;
        }

        
        std::map<Vec3D *, gridtype, classcomp>::const_iterator it;
        // assign an ID to new vertices or find ID for existing vertices
        int tempID[3];
        Vec3D *pCenter = new Vec3D(0,0,0);
        for (int i = 0; i < 3; i++) {
            Vec3D *pCur = pF->getVertex(i);
            pCenter->add(pCur);
 
            //printf(" vertex %lld (%f %f %f) \n", aiID[i], pCur->m_fX, pCur->m_fY, pCur->m_fZ);
            it = m_mV2I.find(pCur);
            if (it == m_mV2I.end()) {
                // new vertex: use provided ids 
                tempID[i]    = aiID[i];
                m_mV2I[pCur] = aiID[i];
                m_mI2V[aiID[i]] = pCur;
            } else {
                //                                if (aiID[i] != it->second)      printf(" vertex %lld (%f %f %f) already in: registered with id %lld (%f %f %f)\n", aiID[i], pCur->m_fX, pCur->m_fY, pCur->m_fZ, it->second, it->first->m_fX, it->first->m_fY, it->first->m_fZ);
                tempID[i] = it->second;
            }
        }

        pCenter->normalize();
        // in the new face every vertex is connected to all other vertices in the face
        for (int i = 0; i < 3; i++) {
            m_vLinks[tempID[i]].insert(tempID[(i+1)%3]);
            m_vLinks[tempID[i]].insert(tempID[(i+2)%3]);

            insertCenter(tempID[i], pCenter);
        }
        m_iNumFaces++;
        
    }

    return iResult;
}




//-----------------------------------------------------------------------------
// addPolyFace
//
int VertexLinkage::addPolyFace(int iNumVerts, Vec3D **apVerts, int *aiID) {
   int iResult = 0;
    //    printf("AddFace\n");

    if (m_iAddType == ADDTYPE_FACEID) {
        // has started with other addFace method
        iResult = -1;

    } else {
        
        if (m_iAddType == ADDTYPE_NONE) {
            // first time: set type
            m_iAddType = ADDTYPE_FACE;
        }


        std::map<Vec3D *, gridtype, classcomp>::const_iterator it;
        // assign an ID to new vertices or find ID for existing vertices
        int tempID[6];
        Vec3D *pCenter = new Vec3D(0,0,0);
        for (int i = 0; i < iNumVerts; i++) {
            Vec3D *pCur = apVerts[i];
            pCenter->add(pCur);
            it = m_mV2I.find(pCur);
            if (it == m_mV2I.end()) {
          
                // new vertex: use next higher ID
                tempID[i]    = aiID[i];
                m_mV2I[pCur] = aiID[i];
                m_mI2V[aiID[i]] = pCur;
                m_iCurID++;
            } else {
                // vertex already known
                tempID[i] = it->second;
            }
            
        }

        pCenter->normalize();
        // in the new face every vertex is connected to all other vertices in the face
        for (int i = 0; i < iNumVerts; i++) {
            m_vLinks[tempID[i]].insert(tempID[(i+1)%iNumVerts]);
            m_vLinks[tempID[i]].insert(tempID[(i-1+iNumVerts)%iNumVerts]);
            
            insertCenter(tempID[i], pCenter);
        }
        m_iNumFaces++;
     
    }
    return iResult;


}


//-----------------------------------------------------------------------------
// addPolyFace
//
int VertexLinkage::addPolyFace2(int iNumVerts, PolyFace *pFace, int *aiID) {
   int iResult = 0;
    //    printf("AddFace\n");

    if (m_iAddType == ADDTYPE_FACEID) {
        // has started with other addFace method
        iResult = -1;

    } else {
        
        if (m_iAddType == ADDTYPE_NONE) {
            // first time: set type
            m_iAddType = ADDTYPE_FACE;
        }


        std::map<Vec3D *, gridtype, classcomp>::const_iterator it;
        // assign an ID to new vertices or find ID for existing vertices
        int tempID[6];
        Vec3D *pCenter = new Vec3D(0,0,0);

        for (int i = 0; i < iNumVerts; i++) {
            Vec3D *pCur = pFace->getVertex(i);

            pCenter->add(pCur);
            it = m_mV2I.find(pCur);
            if (it == m_mV2I.end()) {
          
                // new vertex: use next higher ID
                tempID[i]    = aiID[i];
                m_mV2I[pCur] = aiID[i];
                m_mI2V[aiID[i]] = pCur;
                m_iCurID++;
            } else {
                // vertex already known
                tempID[i] = it->second;
            }
            
        }
        pCenter->scale(1.0/iNumVerts);
        // in the new face every vertex is connected to all other vertices in the face
        for (int i = 0; i < iNumVerts; i++) {
            m_vLinks[tempID[i]].insert(tempID[(i+1)%iNumVerts]);
            m_vLinks[tempID[i]].insert(tempID[(i-1+iNumVerts)%iNumVerts]);
            
            //            Vec3D *ppp = m_mI2V[tempID[i]];
            //            printf("Node %d (%f, %f, %f): add c %f, %f, %f\n", tempID[i], ppp->m_fX, ppp->m_fY, ppp->m_fZ, pCenter->m_fX, pCenter->m_fY, pCenter->m_fZ);
            insertFlatCenter(tempID[i], pCenter);
        }
        m_iNumFaces++;
     
    }
    return iResult;


}


//-----------------------------------------------------------------------------
// calcArea
//   calculate area of a node from its hex elements
//
double VertexLinkage::calcArea(gridtype lNode) {
    double dArea = 0.0;

    Vec3D *pC = m_mI2V[lNode];
    std::set<std::pair< double, gridtype> >  &sL = m_mI2H[lNode];
    std::set<std::pair< double, gridtype> >::const_iterator it;
    Vec3D *pPrev = getCenter(sL.rbegin()->second);

    for (it = sL.begin(); it != sL.end(); it++) {
        Vec3D *pN = getCenter(it->second);
        Vec3D N2(pN);
        N2.subtract(pPrev);

        Vec3D N1(pPrev);
        N1.subtract(pC);

        Vec3D *pX = N1.crossProduct(&N2);
        dArea += pX->calcNorm();
        delete pX;
        
        pPrev = pN;
    }
    dArea /= 2;
    return dArea;
}

//-----------------------------------------------------------------------------
// display
//
void VertexLinkage::display(FILE *fOut) {
    if (fOut == NULL) {
        fOut = stdout;
    }
    fprintf(fOut, "Vertex list:\n");
    std::map<gridtype, Vec3D *>::const_iterator it1;
    for (it1 = m_mI2V.begin(); it1 != m_mI2V.end(); it1++) {
        double theta;
        double phi;
        cart2Sphere(it1->second, &theta, &phi);

        //        printf(" %04lld: (%15.13f,%15.13f,%15.13f) (%8.5f %8.5f)\n", it1->first, it1->second->m_fX, it1->second->m_fY, it1->second->m_fZ, theta, phi);
        fprintf(fOut, " %04d: (%15.13e,%15.13e,%15.13e) (%8.5f %8.5f)\n", it1->first, it1->second->m_fX, it1->second->m_fY, it1->second->m_fZ, theta, phi);
    }

    vlinks::iterator it2;
    for (it2 = m_vLinks.begin(); it2 != m_vLinks.end(); it2++) {
        fprintf(fOut, "%04d:(%zd) ", it2->first, it2->second.size());
        std::set<gridtype>::iterator st;
        for (st = it2->second.begin(); st != it2->second.end(); st++) {
            fprintf(fOut, "%04d ", (*st));
        }
        fprintf(fOut, "\n");
    }
}

/*
//-----------------------------------------------------------------------------
// getVertexID
//
gridtype VertexLinkage::getVertexID(Vec3D *pv) {
    int iID = -1;
    std::map<Vec3D *, gridtype, classcomp>::const_iterator it = m_mV2I.find(pv);
    if (it != m_mV2I.end()) {
        iID = it->second;
    }
    //    printf(" %p(%f,%f,%f)->%d\n", pv, pv->m_fX, pv->m_fY, pv->m_fZ, iID);
    return iID;
}
*/

//-----------------------------------------------------------------------------
// getVertex
//
Vec3D *VertexLinkage::getVertex(gridtype lID) {
    Vec3D *pV = NULL;
    std::map<gridtype, Vec3D *>::const_iterator it = m_mI2V.find(lID);
    if (it != m_mI2V.end()) {
        pV = it->second;
    }
    return pV;
}


//-----------------------------------------------------------------------------
// destroyVertices
//
void VertexLinkage::destroyVertices() {
    std::map<gridtype, Vec3D *>::const_iterator it1;
    for (it1 = m_mI2V.begin(); it1 != m_mI2V.end(); it1++) {
        delete it1->second;
    }
    m_iNumFaces=0;
}



//-----------------------------------------------------------------------------
// collectNeighborIDs
//
int VertexLinkage:: collectNeighborIDs(gridtype iID, int iDist, std::set<gridtype> & sIds) {
    std::set<gridtype> s0;
    s0.insert(iID);
    int iCount = 0; 

    while (iCount < iDist) {
        std::set<gridtype> sNew;
        std::set<gridtype>::const_iterator it;
        for (it = s0.begin(); it != s0.end(); it++) {
            std::set<gridtype> sLinks = getLinksFor(*it);
            sNew.insert(sLinks.begin(), sLinks.end());
        }
        
        sIds.insert(s0.begin(), s0.end());
        s0.clear();
        s0.insert(sNew.begin(), sNew.end());
        iCount++;
    }
    std::set<gridtype>::iterator it = sIds.find(iID);
    if (it != sIds.end()) {
        sIds.erase(it);
    }
    return (int)sIds.size();
}

//-----------------------------------------------------------------------------
// getCenter
//
Vec3D *VertexLinkage::getCenter(gridtype lID) {
    Vec3D *pV = NULL;
    if (lID < (gridtype)m_vCenters.size()) {
        pV = m_vCenters[lID];
    }
    return pV;
}

