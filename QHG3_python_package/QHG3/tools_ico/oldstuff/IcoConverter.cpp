#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "strutils.h"
#include "LineReader.h"
#include "DescReader.h"

#include "HeaderBase.h"
#include "WorldHeader.h"
#include "SnapHeader.h"
#include "PopHeader.h"

#include "IcoLoc.h"
#include "Icosahedron.h"
//#include "RectLoc.h"
#include "IcoConverter.h"

#define NUMBLOCKS 10000
//-----------------------------------------------------------------------------
// createInstance
//
IcoConverter *IcoConverter::createInstance(IcoLoc *pIcoLoc, const char *pIcoName, bool bPreSel) {
    IcoConverter *pIC = new IcoConverter(pIcoLoc, pIcoName, bPreSel);
    int iResult = pIC->init();
    if (iResult != 0) {
        delete pIC;
        pIC = NULL;
    }
    return pIC;
}

//-----------------------------------------------------------------------------
// constructor
//
IcoConverter::IcoConverter(IcoLoc *pIcoLoc, const char *pIcoName, bool bPreSel) 
    : m_pIcoLoc(pIcoLoc),
      m_pIcoName(pIcoName),
      m_bPreSel(bPreSel),
      m_pHB(NULL) {

}

//-----------------------------------------------------------------------------
// destructor
//
IcoConverter::~IcoConverter() {
    if (m_pHB != NULL) {
        delete m_pHB;
    }
}

//-----------------------------------------------------------------------------
// init
//
int IcoConverter::init() {
    int iResult = 0;
    
    return iResult;
}

//-----------------------------------------------------------------------------
// getHeader
//
HeaderBase *IcoConverter::getHeader(const char *pInputFile) {
    if (m_pHB != NULL) {
        delete m_pHB;
    }
    m_pHB = NULL;

    int iResult = 0;
    LineReader *pLR = LineReader_std::createInstance(pInputFile, "rb");
    if (pLR != NULL) {
        char *pLine = trim(pLR->getNextLine());

        if (strcmp(pLine, MAGIC_POP) == 0) {
            m_pHB = new PopHeader(); // special for that one!!!Pop's data descrioption does n ot contain coords
            /*            
                          } else if (strcmp(pLine, MAGIC_WORLD) == 0) {
                          pHB = new WorldHeader();
            */
        } else {
            iResult = -1;
        }
        

        if (m_pHB != NULL) {
            pLR->seek(0, SEEK_SET);
            iResult = m_pHB->read(pLR, BIT_ICOFILE | BIT_CLASS | BIT_SPECIES);
            if (iResult == 0) {
                m_pHB->setIcoFile(m_pIcoName);
                m_pHB->m_bPreSel = m_bPreSel;
            } else {
                printf("Header read error\n");
                printf("  %s\n", m_pHB->getErrMess());
                delete m_pHB;
                m_pHB = NULL;
            }
        } else {
            iResult = -1;
            printf("Couldn't find header type for [%s]\n", pLine);
        }
        delete pLR;

    } else {
        iResult = -1;
        printf("Couldn't open LineReader for [%s]\n", pInputFile);
    }

    return m_pHB;
}

//-----------------------------------------------------------------------------
// nodifyFile
//
int  IcoConverter::nodifyFile(const char *pInput, const char *pOutput) {
    int iResult = -1;
    getHeader(pInput);
    if (m_pHB != NULL) {
        if (m_pHB->m_bBinary) {
            iResult = nodifyFileBin(pInput, pOutput);
        } else {
            iResult = nodifyFileAsc(pInput, pOutput);
        }
    } else {
        printf("Couldn't extract header from [%s]\n", pInput);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// coordifyFile
//
int  IcoConverter::coordifyFile(const char *pInput, const char *pOutput) {
    int iResult = -1;
    getHeader(pInput);
    if (m_pHB != NULL) {
        if (m_pHB->m_bBinary) {
            iResult = coordifyFileBin(pInput, pOutput);
        } else {
            iResult = coordifyFileAsc(pInput, pOutput);
        }
    } else {
        printf("Couldn't extract header from [%s]\n", pInput);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// nodifyFileBin
//
int  IcoConverter::nodifyFileBin(const char *pInput, const char *pOutput) {
    int iResult = 0;
    
                
    int iCoordType = m_pHB->m_iCoordType;

    if (iCoordType == COORD_GEO) {
        int iDataSizePure = DescReader::calcDataSize(m_pHB->m_sDataDesc);
        
        int iDataSizeRead = iDataSizePure+2*((iCoordType==COORD_GEO)?sizeof(double):sizeof(short));
        int iDataSizeNew  = iDataSizePure+sizeof(gridtype);

        FILE * fIn = fopen(pInput, "rb");
        if (fIn != NULL) {
            FILE *fOut = fopen(pOutput, "wb");
            if (fOut != NULL) {
                //@@ATTENTION DUMMY
                // set ico id

                m_pHB->m_iCoordType = COORD_NODE;
                m_pHB->write(fOut, true);

                int iConverted = 0;
                printf("Seeking pos %d\n",  m_pHB->getAscHeaderSize());
                fseek(fIn, m_pHB->getAscHeaderSize(), SEEK_SET);
                unsigned char *pInBuffer  = new unsigned char[NUMBLOCKS*iDataSizeRead];
                unsigned char *pOutBuffer = new unsigned char[NUMBLOCKS*iDataSizeNew];
                while ((iResult == 0) && !feof(fIn)) {
                    int iRead = fread(pInBuffer, iDataSizeRead, NUMBLOCKS, fIn);
                    if (iRead != NUMBLOCKS) {
                        if (ferror(fIn)) {
                            iResult = -1;
                        }
                    }
                    if (iResult == 0) {
                                      
                        unsigned char * p0 = pInBuffer;
                        unsigned char * p1 = pOutBuffer;

                        for (int i = 0; i < iRead; i++) {
                            // get longitude & latitude
                            double dLon; 
                            double dLat;
                            unsigned char *p0A = NULL;
                            p0A = getMem(&dLon, p0, sizeof(double));
                            p0A = getMem(&dLat, p0A, sizeof(double));
                            dLon *= M_PI/180;
                            dLat *= M_PI/180;
                            
                            // p0A now points to the bstart oof the actual item data
                            // find the node
                            gridtype lNodeID = m_pIcoLoc->findNode(dLon, dLat);
                            if (lNodeID >= 0) {
                                // put the id to the out buffer
                                unsigned char *p1A = putMem(p1, &lNodeID, sizeof(gridtype));
                                // append the rest of the item
                                memcpy(p1A, p0A, iDataSizePure);
                                p0 += iDataSizeRead;         
                                p1 += iDataSizeNew;         
                            } else {
                                printf("Couldn't find node for [%f,%f] (incomplete icosahedron?)\n", dLon, dLat);
                                iResult = -1;
                            }
                        }
                        int iWritten = fwrite(pOutBuffer, iDataSizeNew, iRead, fOut);
                        if (iWritten != iRead) {
                            iResult = -1;
                            printf("Error during write\n");
                        }
                        iConverted += iRead;
                    } else {
                        printf("Read error\n");
                    }
                }
                printf(">>> converted %d items\n", iConverted);
                delete[] pInBuffer;
                delete[] pOutBuffer;
                fclose(fOut);
            } else {
                iResult = -1;
                printf("Couldn't open file for writing [%s]\n", pOutput);
            }
            fclose(fIn);
        } else {
            iResult = -1;
            printf("Couldn't open file for reading [%s]\n", pInput);
        }
    } else {
        iResult = -1;
        printf("Only conversions of GEO\n");
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// nodifyFileAsc
//
int  IcoConverter::nodifyFileAsc(const char *pInput, const char *pOutput) {
    int iResult = 0;
  
  
                
    int iCoordType = m_pHB->m_iCoordType;

    if (iCoordType == COORD_GEO)  {
                
        char sLineIn[2048];
        *sLineIn  = '\0';

        FILE *fOut = fopen(pOutput, "wt");
        if (fOut != NULL) {
            //@@ATTENTION DUMMY
            // set ico id

            m_pHB->m_iCoordType = COORD_NODE;
            m_pHB->write(fOut, false);
            LineReader *pLR = LineReader_std::createInstance(pInput, "rt");             
            pLR->seek(m_pHB->getAscHeaderSize(), SEEK_SET);

            bool bOK = true;
            int iConverted = 0;
            while (!pLR->isEoF() && (iResult == 0)) {
                char *p = pLR->getNextLine();
                if (p != NULL) {
                    char *pEnd;
                    double dLon;
                    double dLat;
                    bOK = false;
                    dLon = strtof(p, &pEnd);
                    if (isspace(*pEnd)) {
                        dLat = strtof(pEnd, &pEnd);
                        if (isspace(*pEnd) || (*pEnd == ':')) {
                            dLon *= M_PI/180;
                            dLat *= M_PI/180;
                            bOK = true;
                        }
                        
                    }
                    if (bOK) {
                        // find the node
                        gridtype lNodeID = m_pIcoLoc->findNode(dLon, dLat);
                        if (lNodeID >= 0) {
                            fprintf(fOut, "%d %s\n", lNodeID, pEnd);
                            iConverted ++;
                        } else {
                            printf("Couldn't find node for [%f,%f] (incomplete icosahedron?)\n", dLon, dLat);
                            iResult = -1;
                        }
                    } else {
                        iResult = -1;
                        printf("Invalid number for coordinate in [%s]\n", p);
                    }
                }
                
            }
            printf(">>> converted %d items\n", iConverted);
            fclose(fOut);
            delete pLR;
        } else {
            iResult = -1;
            printf("Couldn't open file for writing [%s]\n", pOutput);
        }
    } else {
        iResult = -1;
        printf("Only conversions of GEO\n");
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// coordifyFileBin
//
int  IcoConverter::coordifyFileBin(const char *pInput, const char *pOutput) {

    int iResult = 0;
    int iCoordType = m_pHB->m_iCoordType;


    if (iCoordType == COORD_NODE) {

        int iDataSizePure = DescReader::calcDataSize(m_pHB->m_sDataDesc); 
        int iDataSizeRead = iDataSizePure+sizeof(gridtype);
        int iDataSizeNew = iDataSizePure+2*sizeof(double);

        FILE * fIn = fopen(pInput, "rb");
        if (fIn != NULL) {
            FILE *fOut = fopen(pOutput, "wb");
            if (fOut != NULL) {
                //@@ATTENTION DUMMY
                // set ico id

                m_pHB->m_iCoordType = COORD_GEO;
                m_pHB->write(fOut, true);

                int iConverted = 0;
                printf("Seeking pos %d\n",  m_pHB->getAscHeaderSize());
                fseek(fIn, m_pHB->getAscHeaderSize(), SEEK_SET);
                unsigned char *pInBuffer  = new unsigned char[NUMBLOCKS*iDataSizeRead];
                unsigned char *pOutBuffer = new unsigned char[NUMBLOCKS*iDataSizeNew];
                while ((iResult == 0) && !feof(fIn)) {
                    int iRead = fread(pInBuffer, iDataSizeRead, NUMBLOCKS, fIn);
                    if (iRead != NUMBLOCKS) {
                        if (ferror(fIn)) {
                            iResult = -1;
                        }
                    }
                    if (iResult == 0) {
                        unsigned char * p0 = pInBuffer;
                        unsigned char * p1 = pOutBuffer;

                        for (int i = 0; i < iRead; i++) {
                            gridtype lNodeID;
                            unsigned char *p0A;
                            unsigned char *p1A;
                                        
                            p0A = getMem(&lNodeID, p0, sizeof(gridtype));
                            // p0A now points to the start of the actual item data
                            // find the coords
                            double dLon;
                            double dLat;
                            bool bFound =  m_pIcoLoc->findCoords(lNodeID, &dLon, &dLat);
                            if (bFound) {
                                dLon *= 180/M_PI;
                                dLat *= 180/M_PI;
                                // put the coords to the out buffer
                                p1A = putMem(p1,  &dLon, sizeof(double));
                                p1A = putMem(p1A, &dLat, sizeof(double));
                                // append the rest of the item
                                memcpy(p1A, p0A, iDataSizePure);
                                p0 += iDataSizeRead;         
                                p1 += iDataSizeNew;         
                            } else {
                                printf("Couldn't find coords for node [%d] (incomplete icosahedron?)\n", lNodeID);
                                iResult = -1;
                            }
                        }
                        int iWritten = fwrite(pOutBuffer, iDataSizeNew, iRead, fOut);
                        if (iWritten != iRead) {
                            iResult = -1;
                            printf("Error during write\n");
                        }
                        iConverted += iRead;
                    } else {
                        printf("Read error\n");
                    }
                }
                delete[] pInBuffer;
                delete[] pOutBuffer;
                printf(">>> converted %d items\n", iConverted);
                fclose(fOut);
            } else {
                iResult = -1;
                printf("Couldn't open file for writing [%s]\n", pOutput);
            }
            fclose(fIn);
        } else {
            iResult = -1;
            printf("Couldn't open file for reading [%s]\n", pInput);
        }
    } else {
        iResult = -1;
        printf("Only NODE can be coordified\n");
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  coordifyFileAsc
//
int  IcoConverter::coordifyFileAsc(const char *pInput, const char *pOutput) {
    int iResult = 0;
            
    int iCoordType=-1;
    iCoordType = m_pHB->m_iCoordType;

    if (iCoordType == COORD_NODE) {
                    
        char sLineIn[2048];
        *sLineIn  = '\0';
                     

        FILE *fOut = fopen(pOutput, "wt");
        if (fOut != NULL) {
            //@@ATTENTION DUMMY
            // set ico id
            m_pHB->m_iCoordType = COORD_GEO;

            m_pHB->write(fOut, false);
            LineReader *pLR = LineReader_std::createInstance(pInput, "rt");             
            pLR->seek(m_pHB->getAscHeaderSize(), SEEK_SET);
            int iConverted = 0;
            while (!pLR->isEoF() && (iResult == 0)) {
                char *p = pLR->getNextLine();
                if (p != NULL) {
                    char *pEnd;
                    gridtype lNodeID = strtol(p, &pEnd, 10);
                    if (isspace(*pEnd)) {

                        // find the coords
                        double dLon;
                        double dLat;
                        bool bFound =  m_pIcoLoc->findCoords(lNodeID, &dLon, &dLat);
                        if (bFound) {
                            dLon *= 180/M_PI;
                            dLat *= 180/M_PI;
                            // put the coords to the out buffer
                            fprintf(fOut, "%lf %lf %s\n", dLon, dLat, pEnd);
                            iConverted ++;
                        } else {
                            printf("Couldn't find node for [%f,%f] (incomplete icosahedron?)\n", dLon, dLat);
                            iResult = -1;
                        }
                    } else {
                        printf("Invalid number for node ID\n");
                    }

                }
            }
            printf(">>> converted %d items\n", iConverted);
            fclose(fOut);
            delete pLR;
        } else {
            iResult = -1;
            printf("Couldn't open file for writing [%s]\n", pOutput);
        }
    } else {
        iResult = -1;
        printf("Only NODE can be coordified\n");
    }
    return iResult;

}
