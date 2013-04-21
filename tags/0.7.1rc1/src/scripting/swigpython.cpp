/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifdef USE_PYTHON
#include "scripting/swigpython.h"
#include <QFile>
#include <QThread>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#ifndef SWIGCPP
#include "canorus.h"
#endif

#include <iostream> // used for reporting errors in scripts
using namespace std;
//#include <pthread.h>

class CAPyconsoleThread : public QThread {
protected:
	void run() { CASwigPython::callPycli(NULL); }
};

PyThreadState * CASwigPython::mainThreadState;
PyThreadState * CASwigPython::pycliThreadState;

/// Load 'CanorusPython' module and initialize classes - defined in SWIG wrapper class
extern "C" void init_CanorusPython();

/*!
	Initializes Python and loads base 'CanorusPython' module. Call this before any other
	Python operations! Call this before calling toPythonObject() or any other conversation
	functions as well!
*/
void CASwigPython::init() {

	Py_Initialize();
	PyEval_InitThreads(); // our python will use GIL

	init_CanorusPython();
    PyRun_SimpleString("import sys");

	// add path to scripts to Scripting path
	if (QDir::searchPaths("scripts").size()) {
		PyRun_SimpleString((QString("sys.path.append('")+QDir::searchPaths("scripts")[0]+"')").toStdString().c_str());
	} else {
		std::cerr << "Error: scripts/ not found" << std::endl;
	}

	// add path to CanorusPython modules to Scripting path
	if (QFileInfo("base:CanorusPython.py").exists()) {
		PyRun_SimpleString((QString("sys.path.append('")+QFileInfo("base:CanorusPython.py").absolutePath()+"')").toStdString().c_str());
	}

#ifdef Q_WS_WIN
	if ( QFileInfo("base:_CanorusPython.dll").exists() ) {
		PyRun_SimpleString((QString("sys.path.append('")+QFileInfo("base:_CanorusPython.dll").absolutePath()+"')").toStdString().c_str());
	} else {
		std::cerr << "Error: _CanorusPython.dll not found" << std::endl;
	}

	if ( QDir("base:pythonLib").exists() ) {
		PyRun_SimpleString((QString("sys.path.append('")+QDir("base:pythonLib").absolutePath()+"')").toStdString().c_str());
	} else {
		std::cerr << "Error: pythonLib/ not found" << std::endl;
	}
#else
	if (QFileInfo("base:_CanorusPython.so").exists()) {
		PyRun_SimpleString((QString("sys.path.append('")+QFileInfo("base:_CanorusPython.so").absolutePath()+"')").toStdString().c_str());
	} else {
		std::cerr << "Error: _CanorusPython.so not found" << std::endl;
	}
#endif

    mainThreadState = PyThreadState_Get();
    PyEval_ReleaseLock();

       // my section with thread initialization
PyEval_AcquireLock();
    PyInterpreterState * mainInterpreterState = mainThreadState->interp;

    pycliThreadState = PyThreadState_New(mainInterpreterState);
    PyThreadState_Swap(mainThreadState);

PyEval_ReleaseLock();
}

/*!
	Calls an external Python function in the given module with the list of arguments and return the Python object the function returned.

	\param fileName Absolute path to the filename of the script
	\param function Function or method name.
	\param args List of arguments in Python's PyObject pointer format. Use toPythonObject() to convert C++ classes to Python objects.
	\param autoReload automatically reload module if it was imported before, defaults to false

	\warning You have to add path of the plugin to Python path before, manually! This is usually done by CAPlugin::callAction("onInit").
*/


//pthread_t *tid=NULL;
QThread *qtid=NULL;
QString thr_fileName;
QString thr_function;
QList<PyObject*> thr_args;

PyObject *CASwigPython::callFunction(QString fileName, QString function, QList<PyObject*> args, bool autoReload) {

	if (!QFile::exists(fileName))
		return 0;

	// run pycli in pthread, this is temporary solution
	if (fileName.contains("pycli") && (!function.contains("init"))) {
	    //tid = new pthread_t;
		qtid = new CAPyconsoleThread();
		thr_fileName = fileName;
		thr_args = args;
		thr_function = function;
		qtid->start();
		//pthread_create(tid, NULL, &callPycli, NULL);

		return args.first();
	}

	PyObject *pyArgs = PyTuple_New(args.size());
	if (!pyArgs)
		return NULL;
	for(int i=0; i<args.size(); i++)
		PyTuple_SetItem(pyArgs, i, args[i]);

	// Load module, if not yet
	QString moduleName = fileName.left(fileName.lastIndexOf(".py"));
	moduleName = moduleName.remove(0, moduleName.lastIndexOf("/")+1);

	PyEval_AcquireLock();
	
	PyObject *pyModule;
	if (autoReload) {
		PyObject *moduleDict = PyImport_GetModuleDict(); // borrowed ref.
		PyObject *ourModuleName = PyString_FromString((char*)moduleName.toStdString().c_str()); // new ref.
		pyModule = PyDict_GetItem(moduleDict, ourModuleName); // borrowed ref.
		Py_DECREF(ourModuleName);

		if (pyModule == NULL) // not imported yet
			pyModule = PyImport_ImportModule((char*)moduleName.toStdString().c_str());
		else
			Py_XDECREF(PyImport_ReloadModule(pyModule)); // we don't need the reference returned from ReloadModule
	} else {
		pyModule = PyImport_ImportModule((char*)moduleName.toStdString().c_str());
	}
	if (PyErr_Occurred()) { PyErr_Print(); PyEval_ReleaseLock(); return NULL; }

	// Get function object
	PyObject *pyFunction = PyObject_GetAttrString(pyModule, (char*)function.toStdString().c_str());
	if (PyErr_Occurred()) { PyErr_Print(); PyEval_ReleaseLock(); return NULL; }

	// Call the actual function
	PyObject *ret;
	if (args.size())
		ret = PyEval_CallObject(pyFunction, pyArgs);
	else
		ret = PyEval_CallObject(pyFunction, NULL);
	if (PyErr_Occurred()) { PyErr_Print(); PyEval_ReleaseLock(); return NULL; }

	Py_DECREF(pyFunction);
	Py_DECREF(pyModule);
//	Py_DECREF(pyArgs); /// \todo Crashes if uncommented?!
	for (int i=0; i<args.size(); i++)
		Py_DECREF(args[i]);

    PyEval_ReleaseLock();
	return ret;
}



/*!
	Function for intializing python-CLI pycli, called asynchronously from 'callFunction' (it's a copy to avoid confusion)
	temporary solution

	\param fileName Absolute path to the filename of the script
	\param args -> document reference, pyCli widget referece **

*/
void *CASwigPython::callPycli(void*) {


    PyEval_AcquireLock();
    PyThreadState_Swap(pycliThreadState);

	QString fileName = thr_fileName;
	QString function = thr_function;
	QList<PyObject*> args = thr_args;

	if (!QFile::exists(fileName)){
//		pthread_exit((void*)NULL);
	}

	PyObject *pyArgs = Py_BuildValue("(OO)", args[0], args[1]);

	// Load module, if not yet
	QString moduleName = fileName.left(fileName.lastIndexOf(".py"));
	moduleName = moduleName.remove(0, moduleName.lastIndexOf("/")+1);

	PyObject *pyModule = PyImport_ImportModule((char*)moduleName.toStdString().c_str());

	if (PyErr_Occurred()) { PyErr_Print(); PyEval_ReleaseLock(); return NULL; }

	// Get function object

	//PyObject *pyFunction = PyObject_GetAttrString(pyModule, "pycli");
	PyObject *pyFunction = PyObject_GetAttrString(pyModule, (char*)function.toStdString().c_str());

	if (PyErr_Occurred()) { PyErr_Print(); PyEval_ReleaseLock(); return NULL; }

	// Call the actual function
	//
	PyObject *ret;
	ret = PyEval_CallObject(pyFunction, pyArgs);
	if (PyErr_Occurred()) { PyErr_Print(); PyEval_ReleaseLock(); return NULL; }

	Py_DECREF(pyFunction);
//	Py_DECREF(pyArgs); /// \todo Crashes if uncommented?!d
	Py_DECREF(pyModule);
	for (int i=0; i<args.size(); i++)
		Py_DECREF(args[i]);


    PyThreadState_Swap(mainThreadState);
    PyEval_ReleaseLock();

//	pthread_exit((void*)NULL);
    return ret;
}

/*
	\function static PyObject* CASwigPython::toPythonObject(void *object, CAClassType type)

	Python uses its wrapper classes over C++ objects. Use this function to create a Python wrapper object out of the C++ one of type \a type.
	See CAClassType for details on the types.
*/


#endif
