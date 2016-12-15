#include <gtkmm.h>

#include "RegionSplitter.h"
#include "LonLatSplitter.h"

#include "TilingsIcoSphereTable.h"

// these header files contain the data from the png images created like this
//   gdk-pixbuf-csource --raw --name=oinkk_inline oink.png > oink.h

#include "tiles_sphere_merid.h"
#include "tiles_sphere_caps.h"
//-----------------------------------------------------------------------------
// constructor
//   
// 
TilingsIcoSphereTable::TilingsIcoSphereTable() 
    : m_pRS(NULL),
      m_radMeridian("Meridian"),
      m_radCaps("Caps"),
      m_lblDummy1("     "),
      m_lblDummy2("           "),
      m_lblTimesM(" x "),
      m_lblTimesC(" x "),
      m_lblCapSize("Cap Size: "),
      m_lblNumTilesM("NumTiles: ", Gtk::ALIGN_RIGHT),
      m_lblNumTilesC("NumTiles: 2 + ", Gtk::ALIGN_RIGHT),
      m_chkAutoM("auto"),
      m_chkAutoC("auto"),
      m_imgMerid(Gdk::Pixbuf::create_from_inline(-1, tiles_sphere_merid_inline)),
      m_imgCaps(Gdk::Pixbuf::create_from_inline(-1, tiles_sphere_caps_inline)) {

    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp2;
    m_radMeridian.set_group(gDisp2);
    m_radCaps.set_group(gDisp2);


    m_radMeridian.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsIcoSphereTable::mode_action));
    m_radCaps.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsIcoSphereTable::mode_action));
    m_chkAutoM.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsIcoSphereTable::mode_autoM));
    m_chkAutoC.signal_toggled().connect( 
      sigc::mem_fun(*this, &TilingsIcoSphereTable::mode_autoC));


    signal_realize().connect( 
      sigc::mem_fun(*this, &TilingsIcoSphereTable::realize_action));


    
    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    attach(m_radMeridian,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtMeridW,   1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTimesM,   2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblNumTilesM,   1, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtMeridH,   3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_chkAutoM,   3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_hSep1,      0, 5, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_radCaps,  0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtCapsW,   1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTimesC,   2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblDummy2, 1, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblNumTilesC,   1, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtCapsH,   3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_lblCapSize, 2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblDummy1, 3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtCapsS, 3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_chkAutoC,   3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
       
    attach(m_imgMerid,   5, 6, 0, 7, iXOpt, iYOpt, 50, iYPad);
    attach(m_imgCaps,   5, 6, 0, 7, iXOpt, iYOpt, 50, iYPad);


    m_imgCaps.set_visible(false);
    m_imgMerid.set_visible(true);

    mode_merid();

    int iEW;
    int iEH;
    m_txtMeridW.get_size_request(iEW, iEH);
    m_txtMeridW.set_size_request(40, iEH);
    m_txtMeridH.set_size_request(40, iEH);
    m_txtCapsW.set_size_request(40, iEH);
    m_txtCapsH.set_size_request(40, iEH);
    m_txtCapsS.set_size_request(40, iEH);

    m_txtCapsS.get_size_request(iEW, iEH);
    m_lblDummy1.set_size_request(iEW, 30+iEH+2*iYPad);

    int iEW2;
    int iEH2;
    m_txtCapsW.get_size_request(iEW, iEH);
    m_lblCapSize.get_size_request(iEW2, iEH2);
    m_lblDummy2.set_size_request(50+iEW+iEW2+iXPad, iEH);
    
}

//-----------------------------------------------------------------------------
// destructor
//   
// 
TilingsIcoSphereTable::~TilingsIcoSphereTable() {
    if (m_pRS != NULL) {
        delete m_pRS;
    }
}

//-----------------------------------------------------------------------------
// mode_action
//   
// 
void TilingsIcoSphereTable::mode_action() {
    if (m_radMeridian.get_active()) {
        m_imgMerid.set_visible(true);
        m_imgCaps.set_visible(false);
        mode_merid();
    } else {
        m_imgMerid.set_visible(false);
        m_imgCaps.set_visible(true);

        mode_caps();
    }
}

//-----------------------------------------------------------------------------
// mode_merid
//   
// 
void TilingsIcoSphereTable::mode_merid() {
    m_txtMeridW.set_sensitive(true);
    m_txtMeridH.set_sensitive(true);
    m_chkAutoM.set_sensitive(true);
    m_chkAutoC.set_sensitive(false);
    m_txtCapsW.set_sensitive(false);
    m_txtCapsH.set_sensitive(false);
    m_txtCapsS.set_sensitive(false);
}


//-----------------------------------------------------------------------------
// mode_caps
//   
// 
void TilingsIcoSphereTable::mode_caps() {
    m_txtMeridW.set_sensitive(false);
    m_txtMeridH.set_sensitive(false);
    m_chkAutoM.set_sensitive(false);
    m_chkAutoC.set_sensitive(true);
    m_txtCapsW.set_sensitive(true);
    m_txtCapsH.set_sensitive(true);
    m_txtCapsS.set_sensitive(true);
}

//-----------------------------------------------------------------------------
// mode_autoM
//   
// 
void TilingsIcoSphereTable::mode_autoM() {
    bool bAuto = m_chkAutoM.get_active();
    m_txtMeridW.set_visible(!bAuto);
    m_txtMeridH.set_visible(true);
    m_lblTimesM.set_visible(!bAuto);
    m_lblNumTilesM.set_visible(bAuto);
}

//-----------------------------------------------------------------------------
// mode_autoM
//   
// 
void TilingsIcoSphereTable::mode_autoC() {
    bool bAuto = m_chkAutoC.get_active();
    m_txtCapsW.set_visible(!bAuto);
    m_txtCapsH.set_visible(true);
    m_txtCapsS.set_visible(!bAuto);
    m_lblCapSize.set_visible(!bAuto);
    m_lblTimesC.set_visible(!bAuto);
    m_lblNumTilesC.set_visible(bAuto);
}

//-----------------------------------------------------------------------------
// realize_action
//   called when widget is drawn the first time
// 
void TilingsIcoSphereTable::realize_action() {
    // make sure the correct
    m_chkAutoM.set_active(true);
    m_chkAutoC.set_active(true);

    mode_merid();
    mode_autoM();
    mode_autoC();

    m_txtMeridW.set_text("1");
    m_txtMeridH.set_text("1");
    m_txtCapsW.set_text("1");
    m_txtCapsH.set_text("1");
    m_txtCapsS.set_text("10");

}



//-----------------------------------------------------------------------------
// getSplitter
//   
// 
RegionSplitter *TilingsIcoSphereTable::getSplitter() {
    printf("[TilingsIcoSphereTable::getSplitter]\n");

    // do some stuff
    if (m_pRS != NULL) {
        delete m_pRS;
        m_pRS = NULL;
    }

    if (m_radMeridian.get_active()) {
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
    } else {
        // cap
        int iNY = atoi(m_txtCapsH.get_text().c_str());
        printf("NY:%d\n", iNY);
        if (iNY > 0) {
            if (m_chkAutoC.get_active()) {
                m_pRS = new LonLatSplitter(iNY, -1.0, false);

            } else {
                int iNX = atoi(m_txtCapsW.get_text().c_str());
        printf("NX:%d\n", iNX);
                double dCS = 0;
                const char *pCapS = m_txtCapsS.get_text().c_str();
        printf("CapS:%s\n", pCapS);
                if (strcmp(pCapS, "a") == 0) {
                    dCS = -1;
                } else {
                    dCS = atof(pCapS);
                }
                if ((iNX > 0) && (dCS > 0) && (dCS < 30)) {
                    m_pRS = new LonLatSplitter(iNX, iNY, dCS);
                }
            }
        }
    }
    return m_pRS;
}
