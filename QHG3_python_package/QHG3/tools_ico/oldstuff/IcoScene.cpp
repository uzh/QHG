#include <iostream>
#include <GL/gl.h>

#include "GridProjection.h"

#include "icoutil.h"
#include "IcoScene.h"

#include "Icosahedron.h"

const unsigned int IcoScene::TIMEOUT_INTERVAL = 10;

const float IcoScene::CLEAR_COLOR[4] = { 0.5, 0.5, 0.8, 1.0 };
const float IcoScene::CLEAR_DEPTH    = 1.0;

const float IcoScene::LIGHT0_POSITION[4] = { 0.0, 0.0, -30.0, 0.0 };
const float IcoScene::LIGHT0_DIFFUSE[4]  = { 1.0, 1.0, 1.0, 1.0 };
const float IcoScene::LIGHT0_SPECULAR[4] = { 1.0, 1.0, 1.0, 1.0 };
//const float IcoScene::LIGHT0_AMBIENT[4] = { 1.0, 1.0, 1.0, 1.0 };
const float IcoScene::LIGHT0_AMBIENT[4] = { 0.1, 0.1, 0.1, 1.0 };

//-----------------------------------------------------------------------------
//  constructor
//
IcoScene::IcoScene(const char *pFile)
    : m_Menu(0), 
      m_Model(pFile) {
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
    signal_button_press_event().connect(sigc::bind(sigc::mem_fun(m_View, &IcoView::on_button_press_event), this));
    signal_motion_notify_event().connect(sigc::bind(sigc::mem_fun(m_View, &IcoView::on_motion_notify_event), this));

    //
    // Popup menu.
    //
    m_Menu = create_popup_menu();

 
}

//-----------------------------------------------------------------------------
//  destructor
//
IcoScene::~IcoScene() {
}

//-----------------------------------------------------------------------------
//  on_realize
//
void IcoScene::on_realize()  {
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

    //glLightfv(GL_LIGHT0, GL_POSITION, LIGHT0_POSITION);
    //        glLightfv(GL_LIGHT0, GL_DIFFUSE,  LIGHT0_DIFFUSE);
    //    glLightfv(GL_LIGHT0, GL_SPECULAR, LIGHT0_SPECULAR);
       glLightfv(GL_LIGHT0, GL_AMBIENT,  LIGHT0_AMBIENT);

    glEnable(GL_LIGHTING);
    //          glEnable(GL_LIGHT0);

    glEnable(GL_DEPTH_TEST);

    glEnable (GL_BLEND); 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel(GL_SMOOTH);

    gldrawable->gl_end();
    // *** OpenGL END ***
  }

//-----------------------------------------------------------------------------
//  on_configure_event
//
bool IcoScene::on_configure_event(GdkEventConfigure* event)  {
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
bool IcoScene::on_expose_event(GdkEventExpose* event) {
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
bool IcoScene::on_button_press_event(GdkEventButton* event) {
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
bool IcoScene::on_map_event(GdkEventAny* event) {
    return true;
}

//-----------------------------------------------------------------------------
//  on_unmap_event
//
bool IcoScene::on_unmap_event(GdkEventAny* event) {
    timeout_remove();
    
    return true;
}

//-----------------------------------------------------------------------------
//  on_visibility_notify_event
//
bool IcoScene::on_visibility_notify_event(GdkEventVisibility* event) {
    
    return true;
}

//-----------------------------------------------------------------------------
//  on_timeout
//
bool IcoScene::on_timeout() {
    // Invalidate whole window.
    invalidate();
    // Update window synchronously (fast).
    update();
    
    return true;
}


bool IcoScene::doClick(GdkEventButton *e) {
    /*
    int iX = e->x;
    int iY = e->y;
    
    m_Model.select(iX, get_height()-iY);
    */
    return true;
}

//-----------------------------------------------------------------------------
//  timeout_add
//
void IcoScene::timeout_add() {
    if (!m_ConnectionTimeout.connected()) {
        m_ConnectionTimeout = Glib::signal_timeout().connect(sigc::mem_fun(*this, &IcoScene::on_timeout), TIMEOUT_INTERVAL);
    }
}

//-----------------------------------------------------------------------------
//  timeout_remove
//
void IcoScene::timeout_remove() {
    if (m_ConnectionTimeout.connected()) {
        m_ConnectionTimeout.disconnect();
    }
}

//-----------------------------------------------------------------------------
//  set_box
//
void IcoScene::set_box(bool bBox) {
    m_Model.setBox(bBox);
}
//-----------------------------------------------------------------------------
//  setFlat
//
void IcoScene::setFlat(bool bFlat) {
    if (bFlat) {
        m_View.set_scale_range(0.0001, 16);
    } else {
        m_View.set_scale_range(0.5, 16);
    }        
    //   invalidate();
}

//-----------------------------------------------------------------------------
//  toggle_wire
//
void IcoScene::toggle_wire() {
    bool bMode = m_Model.getWireFrame();
    m_Model.setWireFrame(!bMode);
}

//-----------------------------------------------------------------------------
//  toggle_color
//
void IcoScene::toggle_color() {
    m_Model.toggle_color();
    invalidate();
}

//-----------------------------------------------------------------------------
//  setDisplayLevel
//
void IcoScene::setDisplayLevel(int iDispLevel) {
    m_Model.setDisplayLevel(iDispLevel);
    invalidate();
}

//-----------------------------------------------------------------------------
//  toggleDisplayLevel
//
void IcoScene::toggleDisplayLevel(int iDispLevel) {
    m_Model.toggleDisplayLevel(iDispLevel);
    invalidate();
}

//-----------------------------------------------------------------------------
//  reset
//
void IcoScene::reset() {
    m_View.reset();
    
    invalidate();
}

//-----------------------------------------------------------------------------
//  locate
//
void IcoScene::locate(double dLon, double dLat, gridtype *piNode) {
    m_Model.locate(dLon, dLat, piNode);
    invalidate();
}

//-----------------------------------------------------------------------------
//  clearFace
//
void IcoScene::clearFace() {
    m_Model.clearFace();
    invalidate();
}

//-----------------------------------------------------------------------------
//  create_popup_menu
//
Gtk::Menu* IcoScene::create_popup_menu()  {
    Gtk::Menu* menu = Gtk::manage(new Gtk::Menu());
    
    Gtk::Menu::MenuList& menu_list = menu->items();
    
    // Toggle animation
    menu_list.push_back(Gtk::Menu_Helpers::MenuElem("Toggle Wireframe",
                                                    sigc::mem_fun(*this, &IcoScene::toggle_wire)));
    
    // Init orientation
    menu_list.push_back(Gtk::Menu_Helpers::MenuElem("reset",
                                                    sigc::mem_fun(*this, &IcoScene::reset)));
    
    // Quit
    menu_list.push_back(Gtk::Menu_Helpers::MenuElem("Quit",
                                                    sigc::ptr_fun(&Gtk::Main::quit)));    
    
    return menu;
}









 








