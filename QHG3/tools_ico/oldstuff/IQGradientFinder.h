#ifndef __ICOGRADIENTFINDER_H__
#define __ICOGRADIENTFINDER_H__

#include <vector>

#include "types.h"
#include "IcoNode.h"
#include "Surface.h"
#include "ValueProvider.h"
#include "SnapHeader.h"

class IQGradientFinder {
public:
    IQGradientFinder(Surface *pSurf, ValueProvider *pVP);
    int findGradient(gridtype iStartNode, double dStopVal, double dMinVal);

    //    int savePath(const char *pFileName, bool bWriteNodes, bool bWriteCoords, bool bWriteVals);
    //    int createPathSnap(SnapHeader *pSH, const char *pOutSnap, double dPathValue);
    std::vector<gridtype> &getPath() { return m_vPath;};
    std::map<gridtype, std::vector<gridtype> > &getPaths() { return m_mvPaths;};

    void reset() { m_mvPaths.clear(); m_vPath.clear();};

    gridtype findMergeNode(const std::vector<gridtype> &v1, const std::vector<gridtype> &v2);

    double distToNode(const std::vector<gridtype> &v1, gridtype nStop);
    double getVal(gridtype iID);
protected:

    Surface *m_pSurf;
    ValueProvider *m_pVP;
    std::vector<gridtype> m_vPath;
    std::map<gridtype, std::vector<gridtype> > m_mvPaths;
};



#endif
