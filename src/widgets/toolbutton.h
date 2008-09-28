/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TOOLBUTTON_H_
#define TOOLBUTTON_H_

#include <QToolButton>

#include "widgets/toolbuttonpopup.h"

class QWidget;
class CAMainWin;

class CAToolButton : public QToolButton {
	Q_OBJECT
public:
	CAToolButton( QWidget *parent );
	virtual ~CAToolButton();
	inline int currentId() { return _currentId; }
	virtual void setCurrentId(int id) { _currentId = id; }
	inline bool buttonsVisible() { return (_popupWidget) ? _popupWidget->isVisible() : 0; } // Is the popup widget visible or not
	void setDefaultAction( QAction* );
	virtual void showButtons();
	virtual void hideButtons();

private slots:
	void handleToggled( bool checked );
	void handleTriggered();
	
signals:
	void toggled( bool checked, int id );

protected:
	inline CAMainWin *mainWin() { return _mainWin; }
	inline void setMainWin( CAMainWin *m ) { _mainWin = m; }
	inline void setPopupWidget(QWidget* w) { _popupWidget->setWidget(w); }
	virtual void wheelEvent(QWheelEvent*) = 0;
	void mousePressEvent( QMouseEvent* );
	QPoint calculateTopLeft( QSize widgetSize );
	
	CAMainWin          *_mainWin;         // Pointer to the main window for toolbar location polling etc.
	int                 _currentId;       // current ID of the button
	CAToolButtonPopup  *_popupWidget;	  // container for the floating widget
};

#endif /* TOOLBUTTON_H_ */
