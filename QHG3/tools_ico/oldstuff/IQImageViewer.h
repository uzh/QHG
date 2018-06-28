#ifndef __IQIMAGEVIEWER_H__
#define __IQIMAGEVIEWER_H__

#include <gtkmm.h>
#include "Observer.h"

class GridProjection;
class ProjInfo;

#define DEF_IMG_W 400
#define DEF_IMG_H 200

class IQImageViewer : public Gtk::Frame, public Observer {
//class IQImageViewer : public Gtk::Alignment {
public:
    IQImageViewer(ProjInfo *pPI, int iW, int iH);
    virtual ~IQImageViewer();

    void createImage(bool bInterp, bool bUseQuat, int iPW, int iPH);
    void updateImage();
    int getWidth() { return m_iW;};
    int getHeight() { return m_iH;};
    void setInterp(bool bInterp) { m_bInterp = bInterp;};
    void setUseQuat(bool bUseQuat) { m_bUseQuat = bUseQuat;};

    void notify(Observable *pObs, int iType, const void *pCom);
    void live_action();
    void setFresh(bool bFresh=true) { m_bFresh=bFresh;}
protected:
    
    ProjInfo       *m_pPI;
    int m_iW;
    int m_iH;
    int m_iPW;
    int m_iPH;

    GridProjection   *m_pGP;
    guint8           *m_pImgData;
    Gtk::Image        m_imgPreView;
    Gtk::CheckButton  m_chkLive;
    Gtk::VBox         m_VBox;
    bool m_bInterp;
    bool m_bUseQuat;
    bool m_bLive;
    bool m_bFresh;

    int createGridProjection(bool bPreview);

};


#endif
