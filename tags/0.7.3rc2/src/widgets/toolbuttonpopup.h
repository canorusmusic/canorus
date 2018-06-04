/*!
	Copyright (c) 2007, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TOOLBUTTONPOPUP_H_
#define TOOLBUTTONPOPUP_H_

#include <QWidget>

class CAToolButtonPopup: public QWidget {
Q_OBJECT
public:
		CAToolButtonPopup(QWidget* parent = 0);
		~CAToolButtonPopup() { /* widget is not destroyed! */ } 
		inline void setWidget(QWidget* w) { _widget = w; w->setParent(this); }
		inline QWidget* widget() { return _widget; }
		inline QSize sizeHint() const { return (_widget)?_widget->sizeHint():QSize(0,0); } 
protected:
		void mousePressEvent(QMouseEvent* e);
		QWidget* _widget;
};

#endif /* TOOLBUTTONPOPUP_H_ */
