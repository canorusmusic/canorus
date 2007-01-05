/** @file scripting/swigpython.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_PYTHON
#include "scripting/swigpython.h"
#include <QFile>

//defined in SWIG wrapper class
extern "C" void init_CanorusPython();	///Load 'CanorusPython' module and initialize classes
QString locateResource(QString);

void CASwigPython::init() {
	Py_Initialize();
	init_CanorusPython();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString((QString("sys.path.append('")+locateResource("scripts")+"')").toStdString().c_str());
}
#include<iostream>
//WARNING! You have to add path of the plugin to Python path before, manually!
PyObject *CASwigPython::callFunction(QString fileName, QString function, QList<PyObject*> args) {
	if (!QFile::exists(fileName))
		return 0;
	
	// Prepare arguments
	QString listMask = "(";
	for (int i=0; i<args.size(); i++)
		listMask += "O";
	listMask += ")";
	
	PyObject *list[args.size()];
	for (int i=0; i<args.size(); i++)
		list[i] = args[i];
	
	PyObject *pyArgs = Py_BuildValue((char*)listMask.toStdString().c_str(), *list);
	
	// Load module, if not yet
	QString moduleName = fileName.left(fileName.lastIndexOf(".py"));
	moduleName = moduleName.remove(0, moduleName.lastIndexOf("/")+1);
	PyObject *pyModule = PyImport_ImportModule((char*)moduleName.toStdString().c_str());
	
	// Get function object
	PyObject *pyFunction = PyObject_GetAttrString(pyModule, (char*)function.toStdString().c_str());
	
	// Call the actual function
	PyObject *ret;
	if (args.size())
		ret = PyEval_CallObject(pyFunction, pyArgs);
	else
		ret = PyEval_CallObject(pyFunction, NULL);
	
	Py_DECREF(pyFunction);
	Py_DECREF(pyModule);
	Py_DECREF(pyArgs);
	for (int i=0; i<args.size(); i++)
		Py_DECREF(args[i]);
	
	return ret;
}

#endif
