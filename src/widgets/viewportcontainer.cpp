/*! \file widgets/scrollwidget.h
 * 
 * Copyright (c) 2006-2007, Matevž Jekovec, Itay Perl, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPushButton>
#include <QGridLayout>

#include "widgets/viewportcontainer.h"
#include "widgets/viewport.h"
#include "core/sheet.h"
#include "core/note.h"
#include "core/staff.h"

/*!
	\class CAViewPortContainer
	\brief Holds together resizable viewports.
	
	This class behaves like dynamic QSplitter and can hold up a number of various viewports that can be customly resized.
	Usually this container serves as a layout class for containers in the tab widget.
	
	View port container should always include at least 1 view port. Otherwise there is no point of having the container at all!
	That's why the initial viewport is required in constructor already.
	
	\sa CAViewPort
 */

/*!
	Constructs the initial container having \a v as the initial viewport and a parent widget \a p.
*/
CAViewPortContainer::CAViewPortContainer( QWidget *parent )
 : QSplitter( parent ) {
	setOrientation( Qt::Vertical ); // not side by side
	setCurrentViewPort( 0 );
}

/*!
	Removes all the viewports and finally destroys container.
	\warning This destructor also deletes the viewports!
	
	\todo It would be better for destructor to not delete viewports but QSplitter does that automatically. Any way to not destroy the viewports? -Matevz
*/
CAViewPortContainer::~CAViewPortContainer() {
	// automatically deletes all the children including the splitters
}

/*!
	Splits the given viewport \a v vertically or the last used viewport if none given.
	Returns the newly created viewport.
*/		
CAViewPort* CAViewPortContainer::splitVertically(CAViewPort *v) {
	if ( !v ) v = currentViewPort();
	
	if ( v ) {
		QSplitter *splitter = _viewPortMap[v];
		CAViewPort *newViewPort = v->clone();
		if ( splitter->orientation()==Qt::Vertical ) {
			addViewPort( newViewPort, splitter );
		} else if ( splitter->count()==1 ) {
			splitter->setOrientation( Qt::Vertical );
			addViewPort( newViewPort, splitter );
		} else {
			QSplitter *newSplitter = new QSplitter( Qt::Vertical, 0 );
			int idx = splitter->indexOf(v);
			addViewPort( v, newSplitter);
			addViewPort( newViewPort, newSplitter);
			splitter->insertWidget( idx, newSplitter );
		}
		
		return newViewPort;
	} else {
		return 0;
	}
}

/*!
	Splits the given viewport \a v horizontally or the last used viewport if none given.
	Returns the newly created viewport.
*/		
CAViewPort* CAViewPortContainer::splitHorizontally(CAViewPort *v) {
	if ( !v ) v = currentViewPort();
	
	if ( v ) {
		QSplitter *splitter = _viewPortMap[v];
		CAViewPort *newViewPort = v->clone();
		if (splitter->orientation()==Qt::Horizontal) {
			addViewPort( newViewPort, splitter );
		} else if (splitter->count()==1) {
			splitter->setOrientation( Qt::Horizontal );
			addViewPort( newViewPort, splitter );
		} else {
			QSplitter *newSplitter = new QSplitter( Qt::Horizontal, 0 );
			int idx = splitter->indexOf(v);
			addViewPort( v, newSplitter);
			addViewPort( newViewPort, newSplitter);
			splitter->insertWidget( idx, newSplitter );
		}
		
		return newViewPort;
	} else {
		return 0;
	}
}

/*!
	Unsplits the views so the given viewport \a v is removed.
	If no viewports are given, remove the last active one.
	Returns the pointer to the viewport which was removed. If none was removed (ie. the given viewport was not found or there are no viewports left) returns 0.
	The deleted viewport is also deleted.
	
	\todo The viewport should not be deleted, but only removed from the splitter. However, splitter doesn't provide any removeWidget() analog to addWidget() methods. Any way to do this? -Matevz
*/				
CAViewPort* CAViewPortContainer::unsplit(CAViewPort *v) {
	if (!v)
		v = currentViewPort();
	
	if (v) {
		QSplitter *s = _viewPortMap[v];
		
		if ( s == this && s->count()==1 ) // don't remove the last viewport from the top splitter
			return 0;
		else if ( s != this && s->count()==2 ) { // remove the splitter as well
			int i;
			for ( i=0; s->widget(i)==v; i++ ); // find the viewport which should be moved to its parent
			s->widget(i)->setParent( static_cast<QWidget*>(s->parent()) );
			
			delete s; // delete the splitter and the viewport
			removeViewPort(v);
			return v;
		} else {
			delete v; // removes and deletes viewport from the splitter
			removeViewPort(v);
			return v;
		}
	}
	
	return 0;
}

/*!
	Unsplits all the viewports except the last active one.
*/
QList<CAViewPort*> CAViewPortContainer::unsplitAll() {
	CAViewPort *v = currentViewPort();
	QList<CAViewPort*> list;
	while(count() > 1)
		list << unsplit();
	
	return list;
}

/*!
	Adds and registers the given viewport \a v to the splitter \a s or the top splitter if not specified.
*/ 
void CAViewPortContainer::addViewPort( CAViewPort *v, QSplitter *s ) {
	if (!s)
		s = this;
	_viewPortMap[v] = s;
	
	s->addWidget( v );
	
	setCurrentViewPort( v );
}

/*!
	Removes the given viewport from internal list and reselects the current viewport if the currentViewPort() points to deleted viewport.
*/
void CAViewPortContainer::removeViewPort( CAViewPort *v ) {
	_viewPortMap.remove( v );
	
	if ( v==currentViewPort() )
		setCurrentViewPort( _viewPortMap.keys().last() );
}

/*!
	\fn CAViewPortContainer::lastUsedViewPort()
	Returns the pointer to the last used active viewport.
*/

/*!
	\var CAViewPortContainer::_viewPortMap
	Map of ViewPort : Splitter widgets. Every viewport has a splitter which it belongs to.
	If a viewports is the top-most widget, then it belongs to CAViewPortContainer itself.
*/
