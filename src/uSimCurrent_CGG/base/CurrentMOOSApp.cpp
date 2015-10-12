/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: CurrentMOOSApp.cpp                                   */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <thread>
#include <typeinfo>
#include "MBUtils.h"
#include "NodeRecordUtils.h"
#include "CurrentMOOSApp.h"
#include "Current/ConstantCurrent.h"
#include "Current/RandomCurrent.h"
#include "Current/ShearCurrent.h"
#include "Current/LangmuirCurrent.h"
#include "Current/SolitonCurrent.h"
#include "Current/SpectralCurrent.h"
#include "Current/MultiSpectralCurrent.h"
#include "Current/VortexCurrent.h"
#include "Current/GridCurrent.h"

using namespace std;


//---------------------------------------------------------
// Constructor

CurrentMOOSApp::CurrentMOOSApp()
{
  m_updated_currents = 0;
  m_display_viewer = true;
  
  #ifdef BUILD_GUI_CODE
    m_viewer = NULL;
  #endif
}


//---------------------------------------------------------
// Destructor

CurrentMOOSApp::~CurrentMOOSApp()
{
  #ifdef BUILD_GUI_CODE
    m_viewer->quit();
  #endif
}


//---------------------------------------------------------
// Procedure: BuildReport

bool CurrentMOOSApp::buildReport()
{
  int nb_models_activated = 0;
  for(vector<CurrentModel*>::iterator model = m_current_models.begin() ; 
            model != m_current_models.end() ; 
            model++)
  {
    if((*model)->isActive())
      nb_models_activated ++;
  }
    
  m_msgs << endl;
  m_msgs << "\tcurrent models: " << m_current_models.size() << endl;
  m_msgs << "\tcurrent models activated: " << nb_models_activated << endl;
  m_msgs << endl;
  m_msgs << "\tupdated currents: " << m_updated_currents << endl;
  m_msgs << endl;
  
  #ifdef BUILD_GUI_CODE
    if(m_viewer != NULL)
    {
      m_msgs << "\t- windows size: " << m_viewer->getWindowSize() << "px" << endl;
      m_msgs << "\t- area size: " << m_viewer->getAreaSize() << "m" << endl;
      m_msgs << "\t- arrow gap: " << m_viewer->getArrowGap() << "px" << endl;
      m_msgs << "\t- MOOS time: " << setprecision(16) << m_viewer->getMOOSTime() << endl;
      m_msgs << "\t- operation depth: " << m_viewer->getOperationDepth() << "m" << endl;
    }
  #endif
  
  m_msgs << endl << setprecision(5);
  
  for(map<std::string,NodeRecord>::iterator node = m_map_node_records.begin() ; 
      node != m_map_node_records.end() ; 
      node++)
  {
    m_msgs << "\t" << node->first << " drift: " << endl;
    m_msgs << "\t\tX: " << m_map_node_hdrifts[node->first].first << endl;
    m_msgs << "\t\tY: " << m_map_node_hdrifts[node->first].second << endl << endl;
  }

  return true;
}


//---------------------------------------------------------
// Procedure: OnNewMail

bool CurrentMOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;

  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    bool handled = false;
    
    if(msg.GetKey() == "NODE_REPORT")
      handled = HandleNewNodeReport(msg);
    
    if(!handled)
      cout << "Unhandled Mail: " << msg.GetKey() << endl;
    
    #if 0 // Keep these around just for template
      string key   = msg.GetKey();
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString(); 
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif
  }

  return true;
}


//---------------------------------------------------------
// Procedure: OnConnectToServer

bool CurrentMOOSApp::OnConnectToServer()
{
  RegisterVariables();
  return true;
}


//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool CurrentMOOSApp::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
  m_starting_time = MOOSTime();
  
  #ifdef BUILD_GUI_CODE
    m_viewer = new CurrentViewer(&m_current_models, &m_map_node_records, m_starting_time);
  #endif
  
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
  {
    sParams.reverse();
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++)
    {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);
      
      if(param == "ADD_CURRENT_MODEL")
        AddCurrentModel(toupper(value));
      
      if(!m_current_models.empty())
        m_current_models.back()->setParam(param, value);
        
      if(param == "DISPLAY_VIEWER")
        setBooleanOnString(m_display_viewer, value);
      
      #ifdef BUILD_GUI_CODE
        if(param == "PATH_TO_FONTS_DIRECTORY")
          m_viewer->setPathToFontsDirectory(value);
        
        if(param == "WINDOW_SIZE")
          m_viewer->setWindowSize(atoi(value.c_str()));
        
        if(param == "AREA_SIZE")
          m_viewer->setAreaSize(atof(value.c_str()));
        
        if(param == "ARROW_GAP")
          m_viewer->setArrowGap(atoi(value.c_str()));
        
        if(param == "LOW_DISPLAY_MODE")
        {
          bool low_display_mode;
          setBooleanOnString(low_display_mode, value);
          m_viewer->setLowDisplayMode(low_display_mode);
        }
        
        if(param == "CAPTURE_TIME")
          m_viewer->setMOOSTime(atof(value.c_str()) + MOOSTime());
        
        if(param == "DISPLAY_BACKGROUND")
        {
          bool display_background;
          setBooleanOnString(display_background, value);
          m_viewer->enableBackground(display_background);
        }
        
        if(param == "DISPLAY_NODES")
        {
          bool display_nodes;
          setBooleanOnString(display_nodes, value);
          m_viewer->enableNodesDisplay(display_nodes);
        }
        
        if(param == "AUTO_FOCUS_ON_NODES")
        {
          bool auto_focus;
          setBooleanOnString(auto_focus, value);
          m_viewer->setAutoFocusOnNodes(auto_focus);
        }
        
        if(param == "OPERATION_DEPTH")
          m_viewer->setOperationDepth(atof(value.c_str()));
      #endif
    }
  }
  
  if(!InitializeCurrentModels())
    return false;
  
  #ifdef BUILD_GUI_CODE
    if(m_display_viewer)
    {
      if(!m_viewer->init())
        return false;
      
      // The viewer is launched in a thread
      thread thread_viewer(std::bind(&CurrentViewer::run, m_viewer));
      thread_viewer.detach();
      m_viewer->update(false);
    }
  #endif
  
  RegisterVariables();
  return true;
}


//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool CurrentMOOSApp::Iterate()
{
  AppCastingMOOSApp::Iterate();
  
  #ifdef BUILD_GUI_CODE
    m_viewer->setMOOSTime(MOOSTime());
  #endif
  
  AppCastingMOOSApp::PostReport();
  return true;
}


//---------------------------------------------------------
// Procedure: RegisterVariables

void CurrentMOOSApp::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("NODE_REPORT", 0.0);
}


//---------------------------------------------------------
// Procedure: HandleNewNodeReport

bool CurrentMOOSApp::HandleNewNodeReport(CMOOSMsg msg)
{
  string message_value = msg.GetString();
  string community_name = msg.GetCommunity();
  m_map_node_records[community_name] = string2NodeRecord(message_value);
  
  double drift_x, drift_y, drift_z;
  
  // Here, somehow, we get currents from x, y, depth and time
  if(CurrentModel::getCurrentFromListOfModels(
            m_current_models,
            m_map_node_records[community_name].getX(),
            m_map_node_records[community_name].getY(),
            m_map_node_records[community_name].getDepth(),
            MOOSTime() - m_starting_time,
            drift_x,
            drift_y, 
            drift_z))
  {
    Notify("DRIFT_X_FOR_" + toupper(community_name), drift_x);
    Notify("DRIFT_Y_FOR_" + toupper(community_name), drift_y);
    Notify("DRIFT_Z_FOR_" + toupper(community_name), drift_z);
    m_map_node_hdrifts[community_name].first = drift_x;
    m_map_node_hdrifts[community_name].second = drift_y;
    m_updated_currents ++;
    return true;
  }
  
  else
    return false;
}


//---------------------------------------------------------
// Procedure: ChooseCurrentModel
    
void CurrentMOOSApp::AddCurrentModel(string value)
{
  cout << "New model \"" << value << "\": instantiation..." << endl;
  
  if(value == "CONSTANT")
    m_current_models.push_back(new ConstantCurrent());
  
  else if(value == "RANDOM")
    m_current_models.push_back(new RandomCurrent());
    
  else if(value == "SHEAR")
    m_current_models.push_back(new ShearCurrent());
  
  else if(value == "LANGMUIR")
    m_current_models.push_back(new LangmuirCurrent());
  
  else if(value == "SOLITON")
    m_current_models.push_back(new SolitonCurrent());
    
  else if(value == "SPECTRAL")
    m_current_models.push_back(new SpectralCurrent());
    
  else if(value == "MULTI_SPECTRAL")
    m_current_models.push_back(new MultiSpectralCurrent());
    
  else if(value == "VORTEX")
    m_current_models.push_back(new VortexCurrent());

  else if(value == "GRID")
    m_current_models.push_back(new GridCurrent());
  
  else
    cout << "Unknown CURRENT_MODEL: \"" << value << "\"" << endl;
    
  cout << "New model \"" << value << "\": instantiation...   OK" << endl;
}


//---------------------------------------------------------
// Procedure: InitializeCurrentModel

bool CurrentMOOSApp::InitializeCurrentModels()
{
  for(vector<CurrentModel*>::iterator model = m_current_models.begin() ; 
    model != m_current_models.end() ; 
      model++)
  {
    cout << "Model \"" << (*model)->getType() << "\": initialization..." << endl;
    
    if((*model)->init())
      cout << "Model \"" << (*model)->getType() << "\": initialization...   OK" << endl;
    
    else
    {
      cout << "Model \"" << (*model)->getType() << "\": initialization...   ERROR" << endl;
      return false;
    }
  }
  
  return true;
}
