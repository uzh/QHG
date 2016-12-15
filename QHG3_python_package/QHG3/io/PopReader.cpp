#include <stdio.h>

#include <vector>
#include <hdf5.h>

#include "QDFUtils.h"
#include "PopBase.h"
#include "PopReader.h"

//----------------------------------------------------------------------------
// constructor
//
PopReader::PopReader()
    : m_hFile(H5P_DEFAULT),
      m_hPopGroup(H5P_DEFAULT),
      m_hSpeciesGroup(H5P_DEFAULT),
      m_bOpenedFile(false) {
}

//----------------------------------------------------------------------------
// create
//
PopReader *PopReader::create(const char *pFilename) {
    PopReader *pPR = new PopReader();
    int iResult = pPR->open(pFilename);
    if (iResult != 0) {
        delete pPR;
        pPR = NULL;
    }
    return pPR;
}

//----------------------------------------------------------------------------
// create
//
PopReader *PopReader::create(hid_t hFile) {
    PopReader *pPR = new PopReader();
    int iResult = pPR->open(hFile);
    if (iResult != 0) {
        delete pPR;
        pPR = NULL;
    }
    return pPR;
}

//----------------------------------------------------------------------------
// destructor
//
PopReader::~PopReader() {
    if (m_bOpenedFile) {
        if (m_hPopGroup != H5P_DEFAULT) {
            qdf_closeGroup(m_hPopGroup);
        }
        if (m_hFile != H5P_DEFAULT) {
            qdf_closeFile(m_hFile);
        }
    }
}

//----------------------------------------------------------------------------
// popInfo
//
herr_t popInfo(hid_t loc_id, const char *name, const H5L_info_t*pInfo, void *opdata) {
    H5G_stat_t statbuf;
    int iResult = -1;
    H5Gget_objinfo(loc_id, name, false, &statbuf);

    if (statbuf.type == H5G_GROUP) {
        hid_t hSpecies = qdf_openGroup(loc_id, name); 
        iResult = 0;
        popinfo spi;
        if (iResult == 0) {
            iResult = qdf_extractAttribute(hSpecies, SPOP_ATTR_CLASS_ID, 1, &spi.m_iClassID);
        }
        if (iResult == 0) {
            iResult = qdf_extractSAttribute(hSpecies, SPOP_ATTR_CLASS_NAME, NAMESIZE, spi.m_sClassName);
        }
        if (iResult == 0) {
            iResult = qdf_extractAttribute(hSpecies, SPOP_ATTR_SPECIES_ID, 1, &spi.m_iSpeciesID);
        }
        if (iResult == 0) {
            iResult = qdf_extractSAttribute(hSpecies, SPOP_ATTR_SPECIES_NAME, NAMESIZE, spi.m_sSpeciesName);
        }        
        if (iResult == 0) {
            iResult = qdf_extractAttribute(hSpecies, SPOP_ATTR_NUM_CELL, 1, &spi.m_iNumCells);
        }
        popinfolist *pPIList = (popinfolist *)opdata;
        if (pPIList != NULL) {
            pPIList->push_back(spi);
        }
        //  delete[] pType;
        qdf_closeGroup(hSpecies);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// open
//   open the file and go to the pop group
//   otherwise create it
//
int PopReader::open(const char *pFileName) {
    int iResult = -1;
    m_bOpenedFile = true;
    hid_t hFile = qdf_openFile(pFileName);
    if (hFile != H5P_DEFAULT) {
        iResult = open(hFile);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// open
//   open the file and go to the pop group
//   otherwise create it
//
int PopReader::open(hid_t hFile) {
    int iResult = -1;
    m_hFile = hFile;
    if (m_hFile != H5P_DEFAULT) {
       
        m_hPopGroup = qdf_openGroup(m_hFile, POPGROUP_NAME);
        if (m_hPopGroup > 0) {
            // already exists
            H5Literate(m_hPopGroup, H5_INDEX_NAME, H5_ITER_INC, 0, popInfo, &m_vPopList);
            iResult = 0;
        } else {
            iResult = POP_READER_ERR_NO_POP_GROUP;
        }
    }
    return iResult;
}
    
//----------------------------------------------------------------------------
// read
//
int PopReader::read(PopBase *pPB, const char *pSpeciesName, int iNumCells) {
    int iResult = -1;    

    //    printf("reading data for [%s]\n", pSpeciesName);
    m_hSpeciesGroup = qdf_openGroup(m_hPopGroup, pSpeciesName);
    // read attributes
    if (m_hSpeciesGroup > 0) {
        //  printf("species group open: reading species data\n");

        iResult = pPB->readSpeciesDataQDF(m_hSpeciesGroup);
        if (iResult == 0) {
            if (pPB->getNumCells() == iNumCells) {
                //            printf("read ok: creating AgentDataType\n");
                // set the handles
                hid_t hAgentType = pPB->createAgentDataTypeQDF();
                //@@            printf("open DataSet\n");
                hid_t hDataSet = H5Dopen2(m_hSpeciesGroup, AGENT_DATASET_NAME, H5P_DEFAULT);
                //@@            printf("open DataSpace\n");
                hid_t hDataSpace = H5Dget_space(hDataSet);
                
                //            printf("read agent data\n");
                // now load the data
                iResult = pPB->readAgentDataQDF(hDataSpace, hDataSet, hAgentType);
                //             printf("read agent data: res %d\n", iResult);
                if (iResult == 0) {
                    //                printf("reading additional agent data\n");
                    iResult = pPB->readAdditionalDataQDF(m_hSpeciesGroup);
                    //                printf("read additional agent data: res %d\n", iResult);
                }
                qdf_closeDataSpace(hDataSpace);
                qdf_closeDataSet(hDataSet);
            } else {
                iResult = POP_READER_ERR_CELL_MISMATCH;
                   }
        } else {
            iResult = POP_READER_ERR_READ_SPECIES_DATA;
        }
        qdf_closeGroup(m_hSpeciesGroup);
    } else {
        iResult = POP_READER_ERR_NO_SPECIES_GROUP;
    }
    return iResult;
}
