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
#include "viewport.h"
#include "sheet.h"

#define DEFAULT_VIEWPORT_WIDTH 400
#define DEFAULT_VIEWPORT_HEIGHT 400

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
		 * @param s Sheet which this scrollwidget represents 
		 * @param p Parent widget.
		 */
		CAScrollWidget(CASheet *s, QWidget *p);

		/**
		 * Split the view horizontally.
		 * 
		 * @param v The viewport that should be splitted. If none given, split the last active one.
		 */		
		void splitHorizontally(CAViewPort *v = 0);

		/**
		 * Split the view vertically.
		 * 
		 * @param v The viewport that should be splitted. If none given, split the last active one.
		 */		
		void splitVertically(CAViewPort *v = 0);
		
		/**
		 * Unsplit the views.
		 * 
		 * @param v The viewport that should deleted. If none given, delete the last active one.
		 */				
		void unsplit(CAViewPort *v = 0);
		
		/**
		 * Open a new viewport in a separated window.
		 * 
		 * @param v The viewport the new one should be copied from. If non given, a copy of the last active one is created.
		 */
		void newViewPort(CAViewPort *v = 0);
		
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

	signals:
		/**
		 * The signal which gets emmitted when you click on an empty place.
		 * 
		 * @param e Mouse event which gets emmitted.
		 */
		void mousePressEvent(QMouseEvent *e);
		
		/**
		 * The signal which gets emmitted when you use wheel on an empty place.
		 * 
		 * @param e Wheel event which gets emmitted.
		 */
		void wheelEvent(QWheelEvent *);

	private slots:
		/**
		 * Process the mouse events of the children viewports.
		 * 
		 * @param e Mouse event which gets processed.
		 * @param coords Absolute world coordinates where the mouse cursor was at time of the event.
		 * @param v Pointer to viewport where the event happened.
		 */
		void viewPortMousePressEvent(QMouseEvent *e, QPoint coords, CAViewPort *v);

		/**
		 * Process the wheel events of the children viewports.
		 * 
		 * @param e Wheel event which gets processed.
		 * @param coords Absolute world coordinates where the mouse cursor was at time of the event.
		 * @param v Pointer to viewport where the event happened.
		 */
		void viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *v);
		
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
		QList<CAViewPort *> _viewPorts;	///List of all active viewports (multiple viewports appear, when you use split view etc.)
		QGridLayout *_layout;	///Layout of the viewports widgets
		CAViewPort *_lastUsedViewPort;	///The last viewport user interacted with
};
#endif

