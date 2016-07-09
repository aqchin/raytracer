#include "Triangle.h"
#include "TriangleMesh.h"
#include "Ray.h"

#define EPSILON 0.000001

Triangle::Triangle(TriangleMesh * m, unsigned int i) :
    m_mesh(m), m_index(i)
{
}


Triangle::~Triangle()
{

}

void Triangle::setMesh(TriangleMesh* m) {
  m_mesh = m;

  TriangleMesh::TupleI3 vInd = m_mesh->vIndices()[m_index];
  Vector3 c =
    (m_mesh->vertices()[vInd.x] +
    m_mesh->vertices()[vInd.y] +
    m_mesh->vertices()[vInd.z]) / 3.f;
  cent[0] = c.x;
  cent[1] = c.y;
  cent[2] = c.z;
}


void
Triangle::renderGL()
{
    TriangleMesh::TupleI3 ti3 = m_mesh->vIndices()[m_index];
    const Vector3 & v0 = m_mesh->vertices()[ti3.x]; //vertex a of triangle
    const Vector3 & v1 = m_mesh->vertices()[ti3.y]; //vertex b of triangle
    const Vector3 & v2 = m_mesh->vertices()[ti3.z]; //vertex c of triangle

    glBegin(GL_TRIANGLES);
        glVertex3f(v0.x, v0.y, v0.z);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
    glEnd();
}



bool
Triangle::intersect(HitInfo& result, const Ray& r,float tMin, float tMax)
{
    /*
     * Credit for super fast algorithm for my slow potato toaster goes to Moller-Trumbore
     * http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
     */
    TriangleMesh::TupleI3 vInd = m_mesh->vIndices()[m_index];
    Vector3 v_[3] = {
      m_mesh->vertices()[vInd.x],
      m_mesh->vertices()[vInd.y],
      m_mesh->vertices()[vInd.z] };

    Vector3 e1 = v_[1] - v_[0];
    Vector3 e2 = v_[2] - v_[0];

    Vector3 P = cross(r.d, e2);
    float det = dot(e1, P);

    // check for zero determinant
    if(det > -EPSILON && det < EPSILON) return false;

    float inv_det = 1.f / det;
    Vector3 T = r.o - v_[0];
    float u = dot(T, P) * inv_det;

    // check if u is in valid range
    if(u < 0.f || u > 1.f) return false;

    Vector3 Q = cross(T, e1);
    float v = dot(r.d, Q) * inv_det;

    // check if v is in valid range
    if(v < 0.f || u + v > 1.f) return false;

    float t = dot(e2, Q) * inv_det;

    if(t > EPSILON && (t < tMax && t > tMin)) {
      TriangleMesh::TupleI3 nInd = m_mesh->nIndices()[m_index];
      float w = (1.f - u - v);
      result.material = this->m_material;
      Vector3 N =
        w * m_mesh->normals()[nInd.x] +
        u * m_mesh->normals()[nInd.y] +
        v * m_mesh->normals()[nInd.z];
      N.normalize();
      result.N = N;
      //result.P = w * v_[0] + u * v_[1] + v * v_[2];
      result.P = r.o + r.d * t;
      result.t = t;

      return true;
    }

    return false;
}

Vector3 Triangle::getMin() {

  TriangleMesh::TupleI3 vInd = m_mesh->vIndices()[m_index];
  Vector3 v_[3] = {
    m_mesh->vertices()[vInd.x],
    m_mesh->vertices()[vInd.y],
    m_mesh->vertices()[vInd.z] };

  Vector3 v = Vector3(
    (v_[0].x < v_[1].x) ? v_[0].x : v_[1].x,
    (v_[0].y < v_[1].y) ? v_[0].y : v_[1].y,
    (v_[0].z < v_[1].z) ? v_[0].z : v_[1].z );

  return Vector3(
    (v.x < v_[2].x) ? v.x : v_[2].x,
    (v.y < v_[2].y) ? v.y : v_[2].y,
    (v.z < v_[2].z) ? v.z : v_[2].z );
}

Vector3 Triangle::getMax() {

  TriangleMesh::TupleI3 vInd = m_mesh->vIndices()[m_index];
  Vector3 v_[3] = {
    m_mesh->vertices()[vInd.x],
    m_mesh->vertices()[vInd.y],
    m_mesh->vertices()[vInd.z] };

  Vector3 v = Vector3(
    (v_[0].x < v_[1].x) ? v_[1].x : v_[0].x,
    (v_[0].y < v_[1].y) ? v_[1].y : v_[0].y,
    (v_[0].z < v_[1].z) ? v_[1].z : v_[0].z );

  return Vector3(
    (v.x < v_[2].x) ? v_[2].x : v.x,
    (v.y < v_[2].y) ? v_[2].y : v.y,
    (v.z < v_[2].z) ? v_[2].z : v.z);
}