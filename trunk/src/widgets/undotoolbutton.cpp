/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/undotoolbutton.h"

#include <QWheelEvent>
#include <QAction>

CAUndoToolButton::CAUndoToolButton( QUndoStack *undoStack, QIcon icon, QWidget *parent )
 : CAToolButton( parent )
{
	setCheckable( false );
	setIcon(icon);
	
	_icon = icon;
	_undoView = new QUndoView( undoStack );
}

CAUndoToolButton::~CAUndoToolButton() {
	delete _undoView;
	setDefaultAction(0);
}

void CAUndoToolButton::setCurrentId( int id ) {
	CAToolButton::setCurrentId( id );
}

void CAUndoToolButton::wheelEvent( QWheelEvent *e ) {
	
}

void CAUndoToolButton::showButtons() {
	_undoView->show();
}

void CAUndoToolButton::hideButtons( int buttonId ) {
	_undoView->hide();
}

void CAUndoToolButton::hideButtons() {
	_undoView->hide();
}

void CAUndoToolButton::setDefaultAction( QAction *action ) {
	CAToolButton::setDefaultAction(action);
	if ( defaultAction() ) {
		defaultAction()->setCheckable( false );
		defaultAction()->setIcon( _icon );
	}
}
