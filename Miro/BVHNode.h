#ifndef CSE168_BVHNODE_H_INCLUDED
#define CSE168_BVHNODE_H_INCLUDED

#include "Object.h"
#include "Ray.h"
#include <vector>

using namespace std;

struct Stats {
  int LEAF = 0;
  int NODE = 0;
  int TRI = 0;
  int HIT_NODE = 0;
  int HIT_LEAF = 0;
  int HIT_TRI = 0;
  int RAY_TRI = 0;
  int RAY_BOX = 0;
};

class BVHNode {
public:
  int rb_int = 0;
  bool tag = false;

  BVHNode();
  BVHNode(Objects*);

  bool hit(const Ray&, float, float) const;
  bool isLeaf() const {return children.empty();}

  bool split();
  void setBounds();

  float gettMin(const Ray&);

  Objects* objs;
  vector<BVHNode*> children;

  void getStats(Stats&);

  Vector3 bound[2];

protected:
  Vector3 cents[2];

  void count(Stats&, BVHNode*, BVHNode*);
};

#endif