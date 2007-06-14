/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QSplitter>
#include <QHash>

class CAViewPort;
class CASheet;

class CAViewPortContainer : public QSplitter {
Q_OBJECT

public:
	CAViewPortContainer( QWidget *p );
	~CAViewPortContainer();
	
	void addViewPort( CAViewPort *v, QSplitter *s=0 );
	void removeViewPort( CAViewPort *v );
	CAViewPort* splitHorizontally( CAViewPort *v = 0 );
	CAViewPort* splitVertically( CAViewPort *v = 0 );
	
	CAViewPort* unsplit( CAViewPort *v = 0 );		
	QList<CAViewPort*> unsplitAll();
	
	inline bool contains(CAViewPort* v) { return _viewPortMap.contains(v); } 
	inline QList<CAViewPort*> viewPortList() { return _viewPortMap.keys(); }
	
	inline void setCurrentViewPort( CAViewPort *v ) { _currentViewPort = v; }		
	inline CAViewPort* currentViewPort() { return _currentViewPort; }
	
private:
	QHash<CAViewPort*, QSplitter*> _viewPortMap;
	CAViewPort *_currentViewPort;
	CASheet *_sheet;
};
#endif
