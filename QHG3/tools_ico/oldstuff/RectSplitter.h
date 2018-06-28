#ifndef __RECTSPLITTER_H__
#define __RECTSPLITTER_H__

#include <vector>
class Region;
class GridProjection;

typedef std::vector<unsigned short> VECUS;

#define CUTOFF_X 5
#define CUTOFF_Y 5

class RectSplitter : public RegionSplitter {
public:
    RectSplitter(GridProjection *pGP, double dH, int iNX, int iNY, bool bStrict);
    RectSplitter(GridProjection *pGP, double dH, int iNumTiles, bool bGrid, bool bStrict);

    //    virtual ~RegionSplitter()
    virtual Region **createRegions(int *piNumTiles);
protected:
    int findBestPartition(unsigned short iNumRegions);
    int findIrregularPartition();
    int findRegularPartition();

    int compatiblePairs(VECUS vFactors, VECUS &vFactors2);

    void prepareFactors(int iNum);
    void recursiveSplit(VECUS vPrimes, 
                        VECUS vPowers, 
                        unsigned short iStart, 
                        unsigned short iF, 
                        VECUS &vFactors);

    int doForcedPartition(int iTX, int iTY, int iCutOffX=CUTOFF_X, int iCutOffY=CUTOFF_Y);
    int approximatePairs(VECUS vFactors, 
                         float fAspp,
                         int *piFactor1,
                         int *piFactor2);


    GridProjection *m_pGP;
    double m_dH;
    int  m_iNX;
    int  m_iNY;
    int  m_iW;
    int  m_iH;
    bool m_bGrid;
    bool m_bStrict;
    VECUS  m_vFactors;
};


#endif

