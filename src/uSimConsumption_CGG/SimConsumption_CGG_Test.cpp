/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: SimConsumption_CGG_Test.cpp                          */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include "SimConsumption_CGG.h"
#include "SimConsumption_CGG_Test.h"
#include "Tests/UnitTests.h"

void SimConsumption_launchTestsAndExitIfOk()
{
	bool test;
	UnitTests session("uSimConsumption_CGG");
	session.run();
}
