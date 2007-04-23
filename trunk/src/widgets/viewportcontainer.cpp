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
*/
CAViewPortContainer::~CAViewPortContainer() {
	// automatically deletes all the children including the splitters
}

/*!
	Splits the given viewport \a v vertically or the last used viewport if none given.
	Vertical split uses horizontal splitter.
	Returns the newly created viewport.
*/		
CAViewPort* CAViewPortContainer::splitVertically(CAViewPort *v) {
	if ( !v ) v = currentViewPort();
	
	if ( v ) {
		QSplitter *splitter = _viewPortMap[v];
		CAViewPort *newViewPort = v->clone();
		if ( splitter->orientation()==Qt::Horizontal ) {
			addViewPort( newViewPort, splitter );
		} else if ( splitter->count()==1 ) {
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
	Splits the given viewport \a v horizontally or the last used viewport if none given.
	Horizontal split uses vertical splitter.
	Returns the newly created viewport.
*/		
CAViewPort* CAViewPortContainer::splitHorizontally(CAViewPort *v) {
	if ( !v ) v = currentViewPort();
	
	if ( v ) {
		QSplitter *splitter = _viewPortMap[v];
		CAViewPort *newViewPort = v->clone();
		if ( splitter->orientation()==Qt::Vertical ) {
			addViewPort( newViewPort, splitter );
		} else if (splitter->count()==1) {
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
	Unsplits the views so the given viewport \a v is removed.
	If no viewport is given, removes the last active one.
	
	\return The pointer to the viewport which was removed. If none was removed (ie. the given viewport was not found or there are no viewports left) returns 0.
*/				
CAViewPort* CAViewPortContainer::unsplit(CAViewPort *v) {
	if (!v && !(v = currentViewPort()))
		return 0;
	
	QSplitter *s = _viewPortMap[v];
	switch( s->count() )
	{
		case 1:
			// if (s==this). otherwise it'd never get here.
			return 0; 
		case 2:
		{
			QWidget* other = s->widget( 1 - s->indexOf( v ) ); // find the other viewport
			CAViewPort* otherViewPort = dynamic_cast<CAViewPort*>(other);
			if( s != this)
			{
				other->setParent( s->parentWidget() );
				if(otherViewPort)
					_viewPortMap[otherViewPort] = static_cast<QSplitter*>(s->parent());	
				v->setParent(0); // remove the viewport from the splitter
				delete s; // delete the splitter
				removeViewPort(v);
				return v;
			} else if( !otherViewPort )
			{
				// the following is needed to prevent the situation where a splitter is the only child of the viewport container - it causes some problems. -Itay
				QSplitter *sp = static_cast<QSplitter*>(other);
				setOrientation(sp->orientation());
				while(sp->count()) {
					CAViewPort* viewport = dynamic_cast<CAViewPort*>(sp->widget(0));
					sp->widget(0)->setParent( this );
					if(viewport)
						_viewPortMap[viewport] = this;
				}
				delete sp; //delete the splitter after moving the view ports.
			} 
		}
		// falls through only if s == this
		default:
			v->setParent(0); // remove the viewport from the splitter
			removeViewPort(v);
			return v;
	}
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
