#ifndef __IQGRADIENTPANEL_H__
#define __IQGRADIENTPANEL_H__


#include <gtkmm.h>
#include "Observer.h"
#include "Observable.h"

class Overlay;
class ProjInfo;
class TrivialSplitter;
class Surface;

class IQGradientPanel : public Gtk::Table,  public Observer, public Observable {
public:
    IQGradientPanel(IQOverlay *pOverlay, ProjInfo *pPI, Surface *pSurface);
    virtual ~IQGradientPanel();
    
    void setSensitivity(bool bMode);
protected:
    virtual void on_button_track();
    virtual void on_button_clear();
    virtual void on_button_save();
    virtual void start_mode_action();

    // from Observer
    void notify(Observable *pObs, int iType, const void *pCom);
    void setSurface(Surface *pSurface) {m_pSurface = pSurface;};
protected:
    IQOverlay       *m_pOverlay;
    ProjInfo        *m_pPI;
    Surface         *m_pSurface;
   
    Gtk::RadioButton  m_radStartNode;
    Gtk::RadioButton  m_radStartCoord;
    Gtk::Label        m_lblStartNode;
    Gtk::Entry        m_txtStartNode;
    Gtk::Label        m_lblStartLon;
    Gtk::Entry        m_txtStartLon;
    Gtk::Label        m_lblStartLat;
    Gtk::Entry        m_txtStartLat;
    Gtk::HSeparator   m_hSep1;


    Gtk::Label        m_lblStopVal;
    Gtk::Entry        m_txtStopVal;
    Gtk::Label        m_lblMinVal;
    Gtk::Entry        m_txtMinVal;

    Gtk::Button       m_butTrack;
    Gtk::Button       m_butClear;

    Gtk::Label        m_lblOutput;
    Gtk::Entry        m_txtOutput;
    Gtk::Button       m_butSave;
    
    

    bool m_bNodeMode;
    bool m_bNotifyInProgress;
};

#endif

