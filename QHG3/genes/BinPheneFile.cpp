#include <stdio.h>
#include <string.h>

#include "types.h"

#include "BinPheneFile.h"


BinPheneFile *BinPheneFile::createInstance(const char *pFile) {
    BinPheneFile *pBG = new BinPheneFile();
    int iResult = pBG->init(pFile);
    if (iResult != 0) {
        delete pBG;
        pBG = NULL;
    }
    return pBG;
}


//----------------------------------------------------------------------------
// readHeader
//   
int BinPheneFile::init(const char *pFile) {
    int iResult = 0;
    m_fIn = fopen(pFile, "rb");
    if (m_fIn != NULL) {
        m_pCurName = new char[strlen(pFile)+1];
        strcpy(m_pCurName, pFile);
        iResult = readHeader();
    } else {
        fprintf(stderr, "Couldn't open file [%s] for reading\n", pFile);
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// constructor
//   
BinPheneFile::BinPheneFile() 
    : m_pCurName(NULL),
      m_pSpecial(NULL),
      m_iSpecial(0),
      m_fIn(NULL),
      m_iPhenomeSize(0),
      m_iNumBlocks(0),
      m_dTime(0),
      m_iNumPhenomes(0),
      m_iNumLocs(0),
      m_bFull(false),
      m_iCurID(-1),
      m_iNumSubPhenomes(0) {
    memset(m_sMagic, 0, 4);
}

//----------------------------------------------------------------------------
// destructor
//   
BinPheneFile::~BinPheneFile() {
    if (m_pCurName != NULL) {
        delete[] m_pCurName;
    }
    if (m_fIn != NULL) {
        fclose(m_fIn);
    }
    if (m_pSpecial != NULL) {
        delete[] m_pSpecial;
    }

    id_phenomes::iterator it;
    for (it = m_mIDPhen.begin(); it != m_mIDPhen.end(); ++it) {
        delete[] it->second;
    }
    m_mIDPhen.clear();

    
}




//----------------------------------------------------------------------------
// readHeader
//   
int BinPheneFile::readHeader() {
    int iResult = -1;
    if (m_fIn != NULL) {
       
        int iRead = fread(m_sMagic, 4, 1, m_fIn); 
        if (iRead == 1) {

            if (memcmp(m_sMagic, "PHNY", 4) == 0) {
        


                int iHeaderLen = 3*sizeof(int)+sizeof(bool);
                char *pH = new char[iHeaderLen];
                iRead = fread(pH, iHeaderLen, 1, m_fIn);
                
                if (iRead == 1) {
                    char *p = pH;
                    p = getMem(&m_iPhenomeSize, p, sizeof(int));
                    p = getMem(&m_iNumPhenomes, p, sizeof(int));
                    p = getMem(&m_iNumLocs,    p, sizeof(int));
                    p = getMem(&m_bFull,       p, sizeof(bool));
                    m_iNumBlocks = m_iPhenomeSize;
                    iResult = 0;
                } else {
                    fprintf(stderr, "Couldn't read header\n");
                    iResult = -1;
                }
                
                delete[] pH;
            } else {
                fprintf(stderr, "Uknown magic number [%s]\n", m_sMagic);
                iResult = -1;
            }
            
        } else {
            fprintf(stderr, "Couldn't read magic number\n");
            iResult = -1;
        }
    } else {
        fprintf(stderr, "File  is not open\n");
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readLocHeader
//   
int BinPheneFile::readLocHeader() {
    int iResult = -1;
    if (m_fIn != NULL) {
        int iNameLen = 0;
        int iRead = fread(&iNameLen, sizeof(int), 1, m_fIn);
        if (iRead == 1) {
            int iFullLength = iNameLen + sizeof(int) + 4*sizeof(double);
            if (iFullLength > m_iSpecial) {
                if (m_pSpecial != NULL) {
                    delete[] m_pSpecial;
                }
                m_iSpecial = iFullLength;
                m_pSpecial = new char[m_iSpecial];
            }
            //            iRead = fread(m_pSpecial, iFullLength, 1, m_fIn);
            iRead = fread(m_pSpecial, 1, iFullLength, m_fIn);
            if (iRead == iFullLength) {
                double dLon=-1;
                double dLat=-1;
                double dDist=-1;
                m_dTime = -1;
                m_iNumSubPhenomes=0;
                //                         printf("Loc [%s]\n", pSpecial);
                // skip the name
                char *p = m_pSpecial+iNameLen;
                p = getMem(&dLon,  p, sizeof(double));
                p = getMem(&dLat,  p, sizeof(double));
                p = getMem(&dDist, p, sizeof(double)); // not using this currently
                p = getMem(&m_dTime, p, sizeof(double));
                p = getMem(&m_iNumSubPhenomes, p, sizeof(int));
                m_mNamedLocs[m_pSpecial] = coords(dLon, dLat);
                iResult = 0;
            } else {
                fprintf(stderr, "Couldn't read locheader - read %d instead of %d\n", iRead, iFullLength);
                iResult = -1;
            }
        } else {
            fprintf(stderr, "Couldn't read name len\n");
            iResult = -1;
        }

    } else {
        fprintf(stderr, "File  is not open\n");
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readPhenomeHeader
//   
int BinPheneFile::readPhenomeHeader() {
                 
    int iResult = -1;
    if (m_fIn != NULL) {
        m_iCurID       = 0;
        idtype iMomID  = 0;
        idtype iDadID  = 0;
        int iGender = 0;
        int iNodeID = 0;
        double dLon = -1;
        double dLat = -1;
        int iDataLen =  sizeof(idtype)+sizeof(int)+2*sizeof(double);
        if (m_bFull) {
            iDataLen += 2*sizeof(idtype)+sizeof(int);
        }
        char *pH = new char[iDataLen];
        
        int iRead = fread(pH, iDataLen, 1, m_fIn);
        if (iRead == 1) {
            char *p = pH;
            p = getMem(&m_iCurID,     p, sizeof(idtype));
            if (m_bFull) {
                p = getMem(&iMomID,  p, sizeof(idtype));
                p = getMem(&iDadID,  p, sizeof(idtype));
                p = getMem(&iGender, p, sizeof(int));
            }
            p = getMem(&iNodeID, p, sizeof(int));
            p = getMem(&dLon,    p, sizeof(double));
            p = getMem(&dLat,    p, sizeof(double));
            m_vCurIDs.push_back(m_iCurID);
            m_mIDNodes[m_iCurID] = iNodeID;
            m_mIdLocs[m_iCurID]  = coords(dLon, dLat);
            iResult = 0;
        } else {
            fprintf(stderr, "COuldn't read genome header\n");
            iResult = -1;
        }
        delete[] pH;
    } else {
        fprintf(stderr, "File  is not open\n");
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readPhenome
//   
int BinPheneFile::readPhenome() {
                 
    int iResult = 0;
    if (m_fIn != NULL) {
        
        float *pPh =  new float[2*m_iNumBlocks];

        int iRead = fread(pPh, sizeof(float), 2*m_iNumBlocks, m_fIn);

        if (iRead == 2*m_iNumBlocks) {
            id_phenomes::iterator it = m_mIDPhen.find(m_iCurID);
            if (it == m_mIDPhen.end()) {
                m_mIDPhen[m_iCurID] = pPh;
            } else {
                fprintf(stderr, "repeated ID: %ld - make sure your sampling areas don't overlap!\n", m_iCurID);
                iResult = -1;
            }
        } else {
            fprintf(stderr, "Couldn't read genomes\n");
            iResult = -1;
        }
    } else {
        fprintf(stderr, "File  is not open\n");
        iResult = -1;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// show
//   
void BinPheneFile::showHeader() {
    char sMagic[5];
    memcpy(sMagic, m_sMagic, 4);
    sMagic[4] = '\0';

    printf("Header:\n");
    printf("  Magic:       %s\n", sMagic);
    printf("  PhenomeSize: %d\n", m_iPhenomeSize);
    printf("  NumPhenomes: %d\n", m_iNumPhenomes);
    printf("  NumLocs:     %d\n", m_iNumLocs);
    printf("  bFull:       %s\n", m_bFull?"yes":"no");
}


//----------------------------------------------------------------------------
// getMaps
//   
void BinPheneFile::getMaps(id_phenomes &mIDPhen, tnamed_ids &mvIDs, id_node &mIDNodes, id_locs &mIdLocs, named_locs &mNamedLocs) {
    mIDPhen    = m_mIDPhen;
    mvIDs      = m_mvIDs;
    mIDNodes   = m_mIDNodes;
    mIdLocs    = m_mIdLocs;
    mNamedLocs = m_mNamedLocs;
}


//----------------------------------------------------------------------------
// read
//   read a binary phenome file as created by PheneWriter (via QDFPhenSampler)
//   
int BinPheneFile::read() {
     int iResult = 0;
     
     if (m_fIn != NULL) {
         
         printf("%d locs\n", m_iNumLocs);
         for (int i = 0; (iResult == 0) && (i < m_iNumLocs); i++) {
             iResult = readLocHeader();
             if (iResult == 0) {
                 m_vCurIDs.clear();
                 for (int k = 0; (iResult == 0) && (k < m_iNumSubPhenomes); k++) {
                     
                     iResult = readPhenomeHeader();
                     
                     if (iResult == 0) {
                         iResult = readPhenome();
                         if (iResult == 0) {
                         } else {
                             fprintf(stderr, "Couldn't read phenomes\n");
                             iResult = -1;
                         }
                     } else {
                         fprintf(stderr, "Couldn't read phenome header\n");
                         iResult = -1;
                     }
                     //                             delete[] pH;
                 }
                 // pSpecial contains the name with terminating 0
                 m_mvIDs[loctime(m_pSpecial, m_dTime)] = m_vCurIDs;
             } else {
                 fprintf(stderr, "Couldn't read subheader\n");
                 iResult = -1;
             }
             
         }
         printf("Read %zd genome%s\n", m_mIDPhen.size(), (m_mIDPhen.size() != 1)?"s":"");
         
    
         // in case of error delete the array
         if (iResult != 0) {
             id_phenomes::iterator it;
             for (it = m_mIDPhen.begin(); it != m_mIDPhen.end(); ++it) {
                 delete[] it->second;
             }
             m_mIDPhen.clear();
         }

         //         fclose(fIn);
         iResult = m_mIDPhen.size();
     } else {
         fprintf(stderr, "File is not open\n");
         iResult = -1;
     }
     

     return iResult;
}

