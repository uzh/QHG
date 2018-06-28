#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LineReader.h"
#include "DescReader.h"
#include "strutils.h"
#include "types.h"
#include "ids.h"


#include "IcoLoc.h"

#include "PopHeader.h"
#include "PopLoader.h"

//-----------------------------------------------------------------------------
// constructor
//
PopLoader::PopLoader() 
    : m_iDataSize(0),
      m_pLR(NULL),
      m_pPH(NULL),
      m_pBufferForBinaryRead(NULL) {

    m_pPH = new PopHeader();
    *m_sError = '\0';
}

//-----------------------------------------------------------------------------
// createInstance
//
PopLoader *PopLoader::createInstance(const char *pPopFile) {
    PopLoader *pPop = new PopLoader();
    if (pPopFile != NULL) {
        int iResult = pPop->setFile(pPopFile);
        if (iResult != 0) {
            delete pPop;
            pPop = NULL;
        }
    }
    return pPop;
}

//-----------------------------------------------------------------------------
// createInstance
//
PopLoader *PopLoader::createInstance() {
    return PopLoader::createInstance(NULL);
}

//-----------------------------------------------------------------------------
// destructor
//
PopLoader::~PopLoader() {
    if (m_pPH != NULL) {
        delete m_pPH;
    }

    if (m_pLR != NULL) {
        delete m_pLR;
    }
    if (m_pBufferForBinaryRead != NULL) {
        delete[] m_pBufferForBinaryRead;
    }
}

//-----------------------------------------------------------------------------
// setFile
//
int PopLoader::setFile(const char *pPopFile) {
    int iResult = -1;
    if (m_pLR != NULL) {
        delete m_pLR;
    }
    m_pLR = LineReader_std::createInstance(pPopFile, "rt");
    if (m_pLR != NULL) {
        m_pPH->clear();
        printf("[PopLoader::setFile] set file to [%s]\n", pPopFile);
                        
        m_sFileName = pPopFile;

        iResult = 0;
    } else {
        sprintf(m_sError, "[PopLoader::setFile] couldn't open pop file for reading [%s]\n", pPopFile);
    }
    if (*m_sError != '\0') printf("%s", m_sError);
    return iResult;
}





//-----------------------------------------------------------------------------
// asc2Data
//  convert the ascii line to a data array
//
int PopLoader::asc2Data(char *pLine) {
    int iResult = -2;
    char *pCtx;

    unsigned char *pData  = new unsigned char[m_iDataSize];
    unsigned char *pStart = pData;

    long lNodeID;

    char *p = strtok_r(pLine, " \t\n, :;|", &pCtx);
    const char *pDesc = m_pPH->m_sDataDesc;//skip over the first two

    //   printf("[PopLoader::asc2Data] converting line [%s]; data start at %p\n", pLine, pData);
    // the first number is the node id (only NODE is supported)
    if (p != NULL) {
        if (strToNum(trim(p), &lNodeID)) {
            p = strtok_r(NULL,  "\t\n, :;|", &pCtx);
            printf("[PopLoader::asc2Data] Have node ID %ld\n", lNodeID);
            iResult = 0;
        } else {
            sprintf(m_sError, "[PopLoader::asc2Data] bad number format for node id: [%s]\n", p);
        }
    }

    unumber val;
    char c;
    DescReader *pDR = NULL;
    if (iResult == 0) {
        pDR = new DescReader(pDesc);
        do {
            int iSize = 0;
            c = pDR->getNextItem();
            //            printf("[PopLoader::asc2Data] translating [%s] for desc '%c'\n", p, c);
            if (c != '\0') {
                iSize = DescReader::getValue(p, c, &val);
                if (iSize > 0)  {
                    pData = putMem(pData,  &val, iSize);
                    p = strtok_r(NULL, ", \t\n", &pCtx);
                    /*
                    switch (c) {
                    case 'b':
                    case 'c':
                        printf(" %d", val.c);
                        break;
                    case 's':
                        printf(" %d", val.s);
                        break;
                    case 'i':
                        printf(" %d", val.i);
                        break;
                    case 'l':
                        printf(" %ld", val.l);
                        break;
                    case 'f':
                        printf(" %f", val.f);
                        break;
                    case 'd':
                        printf(" %f", val.d);
                        break;
                    }
                    */
              
                } else {
                    sprintf(m_sError, "[PopLoader::asc2Data] bad number format for [%c]: [%s]\n", c, p);
                    iResult = -1;
                }
            } else {
                sprintf(m_sError, "[PopLoader::asc2Data] data desc [%s] too short\n", pDesc);
                iResult = -1;
            }
        } while ((c != '\0') && (p != NULL) && (iResult == 0) );
        // printf("\n");
    }

    if ((iResult == 0) && (c != '\0')) {
        c = pDR->getNextItem();
        if (c != '\0') {
            printf("WARNING: too many description chars (have a %c) - only needed %d of %d\n", c, pDR->getEffPos(), pDR->getEffLen());
        }
    }

    delete pDR;

    if (iResult == 0) {
        std::pair<size_t, unsigned char *> pp(1, pStart);
        m_mvtmp[lNodeID].push_back(pp);
    }

    if (*m_sError != '\0') printf("%s", m_sError);
    return iResult;
}

//-----------------------------------------------------------------------------
// bin2Data
//  convert the ascii line to a data array
//
int PopLoader::bin2Data(FILE *fData) {
    int iResult = 0;

    // remember data start
    long lPos = ftell(fData);
    // go to end of file
    fseek(fData, 0, SEEK_END);
    long lPosEnd = ftell(fData);
    
   
    // calculate size of an item (nodeID + data)
    ulong iIncDataSize = m_iDataSize + sizeof(long); // PopLoader only supports Node coords
                
    // make sure data size divides file data size
    ldiv_t t = ldiv(lPosEnd-lPos, iIncDataSize); 
    if (t.rem == 0) {
        fseek(fData, lPos, SEEK_SET);

        if (m_pBufferForBinaryRead != NULL) {
            delete[] m_pBufferForBinaryRead;
        }
        m_pBufferForBinaryRead = new unsigned char[t.quot*iIncDataSize];
        // read everything in 1 gulp
        ulong iRead = fread(m_pBufferForBinaryRead, iIncDataSize, t.quot, fData);
        if (ferror(fData)) {
            sprintf(m_sError, "[PopLoader::bin2Data] error during data read\n");
            iResult = -1;
        } else {
            unsigned char *p = m_pBufferForBinaryRead;
            printf("[PopLoader::bin2Data]Have %ld blocks\n", iRead);

            for (int i = 0; i < iRead; i++) {
                long lNodeID;

                p = getMem(&lNodeID, p, sizeof(long));
                    
                if (lNodeID >= 0) {
                    // p points inside an allocated block (pBuffer)
                    // if addData deletes it -> error
                    std::pair<size_t, unsigned char *> pp(1, p);
                    m_mvtmp[lNodeID].push_back(pp);
                    //                    printf("-------------[bin2Data] created ucharp : %p\n", p);
                    p+= m_iDataSize;
                }
            }
            
        }
    

    } else {
        sprintf(m_sError, "[PopLoader::readData] file size (%ld) is not divisible by item size (%ld)\n", lPosEnd-lPos, iIncDataSize);
        iResult = -1;
    }

    if (*m_sError != '\0') printf("%s", m_sError);
    return iResult;
}




//-----------------------------------------------------------------------------
// processHeader
//
int PopLoader::processHeader() {
    int iResult = -1;

    // read the header
    iResult = m_pPH->read(m_pLR);
    if (iResult  == 0) {
        
        // HEADER END has been read; next line should be data
        // Data size of a single item : 
        printf("Header read - calculating datasize\n");
        m_iDataSize = m_pPH->calcDataSize();
        printf("[PopLoader::processHeader]size for %d: [%s]-> %zd\n", m_pPH->getSpecies(), m_pPH->m_sDataDesc, m_iDataSize);

        if (m_pPH->m_iCoordType != COORD_NODE) {
            sprintf(m_sError, "[PopLoader::processHeader] Invalid coord type %d (need NODE)\n", m_pPH->m_iCoordType);
            strcat(m_sError, "[PopLoader::processHeader] convert your pop file to node coords with IcoConv\n");
            iResult = -1;
        }

    } else {
        sprintf(m_sError, "[PopLoader::processHeader]error during header read\n");
        strcat(m_sError, m_pPH->getErrMess());
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// readData
//
int PopLoader::readData() {
    int iResult = -1;
    
    // temporary storage for localized data
    m_mvtmp.clear();
    iResult = processHeader();
    if (iResult == 0) {
        if (m_pPH->isBinary()) {
            // get position in file
            long lPos = m_pLR->tell();
            printf("Opening [%s]\n", m_sFileName.c_str());
            FILE *fData = fopen(m_sFileName.c_str(), "rb");
            if (fData != NULL) {
                fseek(fData, lPos, SEEK_SET);
                iResult = bin2Data(fData); 
                fclose(fData);
                
            } else {
                sprintf(m_sError, "[PopLoader::readData] couldn't open [%s] for binary reading \n", m_sFileName.c_str());
                iResult = -1;
            }
        } else {
            while ((iResult == 0) && !m_pLR->isEoF()) {
                char *pLine = m_pLR->getNextLine();
                if (pLine != NULL) {
                    iResult = asc2Data(pLine);
                }
            }
            printf("[PopLoader::readData]size of mvtmp: %zd\n", m_mvtmp.size());
            
        }
    } else {
        // error already described in readHeader
        // printf("Error in readHeader\n");
    }
    
    if (*m_sError != '\0') printf("oinkoink %s", m_sError);
    return iResult;
}




