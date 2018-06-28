#ifndef _IGCICOPANEL_H__
#define _IGCICOPANEL_H__

#include <gtkmm.h>

#include "Observable.h"
#include "SurfaceHandler.h"
#include "IcoScene.h"
#include "Surface.h"
#include "IcoSurface_OGL.h"
#include "Ico_OGL.h"


class RegionSplitter;

class IGCIcoPanel : public Gtk::Table, public Observable, public SurfaceHandler {
public:
    IGCIcoPanel(IcoScene *pIcoScene);
    virtual ~IGCIcoPanel();
    
    int saveSurface(const char *pNameObj, 
                    const char *pNameIGN, 
                    const char *pNameQDF, 
                    RegionSplitter *pRS);
    int loadSurface(const char *pNameSurface);
    void activateSurface();
    void displaySurface() { m_pIcosahedron->display();};
    int getMode() { return m_iMode; };
protected:

    virtual void setMode(int iMode);
    virtual void mode_action();
    virtual void on_button_subdiv_clicked();
    
    void enableRectSection(bool bEnable);
    
    void fullSubDiv(int iLevel);
    void landSubDiv(int iLevel);
    void rectSubDiv(int iLevel);
    
protected:
    int m_iMode;
    IcoScene *m_pIcoScene;

    Icosahedron *m_pIcosahedron;
    Ico_OGL     *m_pOGL;

    Gtk::Table m_WidgetGrid;

    Gtk::RadioButton  m_radFull; 
    Gtk::RadioButton  m_radRect; 
    Gtk::RadioButton  m_radLand; 
    
    Gtk::HSeparator m_hSep1;
    Gtk::HSeparator m_hSep2;
    Gtk::HSeparator m_hSep3;
    Gtk::HSeparator m_hSep4;
    
    // rect section
    Gtk::Label  m_lblLon;  
    Gtk::Entry  m_txtLonMin;
    Gtk::Entry  m_txtLonMax;
    Gtk::Entry  m_txtDLon;
    Gtk::Label  m_lblLat;  
    Gtk::Entry  m_txtLatMin;
    Gtk::Entry  m_txtLatMax;
    Gtk::Entry  m_txtDLat;

    Gtk::Label  m_lblMin;  
    Gtk::Label  m_lblMax;  
    Gtk::Label  m_lblStep;

    // land section
    Gtk::Label  m_lblSubLandAlt;
    Gtk::Entry  m_txtSubLandAlt;
    

    // action section
    Gtk::Button m_butSubDiv;
    Gtk::Label  m_lblSubDivLev;
    Gtk::Entry  m_txtSubDivLev;

    Gtk::HBox  m_HBox3; 


};




#endif
