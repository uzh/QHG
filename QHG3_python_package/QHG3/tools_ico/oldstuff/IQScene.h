#ifndef __IQSCENE_H__
#define __IQSCENE_H__

#include <gtkmm.h>

#include <gtkglmm.h>

#include <GL/gl.h> 

#include "types.h"
#include "icoutil.h"
#include "IQView.h"
#include "IQModel.h"

class SurfaceManager;
class ProjInfo;

class IQScene : public Gtk::GL::DrawingArea, public Observer {
    friend class IQView;
    friend class IQModel;

public:
    // OpenGL scene related constants:
    static const float CLEAR_COLOR[4];
    static const float CLEAR_DEPTH;

    static const float LIGHT0_POSITION[4];
    static const float LIGHT0_DIFFUSE[4];
    static const float LIGHT0_SPECULAR[4];
    static const float LIGHT0_AMBIENT[4];
    
public:
    explicit IQScene(SurfaceManager *pSurfaceManager, ProjInfo *pPI);
    virtual ~IQScene();
    IQModel &getModel() { return m_Model;};
    void save_image(const char *pName);

    // from Observer
    void notify(Observable *pObs, int iType,  const void *pCom);
protected:
    // signal handlers:
    virtual void on_realize();
    virtual bool on_configure_event(GdkEventConfigure* event);
    virtual bool on_expose_event(GdkEventExpose* event);
    virtual bool on_button_press_event(GdkEventButton* event);
    virtual bool on_map_event(GdkEventAny* event);
    virtual bool on_unmap_event(GdkEventAny* event);
    virtual bool on_visibility_notify_event(GdkEventVisibility* event);

public:
    // Invalidate whole window.
    void invalidate() {
        if (m_bReady) {
            get_window()->invalidate_rect(get_allocation(), false);
        }
    }

    // Update window synchronously (fast).
    void update() { 
        get_window()->process_updates(false); 
    }

    //    void loadIco(const char *pFile);
    //    void loadData(const char *pFile, bool bForceCol=false);
    bool m_bLoading;

public:
    // OpenGL scene related methods:
 
    virtual bool doClick(GdkEventButton *e);

    void set_box(bool bBox);
    void toggle_wire();
    void toggle_color();
    void locate(double dLon, double dLat, gridtype *piNode);
    void clearFace();

    void setPaintMode(int iDisp);
    void toggleHex() { m_Model.toggleHex(); invalidate();};
    void toggleAxis() { m_Model.toggleAxis(); invalidate();};
    void toggleLighting()  { m_Model.toggleLighting(); invalidate();};
    void toggleAlt()  { m_Model.toggleAlt(); invalidate();};
    IQView *getView() { return &m_View;};

    void select(int x, int y) { m_Model.select(x, y); invalidate();};
    void clearMarkers() { m_Model.clearMarkers(); invalidate();};

protected:
    Gtk::Menu* create_popup_menu();

protected:
    // Popup menu:
    Gtk::Menu* m_Menu;

protected:
    // OpenGL scene related objects:
    IQView m_View;
    IQModel m_Model;

    bool m_bReady;
    ProjInfo *m_pPI;
};


#endif
