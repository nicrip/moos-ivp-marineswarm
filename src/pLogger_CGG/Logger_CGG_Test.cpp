/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: Logger_CGG_Test.cpp                                  */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include "Logger_CGG.h"
#include "Logger_CGG_Test.h"
#include "Tests/UnitTests.h"

void Logger_launchTestsAndExitIfOk()
{
	bool test;
	UnitTests session("pLogger_CGG");
	session.run();
}
