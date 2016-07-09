#ifndef CSE168_MATERIAL_H_INCLUDED
#define CSE168_MATERIAL_H_INCLUDED

#include "Miro.h"
#include "Vector3.h"
#include "PhotonMap.h"

class Material
{
public:
    Material();
    virtual ~Material();

    virtual void preCalc() {}
    
    virtual Vector3 phade(const Ray&, const HitInfo&, const Scene&, const PhotonMap&, const PhotonMap&) const;
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

    float m_refInd, f_diff, f_spec, f_trans;
    Vector3 m_diff, m_spec, m_trans, m_bient;
};

#endif // CSE168_MATERIAL_H_INCLUDED
