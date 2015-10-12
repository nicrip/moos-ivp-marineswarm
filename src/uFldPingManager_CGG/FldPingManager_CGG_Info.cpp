/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: FldPingManager_CGG_Info.cpp                          */
/*   DATE: february 2014                                        */
/* ************************************************************ */

#include <cstdlib>
#include <iostream>
#include "FldPingManager_CGG_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("The uFldPingManager application is a tool for simulate communi- ");
  blk("cation using submerged means. It can be used as part of a commu-");
  blk("nication between several AUVs. In practise, the submarine commu-");
  blk("nication use acoustic means with (very) low frequencies so that ");
  blk("it is difficult to share heavy message data. Thus, this applica-");
  blk("tion is based on the emission/reception of short pings in order ");
  blk("to calculate the distance and angles between two vehicles (and  ");
  blk("then compute their relative position). We assumed that a vehicle");
  blk("can send a ping, receive another and is able to know the direc- ");
  blk("tion from which the ping comes (bearing). Contrary to sonar ap- ");
  blk("plications, the rebound of the signals is not studied.          ");
  blk("All the vehicles ping at the same time and listen the foreign   ");
  blk("answers.                                                        ");
  blk("Used in a shoreside community.                                  ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: uFldPingManager_CGG file.moos [OPTIONS]                  ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch uFldPingManager with the given process name        ");
  blk("      rather than uFldPingManager.                              ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of uFldPingManager.           ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uFldPingManager_CGG Example MOOS Configuration                  ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = uFldPingManager_CGG                             ");
  blk("{                                                               ");
  blk("  AppTick           = 100                                       ");
  blk("  CommsTick         = 100                                       ");
  blk("                                                                ");
  blk("  ACOUSTIC_RANGE    = 400.0                                     ");
  blk("  WATER_TEMPERATURE = 12.0                                      ");
  blk("  WATER_SALINITY    = 35.0                                      ");
  blk("                                                                ");
  blk("  EXPONENTIALLY_DECAYING_DETECTION = true                       ");
  blk("}                                                               ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showInterfaceAndExit

void showInterfaceAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uFldPingManager_CGG INTERFACE                                   ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  PING = x=<v>,y=<v>,depth=<v>,date=<v>,name=<v>                ");
  blk("  NODE_REPORT = NAME=<v>,TYPE=<v>,X=<v>,Y=<v>,DEPTH=<v>         ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  PING_REPORT_FOR_$V = dt=<v>,theta=<v>,phi=<v>,name=<v>        ");
  blk("  VIEW_COMMS_PULSE                                              ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("uFldPingManager_CGG", "gpl");
  exit(0);
}

