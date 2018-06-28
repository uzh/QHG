/** ***************************************************************************\
*   \file   GeoProvider.h
*   \author jody
*   \brief  Header file for class GeoProvider
*
*   GeoProvider is an abstract base class used to provide the connection
*   between grid-coordinates (x, y) and realworld coordinates (lat, long, alt)
*** ***************************************************************************/
#ifndef __GEOPROVIDER_H__
#define __GEOPROVIDER_H__

#include "RectUtils.h"
#include "DEM.h"

/** ***************************************************************************\
*   \class  GeoProvider
*   \brief  Relation grid - world
*
*   GeoProvider is an abstract base class used to provide the connection
*   between grid-coordinates (x, y) and realworld coordinates (lat, long, alt)
*   The Altitude information is retrieved from a DEM object which is the
*   representation of a Digital Elevation Model.
*   Derived classes must implement a mapping from the plane to spherical
*   coordinates.
*** ***************************************************************************/
class GeoProvider {
public:
    /** ***************************************************************************\
    *   \fn     PrGeoProvider(int iNumGridX, 
    *                         int iNumGridY, 
    *                         bool bDeleteDem);
    *   \brief  constructor
    *
    *   \param  iNumGridX   grid width  (number of cells in X direction)
    *   \param  iNumGridY   grid height (number of cells in X direction)
    *   \param  bDeleteDEM       delete DEM in destructor
    *
    *   Initializes membervariables and calculate deltas and transformation matrix
    *** ***************************************************************************/
    GeoProvider(int iNumGridX, 
                int iNumGridY, 
                bool bDeleteDEM);
    virtual ~GeoProvider();

    /** ***************************************************************************\
    *   \fn     virtual bool getWorldCoords(double dGridX, 
    *                                double dGridY, 
    *                                double &dLon, 
    *                                double &dLat, 
    *                                double &dAlt);
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dGridX      grid x coordinate
    *   \param  dGridY      grid y coordinate
    *   \param  dLon        real longitude  (output)
    *   \param  dLat        real latitude   (output)
    *   \param  dAlt        real altitude   (output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *** ***************************************************************************/
    virtual bool getWorldCoords(double dGridX, 
                                double dGridY, 
                                double &dLon, 
                                double &dLat, 
                                double &dAlt);

    virtual bool getWorldCoords(double dGridX, 
                                double dGridY, 
                                double *pdGeo);

    /** ***************************************************************************\
    *   \fn     virtual bool getGridCoords(double  dLon, 
    *                                      double  dLat, 
    *                                      double &dGridX, 
    *                                      double &dGridY);
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dLon        real longitude
    *   \param  dLat        real latitude 
    *   \param  dGridX      grid x coordinate (output)
    *   \param  dGridY      grid y coordinate (output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *** ***************************************************************************/
    virtual bool getGridCoords(double dLon, 
                                double dLat, 
                                double &dGridX, 
                                double &dGridY);

    void setDEM(DEM *pDEM) {m_pDEM = pDEM;};
    DEM *getDEM() { return m_pDEM; };
    void setGridSpacing(double dXSpacing, double dYSpacing);

    /** *************************************************************************** \
    *   \fn     bool SetTile(rect rTile);
    *   \brief  calculates coordinate offset of this tile
    *
    *   \param  rTile  rectangle of the tile in global coordinates
    *
    *** ***************************************************************************/
    void setTile(rect *prTile);

    /** ***************************************************************************\
    *   \fn     virtual bool getCellArea(double dGridXE, 
    *                                    double dGridYN, 
    *                                    double dGridXW, 
    *                                    double dGridYS, 
    *                                    double &dArea);
    *   \brief  Get area of cell on unit sphere
    *
    *   \param  dGridXE     grid x coordinate of cell's east border
    *   \param  dGridYN     grid y coordinate of cell's north border
    *   \param  dGridXW     grid x coordinate of cell's west border
    *   \param  dGridYS     grid y coordinate of cell's south border
    *   \param  dArea       cell area   (output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *** ***************************************************************************/
    virtual bool getCellArea(double  dGridXE, 
                             double  dGridYN, 
                             double  dGridXW, 
                             double  dGridYS,
                             double &dArea);

    int getTileOffsetY() { return m_iTileOffsetY;};

    virtual double getRadius()=0;
protected:
    /** ***************************************************************************\
    *   \fn     virtual bool getWorldCoordsImpl(double dGridX, 
    *                                           double dGridY, 
    *                                           double &dLon, 
    *                                           double &dLat, 
    *                                           double &dAlt)=0;
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dGridX      grid x coordinate
    *   \param  dGridY      grid y coordinate
    *   \param  dLon        real longitude  (output)
    *   \param  dLat        real latitude   (output)
    *   \param  dAlt        real altitude   (output)
    *
    *   \return true if grid coordinates correspond to a real world position
    *
    *   Same as GetInfo, but corrected with tile offset
    *  
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
    virtual bool getWorldCoordsImpl(double dGridX, 
                                    double dGridY, 
                                    double &dLon, 
                                    double &dLat, 
                                    double &dAlt)=0;

    /** ***************************************************************************\
    *   \fn     virtual bool getGridCoordsImpl(double dLon, 
    *                                          double dLat, 
    *                                          double &dGridX, 
    *                                          double &dGridY)=0;
    *   \brief  Get real world coordinates corresponding to grid coordinates
    *
    *   \param  dLon        real longitude
    *   \param  dLat        real latitude 
    *   \param  dGridX      grid x coordinate  (output)
    *   \param  dGridY      grid y coordinate  (output)
    *
    *   \return true if world coordinates correspond to a grid position(?)
    *
    *  
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
    virtual bool getGridCoordsImpl(double dLon, 
                                   double dLat, 
                                   double &dGridX, 
                                   double &dGridY)=0;

    /** ***************************************************************************\
    *   \fn     virtual bool getCellAreaImpl(double  dGridXE, 
    *                                        double  dGridYN, 
    *                                        double  dGridXW, 
    *                                        double  dGridYS,
    *                                        double &dArea)=0;
    *   \brief  Calculate area of spherical rectangle corresponding to
    *    grid rectangle given by coords
    *
    *   \param  dGridXE      eastern x coordinate
    *   \param  dGridYN      northern y coordinat
    *   \param  dGridXW      western x coordinat
    *   \param  dGridYS      southern y coordinate
    *   \param  dArea        area
    *
    *   \return true if world coordinates correspond to a grid position(?)
    *
    *  
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
     virtual bool getCellAreaImpl(double  dGridXE, 
                                  double  dGridYN, 
                                  double  dGridXW, 
                                  double  dGridYS,
                                  double &dArea)=0;




    int m_iNumGridX;
    int m_iNumGridY;
    int m_iTileOffsetX;
    int m_iTileOffsetY;
    double m_dXGridSpacing;
    double m_dYGridSpacing;

    bool m_bDeleteDEM;
    DEM *m_pDEM;

};



#endif

