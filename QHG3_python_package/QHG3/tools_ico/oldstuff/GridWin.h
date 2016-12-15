#ifndef __ICOWIN_H__
#define __ICOWIN_H__

#include <gtkmm.h>
#include "IcoScene.h"
#include "IGCreatorPanel.h"
#include "IGTilingPanel.h"
#include "IGConverterPanel.h"
#include "Observer.h"

class GridWin : public Gtk::Window, public Observer   {
public:
    static const Glib::ustring APP_NAME;

public:
    explicit GridWin(const char *pFile);
    virtual ~GridWin();

    // from Observer
    void notify(Observable *pObs, int iType,  const void *pCom);


protected:
    // signal handlers:
    virtual void on_button_quit_clicked();
    virtual bool press_action(GdkEventKey* event);
  
    void writeStatus(const char *pText, int iMode);

protected:
    //
    IcoScene m_IcoScene;
    IGCreatorPanel   m_tblCreator;
    IGTilingPanel    m_tblTiling;
    IGConverterPanel m_tblConverter;

    int m_aiStatus[2];

    // member widgets:
    Gtk::Statusbar m_stbStatus;
 
    Gtk::Notebook m_nbkDialogs;

    Gtk::Label m_lblCreator;
    Gtk::Label m_lblTiling;
    Gtk::Label m_lblConverter;

    Gtk::Button m_butQuit;

    Gtk::VBox m_VBox;
    Gtk::HBox  m_HBox; // Win | dialogs
    Gtk::HBox  m_HBox2; // status | quit
};



#endif
