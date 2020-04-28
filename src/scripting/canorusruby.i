/*!
    Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%module CanorusRuby

// We don't have typemaps defined for custom types yet so we disable strongly typed languages by this, but make things easier for us.
// This is used to enable default arguments support :)
%feature("compactdefaultargs");

%feature("ref")   ""
%feature("unref") ""

%{
#include <iostream>
%}

// convert returned QString value to Ruby's String format in UTF8 encoding
%typemap(out) const QString {
    $result = rb_str_new2($1.toUtf8().data());
}

// convert Ruby's String to QString in UTF8 encoding
%typemap(in) const QString {
    VALUE value = rb_inspect($input);
    $1 = QString::fromUtf8(StringValuePtr(value));
}

// convert returned QList value to Ruby's array
%typemap(out) const QList<CAMusElement*>, QList<CAMusElement*>,
              const QList<CANote*>, QList<CANote*>,
              const QList<CARest*>, QList<CARest*>,
              const QList<CAMark*>, QList<CAMark*>,
              const QList<CAPlayable*>, QList<CAPlayable*>,
              const QList<CASyllable*>, QList<CASyllable*>,
              const QList<CAFiguredBasMark*>, QList<CAFiguredBassMark*>,
              const QList<CAFunctionMark*>, QList<CAFunctionMark*>,
              const QList<CAChordName*>, QList<CAChordName*> {
    VALUE arr = rb_ary_new2($1.size());
    for (int i=0; i<$1.size(); i++) {
	    rb_ary_push(arr, CASwigRuby::toRubyObject($1.at(i), CASwigRuby::MusElement));
    }

    $result = arr;
}

// convert Ruby's List to QList
// I found no generic way of doing this yet... -Matevz
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

%include "scripting/canoruslibrary.i"

%{	//toRubyObject() function
#include "scripting/swigruby.h"	//needed for CAClassType

class QString;

VALUE CASwigRuby::toRubyObject(void *object, CASwigRuby::CAClassType type) {
    VALUE rbObj = 0;
    switch (type) {
        case CASwigRuby::String: {
            rbObj = rb_str_new2(((QString*)object)->toUtf8().data());
            break;
        }
        case CASwigRuby::Document: {
            rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CADocument, 0);
            break;
        }
        case CASwigRuby::Resource: {
            rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAResource, 0);
            break;
        }
        case CASwigRuby::Sheet: {
            rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CASheet, 0);
            break;
        }
        case CASwigRuby::Context: {
            switch (static_cast<CAContext*>(object)->contextType()) {
            case CAContext::Staff:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAStaff, 0);
                break;
            case CAContext::LyricsContext:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CALyricsContext, 0);
                break;
            case CAContext::FiguredBassContext:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAFiguredBassContext, 0);
                break;
            case CAContext::FunctionMarkContext:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAFunctionMarkContext, 0);
                break;
            case CAContext::ChordNameContext:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAChordNameContext, 0);
                break;
            default:
                std::cerr << "canorusruby.i: Wrong CAContext::contextType()!" << std::endl;
                break;
            }
            break;
        }
        case CASwigRuby::Voice: {
            rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAVoice, 0);
            break;
        }
        case CASwigRuby::MusElement: {
            CAMusElement *elt = static_cast<CAMusElement*>(object);
            switch (elt->musElementType()) {
            case CAMusElement::Note:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CANote, 0);
                break;
            case CAMusElement::Rest:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CARest, 0);
                break;
            case CAMusElement::KeySignature:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAKeySignature, 0);
                break;
            case CAMusElement::TimeSignature:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CATimeSignature, 0);
                break;
            case CAMusElement::Clef:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAClef, 0);
                break;
            case CAMusElement::Barline:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CABarline, 0);
                break;
            case CAMusElement::FiguredBassMark:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAFiguredBassMark, 0);
                break;
            case CAMusElement::FunctionMark:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAFunctionMark, 0);
                break;
            case CAMusElement::Syllable:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CASyllable, 0);
                break;
            case CAMusElement::Mark:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAMark, 0);
                break;
            case CAMusElement::Slur:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CASlur, 0);
                break;
            case CAMusElement::Tuplet:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CATuplet, 0);
                break;
            case CAMusElement::MidiNote:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAMidiNote, 0);
                break;
            case CAMusElement::ChordName:
                rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAChordName, 0);
                break;
            default:
                std::cerr << "canorusruby.i: Wrong CAMusElement::musElementType()!" << std::endl;
                break;
            }
            break;
        }
        case CASwigRuby::PlayableLength: {
            rbObj = SWIG_NewPointerObj(new CAPlayableLength(*(static_cast<CAPlayableLength*>(object))), SWIGTYPE_p_CAPlayableLength, 0);
            break;
        }
        case CASwigRuby::PyConsoleInterface: {
            rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAPyConsoleInterface, 0);
            break;
        }
        case CASwigRuby::Plugin: {
            rbObj = SWIG_NewPointerObj(object, SWIGTYPE_p_CAPlugin, 0);
            break;
        }
        default: {
            std::cerr << "canorusruby.i: Wrong CACanorusRuby::type!" << std::endl;
            break;
        }
    }

    return rbObj;
}
%}

