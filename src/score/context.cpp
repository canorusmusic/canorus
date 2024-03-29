/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/context.h"

/*!
	\class CAContext
	\brief Line of music elements in the sheet

	CAContext represents usually an infinite graphical space on the sheet where music elements are
	placed in. This idea was introduced by LilyPond and it turned out to be quite useful.

	Contexts can be staffs, lyrics, figured bass context, function mark context, dynamics etc.

	CAContext is an abstract class and different solutions should be done based on it.

	\sa CAStaff, CAFunctionMarkContext
*/

/*!
	Creates a context named \a name and with parent sheet \a s.
*/
CAContext::CAContext(const QString name, CASheet* s)
{
    _sheet = s;
    _name = name;
}

CAContext::~CAContext()
{
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
			Sometimes we want to have a separate context for the crescendo, rit., mf and other marks
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

/*!
	\fn CAContext::clone( CASheet *s )
	Clones the current context with the given parent sheet \a s.

	\sa CAMusElement::clone(), CADocument::clone()
*/

/*!
    \fn CAContext::insertEmptyElement(int timeStart)
    Inserts an empty dependent element (syllable, chord name, figured bass mark, function mark) to the context.
    After the call the elements need to be repositioned manually (subsequent timeStarts and timeLengths will be out of place).
    This function is usually called when initializing the dependent context or inserting a new note.

    \sa CAContext::repositionElements()
*/

/*!
    \fn CAContext::repositionElements()
    Repositions the existing dependent elements (syllables, chord names, figured bass marks, function marks) by setting timeStart and timeLength
    one by one according to the playable music it depends on. The order is preserved.

    \sa CAContext::insertEmptyElement(int)
*/
