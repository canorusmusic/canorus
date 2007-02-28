/** @file widgets/multisplitter.cpp
 * 
 * Copyright (c) 2007, Itay Perl, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include <QSplitter>
#include "widgets/multisplitter.h"

/*!
	\class CAMultiSplitter 
	\brief A class to manage nested QSplitters.
	Used for multi-splitting functionality for viewports.
*/

/*!
	\brief Creates a multi-splitter object.
	@param first Pointer to the first widget to display. This widget is inserted into the main splitter.
	@param parent Pointer to the parent widget.
*/
CAMultiSplitter::CAMultiSplitter(QWidget *first, QWidget *parent)
	: QWidget(parent)
{
	QSplitter *main = new QSplitter(this);
	main->addWidget(first);
	_splitterList << main;
	_lastUsedWidget = first;
	_wCount = 1;
}

/*!
	\brief Destructs a multi-splitter object.
*/
CAMultiSplitter::~CAMultiSplitter()
{
	delete main();
}

/*!
	\brief Adds a widget to the multi-splitter. 
	The function adds a widget to the multi-splitter and places it after the last used widget.
	@param w The widget to insert.
	@param dir The splitting direction.
*/
void CAMultiSplitter::addWidget(QWidget *w, CA::Direction dir)
{
	if(!w) return;
	QSplitter *addTo = 0;
	int addAt = -1; //Invalid index inserts widget at the end
	for(int i=0; i<_splitterList.size(); i++)
	{
		int pos;
		if((pos = _splitterList[i]->indexOf(_lastUsedWidget)) != -1)
		{
			addAt = pos;
			addTo = _splitterList[i];
		}
	}
	if(!addTo)
	{
		printf("Error! _lastUsedWidget not found in any splitter.\n"); fflush(stdout);
		addTo = _splitterList.back();
	}
	
	Qt::Orientation o = (dir == CA::Vertical)?Qt::Horizontal:Qt::Vertical;
	
	if(addTo->count() == 1) 
	{
		addTo->setOrientation(o);
		addTo->addWidget(w);
		_wCount++;
		return;
	}
	if(addTo->orientation() == o) {
		addTo->insertWidget(addAt+1, w);
		_wCount++;
	}
	else {
		_lastUsedWidget->setParent(this); // remove from splitter
		
		QSplitter *newSplitter = new QSplitter(this);
		newSplitter->setOrientation(o);
		newSplitter->addWidget(_lastUsedWidget);
		newSplitter->addWidget(w);
		addTo->insertWidget(addAt, newSplitter);
		_splitterList << newSplitter;
		_wCount++;
	}
}

/*!
	\brief Removes a widget from the multi-splitter.
	@param w The widget to remove
	@param autoDelete If set to true (default), the widget is deleted.
*/
void CAMultiSplitter::removeWidget(QWidget *w, bool autoDelete)
{
	if(!w || _wCount == 1) return;
	bool removeSplitter = false;
	int idx = -1;
	for(int i=1; i<_splitterList.size(); i++) //i=1: don't remove the main splitter.
	{
		if(_splitterList[i]->indexOf(w) != -1 && _splitterList[i]->count() == 2)
		{
			removeSplitter = true;
			idx = i;
		}
	}
	
	if(autoDelete) {
		w->disconnect();
		delete w;
	}
	else
		w->setParent(this); // remove from splitter.
	_wCount--;
	if(removeSplitter)
	{
		// The size of the splitter should be 1.
		QWidget* pSplitterToWidget = _splitterList[idx];
		_splitterList.removeAll((QSplitter*)pSplitterToWidget);
		QWidget *newV = dynamic_cast<QSplitter*>(pSplitterToWidget)->widget(0); // get the only widget.
		newV->setParent(pSplitterToWidget->parentWidget());
		delete pSplitterToWidget;
		pSplitterToWidget = newV;
	}
}

/*!
	\enum CA::Direction Represents the direction of splitting.
*/
 
/*!
	\fn CAMultiSplitter::main()
	\brief Returns the main splitter.
*/

/*!
	\fn CAMultiSplitter::setLastUsedWidget(QWidget *v) 
	\brief Set the last used widget.
	@param v The last used widget.
*/

/*!
	\fn CAMultiSplitter::lastUsedWidget() 
	\brief Get the last used widget.
*/

/*!
	\fn CAMultiSplitter::widgetCount() 
	\brief Returns the number of widgets in the multi-splitter.
 */
