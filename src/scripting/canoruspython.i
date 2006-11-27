/** @file scripting/canoruspython.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%module CanorusPython
%module(directors="1") CanorusPython

//convert returned QString value to Python's String format in UTF8 encoding
%typemap(out) const QString {
	$result = Py_BuildValue("s", $1.toUtf8().data());
}

//convert Python's String to QString in UTF8 encoding
%typemap(in) const QString {
	$1 = QString::fromUtf8(PyString_AsString($input));
}

//convert returned QList value to Python's list
%typemap(out) const QList<CANote*>, QList<CANote*> {
	PyObject *list = PyList_New(0);
	for (int i=0; i<$1.size(); i++)
		PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Note));
	
	$result = list;
}

%include "scripting/context.i"
%include "scripting/document.i"
%include "scripting/muselement.i"
%include "scripting/sheet.i"
%include "scripting/voice.i"
%include "scripting/playable.i"
%include "scripting/staff.i"
%include "scripting/note.i"
%include "scripting/rest.i"
%include "scripting/keysignature.i"
%include "scripting/timesignature.i"
%include "scripting/clef.i"
%include "scripting/barline.i"
%include "scripting/canorusml.i"
%include "scripting/functionmarking.i"
%include "scripting/functionmarkingcontext.i"

%{	//toPythonObject() function
#include "scripting/swigpython.h"	//needed for CAClassType

#ifdef __cplusplus
extern "C"
#endif
SWIGEXPORT void Init_CanorusPython(void);

class QString;

PyObject *CASwigPython::toPythonObject(void *object, CASwigPython::CAClassType type) {
	switch (type) {
		case CASwigPython::Document:
			return SWIG_NewPointerObj(object, SWIGTYPE_p_CADocument, 0);
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
		/*case CASwigPython::CanorusML:	//not implemented yet
			return SWIG_Python_NewPointerObj(object, SWIGTYPE_p_CACanorusML, 0);
			break;*/
	}
}

%}
