#ifndef __TILE_DIALOG_H__
#define __TILE_DIALOG_H__


#include <gtkmm.h>
#include "IGTilingPanel.h"

#define CLICK_CANCEL 0
#define CLICK_SET    1

class TileDialog: public Gtk::Dialog {
public:
    TileDialog(IGTilingPanel *m_pTP);
    virtual ~TileDialog();
    
    
    
};


#endif

