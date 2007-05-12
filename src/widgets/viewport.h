/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

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
	
private slots:
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent *e);		
	virtual void keyPressEvent(QKeyEvent *e);
	
signals:
	void CAMousePressEvent(QMouseEvent *e, CAViewPort *v);
	void CAMouseReleaseEvent(QMouseEvent *e, CAViewPort *v);
	void CAMouseMoveEvent(QMouseEvent *e, CAViewPort *v);
	void CAWheelEvent(QWheelEvent *e, CAViewPort *v);
	void CAKeyPressEvent(QKeyEvent *e, CAViewPort *v);
	
protected:
	inline void setViewPortType(CAViewPortType t) { _viewPortType = t; }
	
	////////////////////////
	// General properties //
	////////////////////////
	CAViewPortType _viewPortType;
};
#endif
