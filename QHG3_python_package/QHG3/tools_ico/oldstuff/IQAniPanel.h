#ifndef __IQANIPANEL_H__
#define __IQANIPANEL_H__


#include <gtkmm.h>

class IQAnimator;
class LineReader;

class IQAniPanel : public Gtk::Table {
public:
    IQAniPanel();
    virtual ~IQAniPanel();
    
    void setAni(IQAnimator *pIQAni) {printf("Getting ani %p\n", pIQAni); m_pIQAni = pIQAni;};
    void setSensitivity(bool bMode);
    void enableAll(bool bEnable);
    void setStop() { on_button_stop_clicked();};
protected:
    virtual void on_button_browse_file_clicked();
    virtual void on_button_browse_dir_clicked();
    virtual void on_button_browse_target_clicked();
    virtual void on_button_go_clicked();
    virtual void on_button_stop_clicked();
    virtual void mode_action();

protected:
    Gtk::RadioButton  m_radMonitor;
    Gtk::Label        m_lblTargetName;
    Gtk::Entry        m_txtTargetName;
    Gtk::Button       m_butTargetBrowse;
    Gtk::Label        m_lblFileMask;
    Gtk::Entry        m_txtFileMask;
    Gtk::CheckButton  m_chkUseQuat;
    Gtk::CheckButton  m_chkUseColor;
    Gtk::HSeparator   m_hSep1;

    Gtk::RadioButton  m_radFile;
    Gtk::HSeparator   m_hSep2;


    Gtk::Label        m_lblInterval;
    Gtk::Entry        m_txtInterval;
    
    Gtk::Button       m_butGo;
    Gtk::Button       m_butStop;
    

    IQAnimator   *m_pIQAni;
    LineReader   *m_pLR;
    bool          m_bMode;

};

#endif

