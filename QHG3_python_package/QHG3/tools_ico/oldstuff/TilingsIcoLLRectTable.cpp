#include <gtkmm.h>

#include "RegionSplitter.h"
#include "LonLatSplitter.h"

#include "TilingsIcoLLRectTable.h"

#include "tiles_sphere_llrect.h"
//-----------------------------------------------------------------------------
// constructor
//   
// 
TilingsIcoLLRectTable::TilingsIcoLLRectTable() 
    : m_pRS(NULL),
      m_lblTimesM(" x "),
      m_lblNumTilesM("NumTiles: ", Gtk::ALIGN_RIGHT),
      m_chkAutoM("auto"),
      m_imgLLRect(Gdk::Pixbuf::create_from_inline(-1, tiles_sphere_llrect_inline)) {



    m_chkAutoM.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsIcoLLRectTable::mode_autoM));


    signal_realize().connect( 
      sigc::mem_fun(*this, &TilingsIcoLLRectTable::realize_action));


    
    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    attach(m_txtMeridW,   1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblTimesM,   2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lblNumTilesM,   1, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtMeridH,   3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_chkAutoM,   3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
       
    attach(m_imgLLRect,   5, 6, 0, 4, iXOpt, iYOpt, 50, iYPad);


    int iEW;
    int iEH;
    m_txtMeridW.get_size_request(iEW, iEH);
    m_txtMeridW.set_size_request(40, iEH);
    m_txtMeridH.set_size_request(40, iEH);


}

//-----------------------------------------------------------------------------
// destructor
//   
// 
TilingsIcoLLRectTable::~TilingsIcoLLRectTable() {
    if (m_pRS != NULL) {
        delete m_pRS;
    }
}

//-----------------------------------------------------------------------------
// mode_autoM
//   
// 
void TilingsIcoLLRectTable::mode_autoM() {
    bool bAuto = m_chkAutoM.get_active();
    m_txtMeridW.set_visible(!bAuto);
    m_txtMeridH.set_visible(true);
    m_lblTimesM.set_visible(!bAuto);
    m_lblNumTilesM.set_visible(bAuto);
}


//-----------------------------------------------------------------------------
// realize_action
//   called when widget is drawn the first time
// 
void TilingsIcoLLRectTable::realize_action() {
    // make sure the correct
    m_chkAutoM.set_active(true);
    mode_autoM();

    m_txtMeridW.set_text("1");
    m_txtMeridH.set_text("1");

}



//-----------------------------------------------------------------------------
// getSplitter
//   
// 
RegionSplitter *TilingsIcoLLRectTable::getSplitter() {
    printf("[TilingsIcoSphereTable::getSplitter]\n");

    // do some stuff
    if (m_pRS != NULL) {
        delete m_pRS;
        m_pRS = NULL;
    }

    int iNY = atoi(m_txtMeridH.get_text().c_str());
    if (iNY > 0) {
        if (m_chkAutoM.get_active()) {
            m_pRS = new LonLatSplitter(iNY, 0.0, false);
        } else {
            int iNX = atoi(m_txtMeridW.get_text().c_str());
            if (iNX > 0) {
                m_pRS = new LonLatSplitter(iNX, iNY, 0);
            }
        }
    }
    return m_pRS;
}
