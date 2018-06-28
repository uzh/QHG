#include <gtkmm.h>

#include "utils.h"
#include "strutils.h"
#include "GeoInfo.h"
#include "Projector.h"
#include "GridProjection.h"

#include "icoutil.h"
#include "RegionSplitter.h"
#include "TrivialSplitter.h"
#include "RectGridCreator.h"
#include "Lattice.h"
#include "Lattice_OGL.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"
#include "IGCFlatPanel.h"

#include "SCellGrid.h"
#include "GridWriter.h"

#define H4 1
#define H6 (sqrt(3)/2)

double realSizes[8][2] = {
    {2*M_PI, M_PI},
    {2, 2}, 
    {2*M_PI, 2*M_PI},
    {M_PI, M_PI},
    {4, 4},
    {2*M_PI, M_PI},
    {2*M_PI, M_PI},
    {1,1}
,
};

//-----------------------------------------------------------------------------
// constructor
//   
// 
IGCFlatPanel::IGCFlatPanel(IcoScene *pIcoScene)
    :  m_pGP(NULL),
       m_pIcoScene(pIcoScene),
       m_pLattice(new Lattice()),
       m_pOGL(NULL),
       m_iGW(0),
       m_iGH(0),
       m_iMode(MODE_RECT_HEX),
       m_lblProjType("Type",  Gtk::ALIGN_LEFT),
       m_lblProjLon("Lon 0",  Gtk::ALIGN_RIGHT),
       m_lblProjLat("Lat 0",  Gtk::ALIGN_RIGHT),
       
       m_lblGridSize("Grid Size",  Gtk::ALIGN_LEFT),
       m_lblTimes1(" x "), 
       m_lblRealSize("Real Size",  Gtk::ALIGN_LEFT),
       m_lblTimes2(" x "), 
       m_lblOffset("Offset",  Gtk::ALIGN_LEFT),
       m_lblPlus(" + "), 
       m_lblConnectivity("Connectivity"),
       m_radConn4("4"),
       m_radConn6("6"), 
       m_butCreate("Create") {


    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp2;
    m_radConn4.set_group(gDisp2);
    m_radConn6.set_group(gDisp2);

    // push button signals
    m_butCreate.signal_clicked().connect(
      sigc::mem_fun(*this, &IGCFlatPanel::on_button_create_clicked));

    // radio button signals
    m_radConn4.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCFlatPanel::mode_action));
    m_radConn6.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCFlatPanel::mode_action));

    m_lstProjTypes.signal_changed().connect( sigc::mem_fun(*this, &IGCFlatPanel::ptype_action) );
  
    m_txtGridW.add_events(Gdk::FOCUS_CHANGE_MASK);
    m_txtGridH.add_events(Gdk::FOCUS_CHANGE_MASK);
    m_txtGridW.signal_focus_out_event().connect(sigc::mem_fun(*this, &IGCFlatPanel::focus_action) );
    m_txtGridH.signal_focus_out_event().connect(sigc::mem_fun(*this, &IGCFlatPanel::focus_action) );

    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
   attach(m_lblProjType,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lstProjTypes,   1, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
       
    attach(m_lblProjLon,     0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtProjLon,     1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblProjLat,     2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtProjLat,     3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    
    attach(m_hSep2,          0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    
    attach(m_lblGridSize,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtGridW,       1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTimes1,      2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtGridH,       3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_lblRealSize,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtRealW,       1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTimes2,      2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtRealH,       3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_lblOffset,      0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtOffsX,       1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblPlus,        2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtOffsY,       3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_hSep3,          0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_lblConnectivity, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_radConn4,        1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_radConn6,        3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_butCreate,       2, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    
  
    int iEW;
    int iEH;
    m_txtGridW.get_size_request(iEW, iEH);
    m_txtGridW.set_size_request(60, iEH);
    m_txtGridW.set_alignment(1);
    m_txtGridH.set_size_request(60, iEH);
    m_txtGridH.set_alignment(1);
    m_txtRealW.set_size_request(60, iEH);
    m_txtRealW.set_alignment(1);
    m_txtRealH.set_size_request(60, iEH);
    m_txtRealH.set_alignment(1);
    m_txtOffsX.set_size_request(60, iEH);
    m_txtOffsX.set_alignment(1);
    m_txtOffsY.set_size_request(60, iEH);
    m_txtOffsY.set_alignment(1);
    m_txtProjLon.set_size_request(60, iEH);
    m_txtProjLon.set_alignment(1);
    m_txtProjLat.set_size_request(60, iEH);
    m_txtProjLat.set_alignment(1);
    
    char s[16];
    sprintf(s, "%d", m_iGW);
    m_txtGridW.set_text(s);
    sprintf(s, "%d", m_iGH);
    m_txtGridH.set_text(s);
    m_txtProjLon.set_text("0");
    m_txtProjLat.set_text("0");
    m_txtOffsX.set_text("c");
    m_txtOffsY.set_text("c");
    m_txtGridW.set_text("0");
    m_txtGridH.set_text("0");


    m_radConn6.set_active(true);


    // fill projection list
    char sTemp[256];
    m_refTreeModelProj = Gtk::ListStore::create(m_ProjColumns);
    m_lstProjTypes.set_model(m_refTreeModelProj);
    for (int i = 0; i < GeoInfo::getNumProj(); i++) {
        Gtk::TreeModel::Row row = *(m_refTreeModelProj->append());
        strcpy(sTemp,  GeoInfo::getName(i));
        char *p = strrchr(sTemp, ' ');
        if (p != 0) {
            *p = '\0';
        }
        row[m_ProjColumns.m_colType] = i;
        row[m_ProjColumns.m_colName] = sTemp;
    }
    m_lstProjTypes.pack_start(m_ProjColumns.m_colName);
    

    m_lstProjTypes.set_active(PR_LINEAR);
    ptype_action();
    createGridProjection(1.0);



    printf("IGCFlatPanel::IGCFlatPanel done\n");

}


//-----------------------------------------------------------------------------
// destructor
//   
// 
IGCFlatPanel::~IGCFlatPanel() {

    if (m_pGP != NULL) {
        delete m_pGP;
    }
   
    if (m_pLattice != NULL) {
        delete m_pLattice;
    }
    if (m_pOGL != NULL) {
        delete m_pOGL;
    }

}


//-----------------------------------------------------------------------------
// on_button_create_clicked
//   
// 
void IGCFlatPanel::on_button_create_clicked() {
    printf("Create...\n");
    bool b4 = m_radConn4.get_active();
    int iNumLinks = b4?4:6;
    double dHCell = b4?H4:H6;

    createGridProjection(dHCell);

  
    m_pLattice->create(iNumLinks, m_pGP);
    m_pIcoScene->refreshImage();
    notifyObservers(NOTIFY_CREATED, (void *)true);

}

//----------------------------------------------------------------------------
// ptype_action
//  change real-size fields
//
void IGCFlatPanel::ptype_action() {
   int iProjType = m_lstProjTypes.get_active_row_number();
    char s[16];
    
    sprintf(s, "%f", realSizes[iProjType][0]);
    m_txtRealW.set_text(s);
    sprintf(s, "%f", realSizes[iProjType][1]);
    m_txtRealH.set_text(s);

    int iGW;
    if (strToNum(m_txtGridW.get_text().c_str(), &iGW)) {
        int iGH = (int)((iGW* realSizes[iProjType][1])/realSizes[iProjType][0]);
        sprintf(s, "%d", iGH);
        m_txtGridH.set_text(s);
    }

    if (iProjType == PR_EQUIRECTANGULAR) {
        m_txtProjLon.set_text("150");
    } else {
        m_txtProjLon.set_text("0");
    }
}

//----------------------------------------------------------------------------
// focus_action
//  
//
bool IGCFlatPanel::focus_action(GdkEventFocus* event) {
    int iG;
    if (strToNum(m_txtGridW.get_text().c_str(), &iG)) {
        char sG[64];
        sprintf(sG, "%d", iG-1);
        m_txtRealW.set_text(sG);
    }
    if (strToNum(m_txtGridH.get_text().c_str(), &iG)) {
        char sG[64];
        sprintf(sG, "%d", iG-1);
        m_txtRealH.set_text(sG);
    }
    return false;
}


//----------------------------------------------------------------------------
// createGridProjection
//
int IGCFlatPanel::createGridProjection(double dHCell) {
    int iResult = -1;

    // use data to create ProjType and ProjGrid

    int iProjType = m_lstProjTypes.get_active_row_number();

    double dLambda0;
    double dPhi0;
    double dRW;
    double dRH;
    double dOX=0.0;
    double dOY=0.0;
    bool bCenterX=false;
    bool bCenterY=false;
    double dFactor = (iProjType == PR_LINEAR)?1:M_PI/180;
    if (strToNum(m_txtProjLon.get_text().c_str(), &dLambda0)) {
        dLambda0 *= dFactor;
        if (strToNum(m_txtProjLat.get_text().c_str(), &dPhi0)) {
            dPhi0 *= dFactor;
            // now the grid params
            if (strToNum(m_txtGridW.get_text().c_str(), &m_iGW)) {
                if (strToNum(m_txtGridH.get_text().c_str(), &m_iGH)) {
                    if (strToNum(m_txtRealW.get_text().c_str(), &dRW)) {
                        if (strToNum(m_txtRealH.get_text().c_str(), &dRH)) {
                            if (strToNum(m_txtOffsX.get_text().c_str(), &dOX)) {
                                iResult = 0;
                            } else {
                                if (m_txtOffsX.get_text().c_str()[0]=='c') {
                                    bCenterX = true;
                                    iResult = 0;
                                }
                            }
                            if (iResult == 0) {
                                if (strToNum(m_txtOffsY.get_text().c_str(), &dOY)) {
                                    iResult = 0;
                                } else {
                                    if (m_txtOffsY.get_text().c_str()[0]=='c') {
                                        bCenterY = true;
                                        iResult =0;
                                    } else {
                                        iResult = -1;
                                    }
                                }
                            }
                        } else {
                            printf("Bad realH: [%s]\n", m_txtRealH.get_text().c_str());
                        }
                    } else {
                        printf("Bad realW: [%s]\n", m_txtRealW.get_text().c_str());
                    }
                } else {
                    printf("Bad gridH: [%s]\n", m_txtGridH.get_text().c_str());
                }
            } else {
                printf("Bad gridW: [%s]\n", m_txtGridW.get_text().c_str());
            }
        } else {
            printf("Bad phi: [%s]\n", m_txtProjLat.get_text().c_str());
        }
    } else {
        printf("Bad lambda: [%s]\n", m_txtProjLon.get_text().c_str());
    }
    // use different grid size if preview is needed
    ProjGrid *pPG = NULL;
    if (iResult == 0) {
            if (bCenterX) {
            dOX = (-1.0*(m_iGW-1.0))/2.0;
        }
        if (bCenterY) {
            if (dHCell == H4) {
                dOY = (-1.0*(m_iGH-1.0))/2.0;
            } else {
                // for symmetric latitude extents
                dOY =  -(((int)((m_iGH)/dHCell)-1)*dHCell)/2.0;
                // for same min lat as in quad case
                // dOY = (-1.0*(m_iGH-1.0))/2.0;
            }
 
        }
        printf("!!proj %d, (%f,%f) %fx%f %fx%f +%f+%f\n", 
               iProjType, dLambda0, dPhi0, dRW, dRH, m_iGW-1.0, m_iGH-1.0 , dOX, dOY);

        pPG = new ProjGrid(m_iGW-1, m_iGH-1.0, dRW, dRH, dOX, dOY, 1.0);
        
        ProjType PT(iProjType, dLambda0, dPhi0, 0, NULL);
        if (m_pGP != NULL) {
            delete m_pGP;
            m_pGP = NULL;
        }
        Projector *pProj = GeoInfo::instance()->createProjector(&PT);
        m_pGP = new GridProjection(pPG, pProj, true, true);
        notifyObservers(NOTIFY_NEW_GP, (void*) m_pGP);
        
         GeoInfo::free();
         notifyObservers(NOTIFY_FLAT_PROJ, (void *) &iProjType);

         //delete pPG;
    }
    return iResult;
}


#define MAX_NEIGHBORS 6
//----------------------------------------------------------------------------
// saveSurface
//
int IGCFlatPanel::saveSurface(const char *pNameObj, 
                              const char *pNameIGN, 
                              const char *pNameQDF, 
                              RegionSplitter *pRS) {
    int iResult = 0;

    if (pNameObj != NULL) {
        iResult = m_pLattice->save(pNameObj);
    }
    if (iResult == 0) {
        stringmap smSurfaceHeaders;
        char sX[8];
        char sY[8];
        char sL[8];
        sprintf(sX, "%d", m_pLattice->getNumX());
        sprintf(sY, "%d", m_pLattice->getNumY());
        sprintf(sL, "%d", m_pLattice->getNumLinks());
        smSurfaceHeaders[SURF_TYPE] = SURF_LATTICE;
        smSurfaceHeaders[SURF_LTC_W] = sX;
        smSurfaceHeaders[SURF_LTC_H] = sY;
        smSurfaceHeaders[SURF_LTC_LINKS] = sL;
        smSurfaceHeaders[SURF_LTC_PERIODIC] = "0";
    
        GridProjection *pGP = m_pLattice->getGridProjection();
        const ProjGrid  *pPG = pGP->getProjGrid();
        const Projector *pPR = pGP->getProjector();
        ProjType PT(pPR->getID(), pPR->getLambda0(), pPR->getPhi0(), 0, NULL);
    
        smSurfaceHeaders[SURF_LTC_PROJ_TYPE] = PT.toString(true);
        smSurfaceHeaders[SURF_LTC_PROJ_GRID] = pPG->toString();
    
        bool bOrderLinks = (m_pLattice->getNumLinks() == 4) && (pPR->getID() == PR_LINEAR);

        bool bSuperCells = true;
    
        double dH = m_radConn4.get_active()?H4:H6;
    
        int iHalo = 1;
        RectGridCreator *pRGC = RectGridCreator::createInstance(m_pLattice, dH, iHalo, pRS, bSuperCells, false); // false: no node ordering
        if (pRGC != NULL) {
        
            for (int i = 0; (iResult == 0) && (i < pRS->getNumRegions()); i++) {
                IcoGridNodes *pIGN = pRGC->getGrid(i);
            
                if (bOrderLinks) {
                    // this only makes sense for LINEAR projection
                    std::map<gridtype, IcoNode*>::iterator it;
                    for (it = pIGN->m_mNodes.begin(); it != pIGN->m_mNodes.end(); it++) {
                        double dLon = it->second->m_dLon;
                        double dLat = it->second->m_dLat;
                        int iNumLinks = it->second->m_iNumLinks;
                        if (iNumLinks <= 4) {
                            int iI[4];
                            
                            for (int i = 0; i < 4; i++) {
                                iI[i] = -1;
                            }
                            
                            for (int i = 0; i < iNumLinks; i++) {
                                IcoNode *pI = pIGN->m_mNodes[it->second->m_aiLinks[i]];
                                double dLon1 = pI->m_dLon;
                                double dLat1 = pI->m_dLat;
                                int iDir = -1;
                                if (dLon1 > dLon) {
                                    iDir = 0; // E
                                } else if (dLon1 < dLon) {
                                    iDir = 2; // W
                                } else if (dLat1 > dLat) {
                                    iDir = 3; // S
                                } else if (dLat1 < dLat) {
                                    iDir = 1; // N
                                }

                                if (iDir >= 0) {
                                    iI[iDir] = it->second->m_aiLinks[i];
                                }
                                
                            }

                            it->second->m_iNumLinks = 4;
                            memcpy(it->second->m_aiLinks, iI, 4*sizeof(int));
                        } else {
                            printf("bad number of links (expeted <= 4) [%d]\n", iNumLinks);
                            iResult = -1;
                        }
                    }
                }

                if ((iResult == 0) && (pNameQDF != NULL)) {
                    SCellGrid *pCG = SCellGrid::createInstance(pIGN);
            
                    GridWriter *pGW = new GridWriter(pCG, &smSurfaceHeaders);
                    iResult = pGW->writeToQDF(pNameQDF, 0, false);
                    if (iResult != 0) {
                        printf("Couldn't save QDF file [%s]\n", pNameQDF);
                        iResult = -1;
                    }
                
                    delete pGW;
                    delete pCG;
                }
            
                if ((iResult == 0) && (pNameIGN != NULL)) {
                
                    char sIGNOut[512];
                    sprintf(sIGNOut, pNameIGN, i);
                
                    iResult = pIGN->write(sIGNOut, m_pLattice->getNumLinks(), false, smSurfaceHeaders);  // false: no tiling info
                    if (iResult != 0) {
                        printf("Couldn't save IGN file [%s]\n", sIGNOut);
                        iResult = -1;
                    }
                }    
                delete pIGN;
            }
            if (iResult == 0) {
                notifyObservers(NOTIFY_TILED, (void *)true);
            }

            delete pRGC;
        } else {
            printf("Couldn't create RectGridCreator\n");
            iResult = -1;
        }
    } else {
        printf("Couldn't write Surface file [%s]\n", pNameObj);
        iResult = -1;
    }
    if (iResult != 0) {
        printf("save failed\n");
    }
    
    return iResult;
}

//----------------------------------------------------------------------------
// loadSurface
//
int IGCFlatPanel::loadSurface(const char *pNameSurface) {
    int iResult = -1;
    printf("[IGCFlatPanel::loadSurface] [%s]\n", pNameSurface);
    iResult = m_pLattice->load(pNameSurface);
    
    if (iResult != 0) {
        printf("Couldn't load surface file [%s]\n", pNameSurface);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// activateSurface
//
void IGCFlatPanel::activateSurface() {
    m_pIcoScene->setFlat(true);
    if (m_pOGL != NULL) {
        delete m_pOGL;
    }
    m_pOGL = new Lattice_OGL(m_pLattice);
    printf("[IGCFlatPanel::activateSurface]surface created: %p, OGL %p\n", m_pLattice, m_pOGL);

    m_pIcoScene->setSurface(m_pLattice, m_pOGL);
    printf("[IGCFlatPanel::activateSurface]surface set: %p\n", m_pLattice);
}

//-----------------------------------------------------------------------------
// setMode
//   
// 
void IGCFlatPanel::setMode(int iMode) {
    bool bOK = false;
    if (m_iMode != iMode) {
        switch (iMode) {
        case MODE_RECT_QUAD:
            bOK = true;
            break;
        case MODE_RECT_HEX:
            bOK = true;
            break;
        }
        if (bOK) {
            m_iMode = iMode;
            notifyObservers(NOTIFY_FLAT_LINK, (void *)&m_iMode);
        }
    }
}

//-----------------------------------------------------------------------------
// mode_action
//   
// 
void IGCFlatPanel::mode_action() {

    if (m_radConn4.get_active()) {
        setMode(MODE_RECT_QUAD);
    } else if (m_radConn6.get_active()) {
        setMode(MODE_RECT_HEX);
    }
}

