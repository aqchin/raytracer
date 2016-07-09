#include "Lambert.h"
#include "Perlin.h"
#include "Ray.h"
#include "Scene.h"
#include "Worley.h"
#include <algorithm>
#include <time.h>

#define EPSILON 0.0001
#define SPECULAR_CONST 420
#define ENV_INDEX 1.00029
#define BOUNCES 2
#define DO_BOUNCE false
#define DO_PPM true
#define M_PI 3.14159265358979

Vector3 hemisphereSample_cos(float u, float v) {
  float phi = v * 2.0 * PI;
  float cosTheta = sqrt(1.0 - u);
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
  return Vector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, 
                 const Vector3 & kr, const Vector3 & ks, const Vector3 & kt, float n, float s, float r) :
    m_kd(kd), m_ka(ka), m_kr(kr), m_ks(ks), m_kt(kt), m_noisiness(n)
{
  m_diff = kd;
  m_spec = ks;
  m_bient = ka;
  m_trans = kt;
  m_refInd = r;

  m_trans.x = max(min(m_trans.x, 1.f - m_spec.x), 0.f);
  m_trans.y = max(min(m_trans.y, 1.f - m_spec.y), 0.f);
  m_trans.z = max(min(m_trans.z, 1.f - m_spec.z), 0.f);
  m_diff.x = max(min(m_diff.x, 1.f - m_spec.x - m_trans.x), 0.f);
  m_diff.y = max(min(m_diff.y, 1.f - m_spec.y - m_trans.y), 0.f);
  m_diff.z = max(min(m_diff.z, 1.f - m_spec.z - m_trans.z), 0.f);

  f_diff = (m_diff.x + m_diff.y + m_diff.z) /3.f;
  f_spec = (m_spec.x + m_spec.y + m_spec.z) / 3.f;
  f_trans = (m_trans.x + m_trans.y + m_trans.z) / 3.f;

  f_trans = max(min(f_trans, 1.f - f_spec), 0.f);
  f_diff = max(min(f_diff, 1.f - f_spec - f_trans), 0.f);
}

Lambert::~Lambert()
{
}

Vector3
Lambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const
{
    Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
    
    const Vector3 viewDir = -ray.d; // d is a unit vector
    
    const Lights *lightlist = scene.lights();

    // reflectance
    if (m_ks != 0 && ray.times <3) {
      Vector3 Wr = -2 * dot(ray.d, hit.N) * hit.N + ray.d;
      Wr.normalize();
      Ray r(hit.P + (EPSILON * Wr), Wr);
      HitInfo hi;
      r.times = ray.times + 1;

      if(scene.trace(hi, r))
          L += m_spec * m_ks * hi.material->shade(r, hi, scene);
    }

    // cellular noise texture
    if(m_noisiness > 0) {
      float at[3] = { hit.P.x, hit.P.y, hit.P.z };
      const long mO = 3;
      float F[mO];
      float delta[mO][3];
      unsigned long *ID = new unsigned long();

      WorleyNoise::noise3D(at, mO, F, delta, ID);
      L += m_noisiness * (0.5f * (F[2] - F[1]));// + PerlinNoise::noise(at[0], at[1], at[2]));
    }

    // loop over all of the lights
    Lights::const_iterator lightIter;
    for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
    {
        PointLight* pLight = *lightIter;
    
        Vector3 l = pLight->position() - hit.P;
        
        // the inverse-squared falloff
        float falloff = l.length2();
        
        // normalize the light direction
        l /= sqrt(falloff);

        // get the diffuse component
        float nDotL = dot(hit.N, l);
        Vector3 result = pLight->color();
        result *= f_diff * m_kd;
        
        L += std::max(0.0f, nDotL/falloff * pLight->wattage() / PI) * result;

        // highlights
        //if (m_ks != 0)
        L += m_spec * pLight->color() * m_ks * max(0.f, pow(dot(viewDir, l), SPECULAR_CONST));
          // / dot(hit.N, l);
    }

    // refraction
    if (m_trans != 0 && ray.times <3) {
      float n = (ray.times % 2 == 0) ? (ENV_INDEX / m_refInd) : (m_refInd / ENV_INDEX);
      float wn = dot(viewDir, hit.N);
      if (wn < 0) wn = -wn;

      Vector3 Wt = -1 * n * (viewDir - wn * hit.N) -
        sqrtf(1 - (n * n) * (1 - wn * wn)) * hit.N;
      Wt.normalize();

      Ray r(hit.P + (EPSILON * Wt), Wt);
      r.times = ray.times + 1;
      HitInfo hi;

      if (scene.trace(hi, r))
        L += m_trans * m_kt * hi.material->shade(r, hi, scene);
    }
    
    // add the ambient component
    L += m_ka;

    if(DO_BOUNCE && ray.times < BOUNCES) {
      float v = rand() / (float)RAND_MAX;
      float u = rand() / (float)RAND_MAX;
      Vector3 coord = hemisphereSample_cos(u,v);

      Vector3 unv = Vector3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
      Vector3 v1 = cross(hit.N, unv);
      Vector3 v2 = cross(v1, hit.N);
      Vector3 dir = coord.x * v1 + coord.z * hit.N + coord.y * v2;
      dir.normalize();
      Ray r(hit.P, dir);
      r.times = ray.times + 1;
      HitInfo hi;

      if(scene.trace(hi, r))
        L+= m_kd * dot(r.d, hit.N) * hit.material->shade(r, hi, scene);
    }
    
    return L;
}

Vector3 Lambert::phade(const Ray& r, const HitInfo& h, const Scene& s, const PhotonMap& p, const PhotonMap& c) const {

  Vector3 L = Vector3(0.f);

  float pos[3] = {h.P.x, h.P.y, h.P.z};
  float nor[3] = {h.N.x, h.N.y, h.N.z};
  float irad[3], caus[3];

  p.irradiance_estimate(irad, pos, nor, .3f, 1000);
  c.irradiance_estimate(caus, pos, nor, 2.f, 10);


  if(f_diff > 0) L += m_diff * Vector3(irad[0], irad[1], irad[2])
    + Vector3(caus[0], caus[1], caus[2]);
  if(f_spec > 0) L += m_spec * phoFlect(r, h, s, p, c);
  if(f_trans > 0) L += m_trans *phoFract(r, h, s, p, c);

  L += m_ka;

  return L;
}

Vector3 Lambert::phoFlect(const Ray& r, const HitInfo& h, const Scene& s, const PhotonMap& p, const PhotonMap& c) const {
  Vector3 L;
  if(r.times <3) {
    Vector3 Wr = -2 * dot(r.d, h.N) * h.N + r.d;
    Wr.normalize();
    Ray r_n(h.P + (EPSILON * Wr), Wr);
    HitInfo hi;
    r_n.times = r.times + 1;

    if (s.trace(hi, r))
      L += m_ks * hi.material->phade(r_n, hi, s, p, c);
  } 
  return L;
}

Vector3 Lambert::phoFract(const Ray& r, const HitInfo& h, const Scene& s, const PhotonMap& p, const PhotonMap& c) const {
  Vector3 L;
  if (r.times <3) {
    Vector3 viewDir = -r.d;
    float n = (r.times % 2 == 0) ? (ENV_INDEX / m_refInd) : (m_refInd / ENV_INDEX);
    float wn = dot(viewDir, h.N);
    if (wn < 0) wn = -wn;

    Vector3 Wt = -n * (viewDir - wn * h.N) -
      sqrtf(1 - (n * n) * (1 - wn * wn)) * h.N;
    Wt.normalize();

    Ray r_n(h.P + (EPSILON * Wt), Wt);
    r_n.times = r.times + 1;
    HitInfo hi;

    if (s.trace(hi, r_n))
      L += m_kt * hi.material->phade(r_n, hi, s, p, c);
  }
  return L;
}