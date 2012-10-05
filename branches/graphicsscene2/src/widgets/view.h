/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef VIEW_H
#define VIEW_H

#include <QWidget>

class QCloseEvent;

class CAView : public QWidget {
Q_OBJECT

public:
	CAView(QWidget *parent=0);

	virtual ~CAView();

	enum CAViewType {
		ScoreView,
		SourceView
	};

	inline CAViewType viewType() { return _viewType; }

	virtual CAView *clone() = 0;
	virtual CAView *clone(QWidget *parent) = 0;

	virtual void rebuild() = 0;

	static const int DEFAULT_VIEW_WIDTH;
	static const int DEFAULT_VIEW_HEIGHT;

protected slots:
	void mousePressEvent(QMouseEvent *e);
	inline void closeEvent(QCloseEvent*) { emit closed(this); }

signals:
	void clicked();
	void closed(CAView*);

protected:
	inline void setViewType(CAViewType t) { _viewType = t; }

	////////////////////////
	// General properties //
	////////////////////////
	CAViewType _viewType;
};
#endif
