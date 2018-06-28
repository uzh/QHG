#ifndef __TILINGSRECTTABLE_H__
#define __TILINGSRECTTABLE_H__

#include <gtkmm.h>

#include "Observer.h"
#include "TilingTable.h"

class GridProjection;

class TilingsRectTable : public TilingTable {
public:
    TilingsRectTable();
    virtual ~TilingsRectTable();

    void mode_action();
    virtual RegionSplitter *getSplitter();
    virtual void setObject(int iWhat, void *pData);
    void setGridProjection(GridProjection *pGP) { m_pGP = pGP;};
protected:
    RegionSplitter *m_pRS;
    GridProjection *m_pGP;
    double          m_dH;

    void mode_usr();
    void mode_reg();
    void mode_var();

    Gtk::RadioButton m_radUser;
    Gtk::RadioButton m_radRegular;
    Gtk::RadioButton m_radVariable;
    Gtk::CheckButton m_chkStrict1;
    Gtk::CheckButton m_chkStrict2;

    Gtk::Entry       m_txtUserW;
    Gtk::Entry       m_txtUserH;
    Gtk::Entry       m_txtRegularN;
    Gtk::Entry       m_txtVariableN;
    Gtk::Label       m_lblTimes;
    Gtk::Label       m_lblTiles1;
    Gtk::Label       m_lblTiles2;
   
    Gtk::Image       m_imgIrr;
    Gtk::Image       m_imgReg;
    Gtk::Image       m_imgVar;
    Gtk::HSeparator  m_hSep1;
    Gtk::HSeparator  m_hSep2;
};

#endif

