/* 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include "core/context.h"

/*!
	\class CAContext
	\brief Line of music elements in the sheet
	
	CAContext represents usually an infinite graphical space on the sheet where music elements are
	placed in. This idea was introduced by LilyPond and it turned out to be quite useful.
	
	Contexts can be staffs, lyrics, figured bass context, function marking context, dynamics etc.
	
	CAContext is an abstract class and different solutions should be done based on it.
	
	\sa CAStaff, CAFunctionMarkingContext
*/

/*!
	Creates a context named \a name and with parent sheet \a s.
*/
CAContext::CAContext(CASheet *s, const QString name) {
	_sheet = s;
	_name = name;
}

/*!
	\enum CAContext::CAContextType
	This enum holds different CAContext types:
		- Staff
			Every context with lines which includes various music elements.
		- Tablature
			Context similar to Staff, but specialized for guitar.
		- Lyrics
			Holds words (syllables) for choir music. Every syllable is assigned to certain CANote.
		- Dynamics
			Sometimes we want to have a separate context for the crescendo, rit., mf and other markings
*/

/*!
	\fn CAContext::name()
	Returns context's name.
	
	\sa setName(), _name
*/

/*!
	\fn CAContext::setName(const QString name)
	Sets the Context's name to \a name.
	
	\sa name(), _name
*/

/*!
	\fn CAContext::findNextMusElement(CAMusElement *elt)
	Finds the next music element to the given \a elt and returns its pointer.
	This method is usually used when walking through notes using cursor keys.
	
	Returns pointer to the elements \a elt right neighbour or 0, if the right neighbour doesn't
	exist.
	
	\sa findPrevMusElement()
*/

/*!
	\fn CAContext::findPrevMusElement(CAMusElement *elt)
	Finds the previous music element to the given \a elt and returns its pointer.
	This method is usually used when walking through notes using cursor keys.
	
	Returns pointer to the elements \a elt left neighbour or 0, if the left neighbour doesn't exist.
	
	\sa findNextMusElement()
*/

/*!
	\fn CAContext::removeMusElement(CAMusElement *elt, bool cleanup)
	Removes the music element \a elt from the context.
	Destroys the object as well, if \a cleanup is true (default).
	
	Returns true, if the element was found and removed; otherwise false.
*/

/*!
	\fn CAContext::sheet()
	Returns the pointer to CASheet which this context belongs to.
	
	\sa setSheet(), _sheet
*/

/*!
	\fn CAContext::setSheet(CASheet *sheet)
	Sets the context's parent sheet to \a sheet.
	
	\sa sheet(), _sheet
*/
