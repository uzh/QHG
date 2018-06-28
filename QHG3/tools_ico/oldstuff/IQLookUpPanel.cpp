#include <gtkmm.h>

#include "utils.h"
#include "LookUp.h"
#include "LookUpFactory.h"
#include "IQLookUpPanel.h"
#include "ProjInfo.h"
#include "notification_codes.h"

IQLookUpPanel::IQLookUpPanel(ProjInfo *pPI) 
    : m_pPI(pPI),
      m_lblLUType("LookUp",  Gtk::ALIGN_LEFT),
      m_butLUSet("Set"),
      m_butRangeReset("Reload Range"),
      m_chkPinValues("Pin values"),
      m_dMinVal(dNegInf),
      m_dMaxVal(dPosInf),
      m_bInit(true) {

    m_pPI->getRange(&m_dMinVal, &m_dMaxVal);
    printf("IQLookUpPanel has minmax(%f,%f)\n", m_dMinVal, m_dMaxVal);
    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        m_lblLUParams[i] = new Gtk::Label("", Gtk::ALIGN_LEFT);
        m_txtLUParams[i] = new Gtk::Entry(); 
        m_txtLUParams[i]->signal_key_press_event().connect(
          sigc::mem_fun(*this, &IQLookUpPanel::press_action), false);
        m_txtLUParamsReal[i] = new Gtk::Entry(); 
        m_txtLUParamsReal[i]->set_sensitive(false);
    }

    m_pPI->addObserver(this);

    signal_key_press_event().connect(sigc::mem_fun(*this, &IQLookUpPanel::press_action), false);
  
    m_lstLUTypes.signal_changed().connect(sigc::mem_fun(*this, &IQLookUpPanel::lu_change_action), false);
    m_butLUSet.signal_clicked().connect(sigc::mem_fun(*this, &IQLookUpPanel::set_action) );
    m_butRangeReset.signal_clicked().connect(sigc::mem_fun(*this, &IQLookUpPanel::reset_action) );
    //   m_chkPinValues.signal_clicked().connect(sigc::mem_fun(*this, &IQLookUpPanel::pinval_action) );
    m_chkPinValues.set_active(false);

    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    attach(m_lblLUType,     0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lstLUTypes,    0, 1, iTop, iTop+1, iXOpt, iYOpt, 5, iYPad);
    attach(m_butLUSet,      1, 3, iTop, iTop+1, iXOpt, iYOpt, 5, iYPad);
    attach(m_butRangeReset, 3, 4, iTop, iTop+1, iXOpt, iYOpt, 5, iYPad);

    attach(m_chkPinValues,  3, 4, iTop+1, iTop+2, iXOpt, iYOpt, 5, iYPad);

    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        iTop++;
        attach(*(m_lblLUParams[i]), 0, 1, iTop, iTop+1, iXOpt, iYOpt, 5, iYPad);
        attach(*(m_txtLUParams[i]), 1, 2, iTop, iTop+1, iXOpt, iYOpt, 5, iYPad);
        attach(*(m_txtLUParamsReal[i]), 2, 3, iTop, iTop+1, iXOpt, iYOpt, 5, iYPad);
    }

    // fill lookup list
    char sTemp[256];
    m_refTreeModelLU = Gtk::ListStore::create(m_LUColumns);
    m_lstLUTypes.set_model(m_refTreeModelLU);
    for (int i = 0; i < LookUpFactory::instance()->getNumLookUps(); i++) {
        Gtk::TreeModel::Row row = *(m_refTreeModelLU->append());
        strcpy(sTemp,  LookUpFactory::instance()->getLookUpName(i));
        char *p = strrchr(sTemp, ' ');
        if (p != 0) {
            *p = '\0';
        }
        row[m_LUColumns.m_colType] = i;
        row[m_LUColumns.m_colName] = sTemp;
    }
    m_lstLUTypes.pack_start(m_LUColumns.m_colName);
    m_lstLUTypes.set_active(LOOKUP_RAINBOW);

    
    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        int iEW;
        int iEH;
        m_txtLUParams[i]->get_size_request(iEW, iEH);
        m_txtLUParams[i]->set_size_request(80, iEH);
        m_txtLUParamsReal[i]->set_size_request(80, iEH);
    }
    
    //    set_action();
    //   lu_change_action();

    printf("Finished IQLookUpPanel\n");
}

//-----------------------------------------------------------------------------
// destructor
//
IQLookUpPanel::~IQLookUpPanel() {
    for (int i = 0; i < MAX_LU_PARAMS; i++) {
        delete m_lblLUParams[i];
        delete m_txtLUParams[i];
        delete m_txtLUParamsReal[i];
    }
    LookUpFactory::instance()->free();
}

//-----------------------------------------------------------------------------
// lu_change_action
//   callback for entry-change event
//
void IQLookUpPanel::lu_change_action() {

    printf("LU changed; min %f, max %f\n", m_dMinVal, m_dMaxVal);
    int iIndex = m_lstLUTypes.get_active_row_number();
    if (iIndex >= 0) {
        char sD[64];
        int iNumParams = LookUpFactory::instance()->getNumParams(iIndex);
        for (int i = 0; i < iNumParams; i++) {
            m_lblLUParams[i]->set_text(LookUpFactory::instance()->getParamName(iIndex, i));
            sprintf(sD, "%f", LookUpFactory::instance()->getParamDefault(iIndex, i));
            m_txtLUParams[i]->set_text(sD);
            m_lblLUParams[i]->show();
            m_txtLUParams[i]->show();
            m_txtLUParamsReal[i]->show();
        }

        for (int i = iNumParams; i < MAX_LU_PARAMS; i++) {
            m_lblLUParams[i]->hide();
            m_txtLUParams[i]->hide();
            m_txtLUParamsReal[i]->hide();
        } 
        
        switch (iIndex) {
        case LOOKUP_UCHAR:
        case LOOKUP_POP:
            sprintf(sD, "%f", m_dMaxVal);
            m_txtLUParamsReal[0]->set_text(sD);
            break;
        case LOOKUP_THRESH:
            sprintf(sD, "%f", (m_dMinVal+m_dMaxVal)/2);
            m_txtLUParamsReal[0]->set_text(sD);
            break;            
        case LOOKUP_ZEBRA:
            sprintf(sD, "%f", 16.0);
            m_txtLUParamsReal[0]->set_text(sD);
            break;
        case LOOKUP_BINVIEW:
        case LOOKUP_VEG:
        case LOOKUP_GEO:
        case LOOKUP_SUN:
        case LOOKUP_RAINBOW:
        case LOOKUP_DENSITY:
        case LOOKUP_SEGMENT:
            sprintf(sD, "%f", m_dMinVal);
            m_txtLUParamsReal[0]->set_text(sD);
            sprintf(sD, "%f", m_dMaxVal);
            m_txtLUParamsReal[1]->set_text(sD);
            
            break;
        default:
            break;
        }
            
        if (iIndex == LOOKUP_SEGMENT) {
            sprintf(sD, "%f", (m_dMaxVal-m_dMinVal)/16);
            m_txtLUParamsReal[2]->set_text(sD);
            sprintf(sD, "%f", 8.0);
            m_txtLUParamsReal[3]->set_text(sD);

        } else {
            m_txtLUParamsReal[2]->set_text("0.0");
        }

        if ((!m_chkPinValues.get_active()) || m_bInit) {
            for (int i = 0; i < iNumParams; i++) {
                m_txtLUParams[i]->set_text(m_txtLUParamsReal[i]->get_text());
            }
            m_bInit = false;
            set_action();
        }
    }
}

void IQLookUpPanel::setRange(double dMin, double dMax) {
    m_dMinVal = dMin;
    m_dMaxVal = dMax;
    printf("Min: %f, max %f\n", m_dMinVal, m_dMaxVal);
    lu_change_action();
}

//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//
void IQLookUpPanel::set_action() {
    
    m_iLUType = m_lstLUTypes.get_active_row_number();
    if (m_iLUType >= 0) {
        int iNumParams = LookUpFactory::instance()->getNumParams(m_iLUType);

        for (int i = 0; i < iNumParams; i++) {
            m_adLUParams[i] = atof(m_txtLUParams[i]->get_text().c_str());
        }

        m_pPI->setLU(m_iLUType, iNumParams, m_adLUParams);

    }
}


//-----------------------------------------------------------------------------
// reset_action
//   callback for button event ("Reset")
//   TODO: set new LookUp Params
void IQLookUpPanel::reset_action() {
    m_pPI->getRange(&m_dMinVal, &m_dMaxVal);
    lu_change_action();
    
}



//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQLookUpPanel::notify(Observable *pObs, int iType, const void *pCom) {
    //    printf("[IQLookUpPanel::notify] received notificvation [%d] from %p\n", iCom , pObs);
    if (pObs == m_pPI) {
        long iCom = (long) pCom;
        switch (iCom) {
        case NOTIFY_DATA_LOADED: 
            m_pPI->getLURange(&m_dMinVal, &m_dMaxVal);
            lu_change_action();
            break;
        default:
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// press_action
//   on return do load
// 
bool IQLookUpPanel::press_action(GdkEventKey *e) {
    if (e->keyval == GDK_Return) {
        enter_pressed();
    } 
    return false;
}

//-----------------------------------------------------------------------------
// enter_pressed
//   on return do load
// 
void IQLookUpPanel::enter_pressed() {
    bool bHit = false;
    for (int i =0; !bHit && (i < MAX_LU_PARAMS); i++) {
        if (m_txtLUParams[i]->has_focus()) {
            bHit = true;
        }
    }
    
    if (bHit) {
        set_action();
    }
}
