/** ***************************************************************************\
*   \file   StatDEM.h
*   \author jody
*   \brief  Header file for class StatDEM
*
*   StatDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   StatDem loads the contents of the entire file into an array.
*** ***************************************************************************/
#ifndef __STATDEM_H__
#define __STATDEM_H__

#include "DEM.h"
/** ***************************************************************************\
*   \class  StatDEM
*   \brief  Digital Elevation Model
*
*   StatDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   The data is read from a file of the form
*   <long>, <lat>, <alt>
*   and stored in an array
*** ***************************************************************************/
class StatDEM : public DEM {
public:

    /** ***************************************************************************\
    *   \fn     StatDEM()
    *   \brief  constructor
    *
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    StatDEM(int iNumLonVals, int iNumLatVals);
    
    /** ***************************************************************************\
    *   \fn     ~StatDEM()
    *   \brief  destructor
    *
    *   
    *   frees array
    *** ***************************************************************************/
    virtual ~StatDEM();

    /** ***************************************************************************\
    *   \fn     bool Load(char *pName);
    *   \brief  read data from file
    *
    *   \param  pName           file name
    *   
    *   \return false if read error
    *
    *   It is asasumed that the data describes a "rectangular" area, i.e
    *   for each lonitude value there is the same number of latitude values 
    *   and vice versa. These numbers are specified by iNumLatVals and iNumLongVals
    *** ***************************************************************************/
    bool load(char *pName);
    
    /** ***************************************************************************\
    *   \fn     double getAltitude(double dLon, double dLat);
    *   \brief  get altitude for specified location
    *
    *   \param  dLon    longitude
    *   \param  dLat    latitude
    *   
    *   \return altitude or NO_VAL, if location outside map
    *
    *
    *** ***************************************************************************/
    double getAltitude(double dLon, double dLat);
    
    
protected:


    double **m_matDEM;

};


#endif
