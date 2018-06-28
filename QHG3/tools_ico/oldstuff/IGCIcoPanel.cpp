#include <gtkmm.h>
#include <omp.h>

#include "utils.h"
#include "icoutil.h"
#include "IGCIcoPanel.h"
#include "Icosahedron.h"
#include "TrivialSplitter.h"
#include "IcoGridCreator.h"
#include "IcoGridNodes.h"
#include "RegionSplitter.h"

#include "SCellGrid.h"
#include "GridWriter.h"

//-----------------------------------------------------------------------------
// constructor
//   
// 
IGCIcoPanel::IGCIcoPanel(IcoScene *pIcoScene)
    :  m_iMode(MODE_ICO_FULL),
       m_pIcoScene(pIcoScene),
       m_pIcosahedron(Icosahedron::create(1, POLY_TYPE_ICO)),
       m_pOGL(NULL),
       m_radFull("Full Sphere"),
       m_radRect("Lon/Lat Rectangle"),
       m_radLand("Landmasses"),

       m_lblLon("Longitude", Gtk::ALIGN_LEFT),
       m_lblLat("Latitude", Gtk::ALIGN_LEFT),
       m_lblMin("Min", Gtk::ALIGN_LEFT),
       m_lblMax("Max", Gtk::ALIGN_LEFT),
       m_lblStep("Step", Gtk::ALIGN_LEFT),
       
       m_lblSubLandAlt("Min Alt", Gtk::ALIGN_LEFT),
      
       m_butSubDiv("Subdivide"),
       m_lblSubDivLev("Level", Gtk::ALIGN_LEFT) {

    m_pIcosahedron->relink();

    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp;
    m_radFull.set_group(gDisp);
    m_radRect.set_group(gDisp);
    m_radLand.set_group(gDisp);
    
    // push button signals
    m_butSubDiv.signal_clicked().connect(
      sigc::mem_fun(*this, &IGCIcoPanel::on_button_subdiv_clicked));
  
    // radio button signals
    m_radFull.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCIcoPanel::mode_action));
    m_radRect.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCIcoPanel::mode_action));
    m_radLand.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCIcoPanel::mode_action));


    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;

    attach(m_radFull,    0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_hSep1,         0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    // rect section
    attach(m_radRect,    0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_lblLon,       1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblLat,       3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblMin,       0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLonMin,    1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLatMin,    3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblMax,       0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLonMax,    1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLatMax,    3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblStep,      0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtDLon,      1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtDLat,      3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_hSep2,         0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
   
    // land section
    attach(m_radLand,    0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblSubLandAlt, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtSubLandAlt, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_hSep3,         0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

   
     // actions  section
    attach(m_lblSubDivLev, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtSubDivLev, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butSubDiv,    3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
   
    int iEW;
    int iEH;
    m_txtLonMin.get_size_request(iEW, iEH);

    m_txtLonMin.set_size_request(70, iEH);
    m_txtLonMax.set_size_request(70, iEH);
    m_txtDLon.set_size_request(70, iEH);

    m_txtLatMin.set_size_request(70, iEH);
    m_txtLatMax.set_size_request(70, iEH);
    m_txtDLat.set_size_request(70, iEH);

    m_txtSubLandAlt.set_size_request(70, iEH);
    m_txtSubDivLev.set_size_request(30, iEH);
   

    m_txtSubLandAlt.set_text("0");
    m_txtSubDivLev.set_text("0");

    // m_radConn6.set_active(true);
    setMode(MODE_ICO_FULL);

    printf("IGCIcoPanel::IGCIcoPanel done\n");

}


//-----------------------------------------------------------------------------
// destructor
//   
// 
IGCIcoPanel::~IGCIcoPanel() {
    if (m_pIcosahedron != NULL) {
        delete m_pIcosahedron;
    }
    if (m_pOGL != NULL) {
        delete m_pOGL;
    }
}

//-----------------------------------------------------------------------------
// setMode
//   
// 
void IGCIcoPanel::setMode(int iMode) {
    bool bOK = true;
    switch (iMode) {
    case MODE_ICO_FULL:
        enableRectSection(false);
        m_txtSubLandAlt.set_sensitive(false);
        break;
    case MODE_ICO_RECT:
        enableRectSection(true);
        m_txtSubLandAlt.set_sensitive(false);
        break;
    case MODE_ICO_LAND:
        enableRectSection(false);
        m_txtSubLandAlt.set_sensitive(true);
        break;
    default:
        bOK = false;
    }
    if (bOK) {
        m_iMode = iMode;
        notifyObservers(NOTIFY_ICO_MODE, &m_iMode);
    }
}

//-----------------------------------------------------------------------------
// on_button_subdiv_clicked
//   
// 
void IGCIcoPanel::on_button_subdiv_clicked() {
    printf("Subdive...\n");
  
    int iLevel = atoi(m_txtSubDivLev.get_text().c_str());
    double c0 = omp_get_wtime();

    switch (m_iMode) {
    case MODE_ICO_FULL:
        fullSubDiv(iLevel);
        break;
    case MODE_ICO_RECT:
        rectSubDiv(iLevel);
        break;
    case MODE_ICO_LAND:
        landSubDiv(iLevel);
        break;
    }
    double c1 = omp_get_wtime();
    printf("IGCIcoPanel::on_button_subdiv_clicked()] creation %fs\n", c1-c0);
    notifyObservers(NOTIFY_CREATED, (void *)false);

}

//-----------------------------------------------------------------------------
// fullSubDiv
//   
// 
void IGCIcoPanel::fullSubDiv(int iLevel) {
    m_pIcosahedron->merge(0);
    m_pIcosahedron->subdivide(iLevel);
    m_pIcosahedron->relink(); 
    printf("[IGCIcoPanel::fullSubDiv]surface: %p\n", m_pIcosahedron);
    m_pIcoScene->refreshImage();
}

//-----------------------------------------------------------------------------
// landSubDiv
//   
// 
void IGCIcoPanel::landSubDiv(int iLevel) {
    float fMinAlt = atof(m_txtSubLandAlt.get_text().c_str());
    m_pIcosahedron->merge(0);

    m_pIcosahedron->setPreSel(true);
    m_pIcosahedron->subdivideLand(m_pIcoScene->getValReader(), fMinAlt, iLevel);
    m_pIcosahedron->relink();
    printf("[IGCIcoPanel::landSubDiv]surface: %p\n", m_pIcosahedron);

    m_pIcoScene->refreshImage();
}

//-----------------------------------------------------------------------------
// rectSubDiv
//   
// 
void IGCIcoPanel::rectSubDiv(int iLevel) {
    tbox tBox(DEG2RAD(atof(m_txtLonMin.get_text().c_str())),
              DEG2RAD(atof(m_txtLonMax.get_text().c_str())),
              DEG2RAD(atof(m_txtLatMin.get_text().c_str())),
              DEG2RAD(atof(m_txtLatMax.get_text().c_str())));
    double dDLon   = DEG2RAD(atof(m_txtDLon.get_text().c_str()));
    double dDLat   = DEG2RAD(atof(m_txtDLat.get_text().c_str()));
    bool bPreSel = false;
    m_pIcosahedron->merge(0);
    m_pIcosahedron->setPreSel(bPreSel);
    printf("[IGCIcoPanel::rectSubDiv]surface before: %p\n", m_pIcosahedron);
    m_pIcosahedron->variableSubDivOMP(iLevel, tBox, dDLon, dDLat);
    printf("[IGCIcoPanel::rectSubDiv]surface after: %p\n", m_pIcosahedron);
    m_pIcosahedron->relink();

    m_pIcoScene->refreshImage();
    // make it nicely visible
    m_pIcoScene->centerPoint(DEG2RAD((atof(m_txtLonMin.get_text().c_str())+atof(m_txtLonMax.get_text().c_str()))/2), 
                           DEG2RAD((atof(m_txtLatMin.get_text().c_str())+atof(m_txtLatMax.get_text().c_str()))/2));
}



//-----------------------------------------------------------------------------
// mode_action
//   
// 
void IGCIcoPanel::mode_action() {
    if (m_radFull.get_active()) {
        setMode(MODE_ICO_FULL);
    } else if (m_radRect.get_active()) {
        setMode(MODE_ICO_RECT);
    } else if (m_radLand.get_active()) {
        setMode(MODE_ICO_LAND);
    }

}
 

//-----------------------------------------------------------------------------
// enableRectSection
//   
// 
void IGCIcoPanel::enableRectSection(bool bEnable) {
    m_txtLonMin.set_sensitive(bEnable);
    m_txtLonMax.set_sensitive(bEnable);
    m_txtDLon.set_sensitive(bEnable);

    m_txtLatMin.set_sensitive(bEnable);
    m_txtLatMax.set_sensitive(bEnable);
    m_txtDLat.set_sensitive(bEnable);
    
}


//----------------------------------------------------------------------------
// saveSurface
//
int IGCIcoPanel::saveSurface(const char *pNameObj, 
                             const char *pNameIGN, 
                             const char *pNameQDF, 
                             RegionSplitter *pRS) {

    int iResult = -1;
    if (pNameObj != NULL) {
        iResult = m_pIcosahedron->save(pNameObj);
    }
    if (iResult == 0) {

        stringmap smSurfaceHeaders;
        smSurfaceHeaders[SURF_TYPE] = SURF_ICOSAHEDRON;
        char sd[8];
        sprintf(sd, "%d", m_pIcosahedron->getSubLevel());
        smSurfaceHeaders[SURF_ICO_SUBLEVEL] = sd;

        int iHalo = 1;
        bool bPresel     = false;
        bool bSuperCells = true;
        bool bNodeOrder  = false;
        
        IcoGridCreator *pIGC = IcoGridCreator::createInstance(m_pIcosahedron, bPresel, iHalo, pRS, bSuperCells, bNodeOrder);
        if (pIGC != NULL) {
            for (int i = 0; (iResult == 0) && (i < pRS->getNumRegions()); i++) {
                IcoGridNodes *pIGN = pIGC->getGrid(i);

                if ((iResult == 0) && (pNameQDF != NULL)) {
                    SCellGrid *pCG = SCellGrid::createInstance(pIGN);
                    GridWriter *pGW = new GridWriter(pCG, &smSurfaceHeaders);
                    iResult = pGW->writeToQDF(pNameQDF, 0, false);
                    if (iResult != 0) {
                        printf("Couldn't save IGN file [%s]\n", pNameQDF);
                        iResult = -1;
                    }
                    
                    delete pGW;
                    delete pCG;
                }

                if ((iResult == 0) && (pNameIGN != NULL)) {

                    char sIGNOut[512];
                    sprintf(sIGNOut, pNameIGN, i);
                    printf("Writing IGN file: [%s]\n", sIGNOut);
                    iResult = pIGN->write(sIGNOut, 6, false, smSurfaceHeaders);  // false: no tiling info
                    
                    if (iResult != 0) {
                        printf("Couldn't save IGN file [%s]\n", sIGNOut);
                        iResult = -1;
                    }
                }
                
                delete pIGN;
            }
            if (iResult == 0) {
                notifyObservers(NOTIFY_TILED, (void *)false);
            }
            
            delete pIGC;
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
int IGCIcoPanel::loadSurface(const char *pNameSurface) {
    int iResult = -1;
    m_pIcosahedron->merge(0);
    printf("[IGCIcoPanel::loadSurface] [%s]\n", pNameSurface);
    iResult = m_pIcosahedron->load(pNameSurface);
    
    if (iResult == 0) {    
        m_pIcosahedron->setPreSel(true);
        m_pIcosahedron->setStrict(true);
        m_pIcosahedron->relink();

        activateSurface();
    } else {
        printf("Couldn't load surface file [%s]\n", pNameSurface);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// activateSurface
//
void IGCIcoPanel::activateSurface() {
    m_pIcoScene->setFlat(false);
    if (m_pOGL != NULL) {
        delete m_pOGL;
    }
    m_pOGL = new Ico_OGL(m_pIcosahedron);
    
    //    printf("[IGCIcoPanel::activateSurface]surface created: %p, OGL %p\n", m_pIcosahedron, m_pOGL);
    m_pIcoScene->setSurface(m_pIcosahedron, m_pOGL);
    //    printf("[IGCIcoPanel::activateSurface]surface set: %p\n", m_pIcosahedron);
}

