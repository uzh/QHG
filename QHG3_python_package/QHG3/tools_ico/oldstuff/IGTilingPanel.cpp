#include <gtkmm.h>
#include <vector>

#include "Observable.h"
#include "icoutil.h"
#include "IGTilingPanel.h"
#include "TilingsRectTable.h"
#include "TilingsIcoSphereTable.h"
#include "TilingsIcoLLRectTable.h"
#include "TilingsIcoLandTable.h"
#include "RegionSplitter.h"

#define IDX_ICO_FULL 0
#define IDX_ICO_RECT 1
#define IDX_ICO_LAND 2
#define IDX_RECT     3

//-----------------------------------------------------------------------------
// constructor
//   
// 
IGTilingPanel::IGTilingPanel()
    : m_iPage(0) {

    TilingsIcoSphereTable *pi = new TilingsIcoSphereTable();
    m_vtTilingTables.push_back(pi);

    TilingsIcoLLRectTable *pilr = new TilingsIcoLLRectTable();
    m_vtTilingTables.push_back(pilr);
    
    TilingsIcoLandTable *pil = new TilingsIcoLandTable();
    m_vtTilingTables.push_back(pil);
    
    TilingsRectTable *ptr = new TilingsRectTable();
    m_vtTilingTables.push_back(ptr);

    for (unsigned int i = 0; i < m_vtTilingTables.size(); i++) {
        m_vtTilingTables[i]->addObserver(this);
        pack_start(*(m_vtTilingTables[i]), Gtk::PACK_EXPAND_WIDGET, 0);
    }

    signal_realize().connect( 
      sigc::mem_fun(*this, &IGTilingPanel::realize_action));

}

//-----------------------------------------------------------------------------
// destructor
//   
// 
IGTilingPanel::~IGTilingPanel() {
    for (unsigned int i = 0; i < m_vtTilingTables.size(); i++) {
        delete m_vtTilingTables[i];
    }
}

//-----------------------------------------------------------------------------
// hideAll
//   
// 
void IGTilingPanel::hideAll() {
    for (unsigned int i = 0; i < m_vtTilingTables.size(); i++) {
        m_vtTilingTables[i]->set_visible(false);
    }
}


//-----------------------------------------------------------------------------
// switchToTable
//   
// 
void IGTilingPanel::switchToTable(bool bFlat, int iType) {
    printf("got switch to %s, mode %x\n", bFlat?"flat":"ico", iType);
    hideAll();
    if (bFlat) {
        // Rectangular regions, Tilesplitter from tiling
        m_iPage = IDX_RECT;
    } else {
        if (iType == MODE_ICO_FULL) {
            m_iPage = IDX_ICO_FULL;
            // LonLatRegion, LonLatSplitter
        } else if (iType == MODE_ICO_RECT) {
            m_iPage = IDX_ICO_RECT;
            // LonLatRegion, LonLatSplitter
            // 
        } else if (iType == MODE_ICO_LAND) {
            m_iPage = IDX_ICO_LAND;
            // currently no tiling for land
            // FullRegion, TrivialSPlitter
        }
    }
    printf("PAGE IS NOW %d\n", m_iPage);
    m_vtTilingTables[m_iPage]->set_visible(true);

}

//-----------------------------------------------------------------------------
// realize_action
//   called when widget is drawn the first time
// 
void IGTilingPanel::realize_action() {
    // make sure the correct 
    hideAll();
    m_vtTilingTables[m_iPage]->set_visible(true);

}

//-----------------------------------------------------------------------------
// getSplitter
//   called when widget is drawn the first time
// 
RegionSplitter *IGTilingPanel::getSplitter() {
    printf("[IGTilingPanel::getSplitter]\n");
    return m_vtTilingTables[m_iPage]->getSplitter();
}




//-----------------------------------------------------------------------------
// notify
//   fromObserver
// 
void  IGTilingPanel::notify(Observable *pObs, int iType,  const void *pCom) {
    switch (iType) {
    case NOTIFY_TILE_SPLITTER:
        printf("notif TILE_SPLITTER - passing on\n");
        notifyObservers(iType, pCom);
        break;
    case NOTIFY_NEW_GP:
        printf("NOTIFY_NEW_GP setting Object\n");
        m_vtTilingTables[IDX_RECT]->setObject(NOTIFY_NEW_GP, (void *)pCom);
        break;
    case NOTIFY_NEW_H:
        printf("NOTIFY_NEW_GP setting Object\n");
        m_vtTilingTables[IDX_RECT]->setObject(NOTIFY_NEW_H, (void *)pCom);
        break;
    }
    
}
