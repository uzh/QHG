#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "Navigation.h"
#include "QDFUtils.h"
#include "NavWriter.h"

//----------------------------------------------------------------------------
// constructor
//
NavWriter::NavWriter(Navigation *pNav) 
    : m_pNav(pNav) {

}

//----------------------------------------------------------------------------
// writeToHDF
//
int NavWriter::writeToQDF(const char *pFileName, float fTime) {
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
int NavWriter::write(hid_t hFile) {
    int iResult = -1;

    if (m_pNav != NULL) {
        iResult = 0;
        hid_t hNavGroup = qdf_opencreateGroup(hFile, NAVGROUP_NAME);
        if (hNavGroup > 0) {
            writeNavAttributes(hNavGroup);
            //            printf("Written NavAttributes\n");
            int iNumPorts = m_pNav->m_mDestinations.size();
            int iNumDests = m_pNav->m_iNumDests;
            int iNumDists = m_pNav->m_iNumDists;

            ushort *pMultiplicities = new ushort[iNumPorts];
            int    *pDestinationIDs = new int[iNumDests];
            double *pDistances      = new double[iNumDists];

            ushort *pM  = pMultiplicities;
            int    *pDI = pDestinationIDs;
            double *pDD = pDistances;

            distancemap::const_iterator it;
            for (it = m_pNav->m_mDestinations.begin(); it != m_pNav->m_mDestinations.end(); ++it) {
                *pM++  = it->second.size();
                *pDI++ = it->first;
                
                distlist::const_iterator itl;
                for (itl = it->second.begin(); itl != it->second.end(); ++itl) {
                    *pDI++ = itl->first;
                    *pDD++ = itl->second;
                }
            }
            
            if (iResult == 0) {
                iResult = qdf_writeArray(hNavGroup, NAV_DS_MULTIPLICITIES, iNumPorts, pMultiplicities);
            }
            
            if (iResult == 0) {
                iResult = qdf_writeArray(hNavGroup, NAV_DS_DEST_IDS,       iNumDests, pDestinationIDs);
            }
            
            if (iResult == 0) {
                iResult = qdf_writeArray(hNavGroup, NAV_DS_DISTANCES,      iNumDists, pDistances);
            }
            
            if (iResult == 0) {
                //                printf("[NavWriter] arrays written\n");
            } else {
                printf("[NavWriter] error writing arrays\n");
            }
            
            delete[] pDistances;
            delete[] pDestinationIDs;
            delete[] pMultiplicities;


            qdf_closeGroup(hNavGroup);
            

        } else {
            iResult = -1;
            printf("[NavWriter] Couldn't open group [%s]\n", NAVGROUP_NAME);
            // couldn't open group
        }
    } else {
        printf("[NavWriter] No Navigation found in CG\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeNavData
//
int NavWriter::writeNavAttributes(hid_t hNavGroup) {
    int iResult = 0;

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV_ATTR_NUM_PORTS,   1, &m_pNav->m_iNumPorts);
    }

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV_ATTR_NUM_DESTS,   1, &m_pNav->m_iNumDests);
    }
    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV_ATTR_NUM_DISTS,   1, &m_pNav->m_iNumDists);
    }

    if (iResult == 0) {
        iResult = qdf_insertAttribute(hNavGroup, NAV_ATTR_SAMPLE_DIST, 1, &m_pNav->m_dSampleDist);
    }


    return iResult;
}
