/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: MSEASOceanModelVisDirect.h                      */
/*    DATE: December 2014                                   */
/************************************************************/

#ifndef MSEASOceanModelVisDirect_HEADER
#define MSEASOceanModelVisDirect_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <octave/oct.h>
#include <octave/octave.h>
#include <octave/parse.h>
#include <octave/toplev.h>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <map>
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"
#include "XYConvexGrid.h"
#include "XYVector.h"

class MSEASOceanModelVisDirect : public AppCastingMOOSApp
{
 public:
   MSEASOceanModelVisDirect();
   ~MSEASOceanModelVisDirect();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();
   bool buildReport();
   void SetupGrid();
   void RequestMSEASData();
   void VisualizeMSEASData();

private: // Configuration variables
  int             m_grid_num_x_els;
  int             m_grid_num_y_els;
  double          m_grid_min_value;
  double          m_grid_max_value;
  double          m_grid_delta_x;
  double          m_grid_delta_y;
  std::string     m_octave_path;
  std::string     m_model_filepath;
  std::string     m_moosvar_prefix;
  double          m_global_lon_origin;
  double          m_global_lat_origin;
  double          m_model_time_warp;

 private: // State variables
  unsigned int    m_iterations;
  double          m_timewarp;
  CMOOSGeodesy    m_geodesy;
  double          m_x;
  double          m_y;
  double          m_depth;
  int             m_grid_num_els;
  ColumnVector    m_model_lon_request;
  ColumnVector    m_model_lat_request;
  ColumnVector    m_model_depth_request;
  std::string     m_model_varnames_request;
  int             m_num_model_vars;
  std::vector<std::string> m_model_varname_vec_request;
  std::map< std::string, int > m_map_varname_idx;       // <variable name, corresponding index into m_model_varname_vec_request>
  Cell            m_model_varname_cell_request;
  std::string     m_time_request_offset_moosvar;
  std::string     m_time_offset_reset_moosvar;
  std::string     m_varname_moosvar;
  std::string     m_x_moosvar;
  std::string     m_y_moosvar;
  std::string     m_depth_moosvar;
  std::string     m_paused_moosvar;
  XYConvexGrid    m_grid;
  std::vector<XYVector> m_vector_field;
  int             m_varnames_size;
  double          m_vector_power;
  double          m_vector_scale;
  octave_value_list m_batch_request;
  octave_value_list m_batch_return;
  Cell            m_batch_return_cell;
  octave_value_list m_time_request;
  octave_value_list m_time_return;
  bool            m_vis_paused;
  bool            m_appcasting_ready;
  bool            m_reset_time_flag;
  double          m_model_time_offset;
  double          m_model_time_request_offset;
  double          m_model_time_request;
  double          m_model_start_time;
  double          m_model_end_time;
  double          m_model_duration;
  Matrix          m_model_times_request;
  Matrix          m_model_values_return;
  clock_t         m_start;
  clock_t         m_end;
  double          m_request_time;
  std::string     m_grid_color_var;
  std::string     m_color_moosvar;
};

#endif
