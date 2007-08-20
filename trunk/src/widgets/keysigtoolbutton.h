/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef KEYSIGTOOLBUTTON_H_
#define KEYSIGTOOLBUTTON_H_

#include "widgets/toolbutton.h"

class CAKeySigToolButton : public CAToolButton {
	Q_OBJECT
public:
	CAKeySigToolButton( QString title, QWidget *parent );
	~CAKeySigToolButton();
	bool buttonsVisible() { return _keySigsList->isVisible(); }
	void setCurrentId(int id);
	
public slots:
	void showButtons();
	void hideButtons( int buttonId );
	void hideButtons();
	
protected:
	void wheelEvent(QWheelEvent*);
	
private:
	QWidget *_keySigsList;
};

#endif /* KEYSIGTOOLBUTTON_H_ */
