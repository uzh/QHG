#include <gtkmm.h>
#include "IGTilingPanel.h"
#include "TileDialog.h"

TileDialog::TileDialog(IGTilingPanel *pTP) 
    : Gtk::Dialog("Tiling", true) {
    
    Gtk::Box *pContents = get_vbox();
    pContents->pack_start(*pTP,Gtk::PACK_EXPAND_WIDGET, 0); 
    add_button("Cancel", CLICK_CANCEL);
    add_button("Set", CLICK_SET);
    show_all();
}
 
TileDialog::~TileDialog() {
}
