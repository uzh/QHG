/*****************************************************************************
 * SCellGrid manages a fixed size array of SCell structs
 *
\****************************************************************************/ 

#ifndef __SCELLGRID_H__
#define __SCELLGRID_H__

#include <stdlib.h>
#include <unistd.h>
#include <hdf5.h>

#include "icoutil.h"
#include "SCell.h"

#include "Geography.h"
#include "Climate.h"
#include "Vegetation.h"
#include "MoveStats.h"
#include "Navigation.h"

#include "IcoGridNodes.h"


#define GRID_TYPE_NONE    -1
#define GRID_TYPE_FLAT4    0
#define GRID_TYPE_FLAT6    1
#define GRID_TYPE_ICO      2
#define GRID_TYPE_IEQ      3

class SCellGrid {
public:
    static SCellGrid *createInstance(IcoGridNodes *pIGN);

    SCellGrid(int iID, uint iNumCells, const stringmap &smSurfaceData);    
    ~SCellGrid();

    int         m_iID;
    uint        m_iNumCells;
    SCell       *m_aCells;
    
    // general grid info
    stringmap m_smSurfaceData;
    int       m_iType;
    int       m_iConnectivity;

    // map ID -> index in m_apCells
    std::map<gridtype, int>   m_mIDIndexes;

    // geographical data
    Geography  *m_pGeography;
    // climate data
    Climate    *m_pClimate;
    //vegetation stuff
    Vegetation *m_pVegetation;
    // move statistics
    MoveStats *m_pMoveStats;
    // navigation
    Navigation *m_pNavigation;

    void setGeography(Geography* pGeo);
    void setClimate(Climate* pClim);
    void setVegetation(Vegetation* pVeg);
    void setMoveStats(MoveStats* pMS);
    void setNavigation(Navigation* pNav);
 
    bool isCartesian();   

    static const char *getGridSType(int iType);
    static int getGridType(const char *pType);

    // (may be removed later)
    void display();
};

#endif
