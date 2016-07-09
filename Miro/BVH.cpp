#include "BVH.h"
#include "Ray.h"
#include "Console.h"
#include <algorithm>
#include <iterator>

using namespace std;

void
BVH::build(Objects * objs)
{
    // construct the bounding volume hierarchy
    debug("Building BVH... ");
    m_objects = objs;
    bvh_root = new BVHNode(objs);
    bvh_root->split();
    printf("done.\n\n");
}

bool BVH::nodeIntersect(BVHNode* bvhn, HitInfo& minHit, const Ray& ray, float tMin, float tMax) const {
  
  bool hit = false;
  if (bvhn->hit(ray, tMin, tMax)) {
    if (bvhn->isLeaf()) {
      HitInfo tempMinHit;
      minHit.t = MIRO_TMAX;

      for (size_t i = 0; i < bvhn->objs->size(); ++i) {
        if ((*bvhn->objs)[i]->intersect(tempMinHit, ray, tMin, tMax)) {
          (*bvhn->objs)[i]->hits++;
          hit = true;
          if (tempMinHit.t < minHit.t) minHit = tempMinHit;
        }
      }
      return hit;
    }
    else if ((bvhn->children)[0]->hit(ray, tMin, tMax) ||
      (bvhn->children)[1]->hit(ray, tMin, tMax)) {
      bvhn->rb_int++;

      HitInfo h[2];
      bool hit[2];

      for (int i = 0; i < 2; i++)
        hit[i] = nodeIntersect((bvhn->children)[i], h[i], ray, tMin, tMax);

      if (hit[0] && hit[1]) {
        if (h[0].t < h[1].t) minHit = h[0];
        else minHit = h[1];
      }
      else if (hit[0]) {
        minHit = h[0];
      }
      else if (hit[1]) {
        minHit = h[1];
      }

      return (hit[0] || hit[1]);
    }
  }
  return false;
}

bool
BVH::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
    // Here you would need to traverse the BVH to perform ray-intersection
    // acceleration. For now we just intersect every object.

  return nodeIntersect(bvh_root, minHit, ray, tMin, tMax);
}