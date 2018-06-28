#ifndef _IQQDFPANEL_H__
#define _IQQDFPANEL_H__


#include <gtkmm.h>

#include "Observer.h"
#include "Observable.h"

class LineReader;
class IQAnimator;
class SurfaceManager;

typedef std::pair<std::string, std::string> tstringpair;
typedef std::vector<tstringpair> tfilterdata;

class IQQDFPanel : public Gtk::Table, public Observer, public Observable {
public:
    IQQDFPanel(SurfaceManager *pSurfaceManager, const char *pSurfFile, const char *pDataFile, bool bPreSel);
    virtual ~IQQDFPanel();
    
    void setDisp(int iDisp);
    void enter_pressed();
    bool press_action(GdkEventKey *e);
    void setDataName(char *pData);
    const char *getDataName();
    void setDispDirect(int iDisp);

    // from Observer
    void notify(Observable *pObs, int iType, const void *pCom);

protected:
    SurfaceManager *m_pSurfaceManager;
  
    virtual void on_button_browse_ico_clicked();
    virtual void on_button_load_ico_clicked();
    virtual void on_button_add_ico_clicked();
    virtual void on_button_browse_data_clicked();
    virtual void on_button_load_data_clicked();
    virtual void ds_change_action();
    

    virtual void disp_action();
    virtual void realize_action();
    std::string chooseFile(tfilterdata &vFilterData);
protected:

    Gtk::Label  m_lblIcoFile;
    Gtk::Entry  m_txtIcoFile;
    Gtk::Button m_butIcoBrowse;
    Gtk::Button m_butIcoLoad;
    Gtk::Button m_butIcoAdd;

    Gtk::HSeparator m_hSep1;
    
    Gtk::Label  m_lblDataFile;
    Gtk::Button m_butDataLoad;

    Gtk::HSeparator m_hSep4;
     
    Gtk::RadioButton  m_radPoints; 
    Gtk::RadioButton  m_radLines; 
    Gtk::RadioButton  m_radPlanes; 
    Gtk::HBox  m_HBox3; 
 
    
    char m_sLastDir[512];
    LineReader *m_pLR;
    int m_iOldDisp;

    
    // for a list
    Gtk::ComboBoxText m_lstDSTypes;
   
    

};




#endif
