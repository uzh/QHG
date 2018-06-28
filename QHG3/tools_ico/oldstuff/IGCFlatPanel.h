#ifndef _IGCFLATPANEL_H__
#define _IGCFLATPANEL_H__

#include <gtkmm.h>

#include "Observable.h"
#include "SurfaceHandler.h"
#include "IcoScene.h"
#include "Lattice.h"

class GridProjection;
class RectGridCreator;
//class Lattice;
class Lattice_OGL;
class IcoGridNodes;
class RegionSplitter;


#define MODE_FULL 0
#define MODE_RECT 1
#define MODE_LAND 2
#define MODE_FLAT 3


class IGCFlatPanel : public Gtk::Table, public Observable, public SurfaceHandler {
public:
    IGCFlatPanel(IcoScene *pIcoScene);
    virtual ~IGCFlatPanel();
    
    int saveSurface(const char *pNameSurface, 
                    const char *pNameIGN, 
                    const char *pNameQDF, 
                    RegionSplitter *pRS);
    int loadSurface(const char *pNameSurface);
    void activateSurface();
    void displaySurface() { m_pLattice->display(); };
protected:
    int createGridProjection(double dSqueeze);
    void ptype_action();
    virtual void on_button_create_clicked();
    virtual bool focus_action(GdkEventFocus* event);
    virtual void setMode(int iMode);
    virtual void mode_action();

    void cleanVertList();
protected:

    GridProjection *m_pGP;
    IcoScene *m_pIcoScene;
    Lattice *m_pLattice;
    Lattice_OGL *m_pOGL;
    int m_iGW;
    int m_iGH;
    int m_iMode;
 
    Gtk::Label    m_lblProjType;
    Gtk::ComboBox m_lstProjTypes;
    Gtk::Label    m_lblProjLon;
    Gtk::Entry    m_txtProjLon;
    Gtk::Label    m_lblProjLat;
    Gtk::Entry    m_txtProjLat;
    Gtk::HBox     m_HBox4; 

    Gtk::HSeparator m_hSep2;

    Gtk::Label    m_lblGridSize;
    Gtk::Entry    m_txtGridW;
    Gtk::Label    m_lblTimes1;
    Gtk::Entry    m_txtGridH;

    Gtk::Label    m_lblRealSize;
    Gtk::Entry    m_txtRealW;  
    Gtk::Label    m_lblTimes2; 
    Gtk::Entry    m_txtRealH;   

    Gtk::Label    m_lblOffset;
    Gtk::Entry    m_txtOffsX;
    Gtk::Label    m_lblPlus;
    Gtk::Entry    m_txtOffsY;
    
    Gtk::HSeparator m_hSep3;
 
    Gtk::Label  m_lblConnectivity;  
    Gtk::RadioButton  m_radConn4; 
    Gtk::RadioButton  m_radConn6; 

 
    Gtk::Button m_butCreate;

    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        
        ModelColumns()
        { add(m_colType); add(m_colName); }
        
        Gtk::TreeModelColumn<int> m_colType;
        Gtk::TreeModelColumn<Glib::ustring> m_colName;
    };
    
    ModelColumns m_ProjColumns;
    
    //Child widgets:
    Glib::RefPtr<Gtk::ListStore> m_refTreeModelProj;
    
};




#endif
