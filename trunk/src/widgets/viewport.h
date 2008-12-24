/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

class QCloseEvent;

class CAViewPort : public QWidget {
Q_OBJECT

public:
	CAViewPort(QWidget *parent=0);
	
	virtual ~CAViewPort();
	
	enum CAViewPortType {
		ScoreViewPort,
		SourceViewPort
	};
	
	inline CAViewPortType viewPortType() { return _viewPortType; }
	
	virtual CAViewPort *clone() = 0;
	virtual CAViewPort *clone(QWidget *parent) = 0;
	
	virtual void rebuild() = 0;
	
	static const int DEFAULT_VIEWPORT_WIDTH;
	static const int DEFAULT_VIEWPORT_HEIGHT;
	
protected slots:
	void mousePressEvent(QMouseEvent *e);
	inline void closeEvent(QCloseEvent*) { emit closed(this); }
	
signals:
	void clicked();
	void closed(CAViewPort*);
	
protected:
	inline void setViewPortType(CAViewPortType t) { _viewPortType = t; }
	
	////////////////////////
	// General properties //
	////////////////////////
	CAViewPortType _viewPortType;
};
#endif
