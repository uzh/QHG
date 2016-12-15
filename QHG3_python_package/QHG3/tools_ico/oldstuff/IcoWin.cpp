#include "trackball.h"

#include "icoutil.h"
#include "IcoWin.h"

const Glib::ustring IcoWin::APP_NAME = "IcoViewer";

#define DEG2RAD(x) ((x*M_PI)/180)

//-----------------------------------------------------------------------------
// constructor
//
IcoWin::IcoWin(const char *pFile, bool enable_anim)
    : m_HBox(false, 0), 
      m_VBox(false, 0), 
      m_IcoScene(pFile, enable_anim), 
      m_butQuit("Quit"),
      m_butSubDiv("Subdivide"),
      m_butMerge("Merge"),
      m_lblSubDivLev("SubDiv Level", Gtk::ALIGN_LEFT),
      m_lblLon("Longitude", Gtk::ALIGN_LEFT),
      m_lblLat("Latitude", Gtk::ALIGN_LEFT),
      m_lblMin("Min", Gtk::ALIGN_LEFT),
      m_lblMax("Max", Gtk::ALIGN_LEFT),
      m_lblStep("Step", Gtk::ALIGN_LEFT),
      m_lblVarDivLev("Vardiv Level", Gtk::ALIGN_LEFT),
      m_butVarSub("Varsubdiv"),
      m_chkStrict("Strict"),
      m_chkShowBox("Show Box"),
      m_chkPreSelect("PreSelect"),
      m_butLocate("Locate"),
      m_butClearFace("Clear"),
      m_lblFile("File", Gtk::ALIGN_LEFT),
      m_butLoad("Load"),
      m_butSave("Save")  {
  
    //
    // Top-level window.
    //

    set_title(APP_NAME);

    // Get automatically redrawn if any of their children changed allocation.
    set_reallocate_redraws(true);

    add(m_HBox);

    //
    // Application scene.
    //

    m_IcoScene.set_size_request(400, 400);

    m_HBox.pack_start(m_IcoScene);


    //
    // Simple quit button.
    //

    m_butQuit.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_quit_clicked));
    m_butSubDiv.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_subdiv_clicked));
    m_butMerge.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_merge_clicked));
    m_butVarSub.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_varsub_clicked));
    m_butSave.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_save_clicked));
    m_butLoad.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_load_clicked));
    m_butLocate.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_locate_clicked));
    m_butClearFace.signal_clicked().connect(
      sigc::mem_fun(*this, &IcoWin::on_button_clearface_clicked));
    
    m_chkShowBox.signal_toggled().connect( sigc::mem_fun(*this, &IcoWin::on_box_action));
    m_chkStrict.signal_toggled().connect( sigc::mem_fun(*this, &IcoWin::on_strict_action));

    signal_key_press_event().connect(sigc::mem_fun(*this, &IcoWin::press_action));
   
    m_IcoScene.signal_button_press_event().connect(sigc::mem_fun(m_IcoScene, &IcoScene::doClick));


    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    
    m_WidgetGrid.attach(m_lblSubDivLev, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtSubDivLev, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_butSubDiv,    1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_butMerge,     2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    m_WidgetGrid.attach(m_hSep1,        0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    
    m_WidgetGrid.attach(m_lblLon,       1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_lblLat,       2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_lblMin,       0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtLonMin,    1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtLatMin,    2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_lblMax,       0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtLonMax,    1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtLatMax,    2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_lblStep,      0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtDLon,      1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtDLat,      2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_lblVarDivLev, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtVarDivLev, 1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_butVarSub,    2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_chkShowBox,   0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_chkStrict,    1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_chkPreSelect, 2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    m_WidgetGrid.attach(m_hSep2,        0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_butLocate,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtLon,       1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtLat,       2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_butClearFace, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_hSep3,        0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    m_WidgetGrid.attach(m_lblFile,      0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_txtFile,      1, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_butSave,      1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_butLoad,      2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    m_WidgetGrid.attach(m_hSep4,        0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    iTop++;



    //    m_WidgetGrid.attach(m_butQuit,    5, 6, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    m_WidgetGrid.attach(m_butQuit,    2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);

    m_VBox.pack_start(m_WidgetGrid, Gtk::PACK_SHRINK, 0);
    //        m_VBox.pack_start(m_butQuit, Gtk::PACK_SHRINK, 0);
    //        m_VBox.pack_start(hSep2, Gtk::PACK_EXPAND_WIDGET, 0);

    m_HBox.pack_start(m_VBox, Gtk::PACK_SHRINK, 0);

    m_txtSubDivLev.set_text("0");
    m_txtLonMin.set_text("-5");
    m_txtLonMax.set_text("50");
    m_txtDLon.set_text("0.1");
    m_txtLatMin.set_text("20");
    m_txtLatMax.set_text("55");
    m_txtDLat.set_text("0.1");
    m_txtVarDivLev.set_text("6");
    m_txtFile.set_text("");

    m_chkStrict.set_active(true);
    m_chkShowBox.set_active(false);
    //    m_chkUseCUDA.set_sensitive(false);
    //
    // Show window.
    //

    show_all();
  }

//-----------------------------------------------------------------------------
// destructor
//
IcoWin::~IcoWin() {
}


//-----------------------------------------------------------------------------
// on_button_quit_clicked
//
void IcoWin::on_button_quit_clicked() {
    Gtk::Main::quit();
}

//-----------------------------------------------------------------------------
// on_button_subdiv_clicked
//
void IcoWin::on_button_subdiv_clicked() {
    int iLevel = atoi(m_txtSubDivLev.get_text().c_str());
    m_IcoScene.subdivide(iLevel);
   
}

//-----------------------------------------------------------------------------
// 
//
void IcoWin::on_button_clearface_clicked() {
    m_IcoScene.clearFace();
}

//-----------------------------------------------------------------------------
// on_button_locate_clicked
//
void IcoWin::on_button_locate_clicked() {
    double dLon = atof(m_txtLon.get_text().c_str());
    double dLat = atof(m_txtLat.get_text().c_str());
    dLon *= M_PI/180;
    dLat *= M_PI/180;
    gridtype iNode;
    m_IcoScene.locate(dLon, dLat, &iNode);
}

//-----------------------------------------------------------------------------
// on_button_merge_clicked
//
void IcoWin::on_button_merge_clicked() {
    int iLevel = atoi(m_txtSubDivLev.get_text().c_str());
    m_IcoScene.merge(iLevel);
}

//-----------------------------------------------------------------------------
// on_button_varsub_clicked
//
void IcoWin::on_button_varsub_clicked() {
    tbox tBox(DEG2RAD(atof(m_txtLonMin.get_text().c_str())),
              DEG2RAD(atof(m_txtLonMax.get_text().c_str())),
              DEG2RAD(atof(m_txtLatMin.get_text().c_str())),
              DEG2RAD(atof(m_txtLatMax.get_text().c_str())));
    double dDLon   = DEG2RAD(atof(m_txtDLon.get_text().c_str()));
    double dDLat   = DEG2RAD(atof(m_txtDLat.get_text().c_str()));
    int iLevel = atoi(m_txtVarDivLev.get_text().c_str());
    bool bPreSel = m_chkPreSelect.get_active();
    m_IcoScene.varsubdiv(bPreSel, iLevel, tBox, dDLon, dDLat);

    //    m_IcoScene.save("oinkoink.dat"); 

    m_IcoScene.centerPoint(DEG2RAD((atof(m_txtLonMin.get_text().c_str())+atof(m_txtLonMax.get_text().c_str()))/2), 
                           DEG2RAD((atof(m_txtLatMin.get_text().c_str())+atof(m_txtLatMax.get_text().c_str()))/2));
}

//-----------------------------------------------------------------------------
// on_butIoton_load_clicked
//
void IcoWin::on_button_load_clicked() {
    m_IcoScene.load(m_txtFile.get_text().c_str()); 
    m_IcoScene.invalidate();
}

//-----------------------------------------------------------------------------
// on_button_save_clicked
//
void IcoWin::on_button_save_clicked() {
    m_IcoScene.save(m_txtFile.get_text().c_str()); 
}
//-----------------------------------------------------------------------------
// on_box_action
//
void IcoWin::on_box_action() {
    bool bBox = m_chkShowBox.get_active();
    m_IcoScene.set_box(bBox);
}
//-----------------------------------------------------------------------------
// on_strict_action
//
void IcoWin::on_strict_action() {
    m_IcoScene.setStrict(m_chkStrict.get_active());
}




//-----------------------------------------------------------------------------
// press_action
//
bool IcoWin::press_action(GdkEventKey* event) {
    
    int iModifiers = gtk_accelerator_get_default_mod_mask ();
    switch (event->keyval) {
    case GDK_w:
        m_IcoScene.toggle_wire();
        break;
    case GDK_c:
        m_IcoScene.toggle_color();
        break;
    case GDK_i:
        m_IcoScene.reset();
        break;
    case GDK_Up:
        m_IcoScene.subdivide();
        break;
    case GDK_Down:
        m_IcoScene.merge();
        break;
    case GDK_Escape:
        Gtk::Main::quit();
        break;
    case GDK_Alt_L:    
    case GDK_Alt_R:    
        m_IcoScene.setDisplayLevel(-1);
        break;
    case GDK_0:    
    case GDK_KP_0:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(0);
        } else {
            m_IcoScene.setDisplayLevel(0);
        }
        break;
    case GDK_1:
    case GDK_KP_1:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(1);
        } else {
         m_IcoScene.setDisplayLevel(1);
        }
        break;
    case GDK_2:
    case GDK_KP_2:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(2);
        } else {
         m_IcoScene.setDisplayLevel(2);
        }
        break;
    case GDK_3:
    case GDK_KP_3:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(3);
        } else {
         m_IcoScene.setDisplayLevel(3);
        }
        break;
    case GDK_4:
    case GDK_KP_4:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(4);
        } else {
         m_IcoScene.setDisplayLevel(4);
        }
        break;
    case GDK_5:
    case GDK_KP_5:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(5);
        } else {
         m_IcoScene.setDisplayLevel(5);
        }
        break;
    case GDK_6:
    case GDK_KP_6:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(6);
        } else {
         m_IcoScene.setDisplayLevel(6);
        }
        break;
    case GDK_7:
    case GDK_KP_7:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(7);
        } else {
         m_IcoScene.setDisplayLevel(7);
        }
        break;
    case GDK_8:
    case GDK_KP_8:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(8);
        } else {
         m_IcoScene.setDisplayLevel(8);
        }
        break;
    case GDK_9:
    case GDK_KP_9:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(9);
        } else {
         m_IcoScene.setDisplayLevel(9);
        }
        break;
    case GDK_a:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(10);
        } else {
         m_IcoScene.setDisplayLevel(10);
        }
        break;
    case GDK_b:
        if ((event->state&iModifiers) == GDK_CONTROL_MASK) {
            m_IcoScene.toggleDisplayLevel(11);
        } else {
         m_IcoScene.setDisplayLevel(11);
        }
        break;
    default:
        return true;
    }
    
    m_IcoScene.invalidate();
    
    return true;
}


