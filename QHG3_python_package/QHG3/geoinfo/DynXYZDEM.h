/** ***************************************************************************\
*   \file   DynXYZDEM.h
*   \author jody
*   \brief  Header file for class DynXYZDEM
*
*   DynXYZDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   It expects a file consistings of lines of the form
*     <lon>,<lat>,<alt>
*   where
*     lines are ordered first by decreasing latitude, 
*     then by increasing longitude:
*       first  line      min longitude, max latitude
*       second line      min longitude+deltaLon, max latitude
*       line #numlon     max longitude, max latitude
*       line #numlon+1   min longitude, max latitude-deltaLat
*       last line        max longitude, min latitude
*
*   DynXYZDem maintains a list of file positions corresponding to line starts
*** ***************************************************************************/
#ifndef __DYNXYZDEM_H__
#define __DYNXYZDEM_H__

#include "DynDEM.h"
/** ***************************************************************************\
*   \class  DynXYZDEM
*   \brief  Digital Elevation Model
*
*
*   DynDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   DynDem maintains a list of file positions corresponding to line starts
*   The data is read from a file of the form
*   <long>, <lat>, <alt>
*   
*** ***************************************************************************/
class DynXYZDEM : public DynDEM {
public:

    /** ***************************************************************************\
    *   \fn     DynXYZDEM()
    *   \brief  constructor
    *
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    DynXYZDEM(int iBufSize);
    
    /** ***************************************************************************\
    *   \fn     ~DynXYZDEM()
    *   \brief  destructor
    *
    *   
    *   frees array
    *** ***************************************************************************/
    virtual ~DynXYZDEM();

    /** ***************************************************************************\
    *   \fn     bool load(char *pName);
    *   \brief  read file and save positions for line starts
    *
    *   \param  pName           file name
    *   
    *   \return false if read error
    *
    *   It is assumed that the data describes a "rectangular" area, i.e
    *   for each lonitude value there is the same number of latitude values 
    *   and vice versa. These numbers are specified by iNumLatVals and iNumLongVals
    *
    *
    *** ***************************************************************************/
    bool load(char *pName);
    
protected:
    int      m_iRecordSize; // including nl
};


#endif
