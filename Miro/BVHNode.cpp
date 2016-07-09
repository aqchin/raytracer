#include "BVHNode.h"
#include "Console.h"
#include <algorithm>

using namespace std;

#define LEAF_TRI 8
#define SPLIT_SAMPLES 10

BVHNode::BVHNode(Objects * o): objs(o) {
  setBounds();
}

bool BVHNode::split() {
  if(objs->size() > LEAF_TRI) {

    float cost[3][SPLIT_SAMPLES]; // [axis(x0, y1, z2)][splits(1..n)]
    int bin_samples = SPLIT_SAMPLES+1;
    float mic[3] = { cents[0].x, cents[0].y, cents[0].z };
    //debug("%f %f %f\n", mic[0], mic[1], mic[2]);

    // Test all split possibilities
    for (int ax = 0; ax < 3; ax++) {
      float c[3] =
      { (cents[1].x - cents[0].x),
        (cents[1].y - cents[0].y),
        (cents[1].z - cents[0].z) };

      float cax = c[ax] /(float)bin_samples;

      for(int seg = 1; seg < bin_samples; seg++) {
        int N1o = 0;
        int N2o = 0;

        for(size_t i = 0; i < (*objs).size(); i++) {
          if((*objs)[i]->cent[ax] <= (mic[ax] + seg*cax)) N1o++;
          else N2o++;
        }
        //debug("%d %f %f % d\n", ax, mic[ax], (mic[ax] + seg*cax), seg);

        c[ax] = cax*seg;
        float SA1 = 2*(c[0]*c[1] + c[0]*c[2] + c[1]*c[2]);
        float SA2;
        if(bin_samples-seg != seg) {
          c[ax] = cax*(bin_samples - seg);
          SA2 = 2*(c[0]*c[1] + c[0]*c[2] + c[1]*c[2]);
        }
        else SA2 = SA1;

        float C1v = SA1 * (float)N1o;
        float C2v = SA2 * (float)N2o;

        cost[ax][seg-1] = C1v + C2v;// + Co * (P1v*N1o + P2v*N2o);
        //debug("%f %f | %d %d | %f\n", SA1, SA2, N1o, N2o, cost[ax][seg - 1]);
      }
    }

    // Get best cost
    int best_axis = 0;
    int best_split = 0;
    float best_cost = cost[0][0];
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < SPLIT_SAMPLES; j++) {
        if(cost[i][j] < best_cost) {
          best_axis = i;
          best_split = j;
          best_cost = cost[i][j];
        }
      }
    }

    Objects* obj1 = new Objects();
    Objects* obj2 = new Objects();
    //children.push_back(new BVHNode(new Objects()));
    //children.push_back(new BVHNode(new Objects()));

    float c[3] =
    { (cents[1].x - cents[0].x),
      (cents[1].y - cents[0].y),
      (cents[1].z - cents[0].z) };
    float dsplit = mic[best_axis] + c[best_axis] * (best_split+1) / bin_samples;

    for(size_t i = 0; i < (*objs).size(); i++) {
      if((*objs)[i]->cent[best_axis] <= dsplit) 
        obj1->push_back((*objs)[i]);
      else obj2->push_back((*objs)[i]);
    }

    children.push_back(new BVHNode(obj1));
    children.push_back(new BVHNode(obj2));

    //delete this->objs;
    this->objs = nullptr;

    return(children[0]->split() || children[1]->split());
  }
  return false;
}

void BVHNode::setBounds() {
  Vector3 min, max, cmin, cmax;
  if (!objs->empty()) {
    min = (*objs)[0]->getMin();
    max = (*objs)[0]->getMax();
    cmin = (*objs)[0]->centroid();
    cmax = (*objs)[0]->centroid();

    for (int i = 1; i < (*objs).size(); i++) {
      Vector3 t_min = (*objs)[i]->getMin();
      Vector3 t_max = (*objs)[i]->getMax();
      Vector3 t_cen = (*objs)[i]->centroid();

      if (t_cen.x < cmin.x) cmin.x = t_cen.x;
      else if (t_cen.x > cmax.x) cmax.x = t_cen.x;
      if (t_cen.y < cmin.y) cmin.y = t_cen.y;
      else if (t_cen.y > cmax.y) cmax.y = t_cen.y;
      if (t_cen.z < cmin.z) cmin.z = t_cen.z;
      else if (t_cen.z > cmax.z) cmax.z = t_cen.z;

      if (t_min.x < min.x) min.x = t_min.x;
      if (t_max.x > max.x) max.x = t_max.x;
      if (t_min.y < min.y) min.y = t_min.y;
      if (t_max.y > max.y) max.y = t_max.y;
      if (t_min.z < min.z) min.z = t_min.z;
      if (t_max.z > max.z) max.z = t_max.z;
    }
  }

  bound[0] = min;
  bound[1] = max;
  cents[0] = cmin;
  cents[1] = cmax;
  //debug("min %f %f %f | max %f %f %f\n", min.x, min.y, min.z, max.x, max.y, max.z);
}

bool BVHNode::hit(const Ray &ray, float tMin, float tMax) const {
  /*
   * Credit for optimized ray-box intersection algorithm
   * http://www.cs.utah.edu/~awilliam/box/box.pdf
   */
  float t_mi[3], t_ma[3];

  t_mi[0] = (bound[ray.sgn[0]].x - ray.o.x) * ray.inv_d.x;
  t_ma[0] = (bound[1 - ray.sgn[0]].x - ray.o.x) * ray.inv_d.x;
  
  t_mi[1] = (bound[ray.sgn[1]].y - ray.o.y) * ray.inv_d.y;
  t_ma[1] = (bound[1 - ray.sgn[1]].y - ray.o.y) * ray.inv_d.y;

  if ((t_mi[0]>t_ma[1]) || (t_mi[1]>t_ma[0])) return false;
  if (t_mi[1]>t_mi[0]) t_mi[0] = t_mi[1];
  if (t_ma[1]<t_ma[0]) t_ma[0] = t_ma[1];

  t_mi[2] = (bound[ray.sgn[2]].z - ray.o.z) * ray.inv_d.z;
  t_ma[2] = (bound[1 - ray.sgn[2]].z - ray.o.z) * ray.inv_d.z;

  if ((t_mi[0]>t_ma[2] || (t_mi[2]>t_ma[0]))) return false;
  if (t_mi[2]>t_mi[0]) t_mi[0] = t_mi[2];
  if (t_ma[2]<t_ma[0]) t_ma[0] = t_mi[2];

  return((t_mi[0]<tMax) || (t_ma[0]>tMin));
}

float BVHNode::gettMin(const Ray &ray) {
  float t_mi[3], t_ma[3];

  t_mi[0] = (bound[ray.sgn[0]].x - ray.o.x) * ray.inv_d.x;
  t_ma[0] = (bound[1 - ray.sgn[0]].x - ray.o.x) * ray.inv_d.x;

  t_mi[1] = (bound[ray.sgn[1]].y - ray.o.y) * ray.inv_d.y;
  t_ma[1] = (bound[1 - ray.sgn[1]].y - ray.o.y) * ray.inv_d.y;

  t_mi[2] = (bound[ray.sgn[2]].z - ray.o.z) * ray.inv_d.z;
  t_ma[2] = (bound[1 - ray.sgn[2]].z - ray.o.z) * ray.inv_d.z;

  return max(max(min(t_mi[0], t_ma[0]), min(t_mi[1], t_ma[1])), min(t_mi[2], t_ma[2]));
}

void BVHNode::getStats(Stats& s) {
  if(this->isLeaf()) {
    s.LEAF++;
    if(tag) s.HIT_LEAF++;
    s.TRI += (int)objs->size();
    for (size_t i = 0; i < objs->size(); i++) {
      if ((*objs)[i]->hit) s.HIT_TRI++;
      s.RAY_TRI += (*objs)[i]->hits;
      (*objs)[i]->hits = 0;
    }
  }
  else {
    s.RAY_BOX += rb_int;
    rb_int = 0;
    s.NODE++;
    if(tag) s.HIT_NODE++;
    count(s, children[0], children[1]);
  }
}
void BVHNode::count(Stats& s, BVHNode* c0, BVHNode* c1) {
  if(c0->isLeaf()) {
    s.LEAF++;
    if(tag) s.HIT_LEAF++;
    s.TRI += (int)c0->objs->size();
    for(size_t i = 0; i < c0->objs->size(); i++) {
      if((*c0->objs)[i]->hit) s.HIT_TRI++;
      s.RAY_TRI += (*c0->objs)[i]->hits;
      (*c0->objs)[i]->hits = 0;
    }
  }
  else {
    s.NODE++;
    if(tag) s.HIT_NODE++;
    count(s, c0->children[0], c0->children[1]);
    s.RAY_BOX += rb_int;
    rb_int = 0;
  }
  if(c1->isLeaf()) {
    s.LEAF++;
    if(tag) s.HIT_LEAF++;
    s.TRI += (int)c1->objs->size();
    for (size_t i = 0; i < c1->objs->size(); i++) {
      if ((*c1->objs)[i]->hit) s.HIT_TRI++;
      s.RAY_TRI += (*c1->objs)[i]->hits;
      (*c1->objs)[i]->hits = 0;
    }
  }
  else {
    s.NODE++;
    if(tag) s.HIT_NODE++;
    count(s, c1->children[0], c1->children[1]);
    s.RAY_BOX += rb_int;
    rb_int = 0;
  }
}