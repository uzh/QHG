#ifndef __IQEFFECTPANEL_H__
#define __IQEFFECTPANEL_H__


#include <gtkmm.h>
#include "Observer.h"
#include "Observable.h"


class IQEffectPanel : public Gtk::Table,  public Observer, public Observable {
public:
    IQEffectPanel(bool bAltSet, float fAltFactor, bool bLightSet);
    virtual ~IQEffectPanel();
    
    void setSensitivity(bool bMode);
protected:
    virtual void on_button_set_alt();
    virtual void on_alt_value_changed();
    virtual bool on_alt_value_changed2(Gtk::ScrollType, double);
    virtual void use_alt();
    virtual void use_light();

    // from Observer
    void notify(Observable *pObs, int iType, const void *pCom);

protected:
    Gtk::CheckButton  m_chkUseAlt;
    Gtk::Button       m_butSetAlt;
    Gtk::HScale        m_sclAlt;

    Gtk::HSeparator   m_hSep1;

    Gtk::CheckButton  m_chkUseLight;

    bool m_bNotifyInProgress;
};

#endif

