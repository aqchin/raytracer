#include <stdio.h>
#include <stdlib.h>
#include "Miro.h"
#include "Camera.h"
#include "Image.h"
#include "Scene.h"
#include "Console.h" 
#include "OpenGL.h"
#include <time.h>

Camera * g_camera = 0;

#define DO_RAND false
#define DO_DOF false
#define FOCUS_L 5.5 // TEAPOT(5.5) CORNELL(10)
#define APERTURE .5
#define M_PI 3.14159265358979

static bool firstRayTrace = true; 

const float HalfDegToRad = DegToRad/2.0f;

Camera::Camera() :
    m_bgColor(0,0,0),
    m_renderer(RENDER_OPENGL),
    m_eye(0,0,0),
    m_viewDir(0,0,-1),
    m_up(0,1,0),
    m_lookAt(FLT_MAX, FLT_MAX, FLT_MAX),
    m_fov((45.)*(PI/180.))
{
    calcLookAt();
}


Camera::~Camera()
{

}


void
Camera::click(Scene* pScene, Image* pImage)
{
    calcLookAt();
    static bool firstRayTrace = false;

    if (m_renderer == RENDER_OPENGL)
    {
        glDrawBuffer(GL_BACK);
        pScene->openGL(this);
        firstRayTrace = true;
    }
    else if (m_renderer == RENDER_RAYTRACE)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDrawBuffer(GL_FRONT);
        if (firstRayTrace)
        {
            pImage->clear(bgColor());
            pScene->raytraceImage(this, g_image);
            firstRayTrace = false;
        }
        
        g_image->draw();
    }
}


void
Camera::calcLookAt()
{
    // this is true when a "lookat" is not used in the config file
    if (m_lookAt.x != FLT_MAX)
    {
        setLookAt(m_lookAt);
        m_lookAt.set(FLT_MAX, FLT_MAX, FLT_MAX);
    }
}


void
Camera::drawGL()
{
    // set up the screen with our camera parameters
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov(), g_image->width()/(float)g_image->height(),
                   0.01, 10000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Vector3 vCenter = eye() + viewDir();
    gluLookAt(eye().x, eye().y, eye().z,
              vCenter.x, vCenter.y, vCenter.z,
              up().x, up().y, up().z);
}


Ray
Camera::eyeRay(int x, int y, int imageWidth, int imageHeight)
{
    // first compute the camera coordinate system 
    // ------------------------------------------

    // wDir = e - (e+m_viewDir) = -m_vView
    const Vector3 wDir = Vector3(-m_viewDir).normalize(); 
    const Vector3 uDir = cross(m_up, wDir).normalize(); 
    const Vector3 vDir = cross(wDir, uDir);    

    // next find the corners of the image plane in camera space
    // --------------------------------------------------------

    const float aspectRatio = (float)imageWidth/(float)imageHeight; 

    const float top     = tan(m_fov*HalfDegToRad); 
    const float right   = aspectRatio*top; 

    const float bottom  = -top; 
    const float left    = -right; 

    // transform x and y into camera space 
    // -----------------------------------
    float u = 0.5f;
    float v = 0.5f;
    if(DO_RAND) {
      u = rand()/(float)RAND_MAX;
      v = rand()/(float)RAND_MAX;
    }

    Vector3 aperatureOffset(0.f);
    if(DO_DOF) {
      float circPoint[2] = {
        cos(rand()/(float)RAND_MAX *2.f*M_PI) *(rand()/(float)RAND_MAX),
        sin(rand()/(float)RAND_MAX *2.f*M_PI) *(rand()/(float)RAND_MAX) }; 
      aperatureOffset = Vector3(circPoint[0] *APERTURE, circPoint[1] *APERTURE, 0.f);
    }
    const float imPlaneUPos = left   + (right - left)*(((float)x+u)/(float)imageWidth); 
    const float imPlaneVPos = bottom + (top - bottom)*(((float)y+v)/(float)imageHeight); 

    Vector3 o = m_eye + aperatureOffset;
    Vector3 d = (imPlaneUPos*uDir + imPlaneVPos*vDir - wDir).normalize();
    if(DO_DOF) {
      d *= FOCUS_L;
      d -= aperatureOffset;
      d.normalize();
    }

    return Ray(o, d);
}
