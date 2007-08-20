/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/toolbutton.h"
#include "ui/mainwin.h"

#include <QMainWindow>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionToolButton>

/*!
	\class CAToolButton
	\brief Tool button with a menu at the side
	
	This widget looks like a button with a small dropdown arrow at the side which opens a
	custom widget (also called "buttons") where user chooses from various actions then.
	When the element is selected, the action's icon is replaced with the previous icon on
	the toolbutton and a signal toggled(bool checked, int id) is emitted.
*/ 

CAToolButton::CAToolButton( QWidget *parent )
 : QToolButton( parent ) {
	setMainWin( parent?dynamic_cast<CAMainWin*>(parent):0 );
	setPopupMode( QToolButton::MenuButtonPopup );	
}

CAToolButton::~CAToolButton() {
}

/*!
	This function is overriden here in order to show buttons when clicked on the arrow.
*/
void CAToolButton::mousePressEvent( QMouseEvent *e ) {
	QStyleOptionToolButton opt;
    opt.init(this);	
	opt.subControls |= QStyle::SC_ToolButtonMenu;
	opt.features |= QStyleOptionToolButton::Menu;
    
	QRect popupr = style()->subControlRect(QStyle::CC_ToolButton, &opt,
	                                       QStyle::SC_ToolButtonMenu, this);
	if (popupr.isValid() && popupr.contains(e->pos())) {
		if ( !buttonsVisible() ) {
			showButtons();
		} else {
			hideButtons();
		}
	}
	QToolButton::mousePressEvent(e);
}
