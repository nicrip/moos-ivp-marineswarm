/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForcesContacts.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef DriftingForcesContacts_HEADER
#define DriftingForcesContacts_HEADER

#include <string>
#include "DriftingForces/BHV_DriftingForces.h"

class BHV_DriftingForcesContacts : public BHV_DriftingForces {
public:
  BHV_DriftingForcesContacts(IvPDomain);
  ~BHV_DriftingForcesContacts() {};

  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters

protected: // State variables
  double m_src_x;
  double m_src_y;
  double m_src_clr;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_DriftingForcesContacts(domain);}
}
#endif
