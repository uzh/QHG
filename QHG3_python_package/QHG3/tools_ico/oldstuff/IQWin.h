#ifndef __ICOWIN_H__
#define __ICOWIN_H__

#include <gtkmm.h>
#include "IQScene.h"
#include "IQDataPanel.h"
#include "IQQDFPanel.h"
#include "IQLookUpPanel.h"
#include "IQOverlay.h"
#include "IQProjPanel.h"
#include "IQGradientPanel.h"
#include "IQAniPanel.h"
#include "IQEffectPanel.h"
#include "IQImageViewer.h"
#include "Observer.h"

class IQAnimator;
class LineReader;
class SurfaceManager;

const int MAX_LU_PARAMS2 = 5;

class IQWin : public Gtk::Window, public Observer   {
public:
    static const Glib::ustring APP_NAME;

public:
    explicit IQWin(const char *pIcoFile, const char *pDataFile, bool bPreSel);
    virtual ~IQWin();

    // from Observer
    void notify(Observable *pObs, int iType,  const void *pCom);

    void setDataName(char *pData);
    void showHelp();
protected:
    // signal handlers:
    virtual bool press_action(GdkEventKey *e);
    virtual void on_button_quit_clicked();
 
    virtual void ani_event();

    void setDisp(int iDisp);
    void writeStatus(const char *pText, int iMode);

protected:
    IQOverlay m_Overlay;
    SurfaceManager *m_pSurfaceManager;
    ProjInfo *m_pPI;
    IQImageViewer m_IV;
    IQAnimator   *m_pIQAni;
 
    IQScene m_IQScene;
    IQDataPanel     m_tblData;
    IQQDFPanel      m_tblQDF;
    IQLookUpPanel   m_tblLookUp;
    IQProjPanel     m_tblProj;
    IQGradientPanel m_tblGrad;
    IQAniPanel    m_tblAni;
    IQEffectPanel m_tblEff;


    Gtk::HBox  m_HBox; // Win | dialogs
    Gtk::VBox  m_VBox; // notebook | [status | Quit]

 
    Gtk::Statusbar m_stbStatus;
    Gtk::Button m_butQuit;
    Gtk::HBox  m_HBox2; // Win | dialogs
    Gtk::VBox  m_VBox2; // VBox | IQImageViewer


    Gtk::Label m_lblTabData;
    Gtk::Label m_lblTabQDF;
    Gtk::Label m_lblTabLU;
    Gtk::Label m_lblTabProj;
    Gtk::Label m_lblTabGrad;
    Gtk::Label m_lblTabAni;
    Gtk::Label m_lblTabEff;
    Gtk::Notebook m_nbkDialogs;

    Glib::Dispatcher *m_pDisp;
  };




#endif
