/*!
	Copyright (c) 2008, Štefan Sakalík, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PYCONSOLEINTERFACE_H_
#define PYCONSOLEINTERFACE_H_

#include <QString>

class CAPyConsole;

class CAPyConsoleInterface {
public:
#if !defined( SWIG ) && !defined( SWIGCPP )
	CAPyConsoleInterface (CAPyConsole* pyConsole);

	// API for pycli plugin
	char* bufferedInput(char* prompt);	// Input goes to script
	void bufferedOutput(char* bufInp, bool bStdErr);
#else
	char* bufferedInput(char* prompt) {}
	void bufferedOutput(char* bufInp, bool bStdErr) {}
#endif	
	CAPyConsoleInterface () {}
	void pluginInit(void);			// when script initializes

private:
#if !defined( SWIG ) && !defined( SWIGCPP )
	CAPyConsole* _pycons;
#endif

};

#endif /* PYCONSOLEINTERFACE_H_ */
