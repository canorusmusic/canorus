/*!
	Copyright (c) 2006-2008, Štefan Sakalík, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "interface/pyconsoleinterface.h"

/*!
	This class serves as a proxy, simple wrapper:
	plugins/pycli   <=>   interface/pyconsoleinterface   <=>   ui/pyconsole
*/

CAPyConsoleInterface::CAPyConsoleInterface(CAPyConsole* pyConsole ) {
	pycons = pyConsole;
}

void CAPyConsoleInterface::plugin_init(void){
	pycons->plugin_init();
}

char* CAPyConsoleInterface::buffered_input(char* prompt) {
	return (pycons->buffered_input(QString(prompt))).toUtf8().data();
}

void CAPyConsoleInterface::buffered_output (char* str, bool bStdErr) {
	QString *q_str = new QString(str);
	pycons->buffered_output(*q_str, bStdErr);
}
