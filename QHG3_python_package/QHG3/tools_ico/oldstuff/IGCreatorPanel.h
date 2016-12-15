#ifndef _IGCREATORPANEL_H__
#define _IGCREATORPANEL_H__

#include <gtkmm.h>

#include "Observable.h"
#include "Observer.h"

#include "IcoScene.h"

#include "IGTilingPanel.h"

#include "IGCIcoPanel.h"
#include "IGCFlatPanel.h"
#include "IGCEQPanel.h"

class RegionSplitter;

class IGCreatorPanel : public Gtk::Table, public Observable,   public Observer {
public:
    IGCreatorPanel(IcoScene *pIcoScene, IGTilingPanel *m_pIGT);
    virtual ~IGCreatorPanel();
    void setReady(bool bReady);
    void loadSurface(const char *pSurface);

    void notify(Observable *pObs, int iType, const void *pCom);

protected:
    virtual void on_button_browse_clicked();
    virtual void on_button_save_clicked();
    virtual void on_page_switch(GtkNotebookPage *page, guint page_num );


protected:
    IcoScene *m_pIcoScene;
    bool m_bFlat;
    bool m_bReady;
    int m_iStateInfo;
    int m_iStateIco;
    int m_iStateFlat;
    IGTilingPanel *m_pIGT;
    RegionSplitter *m_pRS;

    Gtk::Notebook m_nbkSubDialogs;

    Gtk::Label   m_lblIco;
    IGCIcoPanel  m_tblIco;
    Gtk::Label   m_lblFlat;
    IGCFlatPanel m_tblFlat;
    Gtk::Label   m_lblEQ;
    IGCEQPanel   m_tblEQ;
 
    Gtk::Entry  m_txtSaveFile;
    Gtk::Button m_butBrowse;
    Gtk::Button m_butSave;

    Gtk::CheckButton m_chkSurf;
    Gtk::CheckButton m_chkIGN;
    Gtk::CheckButton m_chkQDF;

};




#endif
