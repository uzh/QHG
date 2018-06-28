#include <gtkmm.h>

#include "strutils.h"
#include "notification_codes.h"
#include "IQEffectPanel.h"



IQEffectPanel::IQEffectPanel(bool bAltSet, float fAltFactor, bool bLightSet) 
    : m_chkUseAlt("Altitude mode"),
      m_butSetAlt("Set"),
      m_sclAlt(),
      m_chkUseLight("Light"),
      m_bNotifyInProgress(false) {

    // set Adjustment
    Gtk::Adjustment adj(1000*fAltFactor, 0.0, 200, 1, 10);
    m_sclAlt.set_adjustment(adj);
    m_sclAlt.set_value_pos(Gtk::POS_RIGHT);
    // connect signals
    m_butSetAlt.signal_clicked().connect(
      sigc::mem_fun(*this, &IQEffectPanel::on_button_set_alt));

    m_sclAlt.signal_value_changed().connect(
      sigc::mem_fun(*this, &IQEffectPanel::on_alt_value_changed));
    m_sclAlt.signal_change_value().connect(
      sigc::mem_fun(*this, &IQEffectPanel::on_alt_value_changed2));

    // radio button signals
    m_chkUseAlt.signal_clicked().connect( 
      sigc::mem_fun(*this, &IQEffectPanel::use_alt));
    m_chkUseLight.signal_clicked().connect( 
      sigc::mem_fun(*this, &IQEffectPanel::use_light));

    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;

    iTop++;
    attach(m_chkUseAlt,        0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_sclAlt,           0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butSetAlt,        3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    
    attach(m_hSep1,            0, 6, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_chkUseLight,      0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);

    m_chkUseAlt.set_active(bAltSet);
    m_chkUseLight.set_active(bLightSet);
    int iEH;
    int iEW;
  
    m_sclAlt.get_size_request(iEW, iEH);
    m_sclAlt.set_size_request(240, iEH);

}

IQEffectPanel::~IQEffectPanel() {
}

void IQEffectPanel::setSensitivity(bool bMon) {

}

//-----------------------------------------------------------------------------
// on_button_set_alt
// 
void IQEffectPanel::on_button_set_alt() {
    use_alt();
}

//-----------------------------------------------------------------------------
// on_alt_value_changed
// 
void IQEffectPanel::on_alt_value_changed() {
    printf("alt val %f\n", m_sclAlt.get_value());
}

//-----------------------------------------------------------------------------
// use_alt
// 
void IQEffectPanel::use_alt() {
    if (!m_bNotifyInProgress) {
        bool bUse = m_chkUseAlt.get_active();
        AltData ad;
        ad.bUseAlt = bUse;
        ad.fAltFactor = m_sclAlt.get_value();
        printf("use ALT factor %f\n", ad.fAltFactor);
        notifyObservers(NOTIFY_SET_USE_ALT, &ad);
    }
}

//-----------------------------------------------------------------------------
// use_light
// 
void IQEffectPanel::use_light() {
    if (!m_bNotifyInProgress) {
        bool bUse = m_chkUseLight.get_active();
        notifyObservers(NOTIFY_SET_USE_LIGHT, &bUse);
    }
}

bool IQEffectPanel::on_alt_value_changed2(Gtk::ScrollType st, double v) {
    //    printf("New val %f\n", v);
    return true;
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQEffectPanel::notify(Observable *pObs, int iType, const void *pCom) {
    m_bNotifyInProgress = true;
    switch (iType) {
    case NOTIFY_USE_ALT_SET:
        m_chkUseAlt.set_active(*((bool *) pCom));
        break;
    case NOTIFY_USE_LIGHT_SET:
        m_chkUseLight.set_active(*((bool *) pCom));
        break;
    }
    m_bNotifyInProgress = false;
}
