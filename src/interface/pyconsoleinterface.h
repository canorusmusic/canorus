/*!
	Copyright (c) 2006-2008, Štefan Sakalík, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PYCONSOLEINTERFACE_H_
#define PYCONSOLEINTERFACE_H_

#include <QString>
#include "ui/pyconsole.h"

class CAPyConsoleInterface {
public:
	CAPyConsoleInterface (CAPyConsole* pyConsole);
	// API for pycli plugin
	char* buffered_input(char* prompt);	// Input goes to python interpreter
	void buffered_output(char* bufInp, bool bStdErr);
	void plugin_init(void);			// when script initializes

private:
	CAPyConsole* pycons;

};

#endif /* PYCONSOLEINTERFACE_H_ */
