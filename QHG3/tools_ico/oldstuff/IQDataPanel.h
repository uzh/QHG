#ifndef _IQDATAPANEL_H__
#define _IQDATAPANEL_H__


#include <gtkmm.h>

#include "Observable.h"

class LineReader;
class IQAnimator;
class SurfaceManager;

typedef std::pair<std::string, std::string> tstringpair;
typedef std::vector<tstringpair> tfilterdata;

class IQDataPanel : public Gtk::Table, public Observable {
public:
    IQDataPanel(SurfaceManager *pSurfaceManager, const char *pSurfFile, const char *pDataFile, bool bPreSel);
    virtual ~IQDataPanel();
    
    void setDisp(int iDisp);
    void enter_pressed();
    bool press_action(GdkEventKey *e);
    void setDataName(char *pData);
    const char *getDataName();
    void setDispDirect(int iDisp);

protected:
    SurfaceManager *m_pSurfaceManager;
  
    virtual void on_button_browse_ico_clicked();
    virtual void on_button_load_ico_clicked();
    virtual void on_button_browse_data_clicked();
    virtual void on_button_load_data_clicked();
   

    virtual void disp_action();
    virtual void realize_action();
    std::string chooseFile(tfilterdata &vFilterData);
protected:

    Gtk::Label  m_lblIcoFile;
    Gtk::Entry  m_txtIcoFile;
    Gtk::Button m_butIcoBrowse;
    Gtk::Button m_butIcoLoad;

    Gtk::HSeparator m_hSep1;
    
    Gtk::Label  m_lblDataFile;
    Gtk::Entry  m_txtDataFile;
    Gtk::Button m_butDataBrowse;
    Gtk::Button m_butDataLoad;

    Gtk::HSeparator m_hSep4;
     
    Gtk::RadioButton  m_radPoints; 
    Gtk::RadioButton  m_radLines; 
    Gtk::RadioButton  m_radPlanes; 
    Gtk::HBox  m_HBox3; 
 
    
    char m_sLastDir[512];
    LineReader *m_pLR;
    int m_iOldDisp;
};




#endif
