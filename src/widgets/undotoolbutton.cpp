/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/undotoolbutton.h"

#include "canorus.h"
#include "core/undo.h"
#include "core/undocommand.h"

#include <QWheelEvent>
#include <QAction>

CAUndoToolButton::CAUndoToolButton( QIcon icon, CAUndoToolButtonType type, QWidget *parent )
 : CAToolButton( parent )
{
	setCheckable( false );
	setIcon(icon);
	
	_icon = icon;
	setCurrentId(0);
	setUndoType( type );
	_listWidget = new QListWidget;
	_listWidget->setMouseTracking(true);
	_listWidget->setSelectionMode(QListWidget::MultiSelection);
	setPopupWidget(_listWidget);
	connect( _listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onListWidgetItemClicked(QListWidgetItem*)) );
	connect( _listWidget, SIGNAL(itemEntered(QListWidgetItem*)), this, SLOT(onListWidgetItemEntered(QListWidgetItem*)) );
}

CAUndoToolButton::~CAUndoToolButton() {
	delete _listWidget;
}

void CAUndoToolButton::wheelEvent( QWheelEvent *e ) {
	// do nothing!
}

void CAUndoToolButton::onListWidgetItemClicked( QListWidgetItem *item ) {
	hideButtons();
	emit toggled( false, _listWidget->row(item) );
}

void CAUndoToolButton::onListWidgetItemEntered( QListWidgetItem *item ) {
	for(int i=0; i<_listWidget->count(); i++)
		_listWidget->item(i)->setSelected(i<=_listWidget->row(item));
}

void CAUndoToolButton::showButtons() {
	_listWidget->clear();
	if ( mainWin() ) {
		QList<CAUndoCommand*> *stack = CACanorus::undo()->undoStack(mainWin()->document());
		if ( undoType()==Undo ) {
			for (int i = CACanorus::undo()->undoIndex(mainWin()->document()), delta=0;
			     i >= 0 && delta<20; /// \todo This should be set to maxUndo steps
			     i--, delta++)
				_listWidget->addItem( stack->at(i)->text() );
		} else
		if ( undoType()==Redo ) {
			for (int i = CACanorus::undo()->undoIndex(mainWin()->document())+1, delta=0;
			     i < stack->size() && delta<20; /// \todo This should be set to maxUndo steps
			     i++, delta++)
				_listWidget->addItem( stack->at(i)->text() );
		}
	}
	CAToolButton::showButtons();
}

void CAUndoToolButton::setDefaultAction( QAction *action ) {
	CAToolButton::setDefaultAction(action);
	if ( defaultAction() ) {
		defaultAction()->setCheckable( false );
		defaultAction()->setIcon( _icon );
	}
}
