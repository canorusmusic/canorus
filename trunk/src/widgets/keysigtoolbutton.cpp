/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/keysigtoolbutton.h"

#include <QWheelEvent>

CAKeySigToolButton::CAKeySigToolButton( QString title, QWidget *parent )
 : CAToolButton( parent )
{
	setCheckable( false );
	setText("a minor");
	setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
	
	_keySigsList = new QWidget();
	_keySigsList->setFixedWidth(100);
	_keySigsList->setFixedHeight(100);
}

CAKeySigToolButton::~CAKeySigToolButton() {
	delete _keySigsList;
}

void CAKeySigToolButton::setCurrentId( int id ) {
	CAToolButton::setCurrentId( id );
}

void CAKeySigToolButton::wheelEvent( QWheelEvent *e ) {
	
}

void CAKeySigToolButton::showButtons() {
	_keySigsList->show();
}

void CAKeySigToolButton::hideButtons( int buttonId ) {
	_keySigsList->hide();
}

void CAKeySigToolButton::hideButtons() {
	_keySigsList->hide();
}
