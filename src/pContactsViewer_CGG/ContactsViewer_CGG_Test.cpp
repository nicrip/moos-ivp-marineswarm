/* ************************************************************ */
/*   NAME: Simon Rohou                                          */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: ContactsViewer_CGG_Test.cpp                          */
/*   DATE: June 2014                                            */
/* ************************************************************ */

#include "ContactsViewer_CGG.h"
#include "ContactsViewer_CGG_Test.h"
#include "Tests/UnitTests.h"

void ContactsViewer_launchTestsAndExitIfOk()
{
	bool test;
	UnitTests session("pContactsViewer_CGG");
	session.run();
}
