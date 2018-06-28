#include <string>

#include "LineReader.h"
#include "LookUpFactory.h"
#include "IQWin.h"
#include "IQScene.h"
#include "notification_codes.h"
#include "init.h"
#include "interquat.h"
#include "IQAnimator.h"
#include "ProjInfo.h"
#include "SurfaceManager.h"

const Glib::ustring IQWin::APP_NAME = "IQViewer";


IQWin::IQWin(const char *pIcoFile, const char *pDataFile, bool bPreSel)
    : m_pSurfaceManager(SurfaceManager::createInstance(pIcoFile, pDataFile, &m_Overlay, bPreSel)),
      m_pPI(new ProjInfo(&m_Overlay)),
      m_IV(m_pPI, DEF_IMG_W, DEF_IMG_H),
      m_pIQAni(NULL),
      //      m_pLR(NULL),
      m_IQScene(m_pSurfaceManager, m_pPI),
      m_tblData(m_pSurfaceManager, pIcoFile, pDataFile, bPreSel),
      m_tblQDF(m_pSurfaceManager, pIcoFile, pDataFile, bPreSel),
      m_tblLookUp(m_pPI),
      m_tblProj(m_pPI, &m_IV),
      m_tblGrad(&m_Overlay, m_pPI,m_pSurfaceManager->getSurface()),
      m_tblAni(),
      m_tblEff(INIT_ALT, INIT_ALT_FACTOR, INIT_LIGHT),
  
      m_HBox(false, 0), 
      m_VBox(false, 0), 

      m_butQuit("Quit"),
      m_lblTabData("Data", Gtk::ALIGN_LEFT),
      m_lblTabQDF("QDF", Gtk::ALIGN_LEFT),
      m_lblTabLU("Colors", Gtk::ALIGN_LEFT),
      m_lblTabProj("Projection", Gtk::ALIGN_LEFT),
      m_lblTabGrad("Gradient", Gtk::ALIGN_LEFT),
      m_lblTabAni("Animation", Gtk::ALIGN_LEFT),
      m_lblTabEff("Effects", Gtk::ALIGN_LEFT)  {
    

    //Must be set in ProjInfo
    //    double r01[] = {0, 1};
    //    m_pPI->setLU(LOOKUP_RAINBOW, 2, r01);

    printf("Overlay issssssss %p\n", &m_Overlay);
    printf("Surface issssssss %p\n", m_pSurfaceManager->getSurface());
    //    m_pSurfaceManager->setOverlay(&m_Overlay);
    //   m_tblGrad.setSurface(m_pSurfaceManager->getSurface());

    m_IQScene.getModel().addObserver(this);
    m_tblData.addObserver(this);
    m_tblData.addObserver(&m_IQScene.getModel());
    m_tblQDF.addObserver(this);
    m_tblQDF.addObserver(&m_IQScene.getModel());
    m_tblGrad.addObserver(&m_IQScene.getModel());
    m_tblGrad.addObserver(this);
    m_pSurfaceManager->addObserver(&m_tblGrad);
    m_pSurfaceManager->addObserver(&m_tblQDF);
    m_pSurfaceManager->addObserver(this);
    m_Overlay.addObserver(&m_IQScene.getModel());
    m_IQScene.getModel().addObserver(&m_tblGrad);
    m_Overlay.addObserver(&m_IV);

    m_tblEff.addObserver(&m_IQScene.getModel());
    m_IQScene.getModel().addObserver(&m_IV);
    m_IQScene.getModel().addObserver(&m_tblEff);

    m_pSurfaceManager->addObserver(&m_IQScene.getModel());
    //
    // Top-level window.
    //

    set_title(APP_NAME);

    // Get automatically redrawn if any of their children changed allocation.
    set_reallocate_redraws(true);

    //
    // Application scene.
    //
    m_IQScene.set_size_request(400, 400);


    m_HBox.pack_start(m_IQScene);

  
    // preparing thread communication
    m_pDisp = new Glib::Dispatcher();
    m_pDisp->connect(sigc::mem_fun(*this, &IQWin::ani_event));

    m_pIQAni = new IQAnimator(&m_IQScene, m_pDisp);
    printf("Setting ani %p\n", m_pIQAni);
    m_tblAni.setAni(m_pIQAni);

    // check keyboard input for commands
    signal_key_press_event().connect(
                                     sigc::mem_fun(*this, &IQWin::press_action)/*, false*/);

    //
    // Simple quit button.
    //
    m_butQuit.signal_clicked().connect(
      sigc::mem_fun(*this, &IQWin::on_button_quit_clicked));
  

    /*** later
    
    m_chkShowBox.signal_toggled().connect( sigc::mem_fun(*this, &IcoWin::on_box_action));
    m_chkStrict.signal_toggled().connect( sigc::mem_fun(*this, &IcoWin::on_strict_action));

    signal_key_press_event().connect(sigc::mem_fun(*this, &IcoWin::press_action));
   
    m_IcoScene.signal_button_press_event().connect(sigc::mem_fun(m_IcoScene, &IcoScene::doClick));
    */

    // layout
    m_nbkDialogs.append_page(m_tblData,   m_lblTabData);
    m_nbkDialogs.append_page(m_tblQDF,    m_lblTabQDF);
    m_nbkDialogs.append_page(m_tblLookUp, m_lblTabLU);
    m_nbkDialogs.append_page(m_tblProj,   m_lblTabProj);
    m_nbkDialogs.append_page(m_tblGrad,   m_lblTabGrad);
    m_nbkDialogs.append_page(m_tblAni,    m_lblTabAni);
    m_nbkDialogs.append_page(m_tblEff,    m_lblTabEff);

    m_HBox2.pack_start(m_stbStatus,  Gtk::PACK_EXPAND_WIDGET, 0);
    m_HBox2.pack_start(m_butQuit,  Gtk::PACK_SHRINK, 0);

    //@@    m_HBox.pack_start(m_nbkDialogs, Gtk::PACK_SHRINK, 0);
    m_VBox2.pack_start(m_nbkDialogs, Gtk::PACK_SHRINK, 0);
    Gtk::Button bbb("gasd");
    //    m_VBox2.pack_start(bbb, Gtk::PACK_EXPAND_WIDGET, 0);
    m_VBox2.pack_start(m_IV, Gtk::PACK_SHRINK, 0);

    m_HBox.pack_start(m_VBox2, Gtk::PACK_SHRINK, 0);

    m_VBox.pack_start(m_HBox, Gtk::PACK_EXPAND_WIDGET, 0);
    m_VBox.pack_start(m_HBox2, Gtk::PACK_SHRINK, 0);
    add(m_VBox);
    
    //
    // Show window.
    //

    show_all();

    writeStatus("Hello", 0);
  }

IQWin::~IQWin() {
    if (m_pPI != NULL) {
        delete m_pPI;
    }
    if (m_pIQAni != NULL) {
        delete m_pIQAni;
    }
    if (m_pDisp != NULL) {
        delete m_pDisp;
    }
    if (m_pSurfaceManager != NULL) {
        delete m_pSurfaceManager;
    }

}

void IQWin::on_button_quit_clicked() {
    printf("Bye...\n");
    Gtk::Main::quit();
}


void IQWin::setDisp(int iDisp) {
    m_tblData.setDisp(iDisp);
    m_IQScene.setPaintMode(iDisp);
}
void IQWin::setDataName(char *pData) {
    m_tblData.setDataName(pData);
}


//-----------------------------------------------------------------------------
// press_action
//   on return do load
// 
bool IQWin::press_action(GdkEventKey *e) {
    printf("Win Keyval:%d\n", e->keyval);
    bool bReturn = false;
          
    if ((e->keyval == GDK_space)  && ((e->state & GDK_CONTROL_MASK) != 0)){
        //        printf("xxx %s ", m_txtDataFile.get_text().c_str());
        printf("%s ", m_tblData.getDataName()); m_IQScene.getView()->showQuat(); printf("\n");
    } else if ((e->keyval == GDK_Escape) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        m_IQScene.getView()->setQuat(&m_IQScene,  -0.443635, 0.228756, -0.825457, 0.263593);
    } else if ((e->keyval == GDK_i) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        printf("Tryingscreenshot\n");
        m_IQScene.save_image("image.png");
    } else if ((e->keyval == GDK_c) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        m_IQScene.clearMarkers();
    } else if ((e->keyval == GDK_x) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        m_IQScene.toggleHex();
    } else if ((e->keyval == GDK_y) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        m_IQScene.toggleLighting();
    } else if ((e->keyval == GDK_z) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        m_IQScene.toggleAlt();
    } else if ((e->keyval == GDK_p) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        // show points
        m_tblData.setDispDirect(MODE_POINTS);
    } else if ((e->keyval == GDK_l) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        // show lines
        m_tblData.setDispDirect(MODE_LINES);
    } else if ((e->keyval == GDK_t) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        // show triangles
        m_tblData.setDispDirect(MODE_PLANES);
    } else if ((e->keyval == GDK_a) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        // show triangles
        m_IQScene.toggleAxis();
    } else if ((e->keyval == GDK_h) && ((e->state & GDK_CONTROL_MASK) != 0)) {
        // show triangles
        showHelp();
   }
    return bReturn;
}

void IQWin::showHelp() {
    Gtk::MessageDialog dialog(*this, "Help", false, Gtk::MESSAGE_INFO);
    dialog.set_secondary_text("Navigation\n"
                              "\tLeft mouse drag\t\trotation\n"
                              "\tCenter mouse drag\t\tzooming\n"
                              "Data\n"
                              "\tShift left click\tset marker\n"
                              "\tCtrk left click\tremove marker\n"
                              "Keyboard Shortcuts\n"
                              "\tCtrl-A\t\ttoggle axis\n"
                              "\tCtrl-C\t\tremove marker\n"
                              "\tCtrl-H\t\tshow help\n"
                              "\tCtrl-I\t\tcreate image\n"
                              "\tCtrl-L\t\tset line mode\n"
                              "\tCtrl-P\t\tset point mode\n"
                              "\tCtrl-T\t\tset triangle mode\n"
                              "\tCtrl-X\t\ttoggle hexagon nodes\n"
                              "\tCtrl-Y\t\ttoggle lighting\n"
                              "\tCtrl-Z\t\ttoggle altitude mode\n");

  dialog.run();
}



//-----------------------------------------------------------------------------
// writeStatus
//   write something to the status line (iMode != 0: red text)
// 
void IQWin::writeStatus(const char *pText, int iMode) {
    GdkColor	_cgtk_color;
    GtkWidget*	_cgtk_label;
    switch (iMode) {
    case NOTIFY_MESSAGE:
        gdk_color_parse("Black", &_cgtk_color);
        break;
    case NOTIFY_WARNING:
        gdk_color_parse("Orange", &_cgtk_color);
        break;
    case NOTIFY_ERROR:
        gdk_color_parse("Red", &_cgtk_color);
        break;
    default:
        gdk_color_parse("White", &_cgtk_color);
    }
    _cgtk_label = m_stbStatus.gobj()->label;
    m_stbStatus.push(pText);
    gtk_widget_modify_fg(GTK_WIDGET(_cgtk_label), GTK_STATE_NORMAL, &_cgtk_color);

}


//-----------------------------------------------------------------------------
// notify
//   
// 
void IQWin::notify(Observable *pObs, int iType, const void *pCom) {
 
    if ((pObs == &(m_IQScene.getModel())) || (pObs == m_pSurfaceManager)) {
        
        if (iType == NOTIFY_INVALIDATE) {
            m_IQScene.invalidate();
        } else {
            // expect pCom to be a string
            if ((iType == NOTIFY_MESSAGE) || (iType == NOTIFY_WARNING) ||(iType == NOTIFY_ERROR)){
                writeStatus((char *) pCom, iType);
            }
        }
               
    } else if (pObs == &(m_tblData)) {
        char sMess[512];
        switch (iType) {
        case NOTIFY_LOAD_GRID:
            sprintf(sMess, "loading [%s]...", (char *) pCom);
            writeStatus(sMess, NOTIFY_WARNING);
            break;
        case NOTIFY_LOAD_DATA:
            sprintf(sMess, "loading [%s]...", (char *)pCom);
            writeStatus(sMess, NOTIFY_WARNING);
            setDisp(MODE_PLANES);
            break;
        }
    } else {
        char sMess[512];
        switch (iType) {
        case NOTIFY_CLEAR_MARKERS:
            sprintf(sMess, "clearing marker...");
            m_IQScene.clearMarkers();
            writeStatus(sMess, 0);
            break;
        case NOTIFY_INVALIDATE:
            m_IQScene.invalidate();
            printf("invalidate by notify 2\n");

        }
    }
}

float vPrev[3]={0,0,1};
//-----------------------------------------------------------------------------
//  ani_event
//   triggered by thread
//
void IQWin::ani_event() {
    char sOut[512];
    m_IQScene.invalidate();
    if (m_pIQAni->isRunning()) {
        float *pCurQuat = m_pIQAni->getCurQuat();
        if (pCurQuat != NULL) {
            m_IQScene.getView()->setQuat(&m_IQScene,pCurQuat[0],pCurQuat[1],pCurQuat[2],pCurQuat[3]);
        }
        m_pSurfaceManager->loadData(m_pIQAni->getCurFile(), m_pIQAni->useCol());
        setDataName(m_pIQAni->getCurFile());    

        strcpy(sOut, m_pIQAni->getPNGFile());
        /*
        char* p = strstr(sOut, ".snap");
        if (p != NULL) {
            *p = 0;
        }
        strcat(sOut, "_a.png");
        */
        if (pCurQuat != NULL) {
            float fZ[3]= {0,0,1};
            float fZ2[3];
            printf("@@CurQuat: ");qshow(pCurQuat);
            printf("@@prev Z: %f %f %f\n", vPrev[0], vPrev[1], vPrev[2]);
            qapply(pCurQuat, fZ, fZ2);
            printf("@@new Z: %f %f %f\n", fZ2[0], fZ2[1], fZ2[2]);
            double dP = fZ2[0]*vPrev[0]+fZ2[1]*vPrev[1]+fZ2[2]*vPrev[2];
            if (dP > 1) {
                dP = 1;
            } else if (dP < -1) {
                dP = -1;
            }
            double dA = acos(dP);
            
            printf("@@%s, Angle: %f\n", sOut, dA);
            memcpy(vPrev, fZ2, 3*sizeof(float));
            usleep(5000);
            m_IQScene.save_image(sOut);
        }
    } else {
        m_tblAni.setStop();
    }
   
}
