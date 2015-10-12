/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: CurrentMOOSApp.cpp                                   */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <thread>
#include <math.h>
#include <sstream>
#include <iostream>
#include "AngleUtils.h"
#include "CurrentViewer.h"
#include "SDL/SDL_gfxPrimitives.h" 

#define FPS 50

using namespace std;


//---------------------------------------------------------
// Constructor

CurrentViewer::CurrentViewer(vector<CurrentModel*>* current_models, std::map<std::string,NodeRecord>* map_node_records, double starting_moos_time)
{
  m_text_view_size = 200;
  m_current_models = current_models;
  m_map_node_records = map_node_records;
  m_starting_time = starting_moos_time;
  m_capture_time = m_starting_time;
  m_low_display_mode = false;
    
  setWindowSize(400);
  setAreaSize(500.);
  setArrowGap(30);
  setMOOSTime(starting_moos_time);
  enableBackground(true);
  enableNodesDisplay(true);
  setOperationDepth(200.);
  
  // Initializing all SDL sub-systems
  if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
  {
    cout << "CurrentViewer: SDL_Init error" << endl;
    exit(0);
  }
  
  m_max_drift = 0.;
  m_running = true;
  m_camera_px_position.x = metersToPixels(-m_area_size / 2.);
  m_camera_px_position.y = metersToPixels(-m_area_size / 2.);
  m_running_update = false;
  m_display_grid = false;
}


//---------------------------------------------------------
// Destructor

CurrentViewer::~CurrentViewer()
{
  
}


//---------------------------------------------------------
// Procedure: getWindowSize

int CurrentViewer::getWindowSize()
{
  return m_current_view_size;
}


//---------------------------------------------------------
// Procedure: getAreaSize

double CurrentViewer::getAreaSize()
{
  return m_area_size;
}


//---------------------------------------------------------
// Procedure: getArrowGap

int CurrentViewer::getArrowGap()
{
  return m_arrow_gap;
}


//---------------------------------------------------------
// Procedure: getMOOSTime

double CurrentViewer::getMOOSTime()
{
  return m_moos_time;
}


//---------------------------------------------------------
// Procedure: getOperationDepth

double CurrentViewer::getOperationDepth()
{
  return m_operation_depth;
}


//---------------------------------------------------------
// Procedure: setPathToFontsDirectory

void CurrentViewer::setPathToFontsDirectory(string path)
{
  m_path_to_fonts_directory = path;
}


//---------------------------------------------------------
// Procedure: setWindowSize

void CurrentViewer::setWindowSize(int size)
{
  m_current_view_size = size;
  if(m_current_view_size < 300)
    m_current_view_size = 300;
}


//---------------------------------------------------------
// Procedure: setAreaSize

void CurrentViewer::setAreaSize(double size)
{
  m_area_size = size;
}


//---------------------------------------------------------
// Procedure: setArrowGap

void CurrentViewer::setArrowGap(int gap)
{
  m_arrow_gap = gap;
}


//---------------------------------------------------------
// Procedure: setLowDisplayMode

void CurrentViewer::setLowDisplayMode(bool low_mode)
{
  m_low_display_mode = low_mode;
  
  if(m_low_display_mode)
    m_text_view_size = 0;
}


//---------------------------------------------------------
// Procedure: setAutoFocusOnNodes

void CurrentViewer::setAutoFocusOnNodes(bool auto_focus)
{
  m_auto_focus_on_nodes = auto_focus;
}


//---------------------------------------------------------
// Procedure: setMOOSTime

void CurrentViewer::setMOOSTime(double moos_time)
{
  m_moos_time = moos_time;
}


//---------------------------------------------------------
// Procedure: enableBackground

void CurrentViewer::enableBackground(bool enable)
{
  m_display_background = enable;
}


//---------------------------------------------------------
// Procedure: enableNodesDisplay

void CurrentViewer::enableNodesDisplay(bool enable)
{
  m_nodes_display = enable;
}


//---------------------------------------------------------
// Procedure: setOperationDepth

void CurrentViewer::setOperationDepth(double depth)
{
  m_operation_depth = depth;
}


//---------------------------------------------------------
// Procedure: quit

void CurrentViewer::quit()
{
  m_running = false;
}


//---------------------------------------------------------
// Procedure: pixelsToMeters

double CurrentViewer::pixelsToMeters(int px)
{
  return px * m_area_size / m_current_view_size;
}


//---------------------------------------------------------
// Procedure: metersToPixels

int CurrentViewer::metersToPixels(double m)
{
  return (int)(m * m_current_view_size / m_area_size);
}


//---------------------------------------------------------
// Procedure: init

bool CurrentViewer::init()
{
  // Initialization
  m_current_values = new double**[m_current_view_size];
  for(int i = 0 ; i < m_current_view_size ; i++)
  {
    m_current_values[i] = new double*[m_current_view_size];
    for(int j = 0 ; j < m_current_view_size ; j++)
    {
      m_current_values[i][j] = new double[3];
      m_current_values[i][j][0] = 0.; // drift x
      m_current_values[i][j][1] = 0.; // drift y
      m_current_values[i][j][2] = 0.; // drift z
    }
  }

  TTF_Init();
  string path = m_path_to_fonts_directory + "/Instruction.ttf";
  cout << "Opening: " << path << endl;
  m_font = TTF_OpenFont(path.c_str(), 10);
  
  if(!m_font)
  {
    cout << "CurrentViewer: TTF_OpenFont error. " << TTF_GetError() << endl;
    exit(0);
  }
  
  m_screen = SDL_SetVideoMode(m_current_view_size + m_text_view_size, m_current_view_size, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

  if(m_screen == NULL)
  {
    cout << "CurrentViewer: SDL_SetVideoMode error" << endl;
    return false;
  }
  
  SDL_WM_SetCaption("CurrentViewer", NULL);
  
  return true;
}


//---------------------------------------------------------
// Procedure: run

void CurrentViewer::run()
{
  // Run
  Uint32 start;
  bool active_selection = false, launch_full_update;
  SDL_Rect click_down, selection_rectangle, mouse_position;
  SDL_EnableKeyRepeat(300, 10);
  
  mouse_position.x = 0;
  mouse_position.y = 0;
  
  while(m_running)
  {
    start = SDL_GetTicks();
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      launch_full_update = false;
      
      switch(event.type)
      {
        case SDL_QUIT:
          m_running = false;
          break;
        
        case SDL_MOUSEBUTTONDOWN:
          click_down.x = event.button.x;
          click_down.y = event.button.y;
          selection_rectangle.w = 0;
          selection_rectangle.h = 0;
          active_selection = true;
          break;
        
        case SDL_MOUSEBUTTONUP:
          switch(event.button.button)
          {
            case SDL_BUTTON_LEFT:
              active_selection = false;
              break;
          }
          break;
          
        case SDL_MOUSEMOTION:
          mouse_position.x = event.button.x;
          mouse_position.y = event.button.y;
          break;
          
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym)
          {
              case SDLK_UP:
                  if(!m_running_update)
                    cameraMovement(0, -10);
                  break;
                  
              case SDLK_DOWN:
                  if(!m_running_update)
                    cameraMovement(0, 10);
                  break;
                  
              case SDLK_RIGHT:
                  if(!m_running_update)
                    cameraMovement(10, 0);
                  break;
                  
              case SDLK_LEFT:
                  if(!m_running_update)
                    cameraMovement(-10, 0);
                  break;
                  
              case SDLK_PAGEUP:
                  m_operation_depth -= 5.;
                  break;
                  
              case SDLK_PAGEDOWN:
                  m_operation_depth += 5.;
                  break;
                  
              case SDLK_1:
              case SDLK_F1:
              case SDLK_KP1:
                  reverseCurrentModelActivation(1);
                  launch_full_update = true;
                  break;
                  
              case SDLK_2:
              case SDLK_F2:
              case SDLK_KP2:
                  reverseCurrentModelActivation(2);
                  launch_full_update = true;
                  break;
                  
              case SDLK_3:
              case SDLK_F3:
              case SDLK_KP3:
                  reverseCurrentModelActivation(3);
                  launch_full_update = true;
                  break;
                  
              case SDLK_4:
              case SDLK_F4:
              case SDLK_KP4:
                  reverseCurrentModelActivation(4);
                  launch_full_update = true;
                  break;
                  
              case SDLK_5:
              case SDLK_F5:
              case SDLK_KP5:
                  reverseCurrentModelActivation(5);
                  launch_full_update = true;
                  break;
                  
              case SDLK_6:
              case SDLK_F6:
              case SDLK_KP6:
                  reverseCurrentModelActivation(6);
                  launch_full_update = true;
                  break;
                  
              case SDLK_7:
              case SDLK_F7:
              case SDLK_KP7:
                  reverseCurrentModelActivation(7);
                  launch_full_update = true;
                  break;
                  
              case SDLK_8:
              case SDLK_F8:
              case SDLK_KP8:
                  reverseCurrentModelActivation(8);
                  launch_full_update = true;
                  break;
                  
              case SDLK_9:
              case SDLK_F9:
              case SDLK_KP9:
                  reverseCurrentModelActivation(9);
                  launch_full_update = true;
                  break;
                  
              case SDLK_g:
                  m_display_grid = !m_display_grid;
                  break;
                  
              case SDLK_r:
                  launch_full_update = true;
                  break;
                  
              case SDLK_t:
                  m_capture_time = m_moos_time;
                  break;
                  
              case SDLK_KP_PLUS:
                  m_capture_time += 5 * 60;
                  break;
                  
              case SDLK_KP_MINUS:
                  m_capture_time -= 5 * 60;
                  if(m_capture_time < m_starting_time)
                    m_capture_time = m_starting_time;
                  break;
                  
              case SDLK_i:
                m_area_size *= 0.9;
                launch_full_update = true;
                break;
              
              case SDLK_o:
                m_area_size *= 1.1;
                launch_full_update = true;
                break;
          }
          break;
      }
    }
    
    if(launch_full_update)
    {
      thread thread_update_viewer(std::bind(&CurrentViewer::fullUpdate, this));
      thread_update_viewer.detach();
    }
    
    if(m_display_background)
      for(int i = 0 ; i < m_current_view_size ; i ++)
        for(int j = 0 ; j < m_current_view_size ; j ++)
          setPixelFromCurrentValue(i, j, m_display_grid);
          
    if(active_selection)
    {
      selection_rectangle.w = abs(mouse_position.x - click_down.x);
      selection_rectangle.h = abs(mouse_position.y - click_down.y);
      
      if(event.button.x <= click_down.x)
        selection_rectangle.x = mouse_position.x;
      
      else
        selection_rectangle.x = click_down.x;
      
      if(event.button.y <= click_down.y)
        selection_rectangle.y = mouse_position.y;
      
      else
        selection_rectangle.y = click_down.y;
        
      drawSelectionRectangle(selection_rectangle);
    }
    
    drawArrows();
    drawNodes();
    
    if(!m_low_display_mode)
    {
      drawIntensityScale(m_current_view_size - (m_current_view_size / 3) - 60, 15, m_current_view_size / 3, mouse_position);
      drawMeterScale(10, 15);
      drawInfoBar(mouse_position, active_selection, selection_rectangle);
    }
  
    if((int)m_update_progress < 100)
      drawProgressBar();
    
    if(!m_low_display_mode)
      drawTextPanel(mouse_position);
    
    SDL_Flip(m_screen);
    
    if(1000 / FPS > SDL_GetTicks() - start)
      SDL_Delay(1000 / FPS - (SDL_GetTicks() - start));
  }
  
  TTF_CloseFont(m_font);
  SDL_Quit();
  TTF_Quit();
}


//---------------------------------------------------------
// Procedure: reverserCurrentModelActivation

void CurrentViewer::reverseCurrentModelActivation(int current_model_id)
{
  if(current_model_id <= (*m_current_models).size())
  {
    vector<CurrentModel*>::iterator model = (*m_current_models).begin() + (current_model_id - 1);
    (*model)->switchActivation();
  }
}

  
//---------------------------------------------------------
// Procedure: drawTextPanel

void CurrentViewer::drawTextPanel(SDL_Rect mouse_position)
{
  int k = 0;
  SDL_Rect text_panel_position;
  text_panel_position.x = m_current_view_size;
  text_panel_position.y = 0;
  SDL_Surface* text_panel = SDL_CreateRGBSurface(SDL_HWSURFACE, 
                                                 m_text_view_size, 
                                                 m_current_view_size, 
                                                 32, 0, 0, 0, 0);
  SDL_FillRect(text_panel, NULL, SDL_MapRGB(text_panel->format, 242, 241, 240));
  SDL_BlitSurface(text_panel, NULL, m_screen, &text_panel_position);
  SDL_FreeSurface(text_panel);
  
  SDL_Color text_color = {92, 92, 92};
  SDL_Color text_color_deactivate = {192, 192, 192};
  
  ostringstream strs;
  int nb_nodes = (*m_map_node_records).size();
  strs << nb_nodes << " node";
  
  if(nb_nodes > 1)
    strs << "s are";
  
  else
    strs << " is";
    
  strs << " drifting:";
  
  writeTextLabel(m_current_view_size + 10, 5 + k * 10, strs.str(), text_color);
  
  strs.str("");
  int nb_current_models = (*m_current_models).size();
  strs << nb_current_models << " current model";
  
  if(nb_current_models > 1)
    strs << "s are";
  
  else
    strs << " is";
    
  strs << " running:";
  
  k ++;
  writeTextLabel(m_current_view_size + 10, 5 + k * 10, strs.str(), text_color);
  for(vector<CurrentModel*>::iterator model = (*m_current_models).begin() ; 
            model != (*m_current_models).end() ; 
            model++)
  {
    k ++;
    strs.str("");
    strs << (k - 1) << ". " << (*model)->getType() << " (" << (*model)->getName() << ")";
    SDL_Color text_color_model;
    
    if((*model)->isActive())
      text_color_model = text_color;
    
    else
      text_color_model = text_color_deactivate;
    
    writeTextLabel(m_current_view_size + 30, 
                    5 + (k + 1) * 10, 
                    strs.str(), 
                    text_color_model);
  }
  
  k += 4;
  writeTextLabel(m_current_view_size + 10, 5 + k * 10, "Press numeric key to", text_color); 
  k++;
  writeTextLabel(m_current_view_size + 10, 5 + k * 10, "(de)activate a model", text_color);
  
  k += 2;
  writeTextLabel(m_current_view_size + 10, 5 + k * 10, "Press G to display grid", text_color); 
  
  k++;
  writeTextLabel(m_current_view_size + 10, 5 + k * 10, "Press +/- to change time", text_color); 
  
  k++;
  writeTextLabel(m_current_view_size + 10, 5 + k * 10, "Press i/o to zoom", text_color); 
  
  if(mouse_position.x < m_current_view_size)
  {
    k += 2;
    ostringstream strs;
    strs << fixed;
    strs.precision(3);
    strs << (int)pixelsToMeters(mouse_position.x + m_camera_px_position.x) << "," 
          << -(int)pixelsToMeters(mouse_position.y + m_camera_px_position.y);
    writeTextLabel(m_current_view_size + 10, 10 + k * 10, "On (" + strs.str() + "):", text_color); 
    k++;
    strs.str("");
    strs << m_current_values[mouse_position.x][mouse_position.y][0];
    writeTextLabel(m_current_view_size + 10, 10 + k * 10, "  Drift X: " + strs.str() + " m/s", text_color); 
    k++;
    strs.str("");
    strs << m_current_values[mouse_position.x][mouse_position.y][1];
    writeTextLabel(m_current_view_size + 10, 10 + k * 10, "  Drift Y: " + strs.str() + " m/s", text_color); 
    k++;
    strs.str("");
    strs << m_current_values[mouse_position.x][mouse_position.y][2];
    writeTextLabel(m_current_view_size + 10, 10 + k * 10, "  Drift Z: " + strs.str() + " m/s", text_color); 
  }
  
  if(!m_running_update)
  {
    writeTextLabel(m_current_view_size + 78, m_current_view_size - 40, timeToString(m_moos_time - m_starting_time), text_color);
    writeTextLabel(m_current_view_size + 35, m_current_view_size - 30, "Press T to update time", text_color);
    writeTextLabel(m_current_view_size + 35, m_current_view_size - 20, "Press R to update view", text_color);
  }
}


//---------------------------------------------------------
// Procedure: cameraMovement

void CurrentViewer::cameraMovement(int x, int y)
{
  m_camera_px_position.x += x;
  m_camera_px_position.y += y;
  
  if(x > 0)
    for(int i = 0 ; i < m_current_view_size ; i++)
      for(int j = 0 ; j < m_current_view_size ; j++)
      {
        if(i + x < m_current_view_size)
        {
          m_current_values[i][j][0] = m_current_values[i + x][j][0];
          m_current_values[i][j][1] = m_current_values[i + x][j][1];
          m_current_values[i][j][2] = m_current_values[i + x][j][2];
        }
          
        else
        {
          m_current_values[i][j][0] = 0.;
          m_current_values[i][j][1] = 0.;
          m_current_values[i][j][2] = 0.;
        }
      }
  
  if(x < 0)
    for(int i = m_current_view_size - 1 ; i >= 0 ; i--)
      for(int j = 0 ; j < m_current_view_size ; j++)
      {
        if(i + x > 0)
        {
          m_current_values[i][j][0] = m_current_values[i + x][j][0];
          m_current_values[i][j][1] = m_current_values[i + x][j][1];
          m_current_values[i][j][2] = m_current_values[i + x][j][2];
        }
          
        else
        {
          m_current_values[i][j][0] = 0.;
          m_current_values[i][j][1] = 0.;
          m_current_values[i][j][2] = 0.;
        }
      }
  
  if(y > 0)
    for(int i = 0 ; i < m_current_view_size ; i++)
      for(int j = 0 ; j < m_current_view_size ; j++)
      {
        if(j + y < m_current_view_size)
        {
          m_current_values[i][j][0] = m_current_values[i][j + y][0];
          m_current_values[i][j][1] = m_current_values[i][j + y][1];
          m_current_values[i][j][2] = m_current_values[i][j + y][2];
        }
        
        else
        {
          m_current_values[i][j][0] = 0.;
          m_current_values[i][j][1] = 0.;
          m_current_values[i][j][2] = 0.;
        }
      }
  
  if(y < 0)
    for(int i = 0 ; i < m_current_view_size ; i++)
      for(int j = m_current_view_size - 1 ; j >= 0 ; j--)
      {
        if(j + y > 0)
        {
          m_current_values[i][j][0] = m_current_values[i][j + y][0];
          m_current_values[i][j][1] = m_current_values[i][j + y][1];
          m_current_values[i][j][2] = m_current_values[i][j + y][2];
        }
        
        else
        {
          m_current_values[i][j][0] = 0.;
          m_current_values[i][j][1] = 0.;
          m_current_values[i][j][2] = 0.;
        }
      }
}


//---------------------------------------------------------
// Procedure: drawNodes

void CurrentViewer::drawNodes()
{
  SDL_Color white = {255, 255, 255};
  SDL_Rect node_position;
  SDL_Surface* surface_node_out = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8, 32, 0, 0, 0, 0);
  SDL_FillRect(surface_node_out, NULL, SDL_MapRGB(surface_node_out->format, 0, 0, 0));
  SDL_Surface* surface_node_in = SDL_CreateRGBSurface(SDL_HWSURFACE, 6, 6, 32, 0, 0, 0, 0);
  SDL_FillRect(surface_node_in, NULL, SDL_MapRGB(surface_node_in->format, 255, 255, 255));
  
  for(map<std::string,NodeRecord>::iterator node = m_map_node_records->begin() ; 
      node != m_map_node_records->end() ; 
      node++)
  {
    node_position.x = metersToPixels(node->second.getX()) - m_camera_px_position.x - 4;
    node_position.y = -metersToPixels(node->second.getY()) - m_camera_px_position.y - 4;
    SDL_BlitSurface(surface_node_out, NULL, m_screen, &node_position);
    node_position.x ++;
    node_position.y ++;
    SDL_BlitSurface(surface_node_in, NULL, m_screen, &node_position);
    writeTextLabel(node_position.x + 5, node_position.y - 5, node->first, white, true);
  }
  
  SDL_FreeSurface(surface_node_in);
  SDL_FreeSurface(surface_node_out);
}


//---------------------------------------------------------
// Procedure: drawArrows

void CurrentViewer::drawArrows()
{
  for(int i = 0 ; i < m_current_view_size ; i += m_arrow_gap)
    for(int j = 0 ; j < m_current_view_size ; j += m_arrow_gap)
      if(m_current_values[i][j][0] != 0 && m_current_values[i][j][1] != 0 && m_max_drift != 0)
        drawArrow(i,
                  j,
                  m_current_values[i][j][0], 
                  m_current_values[i][j][1],
                  m_max_drift);
}


//---------------------------------------------------------
// Procedure: drawProgressBar

void CurrentViewer::drawProgressBar()
{
  SDL_Rect progressbar_position;
  progressbar_position.x = 0;
  progressbar_position.y = m_current_view_size - 2;
  SDL_Surface* surface_progressbar = SDL_CreateRGBSurface(SDL_HWSURFACE, 
                                                          (int)(m_current_view_size * (m_update_progress / 100.)), 
                                                          2, 32, 0, 0, 0, 0);
  SDL_FillRect(surface_progressbar, NULL, SDL_MapRGB(surface_progressbar->format, 255, 255, 255));
  SDL_BlitSurface(surface_progressbar, NULL, m_screen, &progressbar_position);
  SDL_FreeSurface(surface_progressbar);
}


//---------------------------------------------------------
// Procedure: drawArrow

void CurrentViewer::drawArrow(double arrow_x, double arrow_y, double drift_x, double drift_y, double m_max_drift)
{
  int R, G, B;
  double current_intensity = hypot(drift_x, drift_y) / m_max_drift;
  
  if(m_display_background)
  { 
    R = 20; G = 20; B = 20;
  }
    
  else
  {
    double dR, dG, dB;
    HSV2RGB((1. - current_intensity) * 240., 1., 1., dR, dG, dB);
    R = (int)(dR * 255.);
    G = (int)(dG * 255.);
    B = (int)(dB * 255.);
  }
  
  double arrow_max_length = 0.8 * m_arrow_gap;
  double arrow_length = arrow_max_length * current_intensity;
  double arrow_tip_length = arrow_length / 3.5;
  double arrow_angle = atan2(drift_y, drift_x);
  
  double outside_angle = degToRadians(160.);
  double inside_angle = degToRadians(45.);
  
  double arrow_tip_x = arrow_x + arrow_length * cos(arrow_angle);
  double arrow_tip_y = arrow_y - arrow_length * sin(arrow_angle);
  
  lineRGBA(m_screen, (short)arrow_x, (short)arrow_y, (short)arrow_tip_x, (short)arrow_tip_y, R, G, B, 255);  
  
  short arrow_tip_xs[3];
  short arrow_tip_ys[3];
  
  arrow_tip_xs[0] = arrow_tip_x;
  arrow_tip_xs[1] = arrow_tip_x + cos(outside_angle + arrow_angle) * arrow_tip_length;
  arrow_tip_xs[2] = arrow_tip_x + cos(-outside_angle + arrow_angle) * arrow_tip_length;
  
  arrow_tip_ys[0] = arrow_tip_y;
  arrow_tip_ys[1] = arrow_tip_y - sin(outside_angle + arrow_angle) * arrow_tip_length;
  arrow_tip_ys[2] = arrow_tip_y -sin(-outside_angle + arrow_angle) * arrow_tip_length;
  
  // "MT": multi-threaded capable:
  filledPolygonRGBAMT(m_screen, arrow_tip_xs, arrow_tip_ys, 3, R, G, B, 255, NULL, NULL);
}


//---------------------------------------------------------
// Procedure: writeTextLabel

void CurrentViewer::writeTextLabel(int x, int y, string text, SDL_Color color, bool alpha_background)
{
  SDL_Surface* text_surface = NULL;
  SDL_Rect position;
  position.x = x;
  position.y = y;
  
  if(!(text_surface = TTF_RenderText_Blended(m_font, text.c_str(), color)))
  {
    cout << "CurrentViewer: TTF_RenderText_Blended error" << endl;
    exit(0);
  }
  
  else
  {
    if(alpha_background)
    {
      SDL_Rect text_background_area;
      text_background_area.x = x + 10 - 5;
      text_background_area.y = y;
      text_background_area.w = text_surface->w + 8;
      text_background_area.h = text_surface->h + 2;
      
      SDL_Surface* text_background_surface = getAlphaSurface(text_background_area);
      SDL_FillRect(text_background_surface, NULL, SDL_MapRGBA(text_background_surface->format, 0, 0, 0, 180));
      SDL_BlitSurface(text_background_surface, NULL, m_screen, &text_background_area);
      SDL_FreeSurface(text_background_surface);
    }
    
    position.x += 10;
    SDL_BlitSurface(text_surface, NULL, m_screen, &position);
    SDL_FreeSurface(text_surface);
  }
}


//---------------------------------------------------------
// Procedure: drawInfoBar

void CurrentViewer::drawInfoBar(SDL_Rect mouse_position, bool active_selection, SDL_Rect selection_area)
{
  SDL_Rect infobar_area;
  infobar_area.x = 0;
  infobar_area.y = m_current_view_size - 20;
  infobar_area.w = m_current_view_size;
  infobar_area.h = 20;
  
  SDL_Surface* surface_infobar = getAlphaSurface(infobar_area);
  SDL_FillRect(surface_infobar, NULL, SDL_MapRGBA(surface_infobar->format, 0, 0, 0, 180));
  SDL_BlitSurface(surface_infobar, NULL, m_screen, &infobar_area);
  SDL_FreeSurface(surface_infobar);
  
  ostringstream strs;
  SDL_Color color_white = {255, 255, 255};
  
  // Information: mouse position
  if(mouse_position.x <= m_current_view_size)
  {
    double m_x, m_y;
    pixelsPosToMetersPos(mouse_position.x, mouse_position.y, m_x, m_y);
    strs << (int)m_x << "," << (int)m_y;
    writeTextLabel(5, m_current_view_size - 19, strs.str(), color_white);
  }

  // Information: selection dimension
  if(active_selection)
  {
    strs.str("");
    strs << (int)pixelsToMeters(selection_area.w) << "," << (int)pixelsToMeters(selection_area.h);
    writeTextLabel(m_current_view_size / 4., m_current_view_size - 19, strs.str(), color_white);
  }
  
  // Information: progression value
  if((int)m_update_progress < 100)
  {
    strs.str("");
    strs << (int)m_update_progress << "%";
    writeTextLabel(m_current_view_size / 2., m_current_view_size - 19, strs.str(), color_white);
  }
  
  // Information: capture time
  writeTextLabel(m_current_view_size - 100, m_current_view_size - 19, timeToString(m_capture_time - m_starting_time), color_white);
  
  // Information: operational depth
  strs.str("");
  strs << (int)m_operation_depth << "m";
  writeTextLabel(m_current_view_size - 35, m_current_view_size - 19, strs.str(), color_white);
}


//---------------------------------------------------------
// Procedure: timeToString

string CurrentViewer::timeToString(double time)
{
  ostringstream strs;
  
  int hour = (int)time / 3600;
  int minutes = ((int)time % 3600) / 60;
  int seconds = (int)time % 60;
  
  if(hour < 10)
    strs << "0";
    
  strs << hour << ":";
    
  if(minutes < 10)
    strs << "0";
    
  strs << minutes << ":";
    
  if(seconds < 10)
    strs << "0";
    
  strs << seconds;
  
  return strs.str();
}


//---------------------------------------------------------
// Procedure: getAlphaSurface

SDL_Surface* CurrentViewer::getAlphaSurface(SDL_Rect selection_area)
{
  Uint32 rmask, gmask, bmask, amask;

  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
  #else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
  #endif

  return SDL_CreateRGBSurface(0, 
                              selection_area.w, selection_area.h, 
                              32, 
                              rmask, gmask, bmask, amask);
}


//---------------------------------------------------------
// Procedure: drawSelectionRectangle

void CurrentViewer::drawSelectionRectangle(SDL_Rect selection_area)
{
  SDL_Surface* surface_selection = getAlphaSurface(selection_area);
  SDL_FillRect(surface_selection, NULL, SDL_MapRGBA(surface_selection->format, 0, 0, 0, 120));
  SDL_BlitSurface(surface_selection, NULL, m_screen, &selection_area);
  SDL_FreeSurface(surface_selection);
}


//---------------------------------------------------------
// Procedure: drawMeterScale

void CurrentViewer::drawMeterScale(int x_pos, int y_pos)
{
  int scale_width = m_current_view_size / 3; // px
  ostringstream strs;
  strs << (int)pixelsToMeters(scale_width) << "m";
  
  SDL_Color white = {255, 255, 255};
  SDL_Surface* text_surface = NULL;
  
  if(!(text_surface = TTF_RenderText_Blended(m_font, strs.str().c_str(), white)))
  {
    cout << "CurrentViewer: TTF_RenderText_Blended error" << endl;
    exit(0);
  }
  
  // Background
  SDL_Rect scale_background_area;
  scale_background_area.x = x_pos - 5;
  scale_background_area.y = y_pos - 10;
  scale_background_area.w = scale_width + 10;
  scale_background_area.h = 30;
  
  SDL_Surface* scale_background_surface = getAlphaSurface(scale_background_area);
  SDL_FillRect(scale_background_surface, NULL, SDL_MapRGBA(scale_background_surface->format, 0, 0, 0, 100));
  SDL_BlitSurface(scale_background_surface, NULL, m_screen, &scale_background_area);
  SDL_FreeSurface(scale_background_surface);
  
  // Scale
  SDL_Rect position;
  position.x = x_pos;
  position.y = y_pos;
  
  SDL_Surface* vline = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, 7, 32, 0, 0, 0, 0);
  SDL_Surface* hline = SDL_CreateRGBSurface(SDL_HWSURFACE, scale_width, 1, 32, 0, 0, 0, 0);
  SDL_FillRect(hline, NULL, SDL_MapRGB(hline->format, 255, 255, 255));
  SDL_FillRect(vline, NULL, SDL_MapRGB(hline->format, 255, 255, 255));
  
  position.x = x_pos;
  position.y = y_pos;
  SDL_BlitSurface(hline, NULL, m_screen, &position);
  
  position.x = x_pos;
  position.y = y_pos - 7;
  SDL_BlitSurface(vline, NULL, m_screen, &position);
  
  position.x = x_pos + scale_width - 1;
  position.y = y_pos - 7;
  SDL_BlitSurface(vline, NULL, m_screen, &position);
  
  position.x = x_pos + (scale_width / 2) - (text_surface->w / 2);
  position.y = y_pos;
  SDL_BlitSurface(text_surface, NULL, m_screen, &position);
  
  SDL_FreeSurface(hline);
  SDL_FreeSurface(vline);
  SDL_FreeSurface(text_surface);
}


//---------------------------------------------------------
// Procedure: drawIntensityScale

void CurrentViewer::drawIntensityScale(int x_pos, int y_pos, int scale_width, SDL_Rect mouse_position)
{
  ostringstream strs;
  strs.precision(2);
  strs << fixed;
  strs << m_max_drift;
  SDL_Color white = {255, 255, 255};
  
  SDL_Surface* text_surface_1 = NULL;
  if(!(text_surface_1 = TTF_RenderText_Blended(m_font, "0.00", white)))
  {
    cout << "CurrentViewer: TTF_RenderText_Blended error" << endl;
    exit(0);
  }
  
  SDL_Surface* text_surface_2 = NULL;
  if(!(text_surface_2 = TTF_RenderText_Blended(m_font, strs.str().c_str(), white)))
  {
    cout << "CurrentViewer: TTF_RenderText_Blended error" << endl;
    exit(0);
  }
  
  // Background
  SDL_Rect scale_background_area;
  scale_background_area.x = x_pos - 5;
  scale_background_area.y = y_pos - 10;
  scale_background_area.w = scale_width + 10 + text_surface_1->w + text_surface_2->w;
  scale_background_area.h = 30;
  
  SDL_Surface* scale_background_surface = getAlphaSurface(scale_background_area);
  SDL_FillRect(scale_background_surface, NULL, SDL_MapRGBA(scale_background_surface->format, 0, 0, 0, 100));
  SDL_BlitSurface(scale_background_surface, NULL, m_screen, &scale_background_area);
  SDL_FreeSurface(scale_background_surface);
  
  // Scale
  SDL_Rect position;
  
  SDL_Surface* background = SDL_CreateRGBSurface(SDL_HWSURFACE, scale_width, 12, 32, 0, 0, 0, 0);
  SDL_FillRect(background, NULL, SDL_MapRGB(background->format, 255, 255, 255));
  
  position.x = x_pos + text_surface_1->w;
  position.y = y_pos - 5;
  SDL_BlitSurface(background, NULL, m_screen, &position);
  int R, G, B;
  double dR, dG, dB;
  
  for(int i = 0 ; i < scale_width - 2 ; i++)
  {
    SDL_Surface* intensity_line = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, 10, 32, 0, 0, 0, 0);
    
    HSV2RGB((1. - (i * 1. / scale_width)) * 240., 1., 1., dR, dG, dB);
    R = (int)(dR * 255.);
    G = (int)(dG * 255.);
    B = (int)(dB * 255.);
  
    SDL_FillRect(intensity_line, NULL, SDL_MapRGB(intensity_line->format, R, G, B));
    position.x = x_pos + 1 + i + text_surface_1->w;
    position.y = y_pos - 4;
    SDL_BlitSurface(intensity_line, NULL, m_screen, &position);
    SDL_FreeSurface(intensity_line);
  }
  
  // Cursor on scale:
  if(mouse_position.x < m_current_view_size && m_max_drift != 0)
  {
    double drift = getDriftIntensity(mouse_position.x, mouse_position.y);
    int x_scale = (int)((drift / m_max_drift) * scale_width);
    SDL_Surface* intensity_line_black = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, 12, 32, 0, 0, 0, 0);
    SDL_FillRect(intensity_line_black, NULL, SDL_MapRGB(intensity_line_black->format, 0, 0, 0));
    position.x = x_pos + 1 + x_scale + text_surface_1->w;
    position.y = y_pos - 5;
    SDL_BlitSurface(intensity_line_black, NULL, m_screen, &position);
    SDL_FreeSurface(intensity_line_black);
    
    strs.str("");
    strs.precision(4);
    strs << drift;
    SDL_Color white = {255, 255, 255};
    
    SDL_Surface* text_surface_cursor = NULL;
    if(!(text_surface_cursor = TTF_RenderText_Blended(m_font, strs.str().c_str(), white)))
    {
      cout << "CurrentViewer: TTF_RenderText_Blended error" << endl;
      exit(0);
    }
    
    position.x -= text_surface_cursor->w / 2;
    position.y = y_pos + 4;
    SDL_BlitSurface(text_surface_cursor, NULL, m_screen, &position);
  }
          
  position.x = x_pos - 2;
  position.y = y_pos - 7;
  SDL_BlitSurface(text_surface_1, NULL, m_screen, &position);
  
  position.x = x_pos + 3 + text_surface_1->w + scale_width;
  position.y = y_pos - 7;
  SDL_BlitSurface(text_surface_2, NULL, m_screen, &position);
  
  SDL_FreeSurface(background);
  SDL_FreeSurface(text_surface_1);
  SDL_FreeSurface(text_surface_2);
}


//---------------------------------------------------------
// Procedure: getDriftIntensity(int i, int j)

double CurrentViewer::getDriftIntensity(int i, int j)
{
  double drift_x = m_current_values[i][j][0];
  double drift_y = m_current_values[i][j][1];
  return hypot(drift_x, drift_y);
}


//---------------------------------------------------------
// Procedure: getColorFromCurrentValue

void CurrentViewer::getColorFromCurrentValue(int i, int j, int& R, int& G, int& B)
{
  double dR, dG, dB, S = 1., V = 1.;
  
  double drift_x = m_current_values[i][j][0];
  double drift_y = m_current_values[i][j][1];
  
  double color_intensity;
  
  if(m_max_drift == 0.)
    color_intensity = 0.;
  
  else
    color_intensity = getDriftIntensity(i, j) / m_max_drift;
  
  if(color_intensity > 1.)
    color_intensity = 1.;
  
  HSV2RGB((1. - color_intensity) * 240., S, V, dR, dG, dB);
  R = (int)(dR * 255.);
  G = (int)(dG * 255.);
  B = (int)(dB * 255.);
}


//---------------------------------------------------------
// Procedure: setPixelFromCurrentValue

void CurrentViewer::setPixelFromCurrentValue(int x, int y, bool display_grid)
{
  int R, G, B;
  SDL_Rect px;
  px.x = x;
  px.y = y;
  px.w = 1;
  px.h = 1;
  
  if(display_grid && 
      ((int)fmod(pixelsToMeters(x + m_camera_px_position.x), 300.) == 0 || 
        (int)fmod(pixelsToMeters(y + m_camera_px_position.y), 300.) == 0))
  {
    R = 255; G = 255; B = 255;
  }
  
  else
    getColorFromCurrentValue(x, y, R, G, B);
  
  SDL_FillRect(m_screen, &px, SDL_MapRGB(m_screen->format, R, G, B));
}


//---------------------------------------------------------
// Procedure: fullUpdate

bool CurrentViewer::fullUpdate()
{
  return update(false);
}


//---------------------------------------------------------
// Procedure: lowUpdate

bool CurrentViewer::lowUpdate()
{
  return update(true);
}


//---------------------------------------------------------
// Procedure: update

bool CurrentViewer::update(bool low_update)
{
  if(m_running_update)
    return true;
  
  if(m_auto_focus_on_nodes)
    focusCameraOnNodes();
  
  m_running_update = true;
  
  m_max_drift = 0.;
  double drift_x, drift_y, drift_z, drift, m_x, m_y;
  m_update_progress = 0.;
  int update_nb = 0;
  
  for(int i = 0 ; i < m_current_view_size ; i++)
    for(int j = 0 ; j < m_current_view_size ; j++)
      if(!low_update || (m_current_values[i][j][0] == 0 && m_current_values[i][j][1] == 0))
        update_nb ++;
  
  int k = 0;
  
  for(int i = 0 ; i < m_current_view_size ; i++)
  {
    for(int j = 0 ; j < m_current_view_size ; j++)
    {
      if(true || !low_update || (m_current_values[i][j][0] == 0 && m_current_values[i][j][1] == 0))
      {
        pixelsPosToMetersPos(i, j, m_x, m_y);
        if(!CurrentModel::getCurrentFromListOfModels(
                      (*m_current_models),
                      m_x,
                      m_y,
                      m_operation_depth,
                      m_capture_time - m_starting_time,
                      drift_x,
                      drift_y,
                      drift_z))
        {
          cout << "CurrentViewer: update error" << endl;
          m_running_update = false;
          return false;
        }
        
        else
        {
          m_current_values[i][j][0] = drift_x;
          m_current_values[i][j][1] = drift_y;
          m_current_values[i][j][2] = drift_z;
        }
        
        k ++;
        m_update_progress = k * 100. / update_nb;
      }
      
      drift = hypot(m_current_values[i][j][0], 
                    m_current_values[i][j][1]);
      
      if(m_max_drift < drift)
        m_max_drift = drift;
    }
  }
  
  m_running_update = false;
  return true;
}


//---------------------------------------------------------
// Procedure: HSV2RGB

bool CurrentViewer::HSV2RGB(double H, double S, double V, double& R, double& G, double& B)
{
  double c = 0.0, m = 0.0, x = 0.0;
  
  if(H >= 0. && H < 360. && S >= 0. && S <= 1. && V >= 0. && V <= 1.)
  {
    c = V * S;
    x = c * (1.0 - fabs(fmod(H / 60.0, 2) - 1.0));
    m = V - c;
    
    if(H >= 0.0 && H < 60.0)
    {
      R = c + m;
      G = x + m;
      B = m;
    }
    
    else if(H >= 60.0 && H < 120.0)
    {
      R = x + m;
      G = c + m;
      B = m;
    }
    
    else if(H >= 120.0 && H < 180.0)
    {
      R = m;
      G = c + m;
      B = x + m;
    }

    else if(H >= 180.0 && H < 240.0)
    {
      R = m;
      G = x + m;
      B = c + m;
    }

    else if(H >= 240.0 && H < 300.0)
    {
      R = x + m;
      G = m;
      B = c + m;
    }

    else if(H >= 300.0 && H < 360.0)
    {
      R = c + m;
      G = m;
      B = x + m;
    }

    else
    {
      R = m;
      G = m;
      B = m;
    }
    
    return true;
  }
  
  return false;
}


//---------------------------------------------------------
// Procedure: pixelsPosToMetersPos

void CurrentViewer::pixelsPosToMetersPos(int px_x, int px_y, double& m_x, double& m_y)
{
  m_x = pixelsToMeters(px_x + m_camera_px_position.x);
  m_y = -pixelsToMeters(px_y + m_camera_px_position.y);
}


//---------------------------------------------------------
// Procedure: metersPosToPixelsPos

void CurrentViewer::metersPosToPixelsPos(double m_x, double m_y, int& px_x, int& px_y)
{
  px_x = metersToPixels(m_x) - m_camera_px_position.x;
  px_x = metersToPixels(-m_y) - m_camera_px_position.y;
}


//---------------------------------------------------------
// Procedure: focusCameraOnNodes

void CurrentViewer::focusCameraOnNodes()
{
  bool init = false;
  int min_px_x, max_px_x, min_px_y, max_px_y;
  double min_x = 0., max_x = 0., min_y = 0., max_y = 0.;
  
  for(map<std::string,NodeRecord>::iterator node = m_map_node_records->begin() ; 
      node != m_map_node_records->end() ; 
      node++)
  {
    if(!init || min_x > node->second.getX())
      min_x = node->second.getX();
    
    if(!init || min_y > node->second.getY())
      min_y = node->second.getY();
    
    if(!init || max_x < node->second.getX())
      max_x = node->second.getX();
    
    if(!init || max_y < node->second.getY())
      max_y = node->second.getY();
    
    if(!init)
      init = true;
  }
  
  min_px_x = metersToPixels(min_x);
  min_px_y = metersToPixels(min_y);
  max_px_x = metersToPixels(max_x);
  max_px_y = metersToPixels(max_y);
  
  m_camera_px_position.x = (min_px_x + max_px_x - metersToPixels(m_area_size)) / 2.;
  m_camera_px_position.y = -(metersToPixels(m_area_size) / 2) - ((min_px_y + max_px_y) / 2.);
}

