/*!
    Copyright (c) 2009-2020, Štefan Sakalík, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/
#include <QObject>
#include "score/sheet.h"
#include "score/document.h"
#include "interface/pyconsoleinterface.h"

class CAMainWin { //: public QObject {
// currentSheet (SHEET = OK)

// CAScoreViewport(NOT_OK) {currentScoreViewport}
//   -> selection.size = v->selection().front()->drawableMusElement::DrawableNote?  : BAD BAD BAD
//   -> front()->musElement (OK)

// currentScoreViewport
//   -> musElementSelection ok
//
public:
	CAMainWin () {}
	CASheet *currentSheet() {
		//CAScoreViewPort *v = currentScoreViewPort();
		//if (v) return v->sheet();
		//else return 0;
		return nullptr;
	}

	CAPyConsoleInterface* pyConsoleIface = nullptr;
};

// Needs refactoring!
// Functions in this file should be moved to scripting directory and merged with
// scripting code (canoruspython.i and swigpython.cpp). -Matevz
class CACanorus {
public:
	inline static QList<CAMainWin*>& mainWinList() { return _mainWinList; }
private:
	static QList<CAMainWin*> _mainWinList;
};
