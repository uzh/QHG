#ifndef __TILINGSICOSPHERETABLE_H__
#define __TILINGSICOSPHERETABLE_H__

#include <gtkmm.h>

#include "Observable.h"
#include "TilingTable.h"

class RegionSplitter;


class TilingsIcoSphereTable : public TilingTable {
public:
    TilingsIcoSphereTable();
    virtual ~TilingsIcoSphereTable();

    void mode_action();
    void realize_action();
    virtual RegionSplitter *getSplitter();
    virtual void setObject(int iWhat, void *pData) {};

protected:
    RegionSplitter *m_pRS;
    void mode_merid();
    void mode_caps();
    void mode_autoM();
    void mode_autoC();

    Gtk::RadioButton m_radMeridian;
    Gtk::RadioButton m_radCaps;

    Gtk::Entry       m_txtMeridW;
    Gtk::Entry       m_txtMeridH;
    Gtk::Entry       m_txtCapsW;
    Gtk::Entry       m_txtCapsH;
    Gtk::Entry       m_txtCapsS;
    Gtk::Label       m_lblDummy1;
    Gtk::Label       m_lblDummy2;
    Gtk::Label       m_lblTimesM;
    Gtk::Label       m_lblTimesC;
    Gtk::Label       m_lblCapSize;
    Gtk::Label       m_lblNumTilesM;
    Gtk::Label       m_lblNumTilesC;
    Gtk::CheckButton m_chkAutoM;
    Gtk::CheckButton m_chkAutoC;
    Gtk::Image       m_img;
    Gtk::Image       m_imgMerid;
    Gtk::Image       m_imgCaps;
    Gtk::HSeparator  m_hSep1;
};

#endif

