#include <gtkmm.h>
#include <omp.h>

#include "utils.h"
#include "icoutil.h"
#include "IGCEQPanel.h"
#include "EQsahedron.h"
#include "TrivialSplitter.h"
#include "EQGridCreator.h"
#include "IcoGridNodes.h"
#include "RegionSplitter.h"

#include "SCellGrid.h"
#include "GridWriter.h"

const int MODE_ROTAXIS = 0;
const int MODE_QUAT = 1;
const char *s_aDefValues[2][4] = {
    {"0", "0", "0", "0"},
    {"1", "0", "0", "0"},
};
const char *s_aRadCaptions[2][4]= {
    {"Angle", "X", "Y", "Z"},
    {"R", "I", "J", "K"},
};

//-----------------------------------------------------------------------------
// constructor
//   
// 
IGCEQPanel::IGCEQPanel(IcoScene *pIcoScene)
    :  m_iRotMode(MODE_ROTAXIS),
       m_pIcoScene(pIcoScene),
       m_pEQ(EQsahedron::createInstance(0, true, NULL/*, dNaN, NULL*/)),
       m_pOGL(NULL),
       /*
       m_radRotAxis("Rotation and axis"),
       m_radQuat("Quaternion"),

       m_lblRot("Initial Rotation", Gtk::ALIGN_LEFT),
       */
       m_chkEqualArea("Equal Area Subdivision"),

       m_chkLand("Land"),
       m_lblMinLandAlt("Altitude"),

       m_chkRect("Rect"),
       m_lblLon("Lon"),
       m_lblLat("Lat"),
       m_lblMin("Min"),
       m_lblMax("Max"),

       m_butSubDiv("Subdivide"),
       m_lblSubDivLev("Subdivisions", Gtk::ALIGN_LEFT) {

    m_pEQ->relink();

    /*
    for (int i =0; i < 4; i++) {
        m_ptxtR[i] = new Gtk::Entry;
        m_plblR[i] = new Gtk::Label(s_aRadCaptions[m_iRotMode][i]);
    }

    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp;
    m_radRotAxis.set_group(gDisp);
    m_radQuat.set_group(gDisp);
    */

    // push button signals
    m_butSubDiv.signal_clicked().connect(
      sigc::mem_fun(*this, &IGCEQPanel::on_button_subdiv_clicked));
  
    // radio button signals
    /*
    m_radRotAxis.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCEQPanel::rot_action));
    m_radQuat.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCEQPanel::rot_action));
    */

    // check button signals
    m_chkLand.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCEQPanel::land_action));

    m_chkRect.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGCEQPanel::rect_action));


    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    /*
    attach(m_lblRot,    0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_radRotAxis,   0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_radQuat,      2, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    for (int i = 0; i < 4; i++) {
        attach(*m_plblR[i],    i, i+1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    }
    iTop++;
    for (int i = 0; i < 4; i++) {
        attach(*m_ptxtR[i],    i, i+1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    }
    iTop++;
    */

    attach(m_chkEqualArea, 0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_hSep1,         0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    // lnd
    attach(m_chkLand, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblMinLandAlt, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtMinLandAlt, 2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_hSep2,         0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    // rect
    attach(m_chkRect, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblMin, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblMax, 2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblLon, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLonMin, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLonMax, 2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblLat, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLatMin, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtLatMax, 2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_hSep3,         0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    
     // actions  section
    attach(m_lblSubDivLev, 0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtSubDivLev, 2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butSubDiv,    3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
   
    int iEW;
    int iEH;
    m_txtSubDivLev.get_size_request(iEW, iEH);
    /*
    for (int i = 0; i < 4; i++) {
        m_ptxtR[i]->set_size_request(65, iEH);
        m_ptxtR[i]->set_text(s_aDefValues[m_iRotMode][i]);
    } 
    */   
    m_txtSubDivLev.set_size_request(65, iEH);
    m_txtSubDivLev.set_text("0");
    
    m_txtMinLandAlt.set_size_request(65, iEH);
    m_txtMinLandAlt.set_text("0");

    m_txtLonMin.set_size_request(65, iEH);
    m_txtLonMin.set_text("0");
    m_txtLonMax.set_size_request(65, iEH);
    m_txtLonMax.set_text("0");
    m_txtLatMin.set_size_request(65, iEH);
    m_txtLatMin.set_text("0");
    m_txtLatMax.set_size_request(65, iEH);
    m_txtLatMax.set_text("0");

    m_chkEqualArea.set_active(true);
    m_chkLand.set_active(false);
    setLandMode(false);

    m_chkRect.set_active(false);
    setRectMode(false);


    /*
    setRotMode(MODE_ROTAXIS);
    // for the moment
    m_lblRot.set_sensitive(false);
    m_radRotAxis.set_sensitive(false);
    m_radQuat.set_sensitive(false);
    for (int i = 0; i < 4; i++) {
        m_ptxtR[i]->set_sensitive(false);
        m_plblR[i]->set_sensitive(false);
    }
    */
    printf("IGCEQPanel::IGCEQPanel done\n");

}


//-----------------------------------------------------------------------------
// destructor
//   
// 
IGCEQPanel::~IGCEQPanel() {
    if (m_pEQ != NULL) {
        delete m_pEQ;
    }
    if (m_pOGL != NULL) {
        delete m_pOGL;
    }
}

/*
//-----------------------------------------------------------------------------
// setRotMode
//   
// 
void IGCEQPanel::setRotMode(int iRotMode) {
    bool bOK = true;
    switch (iRotMode) {
    case MODE_ROTAXIS:
        break;
    case MODE_QUAT:
        break;
    default:
        bOK = false;
    }
    if (bOK) {
        m_iRotMode = iRotMode;
        for (int i = 0; i < 4; i++) {
            m_ptxtR[i]->set_text(s_aDefValues[m_iRotMode][i]);
            m_plblR[i]->set_text(s_aRadCaptions[m_iRotMode][i]);
        }    

        notifyObservers(NOTIFY_ROT_MODE, &m_iRotMode);
    }
}
*/
//-----------------------------------------------------------------------------
// setLandMode
//   
// 
void IGCEQPanel::setLandMode(bool bLand) {
    m_lblMinLandAlt.set_sensitive(bLand);
    m_txtMinLandAlt.set_sensitive(bLand);
}

//-----------------------------------------------------------------------------
// setRectMode
//   
// 
void IGCEQPanel::setRectMode(bool bRect) {
    m_lblMin.set_sensitive(bRect);
    m_lblMax.set_sensitive(bRect);
    m_lblLon.set_sensitive(bRect);
    m_lblLat.set_sensitive(bRect);
    m_txtLonMin.set_sensitive(bRect);
    m_txtLonMax.set_sensitive(bRect);
    m_txtLatMin.set_sensitive(bRect);
    m_txtLatMax.set_sensitive(bRect);
}

//-----------------------------------------------------------------------------
// on_button_subdiv_clicked
//   
// 
void IGCEQPanel::on_button_subdiv_clicked() {
    printf("Subdive...\n");
  
    int iLevel = atoi(m_txtSubDivLev.get_text().c_str());
    bool bTegmark = m_chkEqualArea.get_active();
    double c0 = omp_get_wtime();

    // see if we do a "land" grid
    float fMinAlt = fNaN;
    ValReader *pVR = NULL;
    if (m_chkLand.get_active()) {
        fMinAlt = atof(m_txtMinLandAlt.get_text().c_str());
        pVR = m_pIcoScene->getValReader();
    }
        
    // see if we do a rect
    tbox *pbox = NULL;
    tbox box(dNaN,dNaN,dNaN,dNaN);
    if (m_chkRect.get_active()) {
        box.dLonMin = atof(m_txtLonMin.get_text().c_str());
        box.dLonMax = atof(m_txtLonMax.get_text().c_str());
        box.dLatMin = atof(m_txtLatMin.get_text().c_str());
        box.dLatMax = atof(m_txtLatMax.get_text().c_str());
        pbox = &box;
    }
    // delete potential previous EQ
    if (m_pEQ != NULL) {
        delete m_pEQ;
    }

    // create an EQsahedron with requesed number of edge subdivisions
    m_pEQ = EQsahedron::createInstance(iLevel, bTegmark, pVR/*, fMinAlt, pbox*/);
    m_pEQ->relink();
    double c1 = omp_get_wtime();

    printf("IGCEQPanel::on_button_subdiv_clicked()] creatiuon %fs\n", c1-c0);
   
    // this will display the object 
    activateSurface();

    notifyObservers(NOTIFY_CREATED, (void *)false);
   

    if (pbox != NULL) {
        m_pIcoScene->centerPoint(DEG2RAD((pbox->dLonMin+pbox->dLonMax+((pbox->dLonMin>pbox->dLonMax)?360:0))/2), 
                                 DEG2RAD((pbox->dLatMin+pbox->dLatMax)/2));
    }
}


/*
//-----------------------------------------------------------------------------
// rot_action
//   
// 
void IGCEQPanel::rot_action() {
    if (m_radRotAxis.get_active()) {
        setRotMode(MODE_ROTAXIS);
    } else if (m_radQuat.get_active()) {
        setRotMode(MODE_QUAT);
    }
}
*/
//-----------------------------------------------------------------------------
// land_action
//   
// 
void IGCEQPanel::land_action() {
    setLandMode(m_chkLand.get_active());
}

//-----------------------------------------------------------------------------
// rect_action
//   
// 
void IGCEQPanel::rect_action() {
    setRectMode(m_chkRect.get_active());
}

//----------------------------------------------------------------------------
// saveSurface
//
int IGCEQPanel::saveSurface(const char *pNameObj, 
                            const char *pNameIGN, 
                            const char *pNameQDF, 
                            RegionSplitter *pRS) {

    int iResult = 0;
    if (pNameObj != NULL) {
        iResult = m_pEQ->save(pNameObj);
    }

    if (iResult == 0) {
        stringmap smSurfaceHeaders;
        smSurfaceHeaders[SURF_TYPE] = SURF_EQSAHEDRON;
        char sd[8];
        sprintf(sd, "%d", m_pEQ->getSubDivs());
        smSurfaceHeaders[SURF_IEQ_SUBDIVS] = sd;
    
    
        int iHalo = 1;
        bool bSuperCells = true;
        bool bNodeOrder  = false;
        EQGridCreator *pEGC = EQGridCreator::createInstance(m_pEQ, iHalo, pRS, bSuperCells, bNodeOrder);
        if (pEGC != NULL) {
        
            for (int i = 0; (iResult == 0) && (i < pRS->getNumRegions()); i++) {
                IcoGridNodes *pIGN = pEGC->getGrid(i);
            
                SCellGrid *pCG = SCellGrid::createInstance(pIGN);
            
            
                if ((iResult == 0) && (pNameQDF != NULL)) {
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
                    printf("Writing IGN file: [%s]\n", sIGNOut);
                    iResult = pIGN->write(sIGNOut, 6, false, smSurfaceHeaders);  // false: no tiling info
                
                    if (iResult != 0) {
                        printf("Couldn't save IGN file [%s]\n", sIGNOut);
                        iResult = -1;
                    }
                
                }
                delete pIGN;

                if (iResult == 0) {
                    notifyObservers(NOTIFY_TILED, (void *)false);
                }
            
            }
            delete pEGC;
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
int IGCEQPanel::loadSurface(const char *pNameSurface) {
    int iResult = -1;
    if (m_pEQ != NULL) {
        delete m_pEQ;
    }
    m_pEQ = EQsahedron::createEmpty();
    iResult = m_pEQ->load(pNameSurface);
    
    if (iResult == 0) {    
        m_pEQ->relink();

        activateSurface();
    } else {
        printf("Couldn't load surface file [%s]\n", pNameSurface);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// activateSurface
//
void IGCEQPanel::activateSurface() {
    m_pIcoScene->setFlat(false);
    if (m_pOGL != NULL) {
        delete m_pOGL;
    }
    m_pOGL = new EQ_OGL(m_pEQ);
    
    //    printf("[IGCIcoPanel::activateSurface]surface created: %p, OGL %p\n", m_pIcosahedron, m_pOGL);
    m_pIcoScene->setSurface(m_pEQ, m_pOGL);
    //    printf("[IGCIcoPanel::activateSurface]surface set: %p\n", m_pIcosahedron);
}
