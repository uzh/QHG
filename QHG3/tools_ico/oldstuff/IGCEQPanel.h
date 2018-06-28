#ifndef _IGCEQPANEL_H__
#define _IGCEQPANEL_H__

#include <gtkmm.h>

#include "Observable.h"
#include "SurfaceHandler.h"
#include "IcoScene.h"
#include "Surface.h"
#include "IcoSurface_OGL.h"
#include "EQ_OGL.h"

class RegionSplitter;


class IGCEQPanel : public Gtk::Table, public Observable, public SurfaceHandler {
public:
    IGCEQPanel(IcoScene *pIcoScene);
    virtual ~IGCEQPanel();
    
    int saveSurface(const char *pNameObj, 
                    const char *pNameIGN, 
                    const char *pNameQDF, 
                    RegionSplitter *pRS);
    int loadSurface(const char *pNameSurface);
    void activateSurface();
    void displaySurface() { m_pEQ->display();};
protected:
    /*
    void setRotMode(int iRotMode);
    */
    void setLandMode(bool bLand);
    void setRectMode(bool bRect);

    virtual void on_button_subdiv_clicked();
    /*
    virtual void rot_action(); 
    */
    virtual void land_action();
    virtual void rect_action();
    
    int m_iRotMode;
    
protected:
    IcoScene *m_pIcoScene;

    EQsahedron *m_pEQ;
    EQ_OGL     *m_pOGL;

    Gtk::Table m_WidgetGrid;
    
    Gtk::HSeparator m_hSep1;
    Gtk::HSeparator m_hSep2;
    Gtk::HSeparator m_hSep3;
    
    /*
    // rot section
    Gtk::Label  m_lblRot;  
    Gtk::Entry  *m_ptxtR[4];
    Gtk::Label  *m_plblR[4];  
    */

    Gtk::CheckButton m_chkEqualArea;


    // land section
    Gtk::CheckButton m_chkLand;
    Gtk::Label  m_lblMinLandAlt;
    Gtk::Entry  m_txtMinLandAlt;

    // rect section!!!!
    Gtk::CheckButton m_chkRect;
    Gtk::Label  m_lblLon;  
    Gtk::Entry  m_txtLonMin;
    Gtk::Entry  m_txtLonMax;
    Gtk::Label  m_lblLat;  
    Gtk::Entry  m_txtLatMin;
    Gtk::Entry  m_txtLatMax;
    Gtk::Label  m_lblMin;  
    Gtk::Label  m_lblMax;  
    
    // action section
    Gtk::Button m_butSubDiv;
    Gtk::Label  m_lblSubDivLev;
    Gtk::Entry  m_txtSubDivLev;

    Gtk::HBox  m_HBox3; 


};




#endif
