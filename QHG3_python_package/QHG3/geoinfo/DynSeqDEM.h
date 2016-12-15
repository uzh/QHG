/** ***************************************************************************\
*   \file   DynSeqDEM.h
*   \author jody
*   \brief  Header file for class DynSeqDEM
*
*   DynSeqDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   DynSeqDem maintains a list of file positions corresponding to line starts.
*   Scans file to find the start points
*** ***************************************************************************/
#ifndef __DYNSEQDEM_H__
#define __DYNSEQDEM_H__

#include "DEM.h"
#include "DynDEM.h"
/** ***************************************************************************\
*   \class  DynSeqDEM
*   \brief  Digital Elevation Model
*
*
*   DynDEM represents a Digital Elevation Model, i.e. a mapping from 
*   latitude and longitude to altitude.
*   DynSeqDem maintains a list of file positions corresponding to line starts
*   The data is read from a file of the form
*   <long>, <lat>, <alt>
*   
*** ***************************************************************************/
class DynSeqDEM : public DynDEM {
public:

    /** ***************************************************************************\
    *   \fn     DynDEM()
    *   \brief  constructor
    *
    *   
    *   Initializes membervariables
    *** ***************************************************************************/
    DynSeqDEM(int iNumLonVals, int iNumLatVals, int iBufSize);
    
    /** ***************************************************************************\
    *   \fn     ~DynDEM()
    *   \brief  destructor
    *
    *   
    *   frees array
    *** ***************************************************************************/
    virtual ~DynSeqDEM();

    /** ***************************************************************************\
    *   \fn     bool Load(char *pName, int iNumLatVals, int iNumLongVals);
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
    
};


#endif
