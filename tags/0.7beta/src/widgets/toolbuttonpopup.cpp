/*!
	Copyright (c) 2007, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QWidget>
#include <QMouseEvent>

#include "widgets/toolbuttonpopup.h"

/*!
	\class CAToolButtonPopup
	\brief Container for tool button popup widgets
	
	This container sets the correct popup behavior for tool button floating widgets.
*/

CAToolButtonPopup::CAToolButtonPopup(QWidget* parent) : QWidget(parent), _widget(0) {
	setWindowFlags( Qt::Popup );
}

void CAToolButtonPopup::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);
	if(!QRect(x(), y(), width(), height()).contains(e->globalPos()))
		hide();
}
