#ifndef __ICOSCENE_H__
#define __ICOSCENE_H__

#include <gtkmm.h>

#include <gtkglmm.h>

#include <GL/gl.h>

#include "types.h"
#include "icoutil.h"
#include "IcoView.h"
#include "IcoModel.h"

class IcoSurface_OGL;
class GridProjection;

class IcoScene : public Gtk::GL::DrawingArea {
    friend class IcoView;
    friend class IcoModel;

public:
    static const unsigned int TIMEOUT_INTERVAL;

    // OpenGL scene related constants:
    static const float CLEAR_COLOR[4];
    static const float CLEAR_DEPTH;

    static const float LIGHT0_POSITION[4];
    static const float LIGHT0_DIFFUSE[4];
    static const float LIGHT0_SPECULAR[4];
    static const float LIGHT0_AMBIENT[4];
    
public:
    explicit IcoScene(const char *pFile);
    virtual ~IcoScene();
    
  protected:
    // signal handlers:
    virtual void on_realize();
    virtual bool on_configure_event(GdkEventConfigure* event);
    virtual bool on_expose_event(GdkEventExpose* event);
    virtual bool on_button_press_event(GdkEventButton* event);
    virtual bool on_map_event(GdkEventAny* event);
    virtual bool on_unmap_event(GdkEventAny* event);
    virtual bool on_visibility_notify_event(GdkEventVisibility* event);
    virtual bool on_timeout();

  public:
    // Invalidate whole window.
    void invalidate() {
      get_window()->invalidate_rect(get_allocation(), false);
    }

    // Update window synchronously (fast).
    void update()
    { get_window()->process_updates(false); }

    Surface        *getSurface() { return m_Model.getSurface();};
    IcoSurface_OGL *getSurfaceOGL() { return m_Model.getSurfaceOGL();};
    ValReader *getValReader() { return m_Model.getValReader();};
    //    GridProjection *getGridProjection() { return m_Model.getGridProjection();};
 
    void refreshImage() { m_Model.refreshImage(); invalidate();};
    void setFlat(bool bFlat);
  protected:
    // timeout signal connection:
    sigc::connection m_ConnectionTimeout;

    void timeout_add();
    void timeout_remove();

  public:
    // OpenGL scene related methods:
 
    virtual bool doClick(GdkEventButton *e);

    void setSurface(Surface *pSurface, IcoSurface_OGL *pSurfaceOGL) { printf("IcoScene set surf %p\n", pSurface); m_Model.setSurface(pSurface, pSurfaceOGL); invalidate();};

    void setPolys(int iNumPolys, int iPolySize, Vec3D **apvPolys, GridProjection *pGP);

    void set_box(bool bBox);
    //    void setPolyType(int iPolyType);
    void toggle_wire();
    void toggle_color();
    void setDisplayLevel(int iDispLevel);
    void toggleDisplayLevel(int iDispLevel);
    void reset();
    //    void subdivide();
    void locate(double dLon, double dLat, gridtype *piNode);
    void clearFace();
    //    void merge();
    //    void setStrict(bool bStrict);
    //    void setDual(bool bDual);
    //    void setSubdivision(int iLevel);
    //    void subland(float fMinAlt, int iLevel);
    //    void subdivide(int iLevel);
    //    void merge(int iLevel);
    //    void varsubdiv(bool bPreSel, int iMaxLevel, tbox &tBox, double dDLon, double dDLat);
    void centerPoint(float fLon, float fLat) { m_View.centerPoint(fLon, fLat);};

  protected:
    Gtk::Menu* create_popup_menu();

  protected:
    // Popup menu:
    Gtk::Menu* m_Menu;

  protected:
    // OpenGL scene related objects:
    IcoView m_View;
    IcoModel m_Model;

  };


#endif



/*




























class IcoScene : public Gtk::GL::DrawingArea {
public:
    explicit IcoScene(bool is_sync = true);
    virtual ~IcoScene();
    
    void resetView();
protected:
    void buildIco();
    
protected:
    // signal handlers:
    virtual void on_realize();
    virtual bool on_configure_event(GdkEventConfigure* event);
    virtual bool on_expose_event(GdkEventExpose* event);
    virtual bool on_map_event(GdkEventAny* event);
    virtual bool on_unmap_event(GdkEventAny* event);
    virtual bool on_visibility_notify_event(GdkEventVisibility* event);
    virtual bool on_idle();
    
public:
    // Invalidate whole window.
    void invalidate() {
        get_window()->invalidate_rect(get_allocation(), false);
    }
    
    // Update window synchronously (fast).
    void update()  { 
        get_window()->process_updates(false);
    }
    
protected:
    // idle signal connection:
    sigc::connection m_ConnectionIdle;
    
public:
    // get & set view rotation values.
    void getRotation(GLfloat afViewQuat[4]) { 
        afViewQuat[0] = m_afViewQuat[0];
        afViewQuat[1] = m_afViewQuat[1];
        afViewQuat[2] = m_afViewQuat[2];
        afViewQuat[3] = m_afViewQuat[3];
    }

    void setRotation(GLfloat afViewQuat[4]) {
        m_afViewQuat[0] = afViewQuat[0];
        m_afViewQuat[1] = afViewQuat[1];
        m_afViewQuat[2] = afViewQuat[2];
        m_afViewQuat[3] = afViewQuat[3];
    }
    
protected:
    // OpenGL scene related variables:
    bool m_IsSync;
    GLfloat m_afViewQuat[4];
    GLfloat m_fViewScale;
    GLint m_IcoList;
  
protected:
};

#endif
*/
