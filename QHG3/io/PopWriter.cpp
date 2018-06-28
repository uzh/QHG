#include <stdio.h>
#include <string.h>
#include <hdf5.h>

#include "PopBase.h"

#include "QDFUtils.h"
#include "PopWriter.h"


//----------------------------------------------------------------------------
// showAgent
//
PopWriter::PopWriter(std::vector<PopBase *> vPops) {
    for (uint i = 0; i < vPops.size(); i++) {
        // by default additional data is writtn (true)
        m_mDataTypes[vPops[i]] = popwriteitem(vPops[i]->createAgentDataTypeQDF(), true);
    }
}

//----------------------------------------------------------------------------
// showAgent
//
PopWriter::~PopWriter() {
    popwriteitems::const_iterator it;
    for (it = m_mDataTypes.begin(); it != m_mDataTypes.end(); ++it) {
        qdf_closeDataType(it->second.m_hDataType);
    }
}

//----------------------------------------------------------------------------
// write
//  opens (or creates) file for writing
//  writes pop data
//  closes file
//
int PopWriter::write(const char *pFilename, float fTime, char *pSub, int iDumpMode) {
    int iResult = 0;
    
 
    iResult = write(qdf_opencreateFile(pFilename, fTime), pSub, iDumpMode);

    qdf_closeFile(m_hFile); 
    return iResult;
}

//----------------------------------------------------------------------------
// write
//   writes data to the file given by handle
//   (doesn't open or close file)
//
int PopWriter::write(hid_t hFile, char *pSub, int iDumpMode) {
    int iResult = 0;
    popwriteitems *pmSub = NULL;
    popwriteitems  mSub;
    if (pSub != NULL) {
        char *p=strtok(pSub, ",+");
        while (p != NULL) {
            bool bSearching = true;
            popwriteitems::const_iterator it;
            for (it = m_mDataTypes.begin(); bSearching && (it != m_mDataTypes.end()); ++it) {
                if (strncmp(it->first->getSpeciesName(), p, strlen(it->first->getSpeciesName()-1)) == 0) {
                    bool bAdd = (p[strlen(p)-1] != '#');
                    mSub[it->first] = popwriteitem(it->second.m_hDataType, bAdd);
                    bSearching = false;
                }
            }
            p = strtok(NULL, ",+");
        }
        pmSub = &mSub;
    } else {
        pmSub = &m_mDataTypes;
    }

    herr_t status = H5P_DEFAULT;

    m_hFile = hFile;
    if (m_hFile > 0) {
        // make sure Population group exists

        iResult = opencreatePopGroup();
        if (iResult == 0) {

            popwriteitems::const_iterator it;
            //            for (it = m_mDataTypes.begin(); it != m_mDataTypes.end(); ++it) {
            for (it = pmSub->begin(); it != pmSub->end(); ++it) {
                //                printf("  doing [%s]\n", it->first->getSpeciesName());
                iResult= opencreateSpeciesGroup(it->first, iDumpMode);
                if (iResult == 0) {
            
      
                    // Create the data space for the dataset.
                    // do not use getNumAgentsTotal()!! -> data space is created too large -> 0-filled data atthe end
                    hsize_t dims = 0;
                    if (iDumpMode != -1) {
                        dims = it->first->getNumAgentsMax();
                    } else {
                        dims = it->first->getNumAgentsEffective(); 
                    }
                    // fprintf(stderr, "PopWriter  [%s] numagents total: %lld\n", it->first->getSpeciesName(), dims);
                    hid_t hDataSpace = H5Screate_simple(1, &dims, NULL);
                    
                    if (hDataSpace > 0) {
                        
                        // Create the dataset
                        hid_t hDataSet = H5Dcreate2(m_hSpeciesGroup, AGENT_DATASET_NAME, it->second.m_hDataType, hDataSpace, 
                                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                        
                        if (hDataSet > 0) {
                            // call th population to write agent data        
                            if (iDumpMode != -1) {
                                it->first->dumpAgentDataQDF(hDataSpace, hDataSet, it->second.m_hDataType);
                            } else {
                                it->first->writeAgentDataQDF(hDataSpace, hDataSet, it->second.m_hDataType);
                            }

                            if (it->second.m_bWriteAdditional) {
                                it->first->writeAdditionalDataQDF(m_hSpeciesGroup);
                            }
                            // End access to the dataset and release resources used by it
                            qdf_closeDataSet(hDataSet);
                        }
                        // Terminate access to the data space
                        qdf_closeDataSpace(hDataSpace);
                    }
                    // close the group
                    qdf_closeGroup(m_hSpeciesGroup);
                } else {
                    // couldn't create species group 
                }
            }
            qdf_closeGroup(m_hPopGroup);
        } else {
            // couldn't open or create PopGroup or already exists
        }
        

    } else {
        //couldn't open or create QDF file
        printf("couldn't open or create QDF file\n");
    }
    if (status < 0) {
        iResult = -1;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// opencreatePopGroup
//   if the group exists, open it
//   otherwise create it
//
int PopWriter::opencreatePopGroup() {
    int iResult = -1;
    
    m_hPopGroup = qdf_openGroup(m_hFile, POPGROUP_NAME);
    if (m_hPopGroup > 0) {
        //            printf("already exists: opened\n");
        // already exists
        iResult = 0;
    
    } else {
        // does not exist: create
        m_hPopGroup = qdf_createGroup(m_hFile, POPGROUP_NAME);
        if (m_hPopGroup > 0) {
            // success
            //            printf("didn't exist: create\n");
            iResult = 0;
        }
    }
    return iResult;
}
    

//----------------------------------------------------------------------------
// opencreateSpeciesGroup
//
int PopWriter::opencreateSpeciesGroup(PopBase *pPB, int iDumpMode) {
    int iResult = 0;

    const char *pSpeciesName = pPB->getSpeciesName();
    if (H5Lexists(m_hPopGroup, pSpeciesName, H5P_DEFAULT)) {
        printf("SpeciesGroup '%s' exists; deleting\n", pSpeciesName);
        herr_t status = H5Ldelete(m_hPopGroup, pSpeciesName, H5P_DEFAULT);
        if (status >= 0) {
            printf("  deleted\n");
            iResult = 0;
        } else {
            printf("  failed to delete\n");
            iResult = -1;
        }
    } else {
        //        printf("SpeciesGroup '%s' doesn't exist\n", pSpeciesName);
    }

    if (iResult == 0)  {
        // does not exist: create
        //        printf("creating [%s]\n", pSpeciesName);
        m_hSpeciesGroup = qdf_createGroup(m_hPopGroup, pSpeciesName);
        if (m_hSpeciesGroup > 0) {
            // success
            iResult = 0;
            if (iDumpMode == -1) {
                iResult = pPB->writeSpeciesDataQDF(m_hSpeciesGroup);
            } else {
                iResult = pPB->dumpSpeciesDataQDF(m_hSpeciesGroup, iDumpMode);
            }
        }
    }

    return iResult;
}
   
