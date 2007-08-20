/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TOOLBUTTON_H_
#define TOOLBUTTON_H_

#include <QToolButton>

class QWidget;
class CAMainWin;

class CAToolButton : public QToolButton {
	Q_OBJECT
public:
	CAToolButton( QWidget *parent );
	virtual ~CAToolButton();
	inline int currentId() { return _currentId; }
	virtual void setCurrentId(int id) { _currentId = id; }
	virtual bool buttonsVisible() = 0; // Is the popup widget visible or not
	
public slots:
	virtual void showButtons()=0;
	virtual void hideButtons( int buttonId )=0;
	virtual void hideButtons()=0;
	
signals:
	void toggled( bool checked, int id );

protected:
	inline CAMainWin *mainWin() { return _mainWin; }
	inline void setMainWin( CAMainWin *m ) { _mainWin = m; }
	virtual void wheelEvent(QWheelEvent*) = 0;
	void mousePressEvent( QMouseEvent* );
	
	CAMainWin          *_mainWin;         // Pointer to the main window for toolbar location polling etc.
	int                 _currentId;       // current ID of the button
};

#endif /*TOOLBUTTON_H_*/
