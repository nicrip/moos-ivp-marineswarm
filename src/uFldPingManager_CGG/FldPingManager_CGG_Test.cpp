/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: FldPingManager_CGG_Test.cpp                          */
/*   DATE: february 2014                                        */
/* ************************************************************ */

#include "FldPingManager_CGG.h"
#include "FldPingManager_CGG_Test.h"
#include "Tests/UnitTests.h"
#include "Ping/PingRecord.h"
#include "Acoustic/SoundPropagationInWater.h"
#include "CoordinateSystem/SphericalCoordinateSystem.h"

void FldPingManager_launchTestsAndExitIfOk()
{
	bool test;
	UnitTests session("uFldPingManager_CGG");
	
  /* DEPRECATED
  
    // ping manager
    FldPingManager FPM_Test;
    session.add(FPM_Test.OnStartUp(), "PingManager: OnStartUp execution");
    session.add(FPM_Test.OnConnectToServer(), "PingManager: OnConnectToServer execution");
    session.add(FPM_Test.Iterate(), "PingManager: Iterate execution");
    session.add(FPM_Test.GetRandomRange(400.0) >= 400.0, "PingManager: GetRandomRange");
    FPM_Test.m_sound_speed_min = 1480.;
    FPM_Test.m_sound_speed_max = 1520.;
    FPM_Test.m_sound_speed_variation = false;
    PingRecord ping_to_receive;
    ping_to_receive.setStartPosX(2.);
    ping_to_receive.setStartPosY(2.);
    ping_to_receive.setStartDepth(200.);
    ping_to_receive.setStartingTime(MOOSTime() - (200. / FPM_Test.GetSoundSpeed()));
    NodeRecord node_receiver("foxtrot_receiver", "UUV");
    node_receiver.setX(302.);
    node_receiver.setY(2.);
    node_receiver.setDepth(200.);
    double rho, phi, theta;
    FPM_Test.GetCurrentGap(ping_to_receive, node_receiver,
                FPM_Test.GetSoundSpeed(), MOOSTime(),
                rho, phi, theta);
    session.add(rho - 100. < 0.1, "PingManager: GetCurrentGap");
                    
    // before receiving a ping, we need to know the sender and the receivers
    
    // a sender named alpha_test
      string mail_content = "NAME=alpha_test,TYPE=uuv,TIME=1195844687.236,";
      mail_content += "X=37.49,Y=-47.36,SPD=2.40,HDG=11.17,";
      mail_content += "LAT=43.82507169,LON=-70.33005531,TYPE=KAYAK,MODE=DRIVE,";
      mail_content += "ALLSTOP=clear,index=36,DEP=0,LENGTH=4";
      
      CMOOSMsg mail_node_report_alpha('N', "NODE_REPORT", mail_content);
      // we need to specify the originating community:
      mail_node_report_alpha.m_sOriginatingCommunity = "alpha_test";
      FPM_Test.HandleNewNodeReport(mail_node_report_alpha);
        
      bool the_node_is_saved = (FPM_Test.m_map_node_records.count("alpha_test") == 1);
      session.add(the_node_is_saved, "PingManager: testing new mail 'NODE_REPORT' (alpha_test)");
    
    // a receiver named bravo_test
      mail_content = "NAME=bravo_test,TYPE=uuv,TIME=1195844125.236,";
      mail_content += "X=35.24,Y=-82.55,SPD=4.50,HDG=13.25,";
      mail_content += "LAT=45.82545163,LON=-42.33022531,TYPE=KAYAK,MODE=DRIVE,";
      mail_content += "ALLSTOP=clear,index=36,DEP=0,LENGTH=4";
      CMOOSMsg mail_node_report_bravo('N', "NODE_REPORT", mail_content);
      // we need to specify the originating community:
      mail_node_report_bravo.m_sOriginatingCommunity = "bravo_test";
      FPM_Test.HandleNewNodeReport(mail_node_report_bravo);
        
      the_node_is_saved = (FPM_Test.m_map_node_records.count("bravo_test") == 1);
      session.add(the_node_is_saved, "PingManager: testing new mail 'NODE_REPORT' (bravo_test)");
      
    if(the_node_is_saved)
    {
      // now, the sender is well known ; we can send a ping in the environment:
      
      int ping_number = FPM_Test.m_map_pings_to_be_transmitted["bravo_test"].size();
      
      mail_content = "x=37.49,y=-47.36,depth=235.23,date=175755577.285,name=alpha_test";
      CMOOSMsg mail_ping_from_alpha('N', "PING", mail_content);
      // we need to specify the originating community:
      mail_ping_from_alpha.m_sOriginatingCommunity = "alpha_test";
      FPM_Test.HandleNewPing(mail_ping_from_alpha);
        
      bool a_ping_is_received = (FPM_Test.m_map_pings_to_be_transmitted["bravo_test"].size() == ping_number + 1);
      session.add(a_ping_is_received, "PingManager: testing new mail 'PING'");
      
      if(a_ping_is_received)
      {
        // we can now analyse the ping
        PingRecord last_ping = FPM_Test.m_map_pings_to_be_transmitted["bravo_test"].back();
        session.add(last_ping.getStartPosX() == 37.49 &&
              last_ping.getStartPosY() == -47.36 &&
              last_ping.getStartDepth() == 235.23 &&
              last_ping.getStartingTime() == 175755577.285 &&
              last_ping.getSenderName() == "alpha_test", "PingManager: testing ping analysing");
        
        session.add(FPM_Test.RequestQuit(), "PingManager: exiting");
      }
    }
	*/
  
	session.run();
}
