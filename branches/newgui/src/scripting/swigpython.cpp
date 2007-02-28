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

#include <iostream> // used for reporting errors in scripts

//defined in SWIG wrapper class
extern "C" void init_CanorusPython();	///Load 'CanorusPython' module and initialize classes
QString locateResource(QString);
QString locateResourceDirectory(QString);

void CASwigPython::init() {
	Py_Initialize();
	init_CanorusPython();
	PyRun_SimpleString("import sys");
	// add path to scripts to Scripting path
	PyRun_SimpleString((QString("sys.path.append('")+locateResource("scripts")+"')").toStdString().c_str());
	// add path to CanorusPython modules to Scripting path
	PyRun_SimpleString((QString("sys.path.append('")+locateResourceDirectory("CanorusPython.py")+"')").toStdString().c_str());
	//PyRun_SimpleString("import CanorusPython");	
#ifdef Q_WS_WIN
	PyRun_SimpleString((QString("sys.path.append('")+locateResourceDirectory("_CanorusPython.dll")+"')").toStdString().c_str());
#else
	PyRun_SimpleString((QString("sys.path.append('")+locateResourceDirectory("_CanorusPython.so")+"')").toStdString().c_str());
#endif
}

//WARNING! You have to add path of the plugin to Python path before, manually! This is usually done by CAPlugin::callAction("onInit")
PyObject *CASwigPython::callFunction(QString fileName, QString function, QList<PyObject*> args) {
	if (!QFile::exists(fileName))
		return 0;
	
	///\todo A very ugly hack which supports up to 4 arguments to be passed to Py_BuildValue. If anyone knows how to pass a custom number of arguments (eg. array) to variadic C functions, let me know! -Matevz
	PyObject *pyArgs;
	switch (args.size()) {
	case 1:
		pyArgs = Py_BuildValue("(O)", args[0]);
		break;
	case 2:
		pyArgs = Py_BuildValue("(OO)", args[0], args[1]);
		break;
	case 3:
		pyArgs = Py_BuildValue("(OOO)", args[0], args[1], args[2]);
		break;
	case 4:
		pyArgs = Py_BuildValue("(OOOO)", args[0], args[1], args[2], args[3]);
		break;
	}
	
	// Load module, if not yet
	QString moduleName = fileName.left(fileName.lastIndexOf(".py"));
	moduleName = moduleName.remove(0, moduleName.lastIndexOf("/")+1);
	PyObject *pyModule = PyImport_ImportModule((char*)moduleName.toStdString().c_str());
	if (PyErr_Occurred()) { PyErr_Print(); return NULL; }
	
	// Get function object
	PyObject *pyFunction = PyObject_GetAttrString(pyModule, (char*)function.toStdString().c_str());
	if (PyErr_Occurred()) { PyErr_Print(); return NULL; }
	
	// Call the actual function
	PyObject *ret;
	if (args.size())
		ret = PyEval_CallObject(pyFunction, pyArgs);
	else
		ret = PyEval_CallObject(pyFunction, NULL);
	if (PyErr_Occurred()) { PyErr_Print(); return NULL; }
	
	Py_DECREF(pyFunction);
	Py_DECREF(pyModule);
//	Py_DECREF(pyArgs); /// \todo Crashes if uncommented?!
	for (int i=0; i<args.size(); i++)
		Py_DECREF(args[i]);
	
	return ret;
}

#endif
