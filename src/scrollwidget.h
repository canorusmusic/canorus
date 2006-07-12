/** @file scrollwidget.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QFrame>
#include <QList>
#include <QGridLayout>

#include "kdtree.h"


class CAViewPort;
class CASheet;

/**
 * CAScrollWidget consists of viewports.
 * It is the central widget that represents CASheet object.
 */
class CAScrollWidget : public QFrame {
	Q_OBJECT

	public:
		/**
		 * The default constructor.
		 * 
		 * @param v Pointer to the initial viewPort to be added to the layout.
		 * @param p Parent widget.
		 */
		CAScrollWidget(CAViewPort *v, QWidget *p);
		
		/**
		 * Return the last used active viewport.
		 * 
		 * @return Pointer to the last used CAViewPort.
		 */
		CAViewPort *lastUsedViewPort() { return _lastUsedViewPort; }
		
		/**
		 * Clone the viewPort.
		 *
		 * @return Pointer to the new viewPort class.
		 */
		CAViewPort *clone();

		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Clone the viewPort with a different parent widget.
		 *
		 * @param parent New parent of the cloned widget.
		 * @return Pointer to the new viewPort class.
		 */
		CAViewPort *clone(QWidget *parent);

		/**
		 * Return the number of rows of viewports.
		 * 
		 * @return Number of rows the viewports are arranged in.
		 */
		int viewPortsRows() { return _layout->rowCount(); }

		/**
		 * Return the number of columns of viewports.
		 * 
		 * @return Number of columns the viewports are arranged in.
		 */
		int viewPortsColumns() { return _layout->columnCount(); }

		/**
		 * Split the view horizontally.
		 * 
		 * @param v The viewport that should be splitted. If none given, split the last active one.
		 * @return The new viewport added to the layout.
		 */		
		CAViewPort* splitHorizontally(CAViewPort *v = 0);

		/**
		 * Split the view vertically.
		 * 
		 * @param v The viewport that should be splitted. If none given, split the last active one.
		 * @return The new viewport added to the layout.
		 */		
		CAViewPort* splitVertically(CAViewPort *v = 0);
		
		/**
		 * Open a new viewport in a separated window.
		 * 
		 * @param v The viewport the new one should be copied from. If non given, a copy of the last active one is created.
		 */
		CAViewPort* newViewPort(CAViewPort *v = 0);

		/**
		 * Unsplit the views so the given viewport gets deleted.
		 * If no viewports are given, delete the last active one.
		 * 
		 * @param v Pointer to the viewport that should be deleted. If none given, delete the last active one.
		 * @return Pointer to the viewport which was deleted. If none was deleted (ie. the given viewport was not found or there are no viewports left), return 0.
		 */				
		CAViewPort* unsplit(CAViewPort *v = 0);
		
	private slots:
		
		/**
		 * Process the paint event.
		 * Also called when chlidren widgets get repainted!
		 * 
		 * @param e QPaintEvent which gets processed.
		 */
		void paintEvent(QPaintEvent *e);

		/**
		 * Process the resize event.
		 * 
		 * @param e QResizeEvent which gets processed.
		 */
		void resizeEvent(QResizeEvent *);

	private:
		////////////////////////////////////////////////
		//Basic properties
		////////////////////////////////////////////////
		CAKDTree _musElements;	///List of music elements stored in a tree for faster lookup and other operations. Multiple viewports can share the same list.
		CASheet *_sheet;	///Pointer to the CASheet which this scrollwidget belongs to

		////////////////////////////////////////////////
		//Widgets
		////////////////////////////////////////////////
		QList<CAViewPort *> _viewPorts;	///List of all docked active viewports (multiple viewports appear, when you use split view etc.). Undocked viewports (when triggered by Window->New viewport) are not stored.
		QGridLayout *_layout;	///Layout of the viewports widgets
		CAViewPort *_lastUsedViewPort;	///The last viewport user interacted with
};
#endif

