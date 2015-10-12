/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingTgtWptContacts.h                    */
/*    DATE:                                                 */
/************************************************************/

#ifndef DriftingTgtWptContacts_HEADER
#define DriftingTgtWptContacts_HEADER

#include <string>
#include "DriftingTarget/BHV_DriftingTarget.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"

class BHV_DriftingTgtWptContacts : public BHV_DriftingTarget, public BHV_AcousticPingPlanner {
public:
  BHV_DriftingTgtWptContacts(IvPDomain);
  ~BHV_DriftingTgtWptContacts() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  double m_wpt_x;
  double m_wpt_y;
  std::string m_contact_whitelist_namestring;

protected: // State variables
  std::vector<std::string> m_contact_whitelist_namevector;
  double m_curr_wpt_x;
  double m_curr_wpt_y;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_DriftingTgtWptContacts(domain);}
}
#endif
