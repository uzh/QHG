#ifndef __IQPROJPANEL_H__
#define __IQPROJPANEL_H__

#include <gtkmm.h>
#include <string>
#include "Observer.h"

class GridProjection;
class ProjInfo;
class IQImageViewer;

class IQProjPanel : public Gtk::Table, public Observer {
public:
    IQProjPanel(ProjInfo *pPI, IQImageViewer *pIV);
    virtual ~IQProjPanel();

    void apply_action();

protected:

    int createGridProjection(bool bPreview);
    std::string chooseFile();
    void save_action();
    void ptype_action();
    void interp_action();
    void usequat_action();

    ProjInfo  *m_pPI;
    GridProjection *m_pGP;
    IQImageViewer *m_pIV;
    
    Gtk::Label    m_lblProjType;
    Gtk::ComboBox m_lstProjTypes;
    Gtk::Label    m_lblProjLon;
    Gtk::Entry    m_txtProjLon;
    Gtk::Label    m_lblProjLat;
    Gtk::Entry    m_txtProjLat;
    Gtk::HBox  m_HBox4; 

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
    
    Gtk::CheckButton  m_chkInterp;
    Gtk::CheckButton  m_chkUseQuat;
    Gtk::Button   m_butApply;
    Gtk::Button   m_butSave;
    Gtk::HBox  m_HBox3; 

    int m_iGW;
    int m_iGH;
    int m_iPW;
    int m_iPH;
    bool m_bStartSettings;

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
   
    void notify(Observable *pObs, int iType, const void *pCom);
};



#endif

