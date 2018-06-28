#include <gtkmm.h>

#include "Observable.h"
#include "GridProjection.h"
#include "RegionSplitter.h"
#include "RectSplitter.h"

#include "TilingsRectTable.h"
// the followin h-files contain bitmap data created by
//   gdk-pixbuf-csource -raw -name=myimage_inline myimage.png  
// this data can be read with Gdk::Pixbuf::create_from_inline()
#include "tiles_rect_irreg.h" 
#include "tiles_rect_reg.h" 
#include "tiles_rect_var.h"

//-----------------------------------------------------------------------------
// constructor
//   
// 
TilingsRectTable::TilingsRectTable() 
    : m_pRS(NULL),
      m_pGP(NULL),
      m_dH(sqrt(3)/2),
      m_radUser("User defined"),
      m_radRegular("Regular"),
      m_radVariable("Variable"),
      m_chkStrict1("Strict"),
      m_chkStrict2("Strict"),
      m_lblTimes(" x "),
      m_lblTiles1(" Tiles"),
      m_lblTiles2(" Tiles"),
      m_imgIrr(Gdk::Pixbuf::create_from_inline(-1, tiles_rect_irreg_inline)),
      m_imgReg(Gdk::Pixbuf::create_from_inline(-1, tiles_rect_reg_inline)),
      m_imgVar(Gdk::Pixbuf::create_from_inline(-1, tiles_rect_var_inline)) {

    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp2;
    m_radUser.set_group(gDisp2);
    m_radRegular.set_group(gDisp2);
    m_radVariable.set_group(gDisp2);

    m_radRegular.set_active(true);

    m_chkStrict1.set_active(true);
    m_chkStrict2.set_active(true);


    m_radUser.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsRectTable::mode_action));
    m_radRegular.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsRectTable::mode_action));
    m_radVariable.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsRectTable::mode_action));
    m_chkStrict2.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsRectTable::mode_action));
    m_chkStrict1.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsRectTable::mode_action));


    
    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    attach(m_radUser,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtUserW,   1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTimes,   2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtUserH,   3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_chkStrict1, 2, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_hSep1,      0, 5, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_radRegular,  0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtRegularN, 1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTiles1,   2, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_chkStrict2, 2, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_hSep2,      0, 5, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_radVariable,  0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtVariableN, 1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTiles2,   2, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
       
    attach(m_imgReg,   5, 6, 0, 7, iXOpt, iYOpt, 50, iYPad);
    attach(m_imgIrr,   5, 6, 0, 7, iXOpt, iYOpt, 50, iYPad);
    attach(m_imgVar,   5, 6, 0, 7, iXOpt, iYOpt, 50, iYPad);

    m_imgIrr.set_visible(false);
    m_imgReg.set_visible(true);
    m_imgVar.set_visible(false);
    mode_reg();

    int iEW;
    int iEH;
    m_txtUserW.get_size_request(iEW, iEH);
    m_txtUserW.set_size_request(40, iEH);
    m_txtUserH.set_size_request(40, iEH);
    m_txtRegularN.set_size_request(40, iEH);
    m_txtVariableN.set_size_request(40, iEH);

    m_txtRegularN.set_text("1");
}

//-----------------------------------------------------------------------------
// destructor
//   
// 
TilingsRectTable::~TilingsRectTable() {

}

//-----------------------------------------------------------------------------
// setObject
//   
// 
void TilingsRectTable::setObject(int iWhat, void *pData) {
    if (iWhat == NOTIFY_NEW_GP) {
        printf("[TilingsRectTable::setObject] setting GP %p\n", pData);

        m_pGP = (GridProjection *)pData;
    } else if (iWhat == NOTIFY_NEW_H) {
        m_dH = *(double*) pData;
    }

}

//-----------------------------------------------------------------------------
// mode_action
//   
// 
void TilingsRectTable::mode_action() {
    if (m_radUser.get_active()) {
        bool bStrict= m_chkStrict1.get_active();
        m_imgIrr.set_visible(!bStrict);
        m_imgReg.set_visible(bStrict);
        m_imgVar.set_visible(false);
        mode_usr();
    } else if (m_radRegular.get_active()) {
        bool bStrict= m_chkStrict2.get_active();
        m_imgIrr.set_visible(!bStrict);
        m_imgReg.set_visible(bStrict);
        m_imgVar.set_visible(false);
        mode_reg();
    } else {
        m_imgIrr.set_visible(false);
        m_imgReg.set_visible(false);
        m_imgVar.set_visible(true);

        mode_var();
    }
}

//-----------------------------------------------------------------------------
// mode_action
//   
// 
void TilingsRectTable::mode_var() {

    m_chkStrict1.set_sensitive(false);
    m_chkStrict2.set_sensitive(false);

    m_txtUserH.set_sensitive(false);
    m_txtUserW.set_sensitive(false);
    m_txtRegularN.set_sensitive(false);
    m_txtVariableN.set_sensitive(true);
}

//-----------------------------------------------------------------------------
// mode_action
//   
// 
void TilingsRectTable::mode_reg() {

    m_chkStrict1.set_sensitive(false);
    m_chkStrict2.set_sensitive(true);

    m_txtUserH.set_sensitive(false);
    m_txtUserW.set_sensitive(false);
    m_txtRegularN.set_sensitive(true);
    m_txtVariableN.set_sensitive(false);
}

//-----------------------------------------------------------------------------
// mode_action
//   
// 
void TilingsRectTable::mode_usr() {

    m_chkStrict1.set_sensitive(true);
    m_chkStrict2.set_sensitive(false);

    m_txtUserH.set_sensitive(true);
    m_txtUserW.set_sensitive(true);
    m_txtRegularN.set_sensitive(false);
    m_txtVariableN.set_sensitive(false);
}


//-----------------------------------------------------------------------------
// getSplitter
//   
// 
RegionSplitter *TilingsRectTable::getSplitter() {
    if (m_pRS != NULL) {
        delete m_pRS;
        m_pRS = NULL;
    }
    bool bStrict=false;
    bool bGrid=true;
    int iNX = 0;
    int iNY = 0;
    if (m_radUser.get_active()) {
        bStrict = m_chkStrict1.get_active();
        iNX = atoi(m_txtUserW.get_text().c_str());
        iNY = atoi(m_txtUserH.get_text().c_str());
        m_pRS = new RectSplitter(m_pGP, m_dH, iNX, iNY, bStrict);
    } else  if (m_radRegular.get_active())  {
        bStrict = m_chkStrict2.get_active();
        iNX = atoi(m_txtRegularN.get_text().c_str());
        m_pRS = new RectSplitter(m_pGP, m_dH, iNX, bGrid, bStrict);
            
    } else if (m_radVariable.get_active()) {
        bStrict = false;
        bGrid = false;
        iNX = atoi(m_txtRegularN.get_text().c_str());
        m_pRS = new RectSplitter(m_pGP, m_dH, iNX, bGrid, bStrict);

        // currently no var
    } else {
        // should not happen
    }

    return m_pRS;
}
