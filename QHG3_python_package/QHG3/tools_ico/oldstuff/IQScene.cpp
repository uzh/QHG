#include <iostream>
#include <GL/gl.h>

#include "icoutil.h"
#include "IQScene.h"


#include "SurfaceManager.h"
#include "ProjInfo.h"
#include "notification_codes.h"

const float IQScene::CLEAR_COLOR[4] = { 0.5, 0.5, 0.8, 1.0 };
const float IQScene::CLEAR_DEPTH    = 1.0;

const float IQScene::LIGHT0_POSITION[4] = { -10.0, 10.0, 20.0, 0.0 };
const float IQScene::LIGHT0_DIFFUSE[4]  = { 1.0, 1.0, 1.0, 1.0 };
//const float IQScene::LIGHT0_SPECULAR[4] = { 1.0, 1.0, 1.0, 1.0 };

const float IQScene::LIGHT0_AMBIENT[4] = { 0.05, 0.05, 0.05, 1.0 };

//-----------------------------------------------------------------------------
//  constructor
//
IQScene::IQScene(SurfaceManager *pSurfaceManager, ProjInfo *pPI)
    : m_bLoading(false),
      m_Menu(0), 
      m_Model(pSurfaceManager, pPI),
      m_bReady(false),
      m_pPI(pPI) {

    m_Model.addObserver(this);
    //
    // Configure OpenGL-capable visual.
    //

    Glib::RefPtr<Gdk::GL::Config> glconfig;

    // Try double-buffered visual
    glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGBA   |
                                       Gdk::GL::MODE_DEPTH  |
                                       Gdk::GL::MODE_DOUBLE);
    if (!glconfig) {
        std::cerr << "*** Cannot find the double-buffered visual.\n"
                  << "*** Trying single-buffered visual.\n";
        
        // Try single-buffered visual
        glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGBA   |
                                           Gdk::GL::MODE_DEPTH);
        if (!glconfig) {
            std::cerr << "*** Cannot find any OpenGL-capable visual.\n";
            std::exit(1);
        }
    }
    
    //
    // Set OpenGL-capability to the widget.
    //

    set_gl_capability(glconfig);

    //
    // Add events.
    //
    add_events(Gdk::BUTTON1_MOTION_MASK    |
               Gdk::BUTTON2_MOTION_MASK    |
               Gdk::BUTTON_PRESS_MASK      |
               Gdk::VISIBILITY_NOTIFY_MASK);

    // View transformation signals.
    signal_button_press_event().connect(sigc::bind(sigc::mem_fun(m_View, &IQView::on_button_press_event), this));
    signal_motion_notify_event().connect(sigc::bind(sigc::mem_fun(m_View, &IQView::on_motion_notify_event), this));

    //
    // Popup menu.
    //
    m_Menu = create_popup_menu();

 
}

//-----------------------------------------------------------------------------
//  destructor
//
IQScene::~IQScene() {
}

//-----------------------------------------------------------------------------
//  on_realize
//
void IQScene::on_realize()  {
    // We need to call the base on_realize()
    Gtk::DrawingArea::on_realize();

    //
    // Get GL::Drawable.
    //

    Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

    //
    // GL calls.
    //

    // *** OpenGL BEGIN ***
    if (!gldrawable->gl_begin(get_gl_context()))
      return;

    glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);
    glClearDepth(CLEAR_DEPTH);

    glLightfv(GL_LIGHT0, GL_POSITION, LIGHT0_POSITION);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  LIGHT0_AMBIENT);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  LIGHT0_DIFFUSE);

    glEnable(GL_LIGHTING);
    // start without lightglEnable(GL_LIGHT0);

    glEnable(GL_DEPTH_TEST);

    glEnable (GL_BLEND); 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel(GL_SMOOTH);

    gldrawable->gl_end();
    // *** OpenGL END ***
    m_bReady = true;
  }

//-----------------------------------------------------------------------------
//  on_configure_event
//
bool IQScene::on_configure_event(GdkEventConfigure* event)  {
    //
    // Get GL::Drawable.
    //
    
    Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();
    
    //
    // GL calls.
    //

    // *** OpenGL BEGIN ***
    if (!gldrawable->gl_begin(get_gl_context())) {
        return false;
    }

    m_View.frustum(get_width(), get_height());

    gldrawable->gl_end();
    // *** OpenGL END ***

    return true;
}

//-----------------------------------------------------------------------------
//  on_expose_event
//
bool IQScene::on_expose_event(GdkEventExpose* event) {
    //
    // Get GL::Drawable.
    //

    Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();
    
    //
    // GL calls.
    //
    
    // *** OpenGL BEGIN ***
    if (!gldrawable->gl_begin(get_gl_context()))
        return false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();

    /*    
    float fquat[4];
    m_View.get_quat(fquat[0],fquat[1],fquat[2],fquat[3]);
    m_Model.set_quat(fquat[0],fquat[1],fquat[2],fquat[3]);
    */


    // View transformation.
    m_View.xform();
    
    // Logo model.
    m_Model.draw();

    // update quat in ProjInfo
    m_pPI->setQuat(m_View.getQuat()); 
    // Swap buffers.
    if (gldrawable->is_double_buffered()) {
        gldrawable->swap_buffers();
    } else {
        glFlush();
    }
    gldrawable->gl_end();
    // *** OpenGL END ***

    return true;
}

//-----------------------------------------------------------------------------
//  on_button_press_event
//
bool IQScene::on_button_press_event(GdkEventButton* event) {
    if (event->button == 3) {
        m_Menu->popup(event->button, event->time);
        return true;
    }

    // don't block
    return false;
}

//-----------------------------------------------------------------------------
//  on_map_event
//
bool IQScene::on_map_event(GdkEventAny* event) {
    return true;
}

//-----------------------------------------------------------------------------
//  on_unmap_event
//
bool IQScene::on_unmap_event(GdkEventAny* event) {
    return true;
}

//-----------------------------------------------------------------------------
//  on_visibility_notify_event
//
bool IQScene::on_visibility_notify_event(GdkEventVisibility* event) {
    
    return true;
}

bool IQScene::doClick(GdkEventButton *e) {
    /*
    int iX = e->x;
    int iY = e->y;
    
    m_Model.select(iX, get_height()-iY);
    */
    return true;
}


//-----------------------------------------------------------------------------
//  set_box
//
void IQScene::set_box(bool bBox) {
    m_Model.setBox(bBox);
}

//-----------------------------------------------------------------------------
//  toggle_wire
//
void IQScene::toggle_wire() {
    bool bMode = m_Model.getWireFrame();
    m_Model.setWireFrame(!bMode);
}

//-----------------------------------------------------------------------------
//  toggle_color
//
void IQScene::toggle_color() {
    m_Model.toggle_color();
}


//-----------------------------------------------------------------------------
//  setPaintMode
//
void IQScene::setPaintMode(int iDisp) {
    m_Model.setPaintMode(iDisp);
    invalidate();
}


void IQScene::locate(double dLon, double dLat, gridtype *piNode) {
    m_Model.locate(dLon, dLat, piNode);
    invalidate();
}

void IQScene::clearFace() {
    m_Model.clearFace();
    invalidate();
}



/*
//-----------------------------------------------------------------------------
//  loadIco
//
void IQScene::loadIco(const char *pFile, bool bPreSel) {
    m_Model.loadIco(pFile, bPreSel);
}


//-----------------------------------------------------------------------------
//  loadData
//
void IQScene::loadData(const char *pFile, bool bForceCol) {
    m_bLoading = true;
    printf("IQScene: starting to load [%s]\n", pFile);
    m_Model.loadData(pFile, bForceCol);
    // model must send a notification
}
*/


//-----------------------------------------------------------------------------
//  create_popup_menu
//
Gtk::Menu* IQScene::create_popup_menu()  {
    Gtk::Menu* menu = Gtk::manage(new Gtk::Menu());
    
    Gtk::Menu::MenuList& menu_list = menu->items();
    
    // Toggle animation
    menu_list.push_back(Gtk::Menu_Helpers::MenuElem("Toggle Wireframe",
                                                    sigc::mem_fun(*this, &IQScene::toggle_wire)));
    
    // Quit
    menu_list.push_back(Gtk::Menu_Helpers::MenuElem("Quit",
                                                    sigc::ptr_fun(&Gtk::Main::quit)));    
    
    return menu;
}

 

//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQScene::notify(Observable *pObs, int iType, const void *pCom) {
    // printf("[IQScene::notify] received notification [%d] from %p\n", iCom , pObs);
    if ((pObs == &m_Model) && (iType == 0)){
        long iCom = (long) pCom;
        switch (iCom) {
        case NOTIFY_MODEL_REPAINT: 
            invalidate();
            break;
        case NOTIFY_MODEL_LOADED: 
            //            printf("IQScene:model loaded\n");
            m_bLoading = false;
            break;
        default:
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// save_image
//
void IQScene::save_image(const char *pName) {

    unsigned char *pImage = new unsigned char[get_width()*get_height()*3]; 
    printf("allocating %dx%dx3=%d for %s\n", get_width(),get_height(),get_width()*get_height()*3,pName);
    glReadPixels(0, 0,  get_width(), get_height(), GL_RGB, GL_UNSIGNED_BYTE, pImage);
    Glib::RefPtr< Gdk::Pixbuf > pixbuf;
    pixbuf = Gdk::Pixbuf::create_from_data(pImage, Gdk::COLORSPACE_RGB, false, 8,  get_width(), get_height(), 3*get_width());

    
    // And finally save the image
    pixbuf->save(pName, "png");

    delete[] pImage;

}

