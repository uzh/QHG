#include <gtkmm.h>

#include "IcoScene.h"
#include "IcoView.h"

#include "trackball.h"



#define DEG_2_RAD (G_PI / 180.0)

const float IcoView::NEAR_CLIP   = 2.0;
const float IcoView::FAR_CLIP    = 60.0;

const float IcoView::INIT_POS_X  = 0.0;
const float IcoView::INIT_POS_Y  = 0.0;
const float IcoView::INIT_POS_Z  = -20.0;

const float IcoView::INIT_AXIS_X = 1.0;
const float IcoView::INIT_AXIS_Y = 0.0;
const float IcoView::INIT_AXIS_Z = 0.0;
const float IcoView::INIT_ANGLE  = 0.0;

const float IcoView::INIT_SCALE  = 8.0;

const float IcoView::SCALE_MAX   = 16.0;
const float IcoView::SCALE_MIN   = 0.5;

IcoView::IcoView()
    : m_Scale(INIT_SCALE), 
      m_BeginX(0.0), 
      m_BeginY(0.0),
      m_fMinScale(SCALE_MIN),
      m_fMaxScale(SCALE_MAX)  {
    reset();
}

IcoView::~IcoView() {
}

void IcoView::frustum(int w, int h) {
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

void IcoView::xform() {
    glTranslatef(m_Pos[0], m_Pos[1], m_Pos[2]);
    
    glScalef(m_Scale, m_Scale, m_Scale);
    
    float m[4][4];
    build_rotmatrix(m, m_Quat);
    glMultMatrixf(&m[0][0]);
}

void IcoView::reset() {
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

void IcoView::centerPoint(float fLon, float fLat) {
    float q1[4];
    float fAxisZ[3]={0,0,1};
    axis_to_quat(fAxisZ, fLon+M_PI/2, q1);
    float q2[4];
    float fAxisY[3]={1,0,0};
    axis_to_quat(fAxisY, M_PI/2-fLat, q2);
    add_quats(q2, q1, m_Quat);
    /*
    float q3[4];
    float fAxisX[3]={0,1,0};
    axis_to_quat(fAxisX, -M_PI/2, q3);
    add_quats(q3, m_Quat, m_Quat);
    */
}

bool IcoView::on_button_press_event(GdkEventButton* event,
                                   IcoScene* scene)  {
    m_BeginX = event->x;
    m_BeginY = event->y;
    
    // don't block
    return false;
}

bool IcoView::on_motion_notify_event(GdkEventMotion* event,
                                    IcoScene* scene)  {
    if (scene == 0) {
        return false;
    }

    float w = scene->get_width();
    float h = scene->get_height();
    float x = event->x;
    float y = event->y;
    float d_quat[4];
    bool redraw = false;
    
    // Rotation / Translation
    if (event->state & GDK_BUTTON1_MASK) {
        if ((event->state & GDK_SHIFT_MASK) != 0) {
            m_Pos[0] -= 0.2*(m_BeginX - x);
            m_Pos[1] += 0.2*(m_BeginY - y);
        } else {
            trackball(d_quat,
                      (2.0 * m_BeginX - w) / w,
                      (h - 2.0 * m_BeginY) / h,
                      (2.0 * x - w) / w,
                      (h - 2.0 * y) / h);
            add_quats(d_quat, m_Quat, m_Quat);
        }
        redraw = true;
    }
    
    // Scaling.
    if (event->state & GDK_BUTTON2_MASK) {
      m_Scale = m_Scale * (1.0 + (y - m_BeginY) / h);
      if (m_Scale > m_fMaxScale)
          m_Scale = m_fMaxScale;
      else if (m_Scale < m_fMinScale)
          m_Scale = m_fMinScale;
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

