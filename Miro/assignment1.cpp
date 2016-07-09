#include "assignment1.h"
#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"

#include "PointLight.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "Lambert.h"

#define DIAMOND_IN 2.417
#define GLASS_INL 1.52
#define GLASS_INH 1.65
#define ICE_IN 1.31
#define STP_AIR_IN 1.00029
#define WATER_IN 1.33
#define VACUUM_IN 1.00000

void
makeBunnySceneA1()
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

    Material* mat = new Lambert(Vector3(1.f, 0.f, 1.f), Vector3(0.f),
      Vector3(1.f), Vector3(0.f), Vector3(0.f), 0.f, 1.f);
    Material* f_mat = new Lambert(Vector3(0.f, 1.f, 0.f), Vector3(0.f),
      Vector3(.5f), Vector3(1.f), Vector3(1.f), 0.f, 1.f);

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
    t->setMaterial(f_mat); 
    g_scene->addObject(t);
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
makeSphereSceneA1()
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

  Material* mat = new Lambert(Vector3(1.f, 1.f, 0.f), Vector3(0.f),
    Vector3(1.f), Vector3(0.f), Vector3(0.f), 0.f, 0.f);
  Material* f_mat = new Lambert(Vector3(0.f, 1.f, 0.f), Vector3(0.f),
    Vector3(.5f), Vector3(1.f), Vector3(1.f), 1.f, 1.f);

  /*
  Material* mat = new Lambert(Vector3(1.f, 1.f, 0.f), Vector3(0.f),
    Vector3(1.f), Vector3(0.f), Vector3(0.f), 0.f, 0.f, DIAMOND_IN);
  Material* f_mat = new Lambert(Vector3(0.f, 1.f, 0.f), Vector3(0.f),
    Vector3(.5f), Vector3(1.f), Vector3(1.f), 0.f, 0.f, WATER_IN);
  */

  TriangleMesh * sphere = new TriangleMesh;
  sphere->load("sphere_nice.obj");

  // create all the triangles in the bunny mesh and add to the scene
  for (int i = 0; i < sphere->numTris(); ++i)
  {
    Triangle* t = new Triangle;
    t->setIndex(i);
    t->setMesh(sphere);
    t->setMaterial(mat);
    g_scene->addObject(t);
  }

  // create the floor triangle
  TriangleMesh * floor = new TriangleMesh;
  floor->createSingleTriangle();
  floor->setV1(Vector3(0, -1, 10));
  floor->setV2(Vector3(10, -1, -10));
  floor->setV3(Vector3(-10, -1, -10));
  floor->setN1(Vector3(0, 1, 0));
  floor->setN2(Vector3(0, 1, 0));
  floor->setN3(Vector3(0, 1, 0));

  Triangle* t = new Triangle;
  t->setIndex(0);
  t->setMesh(floor);
  t->setMaterial(f_mat);
  g_scene->addObject(t);

  // let objects do pre-calculations if needed
  g_scene->preCalc();
}

void
makeTeapotSceneA1()
{
  g_camera = new Camera;
  g_scene = new Scene;
  g_image = new Image;

  g_image->resize(512, 512);
  
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
  light->setWattage(700);
  g_scene->addLight(light);
  
  Material* mat = new Lambert(Vector3(1.f, .7f, 0.f), Vector3(0.f),
    Vector3(0.f), Vector3(.5f), Vector3(0.f), 0.f, 1.f, 0.f);
  Material* f_mat = new Lambert(Vector3(0.f, 0.f, 1.f), Vector3(0.f),
    Vector3(.7f), Vector3(0.f), Vector3(0.f), 0.f, 0.f, 0.f);

  /*
  Material* mat = new Lambert(Vector3(1.f, 1.f, 0.f), Vector3(0.f),
    Vector3(0.f), Vector3(0.f), Vector3(1.f), 0.f, 0.f, GLASS_INH);
  Material* f_mat = new Lambert(Vector3(1.f, 0.f, 1.f), Vector3(0.f),
    Vector3(0.f), Vector3(0.f), Vector3(0.f), 1.f, 0.f, 0.f);
  */

  /*
  Material* mat = new Lambert(Vector3(0.f, 0.f, 1.f), Vector3(0.f), 
    Vector3(0.f, 1.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f), 1.f, 0.f, ICE_IN);
  Material* f_mat = new Lambert(Vector3(1.f, 0.f, 0.f), Vector3(0.f),
    Vector3(1.f), Vector3(1.f), Vector3(0.f), 0.f, 1.f, 0.f);
  */

  TriangleMesh * tpot = new TriangleMesh;
  tpot->load("teapot.obj");

  // create all the triangles in the bunny mesh and add to the scene
  for (int i = 0; i < tpot->numTris(); ++i)
  {
    Triangle* t = new Triangle;
    t->setIndex(i);
    t->setMesh(tpot);
    t->setMaterial(mat);
    g_scene->addObject(t);
  }

  // create the floor triangle
  TriangleMesh * floor = new TriangleMesh;
  floor->createSingleTriangle();
  floor->setV1(Vector3(0, 0, 10));
  floor->setV2(Vector3(10, 0, -10));
  floor->setV3(Vector3(-10, 0, -10));
  floor->setN1(Vector3(0, 1, 0));
  floor->setN2(Vector3(0, 1, 0));
  floor->setN3(Vector3(0, 1, 0));

  Triangle* t = new Triangle;
  t->setIndex(0);
  t->setMesh(floor);
  t->setMaterial(f_mat);
  g_scene->addObject(t);

  // let objects do pre-calculations if needed
  g_scene->preCalc();
}