#ifndef CSE168_LAMBERT_H_INCLUDED
#define CSE168_LAMBERT_H_INCLUDED

#include "Material.h"

class Lambert : public Material
{
public:
    Lambert(const Vector3 & kd = Vector3(1), 
            const Vector3 & ka = Vector3(0), 
            const Vector3 & kr = Vector3(0), 
            const Vector3 & ks = Vector3(0), 
            const Vector3 & kt = Vector3(0), 
            float n = 0.f, float s = 0.f, float rInd = 0.f);
    virtual ~Lambert();

    const Vector3 & kd() const {return m_kd;}
    const Vector3 & ka() const {return m_ka;}

    void setKd(const Vector3 & kd) {m_kd = kd;}
    void setKa(const Vector3 & ka) {m_ka = ka;}

    virtual Vector3 phade(const Ray&, const HitInfo&, const Scene&, const PhotonMap&, const PhotonMap&) const;
    Vector3 phoFlect(const Ray&, const HitInfo&, const Scene&, const PhotonMap&, const PhotonMap&) const;
    Vector3 phoFract(const Ray&, const HitInfo&, const Scene&, const PhotonMap&, const PhotonMap&) const;

    virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

protected:
    float m_noisiness;
    Vector3 m_kd;
    Vector3 m_ka;
    Vector3 m_kr, m_ks, m_kt;
};

#endif // CSE168_LAMBERT_H_INCLUDED