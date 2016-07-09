#ifndef CSE168_RAY_H_INCLUDED
#define CSE168_RAY_H_INCLUDED

#include "Vector3.h"

class Ray
{
public:
    Vector3 o,      //!< Origin of ray
            d,      //!< Direction of ray
        inv_d;      //!< Reversed direction of ray
    int times;
    int sgn[3];

    Ray() : o(), d(Vector3(0.0f,0.0f,1.0f)), inv_d(Vector3(0.f,0.f,1.f)), times(0)
    {
        // empty
        sgn[0] = (inv_d.x < 0);
        sgn[1] = (inv_d.y < 0);
        sgn[2] = (inv_d.z < 0);
    }

    Ray(const Vector3& o, const Vector3& d) : o(o), d(d), times(0)
    {
        // empty
        inv_d = Vector3(1 / d.x, 1 / d.y, 1 / d.z);
        sgn[0] = (inv_d.x < 0);
        sgn[1] = (inv_d.y < 0);
        sgn[2] = (inv_d.z < 0);
    }
};


//! Contains information about a ray hit with a surface.
/*!
    HitInfos are used by object intersection routines. They are useful in
    order to return more than just the hit distance.
*/
class HitInfo
{
public:
    float t;                            //!< The hit distance
    Vector3 P;                          //!< The hit point
    Vector3 N;                          //!< Shading normal vector
    const Material* material;           //!< Material of the intersected object

    //! Default constructor.
    explicit HitInfo(float t = 0.0f,
                     const Vector3& P = Vector3(),
                     const Vector3& N = Vector3(0.0f, 1.0f, 0.0f)) :
        t(t), P(P), N(N), material (0)
    {
        // empty
    }
};

#endif // CSE168_RAY_H_INCLUDED
