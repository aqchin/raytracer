#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include <time.h>

Scene * g_scene = 0;

#define SAMPLES 16
#define DO_SAMPLE false
#define DO_PM false
#define DO_CAUSTICS false
#define ENV_INDEX 1.00029
#define SHADOWS false
#define EPSILON 0.0001
#define M_PI 3.14159265358979

Vector3 sphereSampleDir() {
  float x, y, z;
  do {
    x = 2.f*(rand()/(float)RAND_MAX) - 1.f;
    y = 2.f*(rand()/(float)RAND_MAX) - 1.f;
    z = 2.f*(rand()/(float)RAND_MAX) - 1.f;
  } while (x*x + y*y + z*z > 1.f);

  return Vector3(x, y, z).normalize();
}

Vector3 hemiSample_cos(float u, float v) {
  float phi = v * 2.0 * PI;
  float cosTheta = sqrt(1.0 - u);
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
  return Vector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

void
Scene::openGL(Camera *cam)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cam->drawGL();

    // draw objects
    for (size_t i = 0; i < m_objects.size(); ++i)
        m_objects[i]->renderGL();

    m_caustic_pm->drawMap();

    glutSwapBuffers();
}

void
Scene::preCalc()
{
    m_global_pm = new PhotonMap(GLOBAL_PPL * m_lights.size());
    m_caustic_pm = new PhotonMap(GLOBAL_PPL * m_lights.size());

    Objects::iterator it;
    for (it = m_objects.begin(); it != m_objects.end(); it++)
    {
        Object* pObject = *it;
        pObject->preCalc();
    }
    Lights::iterator lit;
    for (lit = m_lights.begin(); lit != m_lights.end(); lit++)
    {
        PointLight* pLight = *lit;
        pLight->preCalc();
    }

    m_bvh.build(&m_objects);
    if(DO_PM) mapPhotons();
    if(DO_CAUSTICS) mapCaustics();
}

void
Scene::raytraceImage(Camera *cam, Image *img)
{
    clock_t time = clock();
    int progress = 0;
    int tru_sample = 1;

    if(DO_SAMPLE) tru_sample = SAMPLES;
    
    // loop over all pixels in the image
    for (int j = 0; j < img->height(); ++j)
    {
        #pragma omp parallel for schedule(dynamic, 2)
        for (int i = 0; i < img->width(); ++i)
        {
            Vector3 shadeResult = Vector3(0.f);

            for(int k = 0; k < tru_sample; ++k) {
              HitInfo hitInfo;
              Ray ray = cam->eyeRay(i, j, img->width(), img->height());
              //float irad[3];
              if (trace(hitInfo, ray))
              {
                if(DO_PM) shadeResult += hitInfo.material->phade(ray, hitInfo, *this, *m_global_pm, *m_caustic_pm);
                else shadeResult += hitInfo.material->shade(ray, hitInfo, *this);
                  /*
                  float pos[3] = {hitInfo.P.x, hitInfo.P.y, hitInfo.P.z};
                  float nor[3] = {hitInfo.N.x, hitInfo.N.y, hitInfo.N.z};
                  m_global_pm->irradiance_estimate(irad, pos, nor, .5f, 500);
                  //cout << irad[0] << " " << irad[1] << " " << irad[2] << endl;
                  if(hitInfo.material->f_diff > 0)
                    shadeResult += hitInfo.material->m_diff * Vector3(irad[0], irad[1], irad[2]);

                  if(hitInfo.material->f_spec > 0)
                    shadeResult += hitInfo.material->m_spec *

                  shadeResult += hitInfo.material->m_bient; */
                if(SHADOWS) {
                  const Lights* ls = lights();
                  for (Lights::const_iterator it = ls->begin(); it != ls->end(); ++it) {
                    Ray r;
                    HitInfo hi;
                    r.d = (*it)->position() - hitInfo.P;
                    r.d.normalize();
                    r.o = hitInfo.P + (r.d * EPSILON);
                    if (trace(hi, r)) {
                      shadeResult = Vector3(0.f);
                      break;
                    }
                  }
                }
              }
            }
            if(DO_SAMPLE) shadeResult /= SAMPLES;
            img->setPixel(i, j, shadeResult);
        }
        img->drawScanline(j);
        glFinish();
        printf("Rendering Progress: %.3f%%\r", (++progress) / float(img->height())*100.0f);
        //printf("Rendering Progress: %.3f%%\r", j/float(img->height())*100.0f);
        fflush(stdout);
    }
    printf("Rendering Progress: 100.000%\n");
    debug("done Raytracing! Time: %.3fs\n", (clock() - time)/(float)CLOCKS_PER_SEC);

    Stats stat;
    m_bvh.bvh_root->getStats(stat);
    //printf("Hit (non-leaf) Nodes: %d\n", stat.HIT_NODE);
    //printf("Hit Leaf Nodes: %d\n", stat.HIT_LEAF);
    //printf("Hit Triangles: %d\n", stat.HIT_TRI);
    printf("BVH Nodes: %d (%d non-leaf nodes, %d leaf nodes)\n",
      stat.NODE + stat.LEAF, stat.NODE, stat.LEAF);
    printf("# Rays: %d\n", SAMPLES * img->height() * img->width());
    printf("Ray-box Intersections: %d\n", stat.RAY_BOX);
    printf("Ray-triangle Intersections: %d\n\n", stat.RAY_TRI);

    stat.RAY_TRI = 0;
    stat.RAY_BOX = 0;
}

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}

void Scene::mapPhotons() {
  int totons = 0;
  //int maptons = 0;

  for(int i = 0; i < m_lights.size(); i++) {
    PointLight* pl = m_lights[i];
    int pho = 0;

    while(pho < GLOBAL_PPL) {

      //#pragma omp parallel for
      //for(int i = 0; i < 1000; i++) {

      // Assuming all point lights
      Vector3 dir = sphereSampleDir();
      Ray r(pl->position(), dir);
      HitInfo hi;
      bool rolling = true;
      Vector3 pv = pl->wattage() * pl->color();

      while(rolling) { 
          
        //#pragma omp critical
        //{
          totons++;
        //}

        if(trace(hi, r)) {
          //cout << "hit" << endl;
          float rr[3];
          float rng = rand()/(float)RAND_MAX;
          rr[0] = hi.material->f_diff; // diffuse
          rr[1] = rr[0] + hi.material->f_spec; // reflection
          rr[2] = rr[1] + hi.material->f_trans; // refraction

          float pos[3] = { hi.P.x, hi.P.y, hi.P.z };
          float dir[3] = { r.d.x, r.d.y, r.d.z };

          if (rng < rr[0]) {
            float v = rand() / (float)RAND_MAX;
            float u = rand() / (float)RAND_MAX;
            Vector3 coord = hemiSample_cos(u, v);

            Vector3 unv = Vector3(rand() / (float)RAND_MAX,
              rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
            Vector3 v1 = cross(hi.N, unv);
            Vector3 v2 = cross(v1, hi.N);
            Vector3 dir = coord.x * v1 + coord.z * hi.N + coord.y * v2;
            dir.normalize();
            Ray d_r(hi.P, dir);
            r = d_r;
            pv *= hi.material->m_diff;
          }
          else if (rng < rr[1]) {
            Vector3 Wr = -2 * dot(r.d, hi.N) * hi.N + r.d;
            Wr.normalize();
            Ray d_r(hi.P + (EPSILON * Wr), Wr);
            r = d_r;

            pv *= hi.material->m_spec;
          }
          else rolling = false;
          //#pragma omp critical
          //{
          //maptons++;
          pho++;
          //}
          float pow[3] = { pv.x, pv.y, pv.z };
          m_global_pm->store(pow, pos, dir);
          //cout << "photon added" << endl;
        }
        else rolling = false;
      }
      //}
    }
  }
  m_global_pm->scale_photon_power(1.f/(float)totons);
  m_global_pm->balance();
  //cout << maptons << " " << totons << endl;
}

void Scene::mapCaustics() {
  int totons = 0;

  for (int i = 0; i < m_lights.size(); i++) {
    PointLight* pl = m_lights[i];
    int pho = 0;

    while (pho < CAUSTIC_PPL) {
      Vector3 dir = sphereSampleDir();
      Ray r(pl->position(), dir);
      HitInfo hi;
      bool rolling = true;
      Vector3 pv = pl->wattage() * pl->color();

      while(rolling) {
        rolling = testCaustic(r, hi, pv, 0, 0, totons, pho);
      }
    }
  }
  //m_caustic_pm->scale_photon_power(1.f/(float)totons);
  m_caustic_pm->balance();
   //cout << totons << endl;
}

bool Scene::testCaustic(Ray& r, HitInfo& h, Vector3 p, int d, int rf, int& tot, int& pho) {

  if(trace(h, r)) {
    float rr[3];
    float rng = rand()/(float)RAND_MAX;
    rr[0] = h.material->f_diff; // diffuse
    rr[1] = rr[0] + h.material->f_spec; // reflection
    rr[2] = rr[1] + h.material->f_trans; // refraction

    float pos[3] = { h.P.x, h.P.y, h.P.z };
    float dir[3] = { r.d.x, r.d.y, r.d.z };

    if(rng < rr[0]) {
      if (d>0 && rf % 2 == 0) {
        float pwr[3] = { p.x, p.y, p.z };
        tot++;
        pho++;
        m_caustic_pm->store(pwr, pos, dir);
        //cout << pos[0] << " found one\n" << endl;
        return false;
      }
    }

    else if (rng < rr[1]) {
      Vector3 Wr = -2 * dot(r.d, h.N) * h.N + r.d;
      Wr.normalize();
      Ray d_r(h.P + (EPSILON * Wr), Wr);

      if(trace(h, d_r))
        testCaustic(d_r, h, p*h.material->m_spec, d+1, rf, tot, pho);
    }

    else {
      Vector3 viewDir = -r.d;
      float n = (rf%2 == 0) ? (ENV_INDEX/h.material->m_refInd) : (h.material->m_refInd/ENV_INDEX);
      float wn = dot(viewDir, h.N);
      if(wn<0) wn = -wn;

      Vector3 Wt = -n *(viewDir -wn *h.N) -sqrtf(1 -(n*n)) *(1 -(wn*wn)) *h.N;
      Wt.normalize();

      Ray r_n(h.P + (EPSILON *Wt), Wt);

      if(trace(h, r_n))
        testCaustic(r_n, h, p*h.material->m_trans, d+1, rf+1, tot, pho);
    }
  }
  return false;
}