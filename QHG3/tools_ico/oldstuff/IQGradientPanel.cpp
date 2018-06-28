#include <gtkmm.h>

#include "strutils.h"
#include "notification_codes.h"
#include "ProjInfo.h"
#include "IcoGridNodes.h"
#include "IcoGridCreator.h"
#include "Icosahedron.h"
#include "IQGradientFinder.h"
#include "IQGradientPanel.h"



IQGradientPanel::IQGradientPanel(IQOverlay *pOverlay, ProjInfo *pPI, Surface *pSurface) 
    : m_pOverlay(pOverlay),
      m_pPI(pPI),
      m_pSurface(pSurface),
      m_radStartNode("Start Node ID"),
      m_radStartCoord("Start Coords"),
      m_lblStartNode("Node ID", Gtk::ALIGN_LEFT),
      m_lblStartLon("Lon", Gtk::ALIGN_LEFT),
      m_lblStartLat("Lat", Gtk::ALIGN_LEFT),
      m_lblStopVal("Stop value", Gtk::ALIGN_LEFT),
      m_lblMinVal("Min value", Gtk::ALIGN_LEFT),
      m_butTrack("Track"),
      m_butClear("Clear"),
      m_lblOutput("Output file", Gtk::ALIGN_LEFT),
      m_butSave("Save"),
      m_bNodeMode(true),
      m_bNotifyInProgress(false) {

    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp;
    m_radStartNode.set_group(gDisp);
    m_radStartCoord.set_group(gDisp);
    m_radStartNode.set_active(true);


    printf("[IQGradientPanel::IQGradientPanel] has overlay %p\n", m_pOverlay);
    printf("[IQGradientPanel::IQGradientPanel] has surface %p\n", m_pSurface);
    // connect signals
    m_butClear.signal_clicked().connect(
      sigc::mem_fun(*this, &IQGradientPanel::on_button_clear));
    m_butTrack.signal_clicked().connect(
      sigc::mem_fun(*this, &IQGradientPanel::on_button_track));
    m_butSave.signal_clicked().connect(
      sigc::mem_fun(*this, &IQGradientPanel::on_button_save));

    m_radStartNode.signal_toggled().connect(
      sigc::mem_fun(*this, &IQGradientPanel::start_mode_action));
    m_radStartCoord.signal_toggled().connect(
      sigc::mem_fun(*this, &IQGradientPanel::start_mode_action));

    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;

    iTop++;
    attach(m_radStartNode,     0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblStartNode,     1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtStartNode,     2, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_radStartCoord,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblStartLon,      1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtStartLon,      2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblStartLat,      3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtStartLat,      4, 5, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_hSep1,           0, 5, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    
    attach(m_lblStopVal,       0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtStopVal,       1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblMinVal,        0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtMinVal,        1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_butTrack,        2, 5, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butClear,        0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_lblOutput,      0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_txtOutput,      0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butSave,        4, 5, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);

    int iEH;
    int iEW;
  
    m_txtStartNode.get_size_request(iEW, iEH);
    m_txtStartNode.set_size_request(60, iEH);
    m_txtStartLon.set_size_request(60, iEH);
    m_txtStartLat.set_size_request(60, iEH);
    m_txtStopVal.set_size_request(40, iEH);
    m_txtMinVal.set_size_request(40, iEH);

    m_txtStopVal.set_text("1");
    m_txtMinVal.set_text("0");

    setSensitivity(m_bNodeMode);
 
}

IQGradientPanel::~IQGradientPanel() {
}


//-----------------------------------------------------------------------------
// on_button_clear
// 
void IQGradientPanel::on_button_clear() {
    printf("IQGradientPanel::on_button_clear()\n");
    m_txtStartNode.set_text("");
    m_txtStartLon.set_text("");
    m_txtStartLat.set_text("");
    m_txtOutput.set_text("");
    // unselect point
    notifyObservers(NOTIFY_CLEAR_MARKERS, NULL);
    m_pOverlay->clear();
    // clear current paths 
}

//-----------------------------------------------------------------------------
// on_button_save
// 
void IQGradientPanel::on_button_save() {
    printf("IQGradientPanel::on_button_save()\n");
}

//-----------------------------------------------------------------------------
// on_button_track
// 
void IQGradientPanel::on_button_track() {
    printf("IQGradientPanel::on_button_track()\n");
    if (m_pSurface != NULL) {
        printf("Creating IcoGradientFinder\n");
        IQGradientFinder *pIGF = new IQGradientFinder(m_pSurface, m_pPI->getValueProvider());
        gridtype iStartNode=-1;

        if (m_bNodeMode) {
            if (strToNum(m_txtStartNode.get_text().c_str(), &iStartNode)) {
            } else {
                printf("Bad Value for start node: [%s]\n", m_txtStartNode.get_text().c_str());
                iStartNode=-1;
            }
        } else {
            // transform coords to node
            double dLon;
            double dLat;
            if (strToNum(m_txtStartLon.get_text().c_str(), &dLon)) {
                if (strToNum(m_txtStartLat.get_text().c_str(), &dLat)) {
                    printf("Finding node for coords\n");
                    iStartNode = m_pSurface->findNode(dLon*M_PI/180.0, dLat*M_PI/180.0);
                    char sNode[64];
                    sprintf(sNode, "%d", iStartNode);
                    m_txtStartNode.set_text(sNode);
                } else {
                    printf("Bad Value for start Lon: [%s]\n", m_txtStartLon.get_text().c_str());
                }
            } else {
                printf("Bad Value for start Lat: [%s]\n", m_txtStartLat.get_text().c_str());
            }
        }
        if (iStartNode >= 0) {

            double dStopVal;
            double dMinVal;
            if (strToNum(m_txtStopVal.get_text().c_str(), &dStopVal)) {
                if (strToNum(m_txtMinVal.get_text().c_str(), &dMinVal)) {
                    printf("Finding gradient\n");
                    int iResult = pIGF->findGradient(iStartNode, dStopVal, dMinVal);
                    if (iResult == 0) {
                        printf("Found path with %zd elements\n", pIGF->getPath().size());
                        m_pOverlay->addData(pIGF->getPath());
                        // PI will notify
                    } else {
                        printf("Failed to find gradient\n");
                    }
                } else {
                    printf("Bad Value for minval: [%s]\n", m_txtMinVal.get_text().c_str());
                }
            } else {
                printf("Bad Value for stopval: [%s]\n", m_txtStopVal.get_text().c_str());
            }
        }
        delete pIGF;
    }
}


//-----------------------------------------------------------------------------
// start_mode_action
// 
void IQGradientPanel::start_mode_action() {
    printf("IQGradientPanel::start_mode_action()\n");
    m_bNodeMode = m_radStartNode.get_active();
    setSensitivity(m_bNodeMode);
    if (!m_bNodeMode) {
        m_txtStartNode.set_text("");
    }
}

//-----------------------------------------------------------------------------
// setSensitivity
// 
void IQGradientPanel::setSensitivity(bool bNodeMode) {
    m_lblStartNode.set_sensitive(bNodeMode);
    m_txtStartNode.set_sensitive(bNodeMode);
    m_lblStartLon.set_sensitive(!bNodeMode);
    m_txtStartLon.set_sensitive(!bNodeMode);
    m_lblStartLat.set_sensitive(!bNodeMode);
    m_txtStartLat.set_sensitive(!bNodeMode);
}
//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQGradientPanel::notify(Observable *pObs, int iType, const void *pCom) {
    m_bNotifyInProgress = true;
    switch (iType) {
    case NOTIFY_SET_SELECTED: {
        SelectData sd = *((SelectData *) pCom);
        char sNodeID[64];
        char sLon[64];
        char sLat[64];
        sprintf(sNodeID, "%d", sd.lNodeID);
        sprintf(sLon, "%.3f", sd.dLon*180/M_PI);
        sprintf(sLat, "%.3f", sd.dLat*180/M_PI);
        m_txtStartNode.set_text(sNodeID);
        m_txtStartLon.set_text(sLon);
        m_txtStartLat.set_text(sLat);
    }
        break;
    case NOTIFY_DATA_LOADED:
    case NOTIFY_NEW_GRID: {
        printf("Have notify with %p\n", pCom);
        m_pSurface = (Surface *)pCom;
        m_pOverlay->clear();
    }
        break;
    }

    m_bNotifyInProgress = false;



}
