/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPushButton>
#include <QGridLayout>

#include "widgets/viewcontainer.h"
#include "widgets/view.h"
#include "score/sheet.h"
#include "score/note.h"
#include "score/staff.h"

/*!
	\class CAViewContainer
	\brief Holds together resizable views.

	This class behaves like dynamic QSplitter and can hold up a number of various views that can be customly resized.
	Usually this container serves as a layout class for containers in the tab widget.

	View port container should always include at least 1 view port. Otherwise there is no point of having the container at all!
	That's why the initial view is required in constructor already.

	\sa CAView
 */

/*!
	Constructs the initial container having \a v as the initial view and a parent widget \a p.
*/
CAViewContainer::CAViewContainer( QWidget *parent )
 : QSplitter( parent ) {
	setOrientation( Qt::Vertical ); // not side by side
	setCurrentView( 0 );
}

/*!
	Removes all the views and finally destroys container.
	\warning This destructor also deletes the views!
*/
CAViewContainer::~CAViewContainer() {
	// automatically deletes all the children including the splitters
}

/*!
	Splits the given view \a v vertically or the last used view if none given.
	Vertical split uses horizontal splitter.
	Returns the newly created view.
*/
CAView* CAViewContainer::splitVertically(CAView *v) {
	if ( !v && !(v = currentView()))
		return 0;

	QSplitter *splitter = _viewMap[v];
	CAView *newView = v->clone(0);
	if ( splitter->orientation()==Qt::Horizontal ) {
		addView( newView, splitter );
	} else if ( splitter->count()==1 ) {
		splitter->setOrientation( Qt::Horizontal );
		addView( newView, splitter );
	} else {
		QSplitter *newSplitter = new QSplitter( Qt::Horizontal, 0 );
		int idx = splitter->indexOf(v);
		addView( v, newSplitter);
		addView( newView, newSplitter);
		splitter->insertWidget( idx, newSplitter );
	}

	return newView;
}

/*!
	Splits the given view \a v horizontally or the last used view if none given.
	Horizontal split uses vertical splitter.
	Returns the newly created view.
*/
CAView* CAViewContainer::splitHorizontally(CAView *v) {
	if ( !v && !(v = currentView()))
		return 0;

	QSplitter *splitter = _viewMap[v];
	CAView *newView = v->clone(0);
	if ( splitter->orientation()==Qt::Vertical ) {
		addView( newView, splitter );
	} else if (splitter->count()==1) {
		splitter->setOrientation( Qt::Vertical );
		addView( newView, splitter );
	} else {
		QSplitter *newSplitter = new QSplitter( Qt::Vertical, 0 );
		int idx = splitter->indexOf(v);
		addView( v, newSplitter);
		addView( newView, newSplitter);
		splitter->insertWidget( idx, newSplitter );
	}

	return newView;
}

/*!
	Unsplits the views so the given view \a v is removed.
	If no view is given, removes the last active one.

	\return The pointer to the view which was removed. If none was removed (ie. the given view was not found or there are no views left) returns 0.
*/
CAView* CAViewContainer::unsplit(CAView *v) {
	if (!v && !(v = currentView()))
		return 0;

	QSplitter *s = _viewMap[v];
	switch( s->count() )
	{
		case 1:
			// if (s==this). otherwise it'd never get here.
			return 0;
		case 2:
		{
			QWidget* other = s->widget( 1 - s->indexOf( v ) ); // find the other view
			CAView* otherView = dynamic_cast<CAView*>(other);
			if( s != this)
			{
				other->setParent( s->parentWidget() );
				if(otherView)
					_viewMap[otherView] = static_cast<QSplitter*>(s->parent());
				removeView(v);
				delete s; // delete the splitter
				return v;
			} else if( !otherView )
			{
				// the following is needed to prevent the situation where a splitter is the only child of the view container - it causes some problems. -Itay
				QSplitter *sp = static_cast<QSplitter*>(other);
				setOrientation(sp->orientation());
				while(sp->count()) {
					CAView* view = dynamic_cast<CAView*>(sp->widget(0));
					sp->widget(0)->setParent( this );
					if(view)
						_viewMap[view] = this;
				}
				delete sp; //delete the splitter after moving the view ports.
			}
		}
		// falls through only if s == this
		default:
			removeView(v);
			return v;
	}
}

/*!
	Unsplits all the views except the last active one.
*/
QList<CAView*> CAViewContainer::unsplitAll() {
	QList<CAView*> list;
	while(count() > 1)
		list << unsplit();

	return list;
}

/*!
	Adds and registers the given view \a v to the splitter \a s or the top splitter if not specified.
*/
void CAViewContainer::addView( CAView *v, QSplitter *s ) {
	if (!s)
		s = this;
	_viewMap[v] = s;

	s->addWidget( v );

	setCurrentView( v );
}

/*!
	Removes the given view from internal list and reselects the current view if the currentView() points to deleted view.
*/
void CAViewContainer::removeView( CAView *v ) {
	_viewMap.remove( v );
	delete v;

	if ( v==currentView() )
		setCurrentView( _viewMap.keys().last() );
}

/*!
	\fn CAViewContainer::lastUsedView()
	Returns the pointer to the last used active view.
*/

/*!
	\var CAViewContainer::_viewMap
	Map of View : Splitter widgets. Every view has a splitter which it belongs to.
	If a views is the top-most widget, then it belongs to CAViewContainer itself.
*/
