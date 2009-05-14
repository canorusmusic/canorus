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
	CAMainWin () { pyConsoleIface = NULL; };
	CASheet *currentSheet() {
		//CAScoreViewPort *v = currentScoreViewPort();
		//if (v) return v->sheet();
		//else return 0;
		return NULL;
	};

	CAPyConsoleInterface* pyConsoleIface;
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
