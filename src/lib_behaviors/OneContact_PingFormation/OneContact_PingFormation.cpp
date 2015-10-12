/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: OneContact_PingFormation.cpp                    */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#include "OneContact_PingFormation.h"

using namespace std;

//-----------------------------------------------------------
// Procedure: Constructor

OneContact_PingFormation::OneContact_PingFormation(IvPDomain gdomain) : ContactsPing_Formation(gdomain)
{
  setParam("name", "OneContact_PingFormation");
}


//-----------------------------------------------------------
// Procedure: getContact

PingContact OneContact_PingFormation::getContact()
{
  return m_map_contacts.begin()->second;
}


//-----------------------------------------------------------
// Procedure: setParam

bool OneContact_PingFormation::setParam(string param, string param_val) 
{
  if(ContactsPing_Formation::setParam(param, param_val))
    return true;
  
  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));
  
  if(param == "contact_name")
    return ContactsPing_Formation::addContactToWhiteList(param_val);
  
  return false;
}
