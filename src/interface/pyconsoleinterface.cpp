/*!
	Copyright (c) 2006-2008, Štefan Sakalík, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "interface/pyconsoleinterface.h"
#ifndef SWIGCPP
#include "widgets/pyconsole.h"
#endif

/*!
	This class serves as a proxy, simple wrapper:
	plugins/pycli   <=>   interface/pyconsoleinterface   <=>   ui/pyconsole
*/

#ifndef SWIGCPP
CAPyConsoleInterface::CAPyConsoleInterface(CAPyConsole* pyConsole ) {
	_pycons = pyConsole;
}
#endif

void CAPyConsoleInterface::pluginInit(void){
#ifndef SWIGCPP
	_pycons->plugin_init();
#endif
}

#ifndef SWIGCPP
char* CAPyConsoleInterface::bufferedInput(char* prompt) {
	return (_pycons->buffered_input(QString(prompt))).toUtf8().data();
}

void CAPyConsoleInterface::bufferedOutput (char* str, bool bStdErr) {
	QString *q_str = new QString(str);
	_pycons->buffered_output(*q_str, bStdErr);
}
#endif
