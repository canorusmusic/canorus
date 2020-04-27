/*!
	Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%module CanorusPython

// We don't have typemaps defined for custom types yet so we disable strongly typed languages by
// this, but make things much easier for us.
// In practice, it turned out this is used to enable default arguments support in Python :)
%feature("compactdefaultargs");

// don't add the object to Python object reference list so Python's gc doesn't touch it
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

// non-const QString version
%typemap(out) QString {
    $result = Py_BuildValue("s", $1.toUtf8().data());
}

// convert Python's String to QString in UTF8 encoding
%typemap(in) const QString, QString {
    $1 = QString::fromUtf8(PyUnicode_AsUTF8($input));
}
%typemap(in) const QString&, QString& {
    (*$1) = QString::fromUtf8(PyUnicode_AsUTF8($input));
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

// convert Python list to QList
%typecheck(SWIG_TYPECHECK_LIST)
    QList<CAMusElement*>, const QList<CAMusElement*>,
    QList<CAContext*>, const QList<CAContext*>
{
    $1 = (PyList_Check($input)) ? 1 : 0;
}
%typemap(in) const QList<CAMusElement*>, QList<CAMusElement*> {
    $1 = QList<CAMusElement*>();
    for (int i=0; i<PyList_Size($input); i++) {
    	void *listp;
    	SWIG_ConvertPtr(PyList_GetItem( $input, i ), &listp,SWIGTYPE_p_CAMusElement, 0 |  0 );
    	$1.append(reinterpret_cast<CAMusElement*>(listp));
    }
}
%typemap(in) const QList<CAContext*>, QList<CAContext*> {
    $1 = QList<CAContext*>();
    for (int i=0; i<PyList_Size($input); i++) {
    	void *listp;
    	SWIG_ConvertPtr(PyList_GetItem( $input, i ), &listp,SWIGTYPE_p_CAContext, 0 |  0 );
    	$1.append(reinterpret_cast<CAContext*>(listp));
    }
}

%rename(QListCAPlugin) QList<CAPlugin*>;
%typemap(in) const QListCAPlugin, QListCAPlugin {
    $1 = QList<CAPlugin*>();
    for (int i=0; i<PyList_Size($input); i++) {
        void *listp;
        SWIG_ConvertPtr(PyList_GetItem( $input, i ), &listp,SWIGTYPE_p_CAPlugin, 0 |  0 );
        $1.append(reinterpret_cast<CAPlugin*>(listp));
    }
}

// convert ordinary integers to python integers
%typemap(out) const QList<int>, QList<int> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, PyInt_FromLong($1[i]));
    }

    $result = list;
}
%typemap(out) const QList<int>&, QList<int>& {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1->size(); i++) {
        PyList_Append(list, PyInt_FromLong($1->at(i)));
    }

    $result = list;
}

// Swig has default bindings only for converting std::vector<> and other STL types.
// Here we convert Qt's QList to Python list.
// Note - C++ references (QList<T>&) are pointers in Python (QList<T>*).
%typemap(out) const QList<CAMusElement*>, QList<CAMusElement*>,
              const QList<CANote*>, QList<CANote*>,
              const QList<CARest*>, QList<CARest*>,
              const QList<CAMark*>, QList<CAMark*>,
              const QList<CAPlayable*>, QList<CAPlayable*>,
              const QList<CASyllable*>, QList<CASyllable*>,
              const QList<CAFiguredBasMark*>, QList<CAFiguredBassMark*>,
              const QList<CAFunctionMark*>, QList<CAFunctionMark*>,
              const QList<CAChordName*>, QList<CAChordName*> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::MusElement));
    }
	
    $result = list;
}
%typemap(out) const QList<CAMusElement*>&, QList<CAMusElement*>&,
              const QList<CANote*>&, QList<CANote*>&,
              const QList<CARest*>&, QList<CARest*>&,
              const QList<CAMark*>&, QList<CAMark*>&,
              const QList<CAPlayable*>&, QList<CAPlayable*>&,
              const QList<CASyllable*>&, QList<CASyllable*>&,
              const QList<CAFiguredBassMark*>&, QList<CAFiguredBassMark*>&,
              const QList<CAFunctionMark*>&, QList<CAFunctionMark*>&,
              const QList<CAChordName*>&, QList<CAChordName*>& {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1->size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1->at(i), CASwigPython::MusElement));
    }
	
    $result = list;
}
%typemap(out) const QList< QList<CAMidiNote*> >, QList< QList<CAMidiNote*> > {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
            PyObject *curList = PyList_New(0);
            for (int j=0; j<$1.at(i).size(); j++) {
                PyList_Append(curList, CASwigPython::toPythonObject($1.at(i).at(j), CASwigPython::MusElement));
            }
            PyList_Append(list, curList);
        }

    $result = list;
}
%typemap(out) const QList<CAVoice*>, QList<CAVoice*> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Voice));
    }

    $result = list;
}
%typemap(out) const QList<CAVoice*>&, QList<CAVoice*>& {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1->size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1->at(i), CASwigPython::Voice));
    }

    $result = list;
}
%typemap(out) const QList<CAContext*>, QList<CAContext*>,
              const QList<CAStaff*>, QList<CAStaff*>,
              const QList<CALyricsContext*>, QList<CALyricsContext*>,
              const QList<CAFiguredBassContext*>, QList<CAFiguredBassContext*>,
              const QList<CAFunctionMarkContext*>, QList<CAFunctionMarkContext*> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Context));
    }

    $result = list;
}
%typemap(out) const QList<CAContext*>&, QList<CAContext*>&,
              const QList<CAStaff*>&, QList<CAStaff*>&,
              const QList<CALyricsContext*>&, QList<CALyricsContext*>&,
              const QList<CAFiguredBassContext*>&, QList<CAFiguredBassContext*>&,
              const QList<CAFunctionMarkContext*>&, QList<CAFunctionMarkContext*>& {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1->size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1->at(i), CASwigPython::Context));
    }

    $result = list;
}
%typemap(out) const QList<CASheet*>, QList<CASheet*> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Sheet));
    }

    $result = list;
}
%typemap(out) const QList<CASheet*>&, QList<CASheet*>& {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1->size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1->at(i), CASwigPython::Sheet));
    }

    $result = list;
}
%typemap(out) const QList<CAResource*>, QList<CAResource*> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1.at(i), CASwigPython::Resource));
    }

    $result = list;
}
%typemap(out) const QList<CAResource*>&, QList<CAResource*>& {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1->size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject($1->at(i), CASwigPython::Resource));
    }

    $result = list;
}
%typemap(out) const QList<CAPlayableLength>, QList<CAPlayableLength> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject(const_cast<CAPlayableLength*>(&($1.at(i))), CASwigPython::PlayableLength));
    }

    $result = list;
}

%typemap(out) const QList<CAPlugin*>, QList<CAPlugin*> {
    PyObject *list = PyList_New(0);
    for (int i=0; i<$1.size(); i++) {
        PyList_Append(list, CASwigPython::toPythonObject(const_cast<CAPlugin*>($1.at(i)), CASwigPython::Plugin));
    }
	
    $result = list;
}

void markDelete( PyObject* ); // function used to delete Canorus objects inside Python
const char* tr( const char * sourceText, const char * comment = 0, int n = -1 );
bool hasGui();

// the following functions work when a plugin is launched inside Canorus:
void rebuildUi();
void repaintUi();
void acquireGIL();
void releaseGIL();
void setSelection( QList<CAMusElement*> elements, bool centerOn=false );

%include "scripting/canoruslibrary.i"

%{	// toPythonObject() function
#include "scripting/swigpython.h"	//needed for CAClassType

#include <QList>
QList<void*> markedObjects = QList<void*>(); // define markedObjects

void markDelete( PyObject* object ) {
    markedObjects << SWIG_Python_GetSwigThis(object)->ptr;
}

#ifndef SWIGCPP
#include "canorus.h"
#endif

/*!
	Reports error for Python functions which need Canorus GUI but are run from the CLI.
*/
void guiError() {
    std::cerr << "CanorusPython: No Canorus GUI found." << std::endl;
}

void rebuildUi() {
#ifndef SWIGCPP
    CACanorus::rebuildUI();
#else
    guiError();
#endif
}

void repaintUi() {
#ifndef SWIGCPP
    CACanorus::repaintUI();
#else
    guiError();
#endif
}

bool hasGui() {
#ifndef SWIGCPP
    return true;
#else
    return false;
#endif
}

/*!
    Workaround for acquiring GIL while inside Canorus plugin, if signal-slot operations are required.
*/
void acquireGIL() {
    PyEval_RestoreThread(CASwigPython::mainThreadState);
}

/*!
    Workaround for releasing GIL while inside Canorus plugin, if signal-slot operations are required.
*/
void releaseGIL() {
    PyEval_ReleaseThread(CASwigPython::mainThreadState);
}

/*!
    Selects the given elements in the current score view and optionally scrolls
    the view to center them.
*/
void setSelection( QList<CAMusElement*> elements, bool centerOn ) {
#ifndef SWIGCPP
    if (!elements.size() || !elements[0]->context() || !elements[0]->context()->sheet() || !elements[0]->context()->sheet()->document()) {
        return;
    }

    CADocument *doc = elements[0]->context()->sheet()->document();
    QList<CAMainWin*> mainwins = CACanorus::findMainWin(doc);

    if (!mainwins.size() || !mainwins[0]->currentScoreView()) {
        return;
    }

    mainwins[0]->currentScoreView()->clearSelection();
    mainwins[0]->currentScoreView()->addToSelection(elements);

    if (centerOn) {
        CADrawableMusElement *firstElement = mainwins[0]->currentScoreView()->selection()[0];
        mainwins[0]->currentScoreView()->setCenterCoords(
            firstElement->xPos(),
            firstElement->yPos(),
            false
        );
        mainwins[0]->currentScoreView()->setWorldX(
            firstElement->xPos()-12,
            false
        );
    }
#else
    guiError();
#endif
}

const char* tr( const char * sourceText, const char * comment = 0, int n = -1 ) {
    return QObject::tr( sourceText, comment, n ).toUtf8().constData();
}

class QString;

PyObject *CASwigPython::toPythonObject(void *object, CASwigPython::CAClassType type) {
    PyObject *pyObj = nullptr;
    switch (type) {
        case CASwigPython::String: {
            pyObj = Py_BuildValue("s", (static_cast<QString*>(object))->toUtf8().data());
            break;
        }
        case CASwigPython::Document: {
            pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CADocument, 0);
            break;
        }
        case CASwigPython::Resource: {
            pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAResource, 0);
            break;
        }
        case CASwigPython::Sheet: {
            pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CASheet, 0);
            break;
        }
        case CASwigPython::Context: {
            switch (static_cast<CAContext*>(object)->contextType()) {
            case CAContext::Staff:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAStaff, 0);
                break;
            case CAContext::LyricsContext:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CALyricsContext, 0);
                break;
            case CAContext::FiguredBassContext:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAFiguredBassContext, 0);
                break;
            case CAContext::FunctionMarkContext:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAFunctionMarkContext, 0);
                break;
            case CAContext::ChordNameContext:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAChordNameContext, 0);
                break;
            default:
                std::cerr << "canoruspython.i: Wrong CAContext::contextType()!" << std::endl;
                break;
            }
            break;
        }
        case CASwigPython::Voice: {
            pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAVoice, 0);
            break;
        }
        case CASwigPython::MusElement: {
            CAMusElement *elt = static_cast<CAMusElement*>(object);
            switch (elt->musElementType()) {
            case CAMusElement::Note:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CANote, 0);
                break;
            case CAMusElement::Rest:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CARest, 0);
                break;
            case CAMusElement::KeySignature:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAKeySignature, 0);
                break;
            case CAMusElement::TimeSignature:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CATimeSignature, 0);
                break;
            case CAMusElement::Clef:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAClef, 0);
                break;
            case CAMusElement::Barline:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CABarline, 0);
                break;
            case CAMusElement::FiguredBassMark:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAFiguredBassMark, 0);
                break;
            case CAMusElement::FunctionMark:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAFunctionMark, 0);
                break;
            case CAMusElement::Syllable:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CASyllable, 0);
                break;
            case CAMusElement::Mark:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAMark, 0);
                break;
            case CAMusElement::Slur:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CASlur, 0);
                break;
            case CAMusElement::Tuplet:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CATuplet, 0);
                break;
            case CAMusElement::MidiNote:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAMidiNote, 0);
                break;
            case CAMusElement::ChordName:
                pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAChordName, 0);
                break;
            default:
                std::cerr << "canoruspython.i: Wrong CAMusElement::musElementType()!" << std::endl;
                break;
            }
            break;
        }
        case CASwigPython::PlayableLength: {
            pyObj = SWIG_Python_NewPointerObj(0, new CAPlayableLength(*(static_cast<CAPlayableLength*>(object))), SWIGTYPE_p_CAPlayableLength, 0);
            break;
        }
        case CASwigPython::PyConsoleInterface: {
            pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAPyConsoleInterface, 0);
            break;
        }
        case CASwigPython::Plugin: {
            pyObj = SWIG_Python_NewPointerObj(0, object, SWIGTYPE_p_CAPlugin, 0);
            break;
        }
        default: {
            std::cerr << "canoruspython.i: Wrong CACanorusPython::type!" << std::endl;
            break;
        }
    }

    return pyObj;
}

%}
