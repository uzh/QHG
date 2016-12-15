/** ***************************************************************************\
*   \file   DynDEM.h
*   \author jody
*   \brief  Header file for class DynDEM
*
*   DynDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   DynDem maintains a list of file positions corresponding to line starts
*   in order to optimize access
*** ***************************************************************************/
#ifndef __DYNDEM_H__
#define __DYNDEM_H__

#include "DEM.h"
/** ***************************************************************************\
*   \class  DynDEM
*   \brief  Digital Elevation Model
*
*
*   DynDEM is an abstract base class for Implementations of 
*   dynamically loading Digital Elevation Models. 
*   DynDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   DynDem maintains a list of file positions corresponding to line starts
*   The data is read from a file of the form
*   <long>, <lat>, <alt>
*   
*** ***************************************************************************/
class DynDEM : public DEM {
public:

    /** ***************************************************************************\
    *   \fn     DynDEM()
    *   \brief  constructor
    *
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    DynDEM(int iNumLonVals, int iNumLatVals, int iBufSize);
    
    /** ***************************************************************************\
    *   \fn     ~DynDEM()
    *   \brief  destructor
    *
    *   
    *   frees array
    *** ***************************************************************************/
    virtual ~DynDEM();
    
    /** ***************************************************************************\
    *   \fn     CreateBuffers()
    *   \brief  allocate buffers
    *
    *   
    *   Allocates buffers
    *** ***************************************************************************/
    virtual void createBuffers();

    /** ***************************************************************************\
    *   \fn     bool load(char *pName);
    *   \brief  read file and save positions for line starts
    *
    *   \param  pName           file name
    *   
    *   \return false if read error
    *
    *   It is assumed that the data describes a "rectangular" area, i.e
    *   for each longitude value there is the same number of latitude values 
    *   and vice versa. These numbers are specified by iNumLatVals and iNumLongVals
    *
    *   This method must be implemented by derived classes
    *** ***************************************************************************/
    bool load(char *pName)=0;
    
    /** ***************************************************************************\
    *   \fn     double getAltitude(double dLon, double dLat);
    *   \brief  get altitude for specified location
    *
    *   \param  dLon    longitude (Degrees)
    *   \param  dLat    latitude  (Degrees)
    *   
    *   \return altitude or NO_VAL, if location outside map
    *
    *
    *** ***************************************************************************/
    double getAltitude(double dLon, double dLat);


    /** ***************************************************************************\
    *   \fn     void readLongitudes(int iLatIndex)
    *   \brief  reads longitudes for latitude corresponding to index
    *
    *   \param  iLatIndex    latitude index
    *   
    *
    *** ***************************************************************************/
    void readLongitudes(int iLatIndex);    
protected:

    int      findLineIndex(int iSearch);
    void     reorder(int iIndex);
    unsigned long getFilePos();

    long    *m_aLineStarts;
    double **m_aCurLines;
    int     *m_aiCurLines;
    int     *m_aiRowOrder;
    int      m_iBufSize;
    FILE    *m_fData;
};


#endif
