/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
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
			// Qt objects
			String,

			// Canorus objects
			Document,
			Sheet,
			Context,
			Staff,
			Voice,
			FunctionMarkContext,
			MusElement,
			Playable,
			Note,
			Rest,
			KeySignature,
			TimeSignature,
			Clef,
			Barline,
			FunctionMark,
			LyricsContext,
			Syllable,
			Mark,
			Slur,
			Tuplet,

			// Console
			PyConsoleInterface
		};

		static void init();
		static PyObject *callFunction(QString fileName, QString function, QList<PyObject*> args);
		static void *callPycli(void*);
		static PyObject *toPythonObject(void *object, CAClassType type);	// defined in scripting/canoruspython.i

        static PyThreadState *mainThreadState, *pycliThreadState;
};

#endif /*SWIGPYTHON_H_*/
#endif
