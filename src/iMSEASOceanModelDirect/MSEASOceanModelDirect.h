/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: MSEASOceanModelDirect.h                         */
/*    DATE: November 2014                                   */
/************************************************************/

#ifndef MSEASOceanModelDirect_HEADER
#define MSEASOceanModelDirect_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <octave/oct.h>
#include <octave/octave.h>
#include <octave/parse.h>
#include <octave/toplev.h>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <map>
#include "NodeRecord.h"
#include <boost/dynamic_bitset.hpp>

class MSEASOceanModelDirect : public AppCastingMOOSApp
{
 public:
   MSEASOceanModelDirect();
   ~MSEASOceanModelDirect();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();
   bool HandleNewNodeReport(CMOOSMsg msg);
   bool buildReport();
   void RequestMSEASData();
   void DistributeMSEASData();

 private: // Configuration variables
   std::string              m_octave_path;
   std::string              m_model_filepath;
   std::string              m_moosvar_prefix;
   std::string              m_node_communities;
   std::vector<std::string> m_node_communities_vec;
   double                   m_model_time_warp;

 private: // State variables
   unsigned int             m_iterations;
   double                   m_timewarp;
   ColumnVector             m_model_lon_request;
   ColumnVector             m_model_lat_request;
   ColumnVector             m_model_depth_request;
   std::string              m_model_varnames_request;
   int                      m_num_model_vars;
   std::vector<std::string> m_model_varname_vec_request;
   std::map< std::string, int > m_map_varname_idx;                           // <variable name, corresponding index into m_model_varname_vec_request>
   Cell                     m_model_varname_cell_request;
   double                   m_model_time_request;
   double                   m_model_time_offset;
   double                   m_model_time_request_offset;
   double                   m_model_start_time;
   double                   m_model_end_time;
   double                   m_model_duration;
   octave_value_list        m_batch_request;
   octave_value_list        m_batch_return;
   Cell                     m_batch_return_cell;
   octave_value_list        m_time_request;
   octave_value_list        m_time_return;
   Matrix                   m_model_times_request;
   Matrix                   m_model_values_return;
   bool                     m_reset_time_flag;
   bool                     m_model_pos_setflag;
   bool                     m_model_varname_setflag;
   std::string              m_varname_moosvar;
   std::string              m_time_request_offset_moosvar;
   std::string              m_time_offset_reset_moosvar;
   bool                     m_appcasting_ready;
   clock_t                  m_start;
   clock_t                  m_end;
   double                   m_request_time;
   std::map< std::string, NodeRecord > m_map_node_records;                // <vehicle_name, node_record>
   std::map< std::string, int > m_map_node_idx;                           // <vehicle_name, index into m_node_communities_vec/mseas return matrix>
   std::map< std::string, std::pair<double, double> > m_map_node_hdrifts; // <vehicle_name, <drift_x,drift_y> >
   boost::dynamic_bitset<>  m_nodes_pos_set;
   int                      m_num_communities;
};

#endif
