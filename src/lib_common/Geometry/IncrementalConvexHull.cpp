#include <iostream>
#include "IncrementalConvexHull.h"
#include "GeomUtils.h"
#include "AngleUtils.h"
#include "math.h"
#include <cstdlib>

using namespace std;

IncrementalConvexHull::IncrementalConvexHull()
{
  m_num_points = 0;
  m_order_id = 0;
}

bool IncrementalConvexHull::addPoint(double x, double y)
{
  // THE PROBLEM IS WE ASSUME THAT THE FIRST 3 POINTS ARE ADDED IN ANTI-CLOCKWISE ORDER! HAVE TO CHECK FOR THIS -> CHECK THE SIDE OF THE THIRD POINT ON ADDITION

  if (m_order_id < 3) {
    if (m_order_id == 0) {
      Node *new_node;
      new_node = new(Node);
      new_node->pt.set_vertex(x, y);
      new_node->id = m_order_id;
      new_node->next = NULL;
      new_node->prev = NULL;
      m_base_node = new_node;
      m_curr_node = new_node;
    } else if (m_order_id == 2) {
      bool countercw = (side(m_base_node->pt.x(), m_base_node->pt.y(), m_curr_node->pt.x(), m_curr_node->pt.y(), x, y) == 1);
      if (countercw) {  // ccw, so we can add as usual
        Node *new_node;
        new_node = new(Node);
        new_node->pt.set_vertex(x, y);
        new_node->id = m_order_id;
        new_node->next = m_base_node;
        new_node->prev = m_curr_node;
        m_curr_node->next = new_node;
        m_base_node->prev = new_node;
        m_curr_node = new_node;
      } else {          // cw, so we have to reorder the points to be flowing ccw
        Node *new_node;
        new_node = new(Node);
        new_node->pt.set_vertex(x, y);
        new_node->id = m_order_id;
        new_node->next = m_curr_node;
        new_node->prev = m_base_node;
        m_base_node->next = new_node;
        m_base_node->prev = m_curr_node;
        m_curr_node->next = m_base_node;
        m_curr_node->prev = new_node;
      }
    } else {
      Node *new_node;
      new_node = new(Node);
      new_node->pt.set_vertex(x, y);
      new_node->id = m_order_id;
      new_node->next = m_base_node;
      new_node->prev = m_curr_node;
      m_curr_node->next = new_node;
      m_base_node->prev = new_node;
      m_curr_node = new_node;
    }
    m_num_points++;
  } else {
    updateConvexHull(x, y);
  }
  m_order_id++;
  return(true);
}

void IncrementalConvexHull::updateConvexHull(double x, double y)
{
  if (contains(x, y)) {
    return;
  } else {
    setTangents(x, y);
    if (m_left_tangent_node->next == m_right_tangent_node) {  // insert this new point between these tangent points (no removals required)
      Node *new_node;
      new_node = new(Node);
      new_node->pt.set_vertex(x, y);
      new_node->id = m_order_id;
      new_node->next = m_right_tangent_node;
      new_node->prev = m_left_tangent_node;
      m_right_tangent_node->prev = new_node;
      m_left_tangent_node->next = new_node;
      if (m_left_tangent_node == m_curr_node) {
        m_curr_node = new_node;
      }
      m_num_points++;
    } else {                                                  // remove all points between these tangent points, and insert this new point
      Node *ptr_del_start = m_left_tangent_node->next;        // start and end of chain of deletions
      Node *ptr_del_end = m_right_tangent_node->prev;

      Node *new_node;
      new_node = new(Node);
      new_node->pt.set_vertex(x, y);
      new_node->id = m_order_id;
      new_node->next = m_right_tangent_node;
      new_node->prev = m_left_tangent_node;
      m_right_tangent_node->prev = new_node;
      m_left_tangent_node->next = new_node;

      Node *temp;
      while (ptr_del_start != ptr_del_end) {
//        std::cout << "DELETIONS X: " << ptr_del_start->pt.x() << " Y: " << ptr_del_start->pt.y() << " ID: " << ptr_del_start->id << std::endl;
        if (ptr_del_start == m_curr_node) {
          m_curr_node = new_node;
        } else if (ptr_del_start == m_base_node) {
          m_base_node = m_right_tangent_node;
          m_curr_node = new_node;
        }
        temp = ptr_del_start->next;
        ptr_del_start->next = NULL;
        ptr_del_start->prev = NULL;
        free(ptr_del_start);
        m_num_points--;
        ptr_del_start = temp;
      }

      if (ptr_del_start == ptr_del_end) { // only 1 node to delete
        //std::cout << "DELETIONS X: " << ptr_del_start->pt.x() << " Y: " << ptr_del_start->pt.y() << " ID: " << ptr_del_start->id << std::endl;
        if (ptr_del_start == m_curr_node) {
          m_curr_node = new_node;
        } else if (ptr_del_start == m_base_node) {
          m_base_node = m_right_tangent_node;
          m_curr_node = new_node;
        }
        ptr_del_start->next = NULL;
        ptr_del_start->prev = NULL;
        free(ptr_del_start);
        m_num_points--;
      }
    }
  }
}

void IncrementalConvexHull::deleteAllPoints()
{
  Node *ptr = m_base_node;
  Node *temp;
  while (ptr != m_curr_node) {
    temp = ptr->next;
    ptr->next = NULL;
    ptr->prev = NULL;
    free(ptr);
    ptr = temp;
  }
  ptr->next = NULL;
  ptr->prev = NULL;
  free(ptr);
}

XYPolygon IncrementalConvexHull::getConvexHullAsPolygon()
{
  Node *ptr = m_base_node;
  XYPolygon poly;
  while (ptr != m_curr_node) {
    poly.add_vertex(ptr->pt.x(), ptr->pt.y());
    ptr = ptr->next;
  }
  poly.add_vertex(ptr->pt.x(), ptr->pt.y());
  return poly;
}

int IncrementalConvexHull::getNumPoints(){
  return m_num_points;
}

void IncrementalConvexHull::printPoints()
{
  Node *ptr = m_base_node;
  while (ptr != m_curr_node) {
    std::cout << "X: " << ptr->pt.x() << " Y: " << ptr->pt.y() << " ID: " << ptr->id << std::endl;
    ptr = ptr->next;
  }
  std::cout << "X: " << ptr->pt.x() << " Y: " << ptr->pt.y() << " ID: " << ptr->id << std::endl;
}

bool IncrementalConvexHull::contains(double x, double y)
{
  Node *ptr = m_base_node;
  int prev_side = side(ptr->pt.x(), ptr->pt.y(), ptr->next->pt.x(), ptr->next->pt.y(), x, y);
//  std::cout << "PREV SIDE " << prev_side << std::endl;
  while (ptr != m_curr_node) {
    ptr = ptr->next;
    int curr_side = side(ptr->pt.x(), ptr->pt.y(), ptr->next->pt.x(), ptr->next->pt.y(), x, y);
//    std::cout << "CURR SIDE " <<  curr_side << std::endl;
    if (curr_side != prev_side && curr_side != 0 && prev_side != 0) {
//      std::cout << "NOT INSIDE" << std::endl;
      return(false);
    }
    curr_side = prev_side;
  }

//  std::cout << "INSIDE" << std::endl;
  return(true);
}

int IncrementalConvexHull::side(double x1, double y1, double x2, double y2, double x3, double y3)
{
  double val = ((x2-x1)*(y3-y1)-(y2-y1)*(x3-x1));
  if (val < 0) {
//    std::cout << "RIGHT" << std::endl;
    return -1;
  } else if (val > 0) {
//    std::cout << "LEFT" << std::endl;
    return 1;
  } else {
//    std::cout << "ON LINE" << std::endl;
    return 0;
  }
}

void IncrementalConvexHull::setTangents(double x, double y)
{
  Node *ptr = m_base_node;
  while (ptr != m_curr_node) {
    ptr = ptr->next;
    bool e_prev = (side(ptr->prev->pt.x(), ptr->prev->pt.y(), ptr->pt.x(), ptr->pt.y(), x, y) == 1);
    bool e_next = (side(ptr->pt.x(), ptr->pt.y(), ptr->next->pt.x(), ptr->next->pt.y(), x, y) == 1);
    if ((!e_prev) && e_next) {
      m_right_tangent_node = ptr;
    } else if (e_prev && (!e_next)) {
      m_left_tangent_node = ptr;
    }
  }
  ptr = ptr->next;
  bool e_prev = (side(ptr->prev->pt.x(), ptr->prev->pt.y(), ptr->pt.x(), ptr->pt.y(), x, y) == 1);
  bool e_next = (side(ptr->pt.x(), ptr->pt.y(), ptr->next->pt.x(), ptr->next->pt.y(), x, y) == 1);
  if ((!e_prev) && e_next) {
    m_right_tangent_node = ptr;
  } else if (e_prev && (!e_next)) {
    m_left_tangent_node = ptr;
  }
}
