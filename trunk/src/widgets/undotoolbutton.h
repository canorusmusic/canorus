/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef KEYSIGTOOLBUTTON_H_
#define KEYSIGTOOLBUTTON_H_

#include "widgets/toolbutton.h"

#include <QUndoView>

class QUndoStack;

class CAUndoToolButton : public CAToolButton {
	Q_OBJECT
public:
	CAUndoToolButton( QUndoStack*, QIcon icon, QWidget *parent );
	~CAUndoToolButton();
	bool buttonsVisible() { return _undoView->isVisible(); }
	void setCurrentId(int id);
	void setDefaultAction( QAction* );
	
public slots:
	void showButtons();
	void hideButtons( int buttonId );
	void hideButtons();
	
protected:
	void wheelEvent(QWheelEvent*);
	
private:
	QUndoView *_undoView;
	QIcon _icon;
};

#endif /* KEYSIGTOOLBUTTON_H_ */
