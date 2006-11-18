/** @file scripting/canorusruby.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%module CanorusRuby

//convert returned QString value to Ruby's String format in UTF8 encoding
%typemap(out) const QString {
	$result = rb_str_new2($1.toUtf8().data());
}

//convert Ruby's String to QString in UTF8 encoding
%typemap(in) const QString {
	$1 = QString::fromUtf8(STR2CSTR($input));
}

//convert returned QList value to Ruby's List
%typemap(out) const QList<CANote*>, QList<CANote*> {
	VALUE arr = rb_ary_new2($1.size());
	for (int i=0; i<$1.size(); i++)
		rb_ary_push(arr, CASwigRuby::toRubyObject($1.at(i), CASwigRuby::Note));
	
	$result = arr;
}

//convert Ruby's List to QList
%typemap(in) const QList<T>, QList<T> {
	Check_Type($input, T_ARRAY);
	QList<T> *list = new QList<T>;
	int len = RARRAY($input)->len;
	for (int i=0; i!=len; i++) {
		VALUE inst = rb_ary_entry($input, i);
		T element = NULL;
		Data_Get_Struct(inst, QList<T>, element);
		list->push_back(element);
	}
	$1 = list;
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

%{	//toRubyObject() function
#include "scripting/swigruby.h"	//needed for CAClassType

#ifdef __cplusplus
extern "C"
#endif
SWIGEXPORT void Init_CanorusRuby(void);

class QString;

VALUE CASwigRuby::toRubyObject(void *object, CASwigRuby::CAClassType type) {
	switch (type) {
		case CASwigRuby::Document:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CADocument, 0);
			break;
		case CASwigRuby::Sheet:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CASheet, 0);
			break;
		/*case CASwigRuby::Context:	//CAContext is always abstract
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAContext, 0);
			break;*/
		case CASwigRuby::Staff:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAStaff, 0);
			break;
		case CASwigRuby::Voice:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAVoice, 0);
			break;
		case CASwigRuby::FunctionMarkingContext:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAFunctionMarkingContext, 0);
			break;
		/*case CASwigRuby::MusElement:	//CAMusElement is always abstract
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAMusElement, 0);
			break;*/
		case CASwigRuby::Note:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CANote, 0);
			break;
		case CASwigRuby::Rest:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CARest, 0);
			break;
		case CASwigRuby::KeySignature:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAKeySignature, 0);
			break;
		case CASwigRuby::TimeSignature:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CATimeSignature, 0);
			break;
		case CASwigRuby::Clef:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAClef, 0);
			break;
		case CASwigRuby::Barline:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CABarline, 0);
			break;
		case CASwigRuby::FunctionMarking:
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CAFunctionMarking, 0);
			break;
		/*case CASwigRuby::CanorusML:	//not implemented yet
			return SWIG_Ruby_NewPointerObj(object, SWIGTYPE_p_CACanorusML, 0);
			break;*/
	}
}
%}

