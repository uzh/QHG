#include <gtkmm.h>

#include "RegionSplitter.h"
#include "TrivialSplitter.h"

#include "TilingsIcoLandTable.h"

#include "tiles_sphere_land.h"
//-----------------------------------------------------------------------------
// constructor
//   
// 
TilingsIcoLandTable::TilingsIcoLandTable() 
    : m_pRS(NULL),
      m_lblExplanation("In Land mode, only the trivial splitting is currently available"),
      m_imgLand(Gdk::Pixbuf::create_from_inline(-1, tiles_sphere_land_inline)) {



    
    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    attach(m_lblExplanation,   1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
       
    attach(m_imgLand,   3, 4, 0, 2, iXOpt, iYOpt, 50, iYPad);




}

//-----------------------------------------------------------------------------
// destructor
//   
// 
TilingsIcoLandTable::~TilingsIcoLandTable() {
    if (m_pRS != NULL) {
        delete m_pRS;
    }
}



//-----------------------------------------------------------------------------
// getSplitter
//   
// 
RegionSplitter *TilingsIcoLandTable::getSplitter() {
    printf("[TilingsIcoSphereTable::getSplitter]\n");

    // do some stuff
    if (m_pRS != NULL) {
        delete m_pRS;
        m_pRS = NULL;
    }
    
    m_pRS = new TrivialSplitter();
    return m_pRS;
}
