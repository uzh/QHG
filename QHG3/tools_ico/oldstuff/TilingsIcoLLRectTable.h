#ifndef __TILINGSICOLLRECTTABLE_H__
#define __TILINGSICOSPHERETABLE_H__

#include <gtkmm.h>

#include "Observable.h"
#include "TilingTable.h"

class RegionSplitter;


class TilingsIcoLLRectTable : public TilingTable {
public:
    TilingsIcoLLRectTable();
    virtual ~TilingsIcoLLRectTable();

    void mode_autoM();
    void realize_action();
    virtual RegionSplitter *getSplitter();
    virtual void setObject(int iWhat, void *pData) {};

protected:
    RegionSplitter *m_pRS;

    Gtk::Entry       m_txtMeridW;
    Gtk::Entry       m_txtMeridH;
    Gtk::Label       m_lblTimesM;
    Gtk::Label       m_lblNumTilesM;
    Gtk::CheckButton m_chkAutoM;
    Gtk::Image       m_imgLLRect;

};

#endif

