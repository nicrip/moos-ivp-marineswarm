/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: main.cpp                                        */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#include <string>
#include "MBUtils.h"
#include "ColorParse.h"
#include "FldPingManager_CGG.h"
#include "FldPingManager_CGG_Info.h"
#include "FldPingManager_CGG_Test.h"

using namespace std;

int main(int argc, char * argv[])
{
    return goby::moos::run<FldPingManager>(argc, argv);
}
