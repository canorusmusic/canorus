/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifdef USE_PYTHON
#include "scripting/swigpython.h"
#include <QFile>

#include "canorus.h"

#include <iostream> // used for reporting errors in scripts

/// Load 'CanorusPython' module and initialize classes - defined in SWIG wrapper class
extern "C" void init_CanorusPython();

/*!
	Initializes Python and loads base 'CanorusPython' module. Call this before any other
	Python operations! Call this before calling toPythonObject() or any other conversation
	functions as well!
*/
void CASwigPython::init() {
	Py_Initialize();
	init_CanorusPython();
	PyRun_SimpleString("import sys");
	
	// add path to scripts to Scripting path
	if (CACanorus::locateResource("scripts").size())
		PyRun_SimpleString((QString("sys.path.append('")+CACanorus::locateResource("scripts").at(0)+"')").toStdString().c_str());
	
	// add path to CanorusPython modules to Scripting path
	if (CACanorus::locateResource("CanorusPython.py").size())
		PyRun_SimpleString((QString("sys.path.append('")+CACanorus::locateResourceDir("CanorusPython.py").at(0)+"')").toStdString().c_str());
	
#ifdef Q_WS_WIN
	if (CACanorus::locateResource("_CanorusPython.dll").size()) {
		PyRun_SimpleString((QString("sys.path.append('")+CACanorus::locateResourceDir("_CanorusPython.dll").at(0)+"')").toStdString().c_str());
		PyRun_SimpleString((QString("sys.path.append('")+CACanorus::locateResourceDir("pythonLib").at(0)+"/pythonLib')").toStdString().c_str());
	}
#else
	if (CACanorus::locateResource("_CanorusPython.so").size()) {
		PyRun_SimpleString((QString("sys.path.append('")+CACanorus::locateResourceDir("_CanorusPython.so").at(0)+"')").toStdString().c_str());
	}
#endif
	
	PyRun_SimpleString("import CanorusPython");
}


/*!
	Calls an external Python function in the given module with the list of arguments and return the Python object the function returned.
	
	
	\param fileName Absolute path to the filename of the script
	\param function Function or method name.
	\param args List of arguments in Python's PyObject pointer format. Use toPythonObject() to convert C++ classes to Python objects.
	
	\warning You have to add path of the plugin to Python path before, manually! This is usually done by CAPlugin::callAction("onInit").
*/
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


/*
	\function static PyObject* CASwigPython::toPythonObject(void *object, CAClassType type)
	
	Python uses its wrapper classes over C++ objects. Use this function to create a Python wrapper object out of the C++ one of type \a type.
	See CAClassType for details on the types.
*/


#endif
