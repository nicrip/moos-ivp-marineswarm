/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: main.cpp                                         */
/*   DATE: june 2014                                        */
/* ******************************************************** */

#include <string>
#include "MBUtils.h"
#include "ColorParse.h"
#include "SimShot_CGG.h"
#include "SimShot_CGG_Info.h"
#include "SimShot_CGG_Test.h"

using namespace std;

void launchTestsAndExit();

int main(int argc, char *argv[])
{
  string mission_file;
  string run_command = argv[0];

  for(int i=1; i<argc; i++) {
    string argi = argv[i];
    if((argi=="-v") || (argi=="--version") || (argi=="-version"))
      showReleaseInfoAndExit();
    else if((argi=="-e") || (argi=="--example") || (argi=="-example"))
      showExampleConfigAndExit();
    else if((argi == "-h") || (argi == "--help") || (argi=="-help"))
      showHelpAndExit();
    else if((argi == "-i") || (argi == "--interface"))
      showInterfaceAndExit();
    else if((argi == "-t") || (argi == "--test"))
      SimShot_launchTestsAndExitIfOk();
    else if(strEnds(argi, ".moos") || strEnds(argi, ".moos++"))
      mission_file = argv[i];
    else if(strBegins(argi, "--alias="))
      run_command = argi.substr(8);
    else if(i==2)
      run_command = argi;
  }
  
  if(mission_file == "")
    showHelpAndExit();

  cout << termColor("green");
  cout << "uSimShot_CGG launching as " << run_command << endl;
  cout << termColor() << endl;

  SimShot SimShot;

  SimShot.Run(run_command.c_str(), mission_file.c_str());
  
  return(0);
}
