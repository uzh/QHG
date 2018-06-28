#include <stdio.h>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

#include "utils.h"
#include "geomutils.h"
#include "SnapHeader.h"
#include "IcoNode.h"
#include "Surface.h"
#include "ValueProvider.h"
#include "IQGradientFinder.h"

//-----------------------------------------------------------------------------
// constructor
//
IQGradientFinder::IQGradientFinder(Surface *pSurf, ValueProvider *pVP)
    : m_pSurf(pSurf),
      m_pVP(pVP) {

}


//-----------------------------------------------------------------------------
// findGradient
//
int IQGradientFinder::findGradient(gridtype iStartNode, double dStopVal, double dMinVal) {
    int iResult = 0;
    m_vPath.clear();
    if (dMinVal < dStopVal) {
        bool bGoOn = true;
        gridtype iCurNode = iStartNode;
        double dCurValue = m_pVP->getValue(iCurNode);

        std::set<gridtype> sNeighbors1;
        m_pSurf->collectNeighborIDs(iCurNode, 1, sNeighbors1);

        while (bGoOn && (iResult == 0) && (!std::isnan(dCurValue)) && (dCurValue >= dStopVal)) {
            m_vPath.push_back(iCurNode);
            printf("Added %d\n", iCurNode);
            gridtype iCurMinNode = -1;
            //            double dCurMinVal = dCurValue;
            double dCurMinVal = -1;

            std::set<gridtype>::const_iterator its1;
            for (its1 = sNeighbors1.begin(); its1 != sNeighbors1.end(); ++its1) {
                gridtype iNeighbor = *its1; 
                double v = m_pVP->getValue(iNeighbor);
                if ((v > dCurMinVal) && (v < dCurValue) && (v > dMinVal)) {
                    dCurMinVal = v;
                    iCurMinNode = iNeighbor;
                } 
            }

            if (dCurMinVal < 0) {
                //            if (dCurMinVal >= dCurValue) {
                //                printf("checking larger ring\n");

                iCurMinNode = -1;
                //                dCurMinVal = dCurValue;
                dCurMinVal = -1;

                std::set<gridtype> sNeighbors0;
                std::set<gridtype> sNeighborsT;
                m_pSurf->collectNeighborIDs(iCurNode, 2, sNeighborsT);
                std::set<gridtype> sNeighbors2;
                std::set_difference(sNeighborsT.begin(), sNeighborsT.end(),
                                    sNeighbors0.begin(), sNeighbors0.end(),
                                    std::inserter(sNeighbors2, sNeighbors2.end()));
                std::set<gridtype>::const_iterator its2;
                for (its2 = sNeighbors2.begin(); its2 != sNeighbors2.end(); ++its2) {
                    gridtype iNeighbor = *its2; 
                    double v = m_pVP->getValue(iNeighbor);
                    if ((v > dCurMinVal) && (v < dCurValue) && (v > dMinVal)) {
                        dCurMinVal = v;
                        iCurMinNode = iNeighbor;
                    } 
                }
                //                if (dCurMinVal >= dCurValue) {
                if (dCurMinVal < 0) {
                    printf("nothing found in second ring\n");
                    //                    iResult = -1;
                    bGoOn = false;
                } else {
                    iCurNode = iCurMinNode;
                }
            
            } else {
                iCurNode = iCurMinNode;
            }
            dCurValue = m_pVP->getValue(iCurNode);
        }

        if ((iResult == 0) && !std::isnan(dCurValue)) {
            // save last
            printf("Length of path %zd\n", m_vPath.size());
            m_vPath.push_back(iCurNode);
            m_mvPaths[iStartNode]=m_vPath;
            printf("now have %zd paths\n", m_mvPaths.size());
        } else {
            if (iResult == 0) {
                printf("hit nan-value (shouldn't happen)\n");
                iResult = -1;
            }
        }

    } else {
        iResult = -1;
        printf("Minval (%f) must be less than Stopval (%f)\n", dMinVal, dStopVal);
    }
    return iResult;
}
/*
//-----------------------------------------------------------------------------
// savePath
//
int IQGradientFinder::savePath(const char *pFileName, bool bWriteNodes, bool bWriteCoords, bool bWriteVals) {
    int iResult =-1;
    
    FILE *fOut = fopen(pFileName, "wt");
    if (fOut != NULL) {
        iResult = 0;
        for (unsigned int i = 0; i < m_vPath.size(); i++) {
            gridtype iNode = m_vPath[i];
            if (bWriteNodes) {
                fprintf(fOut, "%lld ", iNode);
            }
            if (bWriteCoords) {
                IcoNode *pIN = m_pIG->m_mNodes[iNode];
                fprintf(fOut, "%f %f  ", pIN->m_dLon, pIN->m_dLat);
            }
            if (bWriteCoords) {
                fprintf(fOut, "%f ", m_pVP->getValue(iNode));
            }
            fprintf(fOut, "\n");

        }
        fclose(fOut);
    } else {
        printf("Couldn't open [%s] for writing\n", pFileName);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// createPathSnap
//
int IQGradientFinder::createPathSnap(SnapHeader *pSH, const char *pOutSnap, double dPathValue) {
    int iResult = -1;

    FILE *fOut = fopen(pOutSnap, "wb");
    if (fOut != NULL) {
        iResult = pSH->write(fOut, true);
        if (iResult == 0) {
                

            nodelist::const_iterator it;
            for (it = m_nlVals.begin(); it != m_nlVals.end(); ++it) {
                gridtype iID = it->first;
                double dValue= it->second;
                std::vector<gridtype>::const_iterator it2 = std::find(m_vPath.begin(), m_vPath.end(), iID);
                if (it2 != m_vPath.end()) {
                    dValue = dPathValue;
                }
                int iW = fwrite(&iID, sizeof(gridtype), 1, fOut);
                iW +=    fwrite(&dValue, sizeof(double), 1, fOut);
                if (iW != 2) {
                    iResult = -1;
                }
            } 
        } else {
            printf("Couldn't write header to [%s]\n", pOutSnap);
        }
        fclose(fOut);
    } else {
        printf("Couldn't open [%s] for writing\n", pOutSnap);
    }
    return iResult;
}
*/

//-----------------------------------------------------------------------------
// findMergeNode
//
gridtype IQGradientFinder::findMergeNode(const std::vector<gridtype> &v1, const std::vector<gridtype> &v2) {
    gridtype nResult = -1;
    printf("L1 %zd, L2 %zd\n", v1.size(), v2.size());
    if (v1.back() == v2.back()) {
        std::vector<gridtype>::const_reverse_iterator it1 = v1.rbegin();
        std::vector<gridtype>::const_reverse_iterator it2 = v2.rbegin();
        
        while ((it1!= v1.rend()) && (it2 != v2.rend()) && (*it1 == *it2)) {
            nResult = *it1;
            it1++; it2++;
        }
    } else {
        printf("No common starting point: (%d != %d)\n", v1.back(), v2.back());
    }
    return nResult;
}

double IQGradientFinder::distToNode(const std::vector<gridtype> &v1, gridtype nStop) {
    double dDist = 0;
    bool bSearching = true;

    
    Vec3D *pvPrev = m_pSurf->getVertex(v1[0]);

    for (unsigned int i = 1; bSearching && (i < v1.size()); i++) {
        gridtype nCur = v1[i];
        Vec3D *pvCur = m_pSurf->getVertex(nCur);

        dDist += spherdist(pvPrev, pvCur, RADIUS_EARTH_KM);

        if (nCur == nStop) {
            bSearching = false;
        }
        pvPrev = pvCur;
    }
    return dDist;
}

