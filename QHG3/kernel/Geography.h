#ifndef __GEOGRAPHY_H__
#define __GEOGRAPHY_H__

class SCellGrid;

typedef double geonumber;

class Geography {
public:
    Geography(uint iNumCells, uint iMaxNeighbors, geonumber dRadius, geonumber dSeaLevel=0);
    Geography();
    virtual ~Geography();

    int init(uint iNumCells, uint iMaxNeighbors, geonumber dRadius, geonumber dSeaLevel);


    //global values
	bool m_bUpdated;
    uint m_iNumCells;
    uint m_iMaxNeighbors;
    geonumber   m_dRadius;      // "real" earth radius 
    geonumber   m_dSeaLevel;    // sea level (may change, depending on climate)
    // cell specific values
    geonumber  *m_adLatitude;   // "real" latitude of cell center  (radians) 
    geonumber  *m_adLongitude;  // "real" longitude of cell center (radians) 
    geonumber  *m_adAltitude;   // "real" altitude of cell center  
    geonumber  *m_adDistances;  // distances to neighbors: distances from cell k at m_aadDistances[N*k, N*k+N-1], where N=max #neighbors
    geonumber  *m_adArea;       // area of cell
    bool       *m_abIce;        // ice (1) or no ice (0)
    geonumber  *m_adWater;      // presence of water (rivers in cell)
    bool       *m_abCoastal;    // coast in neighborhood
    geonumber  *m_adAngles;     // orientation of direction to neighbors: 0->east, pi/2->north, pi->west, 3pi/2 south
                                // this must be calculated
  
    // output
    void writeOutput(float fTime, int iStep, SCellGrid* pCG);

    // additionals
    void calcAngles(SCellGrid* pCG);
	inline void resetUpdated() { m_bUpdated = false; };
};

#endif
 
