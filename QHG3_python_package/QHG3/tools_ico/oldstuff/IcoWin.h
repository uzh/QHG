#ifndef __ICOWIN_H__
#define __ICOWIN_H__

#include <gtkmm.h>
#include "IcoScene.h"

class IcoWin : public Gtk::Window   {
public:
    static const Glib::ustring APP_NAME;

public:
    explicit IcoWin(const char *pFile, bool enable_anim = true);
    virtual ~IcoWin();

protected:
    // signal handlers:
    virtual void on_button_quit_clicked();
    virtual void on_button_subdiv_clicked();
    virtual void on_button_merge_clicked();
    virtual void on_button_varsub_clicked();
    virtual void on_button_load_clicked();
    virtual void on_button_save_clicked();
    virtual void on_button_locate_clicked();
    virtual void on_button_clearface_clicked();
    virtual bool press_action(GdkEventKey* event);
    virtual void on_box_action();
    virtual void on_strict_action();
   


protected:
    // member widgets:
    Gtk::HBox m_HBox;
    Gtk::VBox m_VBox;
    IcoScene m_IcoScene;
    Gtk::Button m_butQuit;
    Gtk::Button m_butSubDiv;
    Gtk::Button m_butMerge;
    Gtk::Label  m_lblSubDivLev;
    Gtk::Entry  m_txtSubDivLev;
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
    Gtk::Label  m_lblVarDivLev;
    Gtk::Entry  m_txtVarDivLev;
    Gtk::Button m_butVarSub;

    Gtk::Table m_WidgetGrid;
    Gtk::CheckButton  m_chkStrict;
    Gtk::CheckButton  m_chkShowBox;
    Gtk::CheckButton  m_chkPreSelect;

    Gtk::HSeparator m_hSep1;
    Gtk::HSeparator m_hSep2;
    Gtk::HSeparator m_hSep3;
    Gtk::HSeparator m_hSep4;

    Gtk::Button m_butLocate;
    Gtk::Button m_butClearFace;
    Gtk::Entry  m_txtLon;
    Gtk::Entry  m_txtLat;

    Gtk::Label  m_lblFile;  
    Gtk::Entry  m_txtFile;
    Gtk::Button m_butLoad;
    Gtk::Button m_butSave;

  };



#endif
