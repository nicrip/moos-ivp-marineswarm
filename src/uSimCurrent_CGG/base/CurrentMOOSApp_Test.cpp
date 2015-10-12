/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: CurrentMOOSApp_Test.cpp                              */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include "CurrentMOOSApp.h"
#include "CurrentMOOSApp_Test.h"
#include "Tests/UnitTests.h"

void CurrentMOOSApp_launchTestsAndExitIfOk()
{
	bool test;
	UnitTests session("uSimCurrent_CGG");
	session.run();
}
