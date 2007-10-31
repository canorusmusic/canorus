/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%module CanorusPython

// We don't have typemaps defined for custom types yet so we disable strongly typed languages by
// this, but make things much easier for us.
// In practice, it turned out this is used to enable default arguments support in Python :)
%feature("compactdefaultargs");

// don't add the object to Python object reference list so Python's gc deletes it
%feature("ref")   ""

// used when a user wants to delete an object
%feature("unref") "
	if (markedObjects.removeAll($this))
		delete $this;
"

%{
#include <iostream>
%}

%typecheck(SWIG_TYPECHECK_STRING)
	QString, const QString
{
	$1 = (PyString_Check($input)) ? 1 : 0; 
}

// convert returned QString value to Python's String format in UTF8 encoding
%typemap(out) const QString {
	$result = Py_BuildValue("s", $1.toUtf8().data());
}

// convert Python's String to QString in UTF8 encoding
%typemap(in) const QString {
	$1 = QString::fromUtf8(PyString_AsString($input));
}

// convert returned QList value to Python's list
// I found no generic way of doing this yet... -Matevz
%typemap(out) const QList<CANote*>, QList<CANote*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Note));
	
	$result = list;
}
%typemap(out) const QList<CAPlayable*>, QList<CAPlayable*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++) {
		if ($1.at(i)->musElementType() == CAMusElement::Note)
			PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Note));
		else
			PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Rest));			
	}
	
	$result = list;
}

void markDelete( PyObject* ); // function used to delete Canorus objects inside Python
const char* tr( const char * sourceText, const char * comment = 0, int n = -1 );

%include "scripting/canoruslibrary.i"

%{	// toPythonObject() function
#include "scripting/swigpython.h"	//needed for CAClassType

#include <QList>
QList<void*> markedObjects = QList<void*>(); // define markedObjects

void markDelete( PyObject* object ) {
	markedObjects << SWIG_Python_GetSwigThis(object)->ptr;
}

const char* tr( const char * sourceText, const char * comment = 0, int n = -1 ) {
	return QObject::tr( sourceText, comment, n ).toUtf8().constData();
}

class QString;

PyObject *CASwigPython::toPythonObject(void *object, CASwigPython::CAClassType type) {
	switch (type) {
		case CASwigPython::String:
			return Py_BuildValue("s", ((QString*)object)->toUtf8().data());
			break;
		case CASwigPython::Document:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CADocument, 0);
			break;
		case CASwigPython::Sheet:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CASheet, 0);
			break;
		/*case CASwigPython::Context:	//CAContext is always abstract
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAContext, 0);
			break;*/
		case CASwigPython::Staff:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAStaff, 0);
			break;
		case CASwigPython::Voice:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAVoice, 0);
			break;
		case CASwigPython::FunctionMarkingContext:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAFunctionMarkingContext, 0);
			break;
		/*case CASwigPython::MusElement:	//CAMusElement is always abstract
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAMusElement, 0);
			break;*/
		case CASwigPython::Playable:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAPlayable, 0);
			break;
		case CASwigPython::Note:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CANote, 0);
			break;
		case CASwigPython::Rest:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CARest, 0);
			break;
		case CASwigPython::KeySignature:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAKeySignature, 0);
			break;
		case CASwigPython::TimeSignature:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CATimeSignature, 0);
			break;
		case CASwigPython::Clef:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAClef, 0);
			break;
		case CASwigPython::Barline:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CABarline, 0);
			break;
		case CASwigPython::FunctionMarking:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAFunctionMarking, 0);
			break;
		case CASwigPython::LyricsContext:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CALyricsContext, 0);
			break;			
		case CASwigPython::Syllable:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CASyllable, 0);
			break;
	}
}

%}
