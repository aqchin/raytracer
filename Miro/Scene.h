#ifndef CSE168_SCENE_H_INCLUDED
#define CSE168_SCENE_H_INCLUDED

#pragma warning (disable : 4267)

#include "Miro.h"
#include "Object.h"
#include "PhotonMap.h"
#include "PointLight.h"
#include "BVH.h"

#define GLOBAL_PPL 1000000
#define CAUSTIC_PPL 10000

class Camera;
class Image;

class Scene
{
public:
  Scene(): 
    m_global_pm(new PhotonMap(GLOBAL_PPL*m_lights.size())), 
    m_caustic_pm(new PhotonMap(CAUSTIC_PPL*m_lights.size())) {}

    void addObject(Object* pObj)        {m_objects.push_back(pObj);}
    const Objects* objects() const      {return &m_objects;}

    void addLight(PointLight* pObj)     {m_lights.push_back(pObj);}
    const Lights* lights() const        {return &m_lights;}

    void preCalc();
    void openGL(Camera *cam);

    void raytraceImage(Camera *cam, Image *img);
    bool trace(HitInfo& minHit, const Ray& ray,
               float tMin = 0.0f, float tMax = MIRO_TMAX) const;

    void mapPhotons();
    void mapCaustics();
    bool testCaustic(Ray&, HitInfo&, Vector3, int, int, int&, int&);

protected:
    Objects m_objects;
    BVH m_bvh;
    Lights m_lights;
    PhotonMap *m_global_pm;
    PhotonMap *m_caustic_pm;
};

extern Scene * g_scene;

#endif // CSE168_SCENE_H_INCLUDED
