/** @file scripting/swigpython.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_PYTHON
#ifndef SWIGPYTHON_H_
#define SWIGPYTHON_H_

#include <Python.h>

#include <QString>
#include <QList>

class CASwigPython {
	public:
		enum CAClassType {
			Document,
			Sheet,
			Context,
			Staff,
			Voice,
			FunctionMarkingContext,
			MusElement,
			Playable,
			Note,
			Rest,
			KeySignature,
			TimeSignature,
			Clef,
			Barline,
			FunctionMarking
		};
		
		static void init();	///Initializes Python and loads base 'CanorusPython' module. Call this before any other Python operations! Call this before calling toPythonObject() or any other conversation functions as well!
		
		/**
	 	 * Call an external Ruby function in the given module with the list of arguments and return its Ruby value.
	 	 * 
	 	 * @param fileName Absolute path to the filename of the script
	 	 * @param function Function or method name.
	 	 * @param args List of arguments in Python's PyObject pointer format. Use toPythonObject() to convert C++ classes to Python objects.
	 	 * @return Python's function return value in PyObject pointerformat.
	 	 */
		static PyObject *callFunction(QString fileName, QString function, QList<PyObject*> args);
		
		/**
 		 * Python uses different objects than C++. They are actually wrappers around the original ones, but still share different memory and structure.
 		 * Use this function to create a Ruby object out of the C++ one.
 		 * 
 		 * @param object Pointer to the C++ object which the Python object should be derived from.
 		 * @param objectType See CAClassType. C++ doesn't support figuring out the object type from the raw pointer - you have to pass its class type as well.
 		 * @return Pointer to the Python object.
 		 */
		static PyObject *toPythonObject(void *object, CAClassType type);	//defined in scripting/canoruspython.i file
};

#endif /*SWIGPYTHON_H_*/
#endif
