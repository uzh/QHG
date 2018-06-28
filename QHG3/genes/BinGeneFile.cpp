#include <stdio.h>
#include <string.h>

#include "types.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "BinGeneFile.h"

#define DEF_BITS_PER_NUC 2


BinGeneFile *BinGeneFile::createInstance(const char *pFile) {
    BinGeneFile *pBG = new BinGeneFile();
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
int BinGeneFile::init(const char *pFile) {
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
BinGeneFile::BinGeneFile() 
    : m_pCurName(NULL),
      m_pSpecial(NULL),
      m_iSpecial(0),
      m_fIn(NULL),
      m_iGenomeSize(0),
      m_iNumBlocks(0),
      m_dTime(0),
      m_iNumGenomes(0),
      m_iNumLocs(0),
      m_iBitsPerNuc(0),
      m_bFull(false),
      m_iCurID(-1),
      m_iNumSubGenomes(0),
      m_bExtended(false) {
    memset(m_sMagic, 0, 4);
}

//----------------------------------------------------------------------------
// destructor
//   
BinGeneFile::~BinGeneFile() {
    if (m_pCurName != NULL) {
        delete[] m_pCurName;
    }
    if (m_fIn != NULL) {
        fclose(m_fIn);
    }
    if (m_pSpecial != NULL) {
        delete[] m_pSpecial;
    }

    id_genomes::iterator it;
    for (it = m_mIDGen.begin(); it != m_mIDGen.end(); ++it) {
        delete[] it->second;
    }
    m_mIDGen.clear();

    
}




//----------------------------------------------------------------------------
// readHeader
//   
int BinGeneFile::readHeader() {
    int iResult = -1;
    if (m_fIn != NULL) {
       
        int iRead = fread(m_sMagic, 4, 1, m_fIn); 
        if (iRead == 1) {

            if ((memcmp(m_sMagic, "GENS", 4) == 0)  ||
                (memcmp(m_sMagic, "GENX", 4) == 0)  ||
                (memcmp(m_sMagic, "GENY", 4) == 0)) {
                
                if ((memcmp(m_sMagic, "GENX", 4) == 0) ||
                    (memcmp(m_sMagic, "GENY", 4) == 0)) {
                    m_bExtended = true;
                } else {
                    m_bExtended = false;
                }

                bool bExtended2 = false;
                if (memcmp(m_sMagic, "GENY", 4) == 0) {
                    bExtended2 = true;
                } else {
                    bExtended2 = false;
                    m_iBitsPerNuc = DEF_BITS_PER_NUC;
                }

                // for GENS,GENX: 3 int, for GENY: 4 int
                int iHeaderLen = (bExtended2?4:3)*sizeof(int)+sizeof(bool);
                char *pH = new char[iHeaderLen];
                iRead = fread(pH, iHeaderLen, 1, m_fIn);
                
                if (iRead == 1) {
                    char *p = pH;
                    p = getMem(&m_iGenomeSize, p, sizeof(int));
                    p = getMem(&m_iNumGenomes, p, sizeof(int));
                    p = getMem(&m_iNumLocs,    p, sizeof(int));
                    if (bExtended2) {
                        p = getMem(&m_iBitsPerNuc,    p, sizeof(int));
                    }
                    p = getMem(&m_bFull,       p, sizeof(bool));
                    
                    if (m_iBitsPerNuc == 1) {
                        m_iNumBlocks = BitGeneUtils::numNucs2Blocks(m_iGenomeSize);
                    } else {
                        m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
                    }
                    iResult = 0;
                } else {
                    fprintf(stderr, "Couldn't header\n");
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
int BinGeneFile::readLocHeader() {
    int iResult = -1;
    if (m_fIn != NULL) {
        int iNameLen = 0;
        int iRead = fread(&iNameLen, sizeof(int), 1, m_fIn);
        if (iRead == 1) {
            int iFullLength = iNameLen+sizeof(int)+(m_bExtended?4:3)*sizeof(double);
            if (iFullLength > m_iSpecial) {
                if (m_pSpecial != NULL) {
                    delete[] m_pSpecial;
                }
                m_iSpecial = iFullLength;
                m_pSpecial = new char[m_iSpecial];
            }
            iRead = fread(m_pSpecial, iFullLength, 1, m_fIn);
            if (iRead == 1) {
                double dLon=-1;
                double dLat=-1;
                double dDist=-1;
                m_dTime = -1;
                m_iNumSubGenomes=0;
                //                         printf("Loc [%s]\n", pSpecial);
                // skip the name
                char *p = m_pSpecial+iNameLen;
                p = getMem(&dLon,  p, sizeof(double));
                p = getMem(&dLat,  p, sizeof(double));
                p = getMem(&dDist, p, sizeof(double)); // not using this currently
                if (m_bExtended) {
                    p = getMem(&m_dTime, p, sizeof(double));
                }
                p = getMem(&m_iNumSubGenomes, p, sizeof(int));
                m_mNamedLocs[m_pSpecial] = coords(dLon, dLat);
                iResult = 0;
            } else {
                fprintf(stderr, "Couldn't read locheader\n");
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
// readGenomeHeader
//   
int BinGeneFile::readGenomeHeader() {
                 
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
            fprintf(stderr, "Couldn't read genome header\n");
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
// readGenome
//   
int BinGeneFile::readGenome() {
                 
    int iResult = 0;
    if (m_fIn != NULL) {
        
        ulong *pG =  new ulong[2*m_iNumBlocks];
        int iRead = fread(pG, sizeof(ulong), 2*m_iNumBlocks, m_fIn);
        if (iRead == 2*m_iNumBlocks) {
            id_genomes::iterator it = m_mIDGen.find(m_iCurID);
            if (it == m_mIDGen.end()) {
                m_mIDGen[m_iCurID] = pG;
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
void BinGeneFile::showHeader() {
    char sMagic[5];
    memcpy(sMagic, m_sMagic, 4);
    sMagic[4] = '\0';

    printf("Header:\n");
    printf("  Magic:      %s\n", sMagic);
    printf("  GenomeSize: %d\n", m_iGenomeSize);
    printf("  NumGenomes: %d\n", m_iNumGenomes);
    printf("  NumLocs:    %d\n", m_iNumLocs);
    printf("  bFull:      %s\n", m_bFull?"yes":"no");
}


//----------------------------------------------------------------------------
// getMaps
//   
void BinGeneFile::getMaps(id_genomes &mIDGen, tnamed_ids &mvIDs, id_node &mIDNodes, id_locs &mIdLocs, named_locs &mNamedLocs) {
    mIDGen     = m_mIDGen;
    mvIDs      = m_mvIDs;
    mIDNodes   = m_mIDNodes;
    mIdLocs    = m_mIdLocs;
    mNamedLocs = m_mNamedLocs;
}


//----------------------------------------------------------------------------
// read
//   read a binary genome file as created by GeneWriter (via QDFSampler)
//   
int BinGeneFile::read() {
     int iResult = 0;
     
     if (m_fIn != NULL) {
         
         printf("%d locs\n", m_iNumLocs);
         for (int i = 0; (iResult == 0) && (i < m_iNumLocs); i++) {
             iResult = readLocHeader();
             if (iResult == 0) {
                 m_vCurIDs.clear();
                 for (int k = 0; (iResult == 0) && (k < m_iNumSubGenomes); k++) {
                     
                     iResult = readGenomeHeader();
                     
                     if (iResult == 0) {
                         iResult = readGenome();
                         if (iResult == 0) {
                         } else {
                             fprintf(stderr, "Couldn't read genomes\n");
                             iResult = -1;
                         }
                     } else {
                         fprintf(stderr, "Couldn't read genome header\n");
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
         printf("Read %zd genome%s\n", m_mIDGen.size(), (m_mIDGen.size() != 1)?"s":"");
         
    
         // in case of error delete the array
         if (iResult != 0) {
             id_genomes::iterator it;
             for (it = m_mIDGen.begin(); it != m_mIDGen.end(); ++it) {
                 delete[] it->second;
             }
             m_mIDGen.clear();
         }

         //         fclose(fIn);
         iResult = m_mIDGen.size();
     } else {
         fprintf(stderr, "File is not open\n");
         iResult = -1;
     }
     

     return iResult;
}

