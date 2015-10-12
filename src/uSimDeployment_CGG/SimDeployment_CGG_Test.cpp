/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: SimDeployment_CGG_Test.cpp                           */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include "SimDeployment_CGG.h"
#include "SimDeployment_CGG_Test.h"
#include "Tests/UnitTests.h"

void SimDeployment_launchTestsAndExitIfOk()
{
	bool test;
	UnitTests session("uSimDeployment_CGG");
	session.run();
}
