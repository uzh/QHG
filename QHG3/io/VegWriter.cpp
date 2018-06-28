#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Vegetation.h"
#include "QDFUtils.h"
#include "VegWriter.h"

//----------------------------------------------------------------------------
// constructor
//
VegWriter::VegWriter(Vegetation *pVeg) 
    : m_pVeg(pVeg) {

    //    m_hCellDataType = createCellDataType();
}

//----------------------------------------------------------------------------
// writeToHDF
//
int VegWriter::writeToQDF(const char *pFileName, float fTime) {
    int iResult = -1;
    hid_t hFile = qdf_opencreateFile(pFileName, fTime);
    if (hFile > 0) {
        iResult = write(hFile);
        qdf_closeFile(hFile);  
    }
    return iResult;
}

//----------------------------------------------------------------------------
// write
//
int VegWriter::write(hid_t hFile) {
    int iResult = -1;
    if (m_pVeg != NULL) {
        //        printf("Writing veg data\n");
        hid_t hVegGroup = qdf_opencreateGroup(hFile, VEGGROUP_NAME);
        if (hVegGroup > 0) {
            writeVegAttributes(hVegGroup);
            
            iResult = qdf_writeArray(hVegGroup, VEG_DS_BASE_NPP,  m_pVeg->m_iNumCells, m_pVeg->m_adBaseANPP);
            //            printf("Written base npp:%d\n", iResult); fflush(stdout);
            iResult = qdf_writeArray(hVegGroup, VEG_DS_NPP,  m_pVeg->m_iNumCells, m_pVeg->m_adTotalANPP);
            //            printf("Written actual npp:%d\n", iResult); fflush(stdout);
            
            qdf_closeGroup(hVegGroup);
        } else {
            // couldn't open group
            iResult =-1;
        }
    }
    //    printf("Written veg data: %d\n", iResult); fflush(stdout);

    return iResult;
}

//----------------------------------------------------------------------------
// writeVegData
//
int VegWriter::writeVegAttributes(hid_t hVegGroup) {
    int iResult = qdf_insertAttribute(hVegGroup, VEG_ATTR_NUM_CELLS,  1, &m_pVeg->m_iNumCells);
   
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hVegGroup, VEG_ATTR_NUM_SPECIES, 1, &m_pVeg->m_iNumVegSpecies);
    }
	

    return iResult;
}
