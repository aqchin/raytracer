#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"

#include "PointLight.h"
#include "Sphere.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "Lambert.h"
#include "MiroWindow.h"
#include "assignment1.h"
#include "assignment2.h"

#define DIAMOND_IN 2.417
#define GLASS_INL 1.52
#define GLASS_INH 1.65
#define ICE_IN 1.31
#define STP_AIR_IN 1.00029
#define WATER_IN 1.33
#define VACUUM_IN 1.00000

void
makeSpiralScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(1.0f, 1.0f, 1.0f));
    g_camera->setEye(Vector3(-5, 2, 3));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);

    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(-3, 15, 3));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(1000);
    g_scene->addLight(light);

    // create a spiral of spheres
    Material* mat = new Lambert(Vector3(1.0f, 0.0f, 0.0f));
    const int maxI = 200;
    const float a = 0.15f;
    for (int i = 1; i < maxI; ++i)
    {
        float t = i/float(maxI);
        float theta = 4*PI*t;
        float r = a*theta;
        float x = r*cos(theta);
        float y = r*sin(theta);
        float z = 2*(2*PI*a - r);
        Sphere * sphere = new Sphere;
        sphere->setCenter(Vector3(x,y,z));
        sphere->setRadius(r/10);
        sphere->setMaterial(mat);
        g_scene->addObject(sphere);
    }
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

/*
void
makeBunnyScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(128, 128);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(-2, 3, 5));
    g_camera->setLookAt(Vector3(-.5, 1, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);

    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(-3, 15, 3));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(500);
    g_scene->addLight(light);

    Material* mat = new Lambert(Vector3(1.0f));

    TriangleMesh * bunny = new TriangleMesh;
    bunny->load("bunny.obj");
    
    // create all the triangles in the bunny mesh and add to the scene
    for (int i = 0; i < bunny->numTris(); ++i)
    {
        Triangle* t = new Triangle;
        t->setIndex(i);
        t->setMesh(bunny);
        t->setMaterial(mat); 
        g_scene->addObject(t);
    }
    
    // create the floor triangle
    TriangleMesh * floor = new TriangleMesh;
    floor->createSingleTriangle();
    floor->setV1(Vector3(  0, 0,  10));
    floor->setV2(Vector3( 10, 0, -10));
    floor->setV3(Vector3(-10, 0, -10));
    floor->setN1(Vector3(0, 1, 0));
    floor->setN2(Vector3(0, 1, 0));
    floor->setN3(Vector3(0, 1, 0));
    
    Triangle* t = new Triangle;
    t->setIndex(0);
    t->setMesh(floor);
    t->setMaterial(mat); 
    g_scene->addObject(t);
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}
*/
void
makeCornellScene()
{
  g_camera = new Camera;
  g_scene = new Scene;
  g_image = new Image;

  g_image->resize(1024, 1024);

  // set up the camera
  g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));;
  g_camera->setEye(Vector3(2.78, 2.73, 6.5));
  g_camera->setLookAt(Vector3(2.78, 2.73, -2.75));
  g_camera->setUp(Vector3(0, 1, 0));
  g_camera->setFOV(45);

  // create and place a point light source
  PointLight * light = new PointLight;
  light->setPosition(Vector3(2.78, 5.4, -2.75));
  light->setColor(Vector3(1, 1, 1));
  light->setWattage(60);
  g_scene->addLight(light);

  Material* wmat = new Lambert(Vector3(1.f,1.f,1.f));
  Material* rmat = new Lambert(Vector3(1.f,.4f,.4f));
  Material* gmat = new Lambert(Vector3(.4f,1.f,.4f));//, Vector3(0.f),);

  Material* obj1_mat = new Lambert(Vector3(1.f, 1.f, .7f), Vector3(0.f),
    Vector3(0.f), Vector3(0.f), Vector3(0.f), 0.f, 0.f, 0.f);

  Material* ref_mat = new Lambert(Vector3(0.f), Vector3(0.f),
    Vector3(0.f), Vector3(0.f), Vector3(1.f), 0.f, 1.f, DIAMOND_IN);

  Material* mir_mat = new Lambert(Vector3(0.f), Vector3(0.f),
    Vector3(0.f), Vector3(1.f), Vector3(0.f), 0.f);

  TriangleMesh* room = new TriangleMesh;

  room->load("cornell_box.obj");

  // create all the triangles in the bunny mesh and `add to the scene
  for (int i = 0; i < room->numTris(); ++i)
  {
    Triangle* t = new Triangle;
    t->setIndex(i);
    t->setMesh(room);
    if (i >= 4 && i < 6)
      t->setMaterial(rmat);
    else if (i >= 6 && i < 8)
      t->setMaterial(gmat);
    else
      t->setMaterial(wmat);
    g_scene->addObject(t);
  }

  Sphere* ref_s = new Sphere;
  ref_s->setCenter(Vector3(4.75, .5f, -1.f));
  ref_s->setRadius(0.5);
  ref_s->setMaterial(ref_mat);
  g_scene->addObject(ref_s);

  Sphere* mir_s = new Sphere;
  mir_s->setCenter(Vector3(1.f, .85, -4.f));
  mir_s->setRadius(0.85);
  mir_s->setMaterial(mir_mat);
  g_scene->addObject(mir_s);
  
  Sphere* dif_s = new Sphere;
  dif_s->setCenter(Vector3(3.f, 1.2, -2.5f));
  dif_s->setRadius(1.2);
  dif_s->setMaterial(obj1_mat);
  g_scene->addObject(dif_s);
  
  // let objects do pre-calculations if needed
  g_scene->preCalc();
}

void
makeCoolScene()
{
  g_camera = new Camera;
  g_scene = new Scene;
  g_image = new Image;

  g_image->resize(512, 512);

  // set up the camera
  g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
  g_camera->setEye(Vector3(-2, 3, 200));
  g_camera->setLookAt(Vector3(-.5, 1, 0));
  g_camera->setUp(Vector3(0, 1, 0));
  g_camera->setFOV(45);

  // create and place a point light source
  PointLight * light = new PointLight;
  light->setPosition(Vector3(-3, 15, 300));
  light->setColor(Vector3(1, 1, 1));
  light->setWattage(250);
  g_scene->addLight(light);

  Material* mat = new Lambert(Vector3(1.f, 0.f, 0.f), Vector3(0.f),
    Vector3(0.f), Vector3(.5f), Vector3(0.f), 0.f, 1.f, 0.f);
  Material* f_mat = new Lambert(Vector3(0.f, 0.f, 1.f), Vector3(0.f),
    Vector3(.7f), Vector3(0.f), Vector3(0.f), 0.f, 0.f, 0.f);

  TriangleMesh * tpot = new TriangleMesh;
  tpot->load("bear.obj");

  // create all the triangles in the bunny mesh and add to the scene
  for (int i = 0; i < tpot->numTris(); ++i)
  {
    Triangle* t = new Triangle;
    t->setIndex(i);
    t->setMesh(tpot);
    t->setMaterial(mat);
    g_scene->addObject(t);
  }

  // let objects do pre-calculations if needed
  g_scene->preCalc();
}

int
main(int argc, char*argv[])
{
    // create a scene
    //makeBunnySceneA1();
    //makeSphereSceneA1();
    makeTeapotSceneA1();
    //makeSpiralScene();
    //makeBunny1Scene();
    //makeBunny20Scene();
    //makeSphereScene();
    //makeTeapotScene();
    //makeCornellScene();
    //makeSponzaScene();
    //makeCoolScene();

    MiroWindow miro(&argc, argv);
    miro.mainLoop();

    return 0; // never executed
}

