/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef VIEWCONTAINER_H
#define VIEWCONTAINER_H

#include <QSplitter>
#include <QHash>

class CAView;
class CASheet;

class CAViewContainer : public QSplitter {
Q_OBJECT

public:
	CAViewContainer( QWidget *p );
	~CAViewContainer();

	void addView( CAView *v, QSplitter *s=0 );
	void removeView( CAView *v );
	CAView* splitHorizontally( CAView *v = 0 );
	CAView* splitVertically( CAView *v = 0 );

	CAView* unsplit( CAView *v = 0 );
	QList<CAView*> unsplitAll();

	inline bool contains(CAView* v) { return _viewMap.contains(v); }
	inline const QList<CAView*> viewList() { return _viewMap.keys(); }

	inline void setCurrentView( CAView *v ) { _currentView = v; }
	inline CAView* currentView() { return _currentView; }

private:
	QHash<CAView*, QSplitter*> _viewMap;
	CAView *_currentView;
	CASheet *_sheet;
};
#endif
