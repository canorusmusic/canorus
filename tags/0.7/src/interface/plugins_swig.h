#include <QObject>
#include "core/sheet.h"
#include "core/document.h"
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

class CACanorus {
public:
	static int mainWinCount() { return 0; };
	static CAMainWin* mainWinAt(int i) { return new CAMainWin(); }
};
