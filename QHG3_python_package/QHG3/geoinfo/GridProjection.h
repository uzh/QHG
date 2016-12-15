/** ***************************************************************************\
*   \file   GridProjection.h
*   \author jody
*   \brief  Header file for class GridProjection
*
*   GridProjection is a class which provides transformations between 
*   arbitrary spheres and arbitrarily scaled planes.
*** ***************************************************************************/
#ifndef __GRIDPROJECTION_H__
#define __GRIDPROJECTION_H__

#include "GeoInfo.h"

class Projector;

/** ***************************************************************************\
*   \class  GridProjection
*   \brief  class GridProjection
*
*   GridProjection is a class which provides transformations between 
*   arbitrary spheres and arbitrarily scaled planes.
*** ***************************************************************************/
class GridProjection {
public:
    /** ***************************************************************************\
    *   \fn     GridProjection(int iNumGridX, 
    *                          int iNumGridY, 
    *                          double dWidth, 
    *                          double dHeight, 
    *                          double dRadius, 
    *                          Projector *pProj);
    *   \brief  constructor
    *
    *   \param  iNumGridX   grid width  (number of cells in X direction)
    *   \param  iNumGridY   grid height (number of cells in X direction)
    *   \param  dWidth      real width of grid (i.e. of tangential rectangle)
    *   \param  dHeight     real height of grid (i.e. of tangential rectangle)
    *   \param  dRadius     real radius of sphere
    *   \param  pProj       projector object
    *
    *   Initializes membervariables and calculate deltas and transformation matrix
    *** ***************************************************************************/
    GridProjection(int iNumGridX,
                   int iNumGridY,
                   double dWidth,
                   double dHeight,
                   double dRadius,
                   Projector *pProj,
                   bool   bDeleteProj);

    /** ***************************************************************************\
    *   \fn     GridProjection(const ProjGrid *ppg, Projector *proj);
    *   \brief  constructor
    *
    *   \param  ppd         pointer to a ProjGrid structure
    *   \param  proj        pointer to a Projector object
    *
    *   Initializes membervariables and calculate deltas and transformation matrix
    *** ***************************************************************************/
    GridProjection(const ProjGrid *ppg, Projector *proj, bool bDeleteGrid, bool bDeleteProj);


    virtual ~GridProjection();

    /** ***************************************************************************\
    *   \fn     virtual bool gridToSphere(double dGridX, 
    *                                     double dGridY, 
    *                                     double &dLon, 
    *                                     double &dLat);
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dGridX      grid x coordinate
    *   \param  dGridY      grid y coordinate
    *   \param  dLon        real longitude  (degrees, output)
    *   \param  dLat        real latitude   (degrees, output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *** ***************************************************************************/
    virtual bool gridToSphere(double dGridX, 
                              double dGridY, 
                              double &dLon, 
                              double &dLat);

    /** ***************************************************************************\
    *   \fn     virtual bool sphereToGrid(double  dLon, 
    *                                     double  dLat,
    *                                     double &dGridX, 
    *                                     double &dGridY);
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dLon        real longitude    (degrees)
    *   \param  dLat        real latitude     (degrees)
    *   \param  dGridX      grid x coordinate (output)
    *   \param  dGridY      grid y coordinate (output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *** ***************************************************************************/
    virtual bool sphereToGrid(double  dLon, 
                              double  dLat,
                              double &dGridX, 
                              double &dGridY);

    bool getCellArea(double  dGridXE, 
                     double  dGridYN, 
                     double  dGridXW, 
                     double  dGridYS,
                     double &dArea);

    const Projector * getProjector() { return m_pProj;};
    const ProjGrid  * getProjGrid()  { return m_pProjGrid;};
protected:
    void init();
    double m_dXOffs;
    double m_dYOffs;

    Projector *m_pProj;
    const ProjGrid  *m_pProjGrid;

    double m_dDeltaX;
    double m_dDeltaY;
 
    bool m_bDeleteProj;
    bool m_bDeleteProjGrid;
};



#endif
