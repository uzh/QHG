#include <gtkmm.h>

#include "IQScene.h"
#include "IQView.h"

#include "trackball.h"



#define DEG_2_RAD (G_PI / 180.0)

const float IQView::NEAR_CLIP   = 2.0;
const float IQView::FAR_CLIP    = 60.0;

const float IQView::INIT_POS_X  = 0.0;
const float IQView::INIT_POS_Y  = 0.0;
const float IQView::INIT_POS_Z  = -22;

const float IQView::INIT_AXIS_X = 1.0;
const float IQView::INIT_AXIS_Y = 0.0;
const float IQView::INIT_AXIS_Z = 0.0;
const float IQView::INIT_ANGLE  = 0.0;

const float IQView::INIT_SCALE  = 8.0;

const float IQView::SCALE_MAX   = 20.0;
const float IQView::SCALE_MIN   = 0.0001;


//-----------------------------------------------------------------------------
// constructor
//
IQView::IQView()
    : m_Scale(INIT_SCALE), 
      m_BeginX(0.0), 
      m_BeginY(0.0)  {
    reset();
}

//-----------------------------------------------------------------------------
// destructor
//
IQView::~IQView() {
}

//-----------------------------------------------------------------------------
// frustum
//
void IQView::frustum(int w, int h) {
    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    if (w > h) {
        float aspect = static_cast<float>(w) / static_cast<float>(h);
        glFrustum(-aspect, aspect, -1.0, 1.0, NEAR_CLIP, FAR_CLIP);
    } else {
        float aspect = static_cast<float>(h) / static_cast<float>(w);
        glFrustum(-1.0, 1.0, -aspect, aspect, NEAR_CLIP, FAR_CLIP);
    }
    
    glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------
// xform
//
void IQView::xform() {
    glTranslatef(m_Pos[0], m_Pos[1], m_Pos[2]);
    
    glScalef(m_Scale, m_Scale, m_Scale);

    float m[4][4];
    build_rotmatrix(m, m_Quat);
    glMultMatrixf(&m[0][0]);
}

//-----------------------------------------------------------------------------
// reset
//
void IQView::reset() {
    m_Pos[0] = INIT_POS_X;
    m_Pos[1] = INIT_POS_Y;
    m_Pos[2] = INIT_POS_Z;
    
    float sine = sin(0.5 * INIT_ANGLE * DEG_2_RAD);
    m_Quat[0] = INIT_AXIS_X * sine;
    m_Quat[1] = INIT_AXIS_Y * sine;
    m_Quat[2] = INIT_AXIS_Z * sine;
    m_Quat[3] = cos(0.5 * INIT_ANGLE * DEG_2_RAD);

    m_Quat0[0] = INIT_AXIS_X * sine;
    m_Quat0[1] = INIT_AXIS_Y * sine;
    m_Quat0[2] = INIT_AXIS_Z * sine;
    m_Quat0[3] = cos(0.5 * INIT_ANGLE * DEG_2_RAD);
 
    m_Scale = INIT_SCALE;
   
}

//-----------------------------------------------------------------------------
// on_button_press_event
//
bool IQView::on_button_press_event(GdkEventButton* event,
                                   IQScene* scene)  {
    m_BeginX = event->x;
    m_BeginY = event->y;
    
    if ((event->state & GDK_SHIFT_MASK) != 0) {
        scene->select(event->x,event->y);
    } else if ((event->state & GDK_CONTROL_MASK) != 0) {
        scene->clearMarkers();
    }
    // don't block
    return false;
}

//-----------------------------------------------------------------------------
// on_motion_notify_event
//
bool IQView::on_motion_notify_event(GdkEventMotion* event,
                                    IQScene* scene)  {
    if (scene == 0) {
        return false;
    }

    if ((event->state & GDK_SHIFT_MASK) != 0) {
        scene->select(event->x,event->y);
    }

    float w = scene->get_width();
    float h = scene->get_height();
    float x = event->x;
    float y = event->y;
    float d_quat[4];
    bool redraw = false;
    
    // Rotation.
    if (event->state & GDK_BUTTON1_MASK) {
         trackball(d_quat,
                             (2.0 * m_BeginX - w) / w,
                             (h - 2.0 * m_BeginY) / h,
                             (2.0 * x - w) / w,
                             (h - 2.0 * y) / h);
        add_quats(d_quat, m_Quat, m_Quat);
        redraw = true;
    }
    
    // Scaling.
    if (event->state & GDK_BUTTON2_MASK) {
      m_Scale = m_Scale * (1.0 + (y - m_BeginY) / h);
      if (m_Scale > SCALE_MAX)
          m_Scale = SCALE_MAX;
      else if (m_Scale < SCALE_MIN)
          m_Scale = SCALE_MIN;
      redraw = true;
    }

    m_BeginX = x;
    m_BeginY = y;
    
    if (redraw) {
        scene->invalidate();
    }
    
 


    // don't block
    return false;
}


//-----------------------------------------------------------------------------
// showQuat
//
void IQView::showQuat() {
    printf("QUAT: %f %f %f %f\n", m_Quat[0], m_Quat[1], m_Quat[2], m_Quat[3]); 
    
    /*
    printf("QUAT0: %f %f %f %f (L %f)\n", m_Quat[0], m_Quat[1], m_Quat[2], m_Quat[3],
           sqrt(m_Quat[0]*m_Quat[0]+m_Quat[1]*m_Quat[1]+m_Quat[2]*m_Quat[2]+m_Quat[3]*m_Quat[3]));
    double dTheta = acos(m_Quat[3]);
    double dS = sin(dTheta);
    printf("QUAT0:      %f; %f, %f, %f\n", dTheta, m_Quat[0]/dS, m_Quat[1]/dS, m_Quat[2]/dS);
    */
}

//-----------------------------------------------------------------------------
// setQuat
//
void IQView::setQuat(IQScene* scene, double dX, double dY, double dZ, double dW) {
    m_Quat[0] = dX;
    m_Quat[1] = dY;
    m_Quat[2] = dZ;
    m_Quat[3] = dW;
    scene->invalidate();
}
