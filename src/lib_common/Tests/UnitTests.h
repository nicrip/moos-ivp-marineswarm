/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: UnitTests.h                                     */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#ifndef TESTS
#define TESTS

#include <math.h>
#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <cstdlib> 
#include "ColorParse.h"

#define	TESTS_VALID_MESSAGE_DURATION		12	// sec
#define	TESTS_ERROR_MESSAGE_COLOR			  "red"
#define TESTS_VALID_MESSAGE_COLOR			  "green"
#define TESTS_DEACTIVATE_COUT				    1
#define EPSILON								          0.000001

using namespace std;

class UnitTests
{
	public:
		UnitTests(string application_name)
		{
			this->application_name = application_name;
			this->tests_number = 0;
			this->valide_tests_number = 0;
			this->tests_ok = true;
			
			inhibitCout();
		}
		
		void inhibitCout()
		{
			if(TESTS_DEACTIVATE_COUT)
			{
				this->filestr.open("/dev/null");
				this->backup = cout.rdbuf();
				this->psbuf = this->filestr.rdbuf();
				cout.rdbuf(this->psbuf);
			}
		}
		
		void reactivateCout()
		{
			if(TESTS_DEACTIVATE_COUT)
			{
				cout.rdbuf(backup);
				filestr.close();
			}
		}
		
		void run()
		{
			reactivateCout();
			
			if(this->tests_number != 0)
			{
				if(this->tests_ok)
				{
					cout << termColor(TESTS_VALID_MESSAGE_COLOR);
					
					if(this->tests_number == 1)
						cout << endl << "\tThe test is valid" << endl;
					
					else
						cout << endl << "\tThe " << this->tests_number << " tests are valid" << endl;
					
					cout << termColor();
					usleep(1000 * 1000 * TESTS_VALID_MESSAGE_DURATION);
					exit(0);
				}
				
				else
				{
					cout << termColor(TESTS_ERROR_MESSAGE_COLOR) << endl << "\t";
					
					if(this->valide_tests_number == 0)
						cout << "No valid test";
					
					else
						cout << (this->tests_number - this->valide_tests_number) << "/" << this->tests_number << " tests are not valid";
					
					cout << endl << termColor();
					cin.sync();
					cin.ignore();
				}
			}
			
			else
			{
				cout << "No test for " << this->application_name << "..." << endl;
				usleep(1000 * 1000 * TESTS_VALID_MESSAGE_DURATION);
				exit(0);
			}
		}
		
		void add(bool result, string text)
		{
			reactivateCout();
			this->tests_number ++;
			cout << this->tests_number << " \t";
			
			if(result)
			{
				this->valide_tests_number ++;
				cout << termColor(TESTS_VALID_MESSAGE_COLOR);
				cout << "[ OK ]";
			}
      
			else
			{
				this->tests_ok = false;
				cout << termColor(TESTS_ERROR_MESSAGE_COLOR);
				cout << "[ !! ]";
			}
			
			cout << termColor();
			cout << " " << text << endl;
			inhibitCout();
		}
		
		static bool areSame(double a, double b)
		{
			return fabs(a - b) < EPSILON;
		}

	private:
		bool tests_ok;
		int tests_number;
		int valide_tests_number;
		string application_name;
		
		// For temporarily disabling std::cout :
		streambuf *psbuf, *backup;
		ofstream filestr;
};

#endif 
