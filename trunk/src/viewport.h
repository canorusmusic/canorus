/** @file viewport.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

#define DEFAULT_VIEWPORT_WIDTH 400
#define DEFAULT_VIEWPORT_HEIGHT 400

/**
 * CAViewPort is the base widget for different types of views of the document.
 * Viewport can represent a score view, envelope view, score source view etc.
 */
class CAViewPort : public QWidget {
	Q_OBJECT
	
	public:
		/**
		 * The default constructor.
		 * 
		 * @param parent The parent widget.
		 * @param musElts Pointer to a KDTree of music elements
		 */
		CAViewPort(QWidget *parent);
		
		/**
		 * Destructor.
		 */
		~CAViewPort();
		
		/**
		 * This enum holds different viewport types:
		 * - ScoreViewPort - The main view of the score. All the music elements (staffs, notes, rests) are rendered to this viewport. 
		 */
		enum CAViewPortType {
			ScoreViewPort
		};
		
		/**
		 * Return the viewport type. See enum CAViewPortType for viewport types.
		 * 
		 * @return Return the viewport's type in CAViewPortType type.
		 */
		CAViewPortType viewPortType() { return _viewPortType; }
		
		/**
		 * Clone the viewport.
		 *
		 * @return Pointer to the new viewPort class.
		 */
		virtual CAViewPort *clone() = 0;

		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Clone the viewport with a different parent widget.
		 *
		 * @param parent New parent of the cloned widget.
		 * @return Pointer to the new viewPort class.
		 */
		virtual CAViewPort *clone(QWidget *parent) = 0;
		
	protected:
		////////////////////////////////////////////////
		//General properties
		////////////////////////////////////////////////
		CAViewPortType _viewPortType;

		////////////////////////////////////////////////
		//Widgets
		////////////////////////////////////////////////
		QWidget *_parent;	///Pointer to the parent widget.
		
};
#endif
