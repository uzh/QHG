
#include "trackball.h"
#include "IcoScene.h"
#include "icoutil.h"
#include "IGCreatorPanel.h"
#include "IGTilingPanel.h"
#include "IGConverterPanel.h"
#include "GridWin.h"

const Glib::ustring GridWin::APP_NAME = "IcoGrid";

#ifndef DEG2RAD
#define DEG2RAD(x) ((x*M_PI)/180)
#endif

//-----------------------------------------------------------------------------
// constructor
//
GridWin::GridWin(const char *pFile)
    : m_IcoScene(pFile),
      m_tblCreator(&m_IcoScene, &m_tblTiling),
      m_tblConverter(&m_IcoScene),
      m_lblCreator("Create Grid", Gtk::ALIGN_LEFT),
      m_lblTiling("Tiling", Gtk::ALIGN_LEFT),
      m_lblConverter("Convert files", Gtk::ALIGN_LEFT),
      m_butQuit("Quit"),
      m_VBox(false, 0),
      m_HBox(false, 0)
      
{
    m_tblConverter.addObserver(this);    
    m_tblCreator.addObserver(this);
    m_tblCreator.addObserver(&m_tblTiling);
    m_tblTiling.addObserver(&m_tblCreator);
 
    memset(m_aiStatus, 0, 2*sizeof(int));
    
    printf("Received file [%s]\n", pFile);
    //
    // Top-level window.
    //

    set_title(APP_NAME);

    // Get automatically redrawn if any of their children changed allocation.
    set_reallocate_redraws(true);

    //
    // Application scene.
    //
    m_IcoScene.set_size_request(400, 400);

    m_HBox.pack_start(m_IcoScene);

  

    
    
    

    //
    // Simple quit button.
    //
    m_butQuit.signal_clicked().connect(
      sigc::mem_fun(*this, &GridWin::on_button_quit_clicked));

    signal_key_press_event().connect(sigc::mem_fun(*this, &GridWin::press_action));

    // layout
    m_HBox.pack_start(m_nbkDialogs, Gtk::PACK_SHRINK, 0);

    m_nbkDialogs.append_page(m_tblCreator,   m_lblCreator);
    //    m_nbkDialogs.append_page(m_tblTiling,    m_lblTiling);
    m_nbkDialogs.append_page(m_tblConverter, m_lblConverter);
    m_tblConverter.set_sensitive(false);


    m_HBox2.pack_start(m_stbStatus,  Gtk::PACK_EXPAND_WIDGET, 0);
    m_HBox2.pack_start(m_butQuit,  Gtk::PACK_SHRINK, 0);

    m_VBox.pack_start(m_HBox, Gtk::PACK_EXPAND_WIDGET, 0);
    m_VBox.pack_start(m_HBox2, Gtk::PACK_SHRINK, 0);

    add(m_VBox);

    //
    // Show window.
    //

    show_all();
    m_tblCreator.setReady(true);
  }

//-----------------------------------------------------------------------------
// destructor
//
GridWin::~GridWin() {
    m_tblCreator.setReady(false);

}


//-----------------------------------------------------------------------------
//  on_button_quit_clicked
//
void GridWin::on_button_quit_clicked() {
    Gtk::Main::quit();
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
// 
void GridWin::notify(Observable *pObs, int iType, const void *pCom) {
    switch (iType) {
    case NOTIFY_LOAD: {
        printf("[GridWin::notify] received file to load: [%s]\n",(char *)pCom); 
        m_tblCreator.loadSurface((char *)pCom);
        m_tblConverter.set_sensitive(false);
        break;
    }
    case NOTIFY_SWITCH: {
        int iStateInfo = *(int*)pCom;
        printf("[GridWin::notify] received switch inf %x)\n",iStateInfo); 
        bool bFlat = ((iStateInfo & STATE_FLAT) != 0);
        printf("[GridWin::notify] flat is %s\n", bFlat?"true":"false"); 
        int iMode  = 0;
        if (bFlat) {
            iMode = iStateInfo & MASK_FLAT_LINK;
        } else {
            iMode = iStateInfo & MASK_ICO_MODE;
        }
        m_tblTiling.switchToTable(bFlat, iMode);
        
        if (m_aiStatus[bFlat] == 2) {
            m_tblConverter.set_sensitive(true);
        } else {
            m_tblConverter.set_sensitive(false);
        }
        break;
    }
    case NOTIFY_CREATED: {
        const long lIndex = (const long)pCom;
        printf("got CREATED for %ld\n", lIndex);
        m_aiStatus[lIndex] = 1;
        m_tblConverter.set_sensitive(false);
        break;
    }
    case NOTIFY_TILED: {
        const long lIndex = (const long)pCom;
        printf("got TILED for %ld\n", lIndex);
        if (m_aiStatus[lIndex] == 1) {
            m_aiStatus[lIndex] = 2;
            m_tblConverter.set_sensitive(true);
        }
        break;
    }
    }
}

//-----------------------------------------------------------------------------
//  press_action
//
bool GridWin::press_action(GdkEventKey* event) {
    printf("press\n");
    //    int iModifiers = gtk_accelerator_get_default_mod_mask ();
    switch (event->keyval) {
    case GDK_w:
        m_IcoScene.toggle_wire();
        break;
    case GDK_c:
        m_IcoScene.toggle_color();
        break;
    case GDK_0:
        m_IcoScene.reset();
        break;
    case GDK_Escape:
        Gtk::Main::quit();
        break;
    }    

    m_IcoScene.invalidate();

    return true;
}

//-----------------------------------------------------------------------------
// writeStatus
//   write something to the status line (iMode != 0: red text)
// 
void GridWin::writeStatus(const char *pText, int iMode) {
    GdkColor	_cgtk_color;
    GtkWidget*	_cgtk_label;
    if (iMode == 0) {
        gdk_color_parse("Black", &_cgtk_color);
    } else {
        gdk_color_parse("Red", &_cgtk_color);
    }
    _cgtk_label = m_stbStatus.gobj()->label;
    m_stbStatus.push(pText);
    gtk_widget_modify_fg(GTK_WIDGET(_cgtk_label), GTK_STATE_NORMAL, &_cgtk_color);

}
