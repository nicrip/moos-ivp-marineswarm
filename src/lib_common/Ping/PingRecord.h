/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: PingRecord.h                                    */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#ifndef PingRecord_HEADER
#define PingRecord_HEADER

#include <string>

class PingRecord
{
  public:
  PingRecord(): m_modem_id(0)
    { m_previous_gap_with_receiver = 0;  };
    ~PingRecord() {};

    void setStartPosX(double x) { m_start_pos_x = x; };
    void setStartPosY(double y) { m_start_pos_y = y; };
    void setStartDepth(double d) { m_start_depth = d; };
    void setSenderName(std::string v) { m_sender_name = v; };
    void setSenderHeading(double v) { m_sender_heading = v; };
    void setStartingTime(double v) { m_starting_time = v; };
    void setRange(double v) { m_range = v; };
    void setPreviousGapWithReceiver(double v) { m_previous_gap_with_receiver = v; };
    void setPreviousDate(double v) { m_previous_date = v; };
    void setData(const std::string& data) { m_data = data; }
    void setModemId(int id) { m_modem_id = id; }

    double getStartPosX() const { return m_start_pos_x; };
    double getStartPosY() const { return m_start_pos_y; };
    double getStartDepth() const { return m_start_depth; };
    std::string getSenderName() const { return m_sender_name; };
    double getSenderHeading() const { return m_sender_heading; };
    double getStartingTime() const { return m_starting_time; };
    double getRange() const { return m_range; };
    double getPreviousGapWithReceiver() const { return m_previous_gap_with_receiver; };
    double getPreviousDate() const { return m_previous_date; };
    std::string getData() const { return m_data; } 
    int getModemId() const { return m_modem_id; }
                
  protected:
    double m_start_pos_x, m_start_pos_y, m_start_depth;
    double m_previous_gap_with_receiver, m_previous_date;
    double m_starting_time, m_range;
    std::string m_sender_name;
    double m_sender_heading;
    std::string m_data;
    int m_modem_id;
};

#endif 
