
#include <string.h>

#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/main.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/separator.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include "xbv.h"

#include "xbvd.h"

#include "utils.h"
#include "QMapHeader.h"
#include "ValReader.h"
#include "SnapReader.h"
#include "LookUp.h"
#include "LookUpFactory.h"
  

const int DEF_XBVD_W = 500;


//@@@@@@@@@ TODO
// initial dialog:
//  name part + Lookup part
//  namepart: txtName + butLoad
//  LookUpPart: dropdown list filled by means of LookUpFactory,
//              variable number of entries from Paramnames...
//  if a name is selected & type is found, choose a default LU
//     BMAP: BVLookUp(0,1) or UCHARLookUp
//     SNAP: PopLookUp or VegLookUp
//  if unable to load: assume generic bin map, show all data

//-----------------------------------------------------------------------------
// constructor
//
xbvd::xbvd(xbv *pxbvCaller) 
    : Gtk::Dialog("Data", false, false),
      m_lblTitleLookUp("LookUp type", Gtk::ALIGN_RIGHT),
      m_butLUSet("Set"),

      m_VBox0(false,5),
      m_HBox4(true,5),

      m_pxbvCaller(pxbvCaller),
      m_pLU(m_pxbvCaller->getLookUp()),
      m_dLUMin(0),
      m_dLUMax(1),
      m_bResetParams(false) {


    
    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        m_lblParams[i] = new Gtk::Label("", Gtk::ALIGN_RIGHT);
        m_txtParams[i] = new Gtk::Entry();
        m_txtParams[i]->signal_key_press_event().connect(sigc::mem_fun(*this, &xbvd::press_action), false);

        m_hboxParams[i] = new Gtk::HBox(true, 5);
        m_hboxParams[i]->pack_start(*m_lblParams[i], Gtk::PACK_EXPAND_WIDGET);
        m_hboxParams[i]->pack_start(*m_txtParams[i], Gtk::PACK_EXPAND_WIDGET);
    }



    // lookup combobox
    m_refTreeModelLU = Gtk::ListStore::create(m_LUColumns);
    m_cboLU.set_model(m_refTreeModelLU);
    int iL = LookUpFactory::instance()->getNumLookUps();
    for (int i = 0; i < iL; i++) {
        Gtk::TreeModel::Row row = *(m_refTreeModelLU->append());
        
        row[m_LUColumns.m_colType] = i;
        row[m_LUColumns.m_colName] = LookUpFactory::instance()->getLookUpName(i);
    }
    m_cboLU.pack_start(m_LUColumns.m_colName);

    // signals:
    // send all key_press events to our handler (load_action on Return)
    signal_key_press_event().connect(sigc::mem_fun(*this, &xbvd::press_action), false);
    // LookUp changed: show/hide/inputs
    m_cboLU.signal_changed().connect(sigc::mem_fun(*this, &xbvd::lu_change_action), false);


    // button signals
    m_butLUSet.signal_clicked().connect(sigc::mem_fun(*this, &xbvd::set_action) );



    // structure: 
    // HBox1 file load

    // HBox4: LookUp cbo + set
    m_HBox4.pack_start(m_lblTitleLookUp,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_cboLU, Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_butLUSet, Gtk::PACK_EXPAND_WIDGET);
   
    // VBox0: entire dialog
    m_VBox0.pack_start(m_HBox4, Gtk::PACK_SHRINK);
    
    // VBox0: all LookUp Params
    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        m_VBox0.pack_start(*m_hboxParams[i], Gtk::PACK_SHRINK);
    }
    
    get_vbox()->add(m_VBox0);


    show_all_children();

    //    show_extents(false);
    // LookUp Parameters
    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        m_hboxParams[i]->hide();
    }

    Gtk::Requisition req = get_vbox()->size_request();
    resize(DEF_XBVD_W, req.height);

    // fill widgets
    setData();


}

//-----------------------------------------------------------------------------
// destructor
//
xbvd::~xbvd() {

    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        delete m_lblParams[i];
        delete m_txtParams[i];
        delete m_hboxParams[i];
    }


}

//-----------------------------------------------------------------------------
// setData
//   fill widgets with Reader/LookUp data
//
void xbvd::setData() {
 

    setLookUpData();
    
   
 
}

//-----------------------------------------------------------------------------
// setDataFromVR
//   fill widgets with Reader/LookUp data
//
void xbvd::setLookUpData() {
   
    LookUpVals luv = m_pxbvCaller->getLookUpVals();
    if (luv.iType >= 0) {
        m_cboLU.set_active(luv.iType);
        

        char sD[64];
        int iNumParams = LookUpFactory::instance()->getNumParams(luv.iType);
        for (int i = 0; i < iNumParams; i++) {
            m_lblParams[i]->set_text(LookUpFactory::instance()->getParamName(luv.iType, i));
            niceNum(sD, luv.vParamVals.at(i));
                //            sprintf(sD, "%f", luv.vParamVals.at(i));
            m_txtParams[i]->set_text(sD);
            m_hboxParams[i]->show();
        }

        for (int i = iNumParams; i < MAX_LU_PARAMS; i++) {
            m_hboxParams[i]->hide();
        } 

        switch (luv.iType) {
        case LOOKUP_UCHAR:
        case LOOKUP_POP:
        case LOOKUP_THRESH:
        case LOOKUP_ZEBRA:
            m_dLUMin = 0;
            m_dLUMax = luv.vParamVals.at(0);
            break;
        case LOOKUP_BINVIEW:
        case LOOKUP_VEG:
        case LOOKUP_GEO:
        case LOOKUP_SUN:
        case LOOKUP_RAINBOW:
        case LOOKUP_DENSITY:
            m_dLUMin = luv.vParamVals.at(0);
            m_dLUMax = luv.vParamVals.at(1);
            break;
        default:
            break;
        }
            
    } else {
        m_cboLU.set_active(-1);
        for (int i = 0; i < MAX_LU_PARAMS; i++) {
            m_hboxParams[i]->hide();
        } 
    }
}


//-----------------------------------------------------------------------------
// lu_change_action
//   callback for entry-change event: clear all fields
//
void xbvd::lu_change_action() {

    printf("LU changed\n");
    int iIndex = m_cboLU.get_active_row_number();
    if (iIndex >= 0) {
        char sD[64];
        int iNumParams = LookUpFactory::instance()->getNumParams(iIndex);
        for (int i = 0; i < iNumParams; i++) {
            m_lblParams[i]->set_text(LookUpFactory::instance()->getParamName(iIndex, i));
            sprintf(sD, "%f", LookUpFactory::instance()->getParamDefault(iIndex, i));
            m_txtParams[i]->set_text(sD);
            m_hboxParams[i]->show();
        }

        for (int i = iNumParams; i < MAX_LU_PARAMS; i++) {
            m_hboxParams[i]->hide();
        } 

        switch (iIndex) {
        case LOOKUP_UCHAR:
        case LOOKUP_POP:
            sprintf(sD, "%f", m_dLUMax);
            m_txtParams[0]->set_text(sD);
            m_bResetParams = true;
            break;
        case LOOKUP_THRESH:
            sprintf(sD, "%f", (m_dLUMin+m_dLUMax)/2);
            m_txtParams[0]->set_text(sD);
            m_bResetParams = true;
            break;            
        case LOOKUP_ZEBRA:
            sprintf(sD, "%f", 16.0);
            m_txtParams[0]->set_text(sD);
            m_bResetParams = true;
            break;
        case LOOKUP_BINVIEW:
        case LOOKUP_VEG:
        case LOOKUP_GEO:
        case LOOKUP_SUN:
        case LOOKUP_RAINBOW:
        case LOOKUP_DENSITY:
        case LOOKUP_SEGMENT:
            if (m_bResetParams) {
                m_pxbvCaller->getRange(m_dLUMin, m_dLUMax);                
            } 
            sprintf(sD, "%f", m_dLUMin);
            m_txtParams[0]->set_text(sD);
            sprintf(sD, "%f", m_dLUMax);
            m_txtParams[1]->set_text(sD);
            
            m_bResetParams = false;
            break;
        default:
            break;
        }
            
        if (iIndex == LOOKUP_SEGMENT) {
            sprintf(sD, "%f", (m_dLUMax-m_dLUMin)/16);
            m_txtParams[2]->set_text(sD);
            sprintf(sD, "%f", 8.0);
            m_txtParams[3]->set_text(sD);

        } else {
            m_txtParams[2]->set_text("0.0");
        }

    
    }
    Gtk::Requisition req = get_vbox()->size_request();
    resize(DEF_XBVD_W, req.height);
    
}



//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvd::set_action() {
    double adParams[5];
    
    int iIndex = m_cboLU.get_active_row_number();
    if (iIndex >= 0) {
        int iNumParams = LookUpFactory::instance()->getNumParams(iIndex);

        for (int i = 0; i < iNumParams; i++) {
            adParams[i] = atof(m_txtParams[i]->get_text().c_str());
        }

        m_pLU = m_pxbvCaller->setLookUp(iIndex, adParams);
    }

        switch (iIndex) {
        case LOOKUP_UCHAR:
        case LOOKUP_POP:
        case LOOKUP_THRESH:
            m_dLUMin = 0;
            m_dLUMax = adParams[0];
            break;
        case LOOKUP_BINVIEW:
        case LOOKUP_VEG:
        case LOOKUP_GEO:
        case LOOKUP_SUN:
        case LOOKUP_RAINBOW:
        case LOOKUP_DENSITY:
            m_dLUMin = adParams[0];
            m_dLUMax = adParams[1];
            break;
        default:
            break;
        }
    

    /*
    double dMin = atof(m_txtLUMin.get_text().c_str());
    double dMax = atof(m_txtLUMax.get_text().c_str());
    */
    //// m_pxbvCaller->setRange(dMin, dMax);
}


//-----------------------------------------------------------------------------
// press_action
//   on return do load
// 
bool xbvd::press_action(GdkEventKey *e) {
    //    printf("Keyval:%d\n", e->keyval);
    bool bReturn = false;
    if (e->keyval == GDK_Return) {
        set_action();
        bReturn = true;
    }
    return bReturn;
}

