#include <stdio.h>
#include <string.h>

#include "strutils.h"
#include "QDFUtils.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "IcoGridNodes.h"
#include "Lattice.h"
#include "EQsahedron.h"
#include "IQSurface_OGL.h"
#include "IQIco_OGL.h"
#include "IQFlat_OGL.h"
#include "IQEQIco_OGL.h"
#include "notification_codes.h"
#include "ValueProvider.h"
#include "SnapValueProvider.h"
#include "QDFValueProvider.h"
#include "SurfaceManager.h"


//-----------------------------------------------------------------------------
// constructor
//
SurfaceManager::SurfaceManager()
    : m_pSurface(NULL),
      m_pSurfaceOGL(NULL),
      m_pValueProvider(NULL),
      m_iType(STYPE_NONE),
      m_pOverlay(NULL),
      m_bQDF(false) {

}

//-----------------------------------------------------------------------------
// createInstance
//
SurfaceManager *SurfaceManager::createInstance(const char *pFile, const char *pDataFile, IQOverlay *pOverlay, bool bPreSel) {
    SurfaceManager *pSurfaceManager = new SurfaceManager();
    if (pSurfaceManager != NULL) {
        pSurfaceManager->setOverlay(pOverlay);
        pSurfaceManager->loadSurface(pFile, pDataFile, bPreSel);
    }
    return pSurfaceManager;
}

//-----------------------------------------------------------------------------
// destructor
//
SurfaceManager::~SurfaceManager() {
    if (m_pSurface != NULL) {
        delete m_pSurface;
    }

    if (m_pSurfaceOGL != NULL) {
        delete m_pSurfaceOGL;
    }

    if (m_pValueProvider != NULL) {
        delete m_pValueProvider;
    }

}

//-----------------------------------------------------------------------------
//  loadSurfaceQDF
//
int SurfaceManager::loadSurfaceQDF(const char *pFile) {
    int iResult = 0;
    // open QDFFile
    hid_t hFile = qdf_openFile(pFile);
    if (hFile != H5P_DEFAULT) {
        printf("OPening group [%s]\n", GRIDGROUP_NAME);fflush(stdout);
        hid_t hGridGroup = qdf_openGroup(hFile, GRIDGROUP_NAME, true);
        if (hGridGroup > 0) {
            char sType[128];
            printf("extracting attribute [%s]\n", SURF_TYPE);fflush(stdout);
            iResult = qdf_extractSAttribute(hGridGroup, SURF_TYPE, 128, sType); 
            if (iResult == 0) {
                if (strcmp(sType, SURF_EQSAHEDRON) == 0) {
                    char sSubDivs[128];
                    int iSubDivs = 0;
                    iResult = qdf_extractSAttribute(hGridGroup, SURF_IEQ_SUBDIVS, 128, sSubDivs);
                    if (iResult == 0) {
                        if (strToNum(sSubDivs, &iSubDivs)) {
                            // now we can create a EQsahedron 
                            EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, true, NULL, fNaN, NULL);
                            if (pEQ != NULL) {
                                pEQ->relink();
                                printf("i-aaah\n");
                                
                                if (m_pSurface != NULL) {
                                    delete m_pSurface;
                                }
                                m_pSurface = pEQ;
                                if (m_pSurfaceOGL != NULL) {
                                    delete m_pSurfaceOGL;
                                }
                                m_pSurfaceOGL = new IQEQIco_OGL(pEQ, m_pOverlay);
                                notifyObservers(NOTIFY_NEW_GRID, m_pSurface);
                                char sMess[128];
                                sprintf(sMess, "IEQ successfully loaded from QDFFile [%s]", pFile);
                                notifyObservers(NOTIFY_MESSAGE, sMess);
                                m_iType = STYPE_REG;
                                if (m_pValueProvider != NULL) {
                                    delete m_pValueProvider;
                                    m_pValueProvider = NULL;
                                }
                                m_pValueProvider = QDFValueProvider::createValueProvider(pFile);
                                printf("SurfaceManager: notifying about [%p]\n", m_pSurface);
                                notifyObservers(NOTIFY_NEW_GRID, m_pSurface);

                                dynamic_cast<QDFValueProvider*>(m_pValueProvider)->show();

                            } else {
                                iResult = -1;
                                printf("[SurfaceManager::loadSurfaceQDF] Attribute [%s] needs to be a number: [%s]\n", SURF_IEQ_SUBDIVS, sSubDivs); 
                            }
                        } else {
                            iResult = -1;
                            printf("[SurfaceManager::loadSurfaceQDF] couldn't create EQsahedron\n"); 
                        }
                    } else {
                        iResult = -1;
                        printf("[SurfaceManager::loadSurfaceQDF] couldn't get attribute [%s]\n", SURF_IEQ_SUBDIVS); 
                    }
                        
                } else {
                    iResult = -1;
                    printf("[SurfaceManager::loadSurfaceQDF] can't handle surfacce type [%s]\n", sType); 
                }
            } else {
                iResult = -1;
                printf("[SurfaceManager::loadSurfaceQDF] couldn't get attribute [%s]\n", SURF_TYPE); 
            }
            qdf_closeGroup(hGridGroup);
        } else {
            iResult = -1;
            printf("[SurfaceManager::loadSurfaceQDF] couldn't open group [%s]\n", GRIDGROUP_NAME); 
        }
        qdf_closeFile(hFile);
    } else {
        iResult = -1;
        printf("[SurfaceManager::loadSurfaceQDF] couldn't open [%s]\n", pFile); 
    }
    if (iResult != 0) {
        char sMess[128];
        sprintf(sMess, "Can't load surface [%s]", pFile);
        notifyObservers(NOTIFY_ERROR, sMess);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  loadSurfaceSnap
//
int SurfaceManager::loadSurfaceSnap(const char *pFile, bool bPreSel) {
    int iResult = 0;    
    
    FILE *fIn = fopen(pFile, "rb");
    if (fIn != NULL) {
        char s[4];
        int iRead = fread(s, 1, 4, fIn);
        if (iRead == 4) {
            if ((s[3] == '3') || s[3] == '\n') {
                s[3] = '\0';
            }
            if (strcmp(s, "ICO") == 0) {
                m_iType = STYPE_ICO;
            } else if (strcmp(s, "LTC") == 0) {
                m_iType = STYPE_FLAT;
            } else if (strcmp(s, "IEQ") == 0) {
                m_iType = STYPE_REG;
            } else {
                m_iType = STYPE_NONE;
                iResult = -1;
            }
        } else {
            *s = '\0';
            printf("Couldn't read from [%s]\n", pFile);
            iResult = -1;
        }
        
        fclose(fIn);

        
        if (iResult == 0) {
            switch (m_iType) {
            case STYPE_ICO:
                iResult = loadIco(pFile, bPreSel);
                break;
            case STYPE_FLAT:
                iResult = loadFlat(pFile);
                break;
            case STYPE_REG:
                iResult = loadEQ(pFile);
                break;
            }
            
            printf("SurfaceManager: notifying about [%p]\n", m_pSurface);
            notifyObservers(NOTIFY_NEW_GRID, m_pSurface);
        }
    } else {
        iResult = -1;
        printf("[IGCreatorPanel::loadSurface] couldn't open [%s]\n", pFile); 
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  loadSurface
//
int SurfaceManager::loadSurface(const char *pFile, const char *pDataFile, bool bPreSel) {
    int iResult = 0;

    // try Snap first
    iResult = loadSurfaceSnap(pFile, bPreSel);
    if (iResult == 0) {
        m_bQDF = false;
    } else {
        // now try snap
        iResult = loadSurfaceQDF(pFile);
        if (iResult == 0) {
            m_bQDF = true;
        }
    }    

    if (iResult == 0) {
        if (*pDataFile != '\0') {
            
            printf("SurfaceManager: loading [%s]\n", pDataFile);
            if (m_bQDF) {
                m_pValueProvider = QDFValueProvider::createValueProvider(pDataFile);
            } else {
                if (m_pValueProvider != NULL) {
                    delete m_pValueProvider;
                    m_pValueProvider = NULL;
                }
                m_pValueProvider = SnapValueProvider::createValueProvider(pDataFile);
            }
            if (m_pValueProvider != NULL) {
                if (m_pSurfaceOGL != NULL) {
                    m_pSurfaceOGL->setValueProvider(m_pValueProvider);
                }
                iResult = 0;

            } else {
                iResult = -1;
                printf("[SurfaceManager::loadSurface] couldn't open [%s]\n", pDataFile); 
                
            }
        }
    }

    if (m_bQDF && (m_pValueProvider != NULL)) {
        dynamic_cast<QDFValueProvider*>(m_pValueProvider)->show();
    }

    return iResult;
}

//-----------------------------------------------------------------------------
//  loadIco
//
int SurfaceManager::loadIco(const char *pFile, bool bPreSel) {
    char sMess[512];
    Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
    pIco->merge(0);
    pIco->setStrict(true);
    pIco->setPreSel(bPreSel);
    int iResult = pIco->load(pFile);
    if (iResult == 0) {
        printf("oinkk\n");
        pIco->relink();

        if (m_pSurface != NULL) {
            delete m_pSurface;
        }
        m_pSurface = pIco;
        if (m_pSurfaceOGL != NULL) {
            delete m_pSurfaceOGL;
        }
        m_pSurfaceOGL = new IQIco_OGL(pIco, m_pOverlay);
        
        sprintf(sMess, "Ico file [%s] successfully loaded", pFile);
        notifyObservers(NOTIFY_MESSAGE, sMess);
        // areacheck(m_pIcosahedron);
    } else {
        sprintf(sMess, "Couldn't load Ico file [%s]", pFile);
        notifyObservers(NOTIFY_ERROR, sMess);
        printf("%s\n", sMess);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  loadFlat
//
int SurfaceManager::loadFlat(const char *pFile) {
    char sMess[512];
    Lattice *pLattice = new Lattice();
    int iResult = pLattice->load(pFile);
    if (iResult == 0) {
        printf("oinkk\n");

        if (m_pSurface != NULL) {
            delete m_pSurface;
        }
        m_pSurface = pLattice;
        if (m_pSurfaceOGL != NULL) {
            delete m_pSurfaceOGL;
        }
        m_pSurfaceOGL = new IQFlat_OGL(pLattice, m_pOverlay);
   
        sprintf(sMess, "lattice file [%s] successfully loaded", pFile);
        notifyObservers(NOTIFY_MESSAGE, sMess);
        //       areacheck(m_pIcosahedron);
    } else {
        sprintf(sMess, "Couldn't load Ico file [%s]", pFile);
        notifyObservers(NOTIFY_ERROR, sMess);
        printf("%s\n", sMess);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
//  loadEQ
//    Since there is only 1 unique EQsahedron per sibdivision,
//    and because creation time is much shorter than relinking,
//    we just get the relevant header info (subdivision level)
//    and create a EQsahedron here
//
int SurfaceManager::loadEQ(const char *pFile) {
    char sMess[512];
    int iResult = -1;

    EQsahedron *pEQ = EQsahedron::createEmpty();
    iResult = pEQ->load(pFile);
    if (iResult == 0) {

        pEQ->relink();
        printf("i-aaah\n");
        
        if (m_pSurface != NULL) {
            delete m_pSurface;
        }
        m_pSurface = pEQ;
        if (m_pSurfaceOGL != NULL) {
            delete m_pSurfaceOGL;
        }
        m_pSurfaceOGL = new IQEQIco_OGL(pEQ, m_pOverlay);
   
        sprintf(sMess, "IEQ file [%s] successfully loaded", pFile);
        notifyObservers(NOTIFY_MESSAGE, sMess);
        //       areacheck(m_pIcosahedron);
        
    } else {
        sprintf(sMess, "Couldn't load IEQ file [%s]", pFile);
        notifyObservers(NOTIFY_ERROR, sMess);
        printf("%s\n", sMess);
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
//  loadData
//
int SurfaceManager::loadData(const char *pDataFile, bool bForceCol) {

  
    int iResult = -1;
    char sMess[512];
    printf("SurfaceManager: loading [%s] (%s mode)\n", pDataFile,m_bQDF?"QDF":"Snap");
    if (m_bQDF) {
        //        m_pValueProvider = QDFValueProvider::createValueProvider(pDataFile);

        printf("setting mode to: [%s]\n", pDataFile);
        m_pValueProvider->setMode(pDataFile);
        printf("QDFValProv: %p\n", m_pValueProvider);
        dynamic_cast<QDFValueProvider*>(m_pValueProvider)->show();
    } else {
        if (m_pValueProvider != NULL) {
            delete m_pValueProvider;
        }
        m_pValueProvider = SnapValueProvider::createValueProvider(pDataFile);
    }

    if (m_pValueProvider != NULL) {
        if (m_pSurfaceOGL != NULL) {
            m_pSurfaceOGL->setValueProvider(m_pValueProvider);
        }
        iResult = 0;
 
        notifyObservers(NOTIFY_NEW_DATA, &bForceCol);
        sprintf(sMess, "Successfully loaded [%s]", pDataFile);
        notifyObservers(NOTIFY_MESSAGE, sMess);
        printf("%s\n", pDataFile);
    } else {
        sprintf(sMess, "Couldn't load data file [%s]", pDataFile);
        notifyObservers(NOTIFY_ERROR, sMess);
    }
    return iResult;
}
//-----------------------------------------------------------------------------
//  addData
//
int SurfaceManager::addData(const char *pDataFile) {

  
    int iResult = -1;
    char sMess[512];
    printf("SurfaceManager: adding [%s] (%s mode)\n", pDataFile,m_bQDF?"QDF":"Snap");
    if (m_bQDF) {
        //        m_pValueProvider = QDFValueProvider::createValueProvider(pDataFile);

        printf("adding data from file: [%s]\n", pDataFile);
        m_pValueProvider->addFile(pDataFile);
        printf("QDFValProv: %p\n", m_pValueProvider);
        dynamic_cast<QDFValueProvider*>(m_pValueProvider)->show();
    } else {

    }

    if (m_pValueProvider != NULL) {
        if (m_pSurfaceOGL != NULL) {
            m_pSurfaceOGL->setValueProvider(m_pValueProvider);
        }
        iResult = 0;
        bool bForceCol = true;
        notifyObservers(NOTIFY_ADD_DATA, &bForceCol);
        sprintf(sMess, "Successfully loaded [%s]", pDataFile);
        notifyObservers(NOTIFY_MESSAGE, sMess);
        printf("%s\n", pDataFile);
    } else {
        sprintf(sMess, "Couldn't load data file [%s]", pDataFile);
        notifyObservers(NOTIFY_ERROR, sMess);
    }
    return iResult;
}



//-----------------------------------------------------------------------------
//  clearData
//
void SurfaceManager::clearData() {
    if (m_pValueProvider != NULL) {
        delete m_pValueProvider;
    }
    m_pValueProvider = NULL;
    notifyObservers(NOTIFY_CLEAR_DATA, 0);
 }
