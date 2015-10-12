/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: CurrentMOOSApp.h                                     */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_CGG_VIEWER_HEADER
#define CURRENT_CGG_VIEWER_HEADER

#include <map>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "Current/CurrentModel.h"
#include "NodeRecord.h"

class CurrentViewer
{
  public:
    CurrentViewer(std::vector<CurrentModel*>* current_models, 
                  std::map<std::string,NodeRecord>* map_node_records, 
                  double starting_moos_time);
    ~CurrentViewer();
    bool init();
    bool update(bool low_update);
    bool fullUpdate();
    bool lowUpdate();
    void run();
    void quit();
    
    // Getters:
    int getWindowSize();
    double getAreaSize();
    int getArrowGap();
    double getMOOSTime();
    double getOperationDepth();
    
    // Setters:
    void setWindowSize(int size);
    void setAreaSize(double size);
    void setArrowGap(int gap);
    void setLowDisplayMode(bool low_mode);
    void setAutoFocusOnNodes(bool auto_focus);
    void setMOOSTime(double moos_time);
    void enableBackground(bool enable);
    void enableNodesDisplay(bool enable);
    void setOperationDepth(double depth);
    void setPathToFontsDirectory(std::string path);

  protected:
    void getColorFromCurrentValue(int i, int j, int& R, int& G, int& B);
    void setPixelFromCurrentValue(int x, int y, bool display_grid);
    bool HSV2RGB(double H, double S, double V, double& R, double& G, double& B);
    void drawSelectionRectangle(SDL_Rect selection_area);
    SDL_Surface* getAlphaSurface(SDL_Rect selection_area);
    void drawInfoBar(SDL_Rect mouse_position, bool active_selection, SDL_Rect selection_area);
    void drawArrow(double arrow_x, double arrow_y, double drift_x, double drift_y, double m_max_drift);
    void drawArrows();
    void drawNodes();
    double pixelsToMeters(int px);
    int metersToPixels(double m);
    void cameraMovement(int x, int y);
    void writeTextLabel(int x, int y, std::string text, SDL_Color color, bool alpha_background = false);
    void reverseCurrentModelActivation(int current_model_id);
    std::string timeToString(double time);
    void drawProgressBar();
    void drawTextPanel(SDL_Rect mouse_position);
    void drawMeterScale(int x_pos, int y_pos);
    void drawIntensityScale(int x_pos, int y_pos, int width, SDL_Rect mouse_position);
    void pixelsPosToMetersPos(int px_x, int px_y, double& m_x, double& m_y);
    void metersPosToPixelsPos(double m_x, double m_y, int& px_x, int& px_y);
    double getDriftIntensity(int i, int j);
    void focusCameraOnNodes();
        
  private:
    std::string m_path_to_fonts_directory;
    int m_current_view_size;
    int m_text_view_size;
    int m_arrow_gap;
    bool m_low_display_mode;
    double m_area_size;
    double m_operation_depth;
    double m_moos_time;
    double m_capture_time;
    double m_starting_time;
    double m_update_progress;
    double m_max_drift;
    bool m_running;
    bool m_running_update;
    bool m_display_background;
    bool m_nodes_display;
    bool m_display_grid;
    bool m_auto_focus_on_nodes;
    SDL_Surface* m_screen;
    SDL_Rect m_camera_px_position;
    TTF_Font* m_font;
    
    // Current models
    std::vector<CurrentModel*>* m_current_models;
    
    // Current values: <drift_x,drift_y,drift_z>
    double*** m_current_values;
    
    // Nodes: <vehicle_name,node_record>
    std::map<std::string,NodeRecord>* m_map_node_records;
};

#endif 
