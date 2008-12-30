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

// convert returned QColor value to Python's tuple of RGBA integers
%typemap(out) const QColor {
	PyObject *tuple = PyTuple_New(4);
	PyTuple_SetItem( tuple, 0, PyInt_FromLong($1.red()) );
	PyTuple_SetItem( tuple, 1, PyInt_FromLong($1.green()) );
	PyTuple_SetItem( tuple, 2, PyInt_FromLong($1.blue()) );
	PyTuple_SetItem( tuple, 3, PyInt_FromLong($1.alpha()) );
	$result = tuple;
}

// convert Python's tuple of RGBA integers to QColor
%typemap(in) const QColor {
	$1 = QColor( PyInt_AsLong( PyTuple_GetItem( $input, 0 ) ),
	             PyInt_AsLong( PyTuple_GetItem( $input, 1 ) ),
	             PyInt_AsLong( PyTuple_GetItem( $input, 2 ) ),
	             (PyTuple_Size($input)>3)?PyInt_AsLong( PyTuple_GetItem( $input, 3 ) ):255
	           );
}

// convert returned QList value to Python's list
// I found no generic way of doing this yet... -Matevz
%typemap(out) const QList<CANote*>, QList<CANote*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Note));
	
	$result = list;
}
%typemap(out) const QList<CARest*>, QList<CARest*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Rest));
	
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
%typemap(out) const QList<CAMark*>, QList<CAMark*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Mark));
	
	$result = list;
}
%typemap(out) const QList<CAContext*>, QList<CAContext*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Context));
	
	$result = list;
}
%typemap(out) const QList<CAStaff*>, QList<CAStaff*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Staff));
	
	$result = list;
}
%typemap(out) const QList<CAVoice*>, QList<CAVoice*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Voice));
	
	$result = list;
}
%typemap(out) const QList<CAPlayableLength>, QList<CAPlayableLength> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject(const_cast<CAPlayableLength*>(&($1.at(i))), CASwigPython::PlayableLength));
	
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
		case CASwigPython::Context: {
			switch (static_cast<CAContext*>(object)->contextType()) {
			case CAContext::Staff:
				return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAStaff, 0);
			case CAContext::LyricsContext:
				return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CALyricsContext, 0);
			case CAContext::FunctionMarkContext:
				return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAFunctionMarkContext, 0);
			default:
				std::cerr << "canoruspython.i: Wrong CAContext::contextType()!" << std::endl;
			}
			break;
		}
		case CASwigPython::Staff:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAStaff, 0);
			break;
		case CASwigPython::Voice:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAVoice, 0);
			break;
		case CASwigPython::FunctionMarkContext:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAFunctionMarkContext, 0);
			break;
		/*case CASwigPython::MusElement:	// TODO: CAMusElement is always abstract and is not subclassed in Python.
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
		case CASwigPython::FunctionMark:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAFunctionMark, 0);
			break;
		case CASwigPython::LyricsContext:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CALyricsContext, 0);
			break;			
		case CASwigPython::Syllable:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CASyllable, 0);
			break;
		case CASwigPython::Mark:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAMark, 0);
			break;
		case CASwigPython::Slur:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CASlur, 0);
			break;
		case CASwigPython::Tuplet:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CATuplet, 0);
			break;
		case CASwigPython::PlayableLength:
			return SWIG_Python_NewPointerObj(new CAPlayableLength(*(static_cast<CAPlayableLength*>(object))), SWIGTYPE_p_CAPlayableLength, 0);
			break;
		case CASwigPython::PyConsoleInterface:
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CAPyConsoleInterface, 0);
			break;
	}
}

%}
