/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Isosceles_PingFormation.cpp                     */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#include "Isosceles_PingFormation.h"

using namespace std;


//-----------------------------------------------------------
// Procedure: Constructor

Isosceles_PingFormation::Isosceles_PingFormation(IvPDomain gdomain) : ContactsPing_Formation(gdomain)
{
  setParam("name", "Isosceles_PingFormation");
  m_display_sub_target_points = false;
}


//-----------------------------------------------------------
// Procedure: setParam

bool Isosceles_PingFormation::setParam(string param, string param_val) 
{
  if(ContactsPing_Formation::setParam(param, param_val))
    return true;
    
  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));

  // Adding an influence in the positioning decision (couple of contact)
  if(param == "couple_of_contact__name")
  {
    m_list_couple_of_contact.push_front(new CoupleOfContact(param_val));
    return true;
  }
  
  if(m_list_couple_of_contact.size() != 0)
  {
    if(param == "couple_of_contact__contact1")
    {
      ContactsPing_Formation::addContactToWhiteList(tolower(param_val));
      m_list_couple_of_contact.front()->setContact1(ContactsPing_Formation::getContact(tolower(param_val)));
      return true;
    }
    
    if(param == "couple_of_contact__contact2")
    {
      ContactsPing_Formation::addContactToWhiteList(tolower(param_val));
      m_list_couple_of_contact.front()->setContact2(ContactsPing_Formation::getContact(tolower(param_val)));
      return true;
    }
    
    if(param == "couple_of_contact__dist_from_midpoint")
    {
      m_list_couple_of_contact.front()->setDistFromMidpoint(dval);
      return true;
    }
    
    if(param == "couple_of_contact__weight")
    {
      m_list_couple_of_contact.front()->setWeight(dval);
      return true;
    }
  }
    
  if(param == "display_sub_target_points")
    return setBooleanOnString(m_display_sub_target_points, param_val);
    
  return false;
}


//-----------------------------------------------------------
// Procedure: onSetParamComplete

void Isosceles_PingFormation::onSetParamComplete() 
{
  ContactsPing_Formation::onSetParamComplete();
  
  vector<string> vector_str_contact_list;
  bool contact1_already_saved, contact2_already_saved;
  
  for(list<CoupleOfContact*>::iterator couple_of_contact = m_list_couple_of_contact.begin(); 
      couple_of_contact != m_list_couple_of_contact.end(); 
      couple_of_contact ++)
  {
    contact1_already_saved = false;
    contact2_already_saved = false;
    
    for(vector<string>::iterator contact_name = vector_str_contact_list.begin(); 
        contact_name != vector_str_contact_list.end(); 
        contact_name ++)
    {
      if((*contact_name) == (*couple_of_contact)->getContact1()->GetName())
        contact1_already_saved = true;
      
      if((*contact_name) == (*couple_of_contact)->getContact2()->GetName())
        contact2_already_saved = true;
    }
    
    if(!contact1_already_saved)
      vector_str_contact_list.push_back((*couple_of_contact)->getContact1()->GetName());
    
    if(!contact2_already_saved)
      vector_str_contact_list.push_back((*couple_of_contact)->getContact2()->GetName());
  }
  
  string str_contact_list = "";
  for(vector<string>::iterator contact_name = vector_str_contact_list.begin(); 
      contact_name != vector_str_contact_list.end(); 
      contact_name ++)
  {
    str_contact_list += (*contact_name) + " ";
  }
  
  postMessage("NEIGHBOR_LIST", str_contact_list);
  postIntMessage("NB_COUPLES", m_list_couple_of_contact.size());
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *Isosceles_PingFormation::onRunState()
{
  if(!ContactsPing_Formation::updatePlatformInfo())
    return 0;
  
  // Computing the centroid target point
  updateCentroidTargetPoint();
  
  return ContactsPing_Formation::onRunState();
}


//-----------------------------------------------------------
// Procedure: updateCentroidTargetPoint

void Isosceles_PingFormation::updateCentroidTargetPoint()
{
  double sub_weight = 0., targetpt_x = 0., targetpt_y, weight_sum = 0.;
  
  for(list<CoupleOfContact*>::iterator couple_of_contact = m_list_couple_of_contact.begin(); 
        couple_of_contact != m_list_couple_of_contact.end(); 
        couple_of_contact ++)
  {
    if((*couple_of_contact)->IsInitialized())
    {
      sub_weight = (*couple_of_contact)->getWeight();
      XYPoint sub_target_point = (*couple_of_contact)->getTargetPoint();
      
      targetpt_x += sub_target_point.x() * sub_weight;
      targetpt_y += sub_target_point.y() * sub_weight;
      weight_sum += sub_weight;
      
      if(m_display_sub_target_points)
      {
        sub_target_point.set_vertex(sub_target_point.x() + m_nav_x, sub_target_point.y() + m_nav_y);
        sub_target_point.set_label(toupper(m_us_name) + "_" + (*couple_of_contact)->getName());
        sub_target_point.set_active("true");
        postMessage("VIEW_POINT", sub_target_point.get_spec());
      }
    }
  }
  
  if(weight_sum != 0.)
    m_relative_target_point.set_vertex(targetpt_x / weight_sum, targetpt_y / weight_sum);
    
  filterCentroidTargetpoint();
}
