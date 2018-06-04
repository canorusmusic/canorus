/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TABWIDGET_H_
#define TABWIDGET_H_

#include <QTabWidget>

class QMouseEvent;

class CATabWidget : public QTabWidget {
	Q_OBJECT
public:
	CATabWidget( QWidget *parent=0 );
	virtual ~CATabWidget();

signals:
	void CANewTab();
	void CAMoveTab( int from, int to );

protected slots:
	void mouseDoubleClickEvent( QMouseEvent * event );

protected:
	void tabInserted(int);
	void tabRemoved(int);
};

#endif /* TABWIDGET_H_ */
