/*!
	Copyright (c) 2018, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "ui/jumptoview.h"
#include "ui/mainwin.h"
#include "widgets/scoreview.h"

#include "layout/drawablebarline.h"

CAJumpToView::CAJumpToView( CAMainWin *p )
: QDialog( p ) {
	setupUi( this );
	setupCustomUi();
}

CAJumpToView::~CAJumpToView() {
}

void CAJumpToView::setupCustomUi() {
}

void CAJumpToView::show() {
//	CAScoreView *v = static_cast<CAMainWin*>(parent())->currentScoreView();

//	QDockWidget::show();
	QDialog::show();
}

void CAJumpToView::accept() {
	int barNumber = uiJumpToBarNum->text().toInt();

	if ( dynamic_cast<CAMainWin*>(parent()) &&
	     static_cast<CAMainWin*>(parent())->currentScoreView() ) {
		CAScoreView *v = static_cast<CAMainWin*>(parent())->currentScoreView();
		QMap<int, CADrawableBarline*> dBarlineMap = v->computeBarlinePositions();

		if (!dBarlineMap.isEmpty()) {
			// if barNumber is outside barlines enumeration, take the first/last barline available
			if (barNumber < dBarlineMap.firstKey() || barNumber >= dBarlineMap.lastKey()) {
				return;
			}

			// shift the view
			v->setWorldX(dBarlineMap[barNumber]->xPos() - v->worldWidth()/2.0);
			v->repaint();

			QDialog::accept();
		}
	}
}
