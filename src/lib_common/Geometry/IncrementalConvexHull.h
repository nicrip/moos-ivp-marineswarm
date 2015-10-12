#ifndef P_INCREMENTAL_CONVEX_HULL_HEADER
#define P_INCREMENTAL_CONVEX_HULL_HEADER

#include <list>
#include <vector>
#include "XYPoint.h"
#include "XYPolygon.h"

class IncrementalConvexHull
{
 public:
  struct Node
  {
    XYPoint pt;
    int id;
    struct Node *next;
    struct Node *prev;
  };

  IncrementalConvexHull();
  ~IncrementalConvexHull() {};

   bool addPoint(double, double);
   XYPolygon getConvexHullAsPolygon();
   void printPoints();
   int getNumPoints();
   void deleteAllPoints();

 protected:
   void updateConvexHull(double, double);
   bool contains(double x, double y);
   int side(double x1, double y1, double x2, double y2, double x3, double y3);
   void setTangents(double x, double y);

 private: // Configuration variables

 private: // State variables
   int                  m_num_points;
   int                  m_order_id;
   Node                 *m_base_node;
   Node                 *m_curr_node;
   Node                 *m_left_tangent_node;
   Node                 *m_right_tangent_node;
};

#endif
