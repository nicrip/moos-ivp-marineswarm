/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: SimShot_CGG_Test.cpp                                 */
/*   DATE: june 2014                                            */
/* ************************************************************ */

#include "SimShot_CGG.h"
#include "SimShot_CGG_Test.h"
#include "Tests/UnitTests.h"

void SimShot_launchTestsAndExitIfOk()
{
	bool test;
	UnitTests session("uSimShot_CGG");
	session.run();
}
