#ifndef __ICOVIEW_H__
#define __ICOVIEW_H__

#include <gtkmm.h>

class IcoScene;

class IcoView : public sigc::trackable {
    friend class IcoScene;

public:
    static const float NEAR_CLIP;
    static const float FAR_CLIP;
    
    static const float INIT_POS_X;
    static const float INIT_POS_Y;
    static const float INIT_POS_Z;
    
    static const float INIT_AXIS_X;
    static const float INIT_AXIS_Y;
    static const float INIT_AXIS_Z;
    static const float INIT_ANGLE;
    
    static const float INIT_SCALE;
    
    static const float SCALE_MAX;
    static const float SCALE_MIN;
    
public:
    IcoView();
    virtual ~IcoView();
    
public:
    void frustum(int w, int h);
    
    void xform();
    
    void reset();
    
    void set_pos(float x, float y, float z)
    { m_Pos[0] = x; m_Pos[1] = y; m_Pos[2] = z; }
    
    void set_quat(float q0, float q1, float q2, float q3)
    { m_Quat[0] = q0; m_Quat[1] = q1; m_Quat[2] = q2; m_Quat[3] = q3; }

    void get_quat(float &q0, float &q1, float &q2, float &q3)
    { q0 = m_Quat[0] ; q1 = m_Quat[1]; q2 = m_Quat[2]; q3 = m_Quat[3]; }
 
    void set_scale(float scale)
    { m_Scale = scale; }

    void set_scale_range(float fMin, float fMax)
    { m_fMinScale = fMin;m_fMaxScale = fMax; }


    void centerPoint(float fLon, float fLat);

protected:
    // Signal handlers:
    virtual bool on_button_press_event(GdkEventButton* event, IcoScene* scene);
    virtual bool on_motion_notify_event(GdkEventMotion* event, IcoScene* scene);
    
private:
    float m_Pos[3];
    float m_Quat[4];
    float m_Quat0[4];
    float m_Scale;
    
    float m_BeginX;
    float m_BeginY;
    
    float m_fMinScale;
    float m_fMaxScale;
};

#endif
