/** @file context.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <QString>

class CASheet;
class CAMusElement;

class CAContext {
	public:
		CAContext(CASheet *s, const QString name);
		
		/**
		 * enum CAContextType holds different CAContext types:
		 * - Staff - Every context with lines which includes various music elements.
		 * - Tablature - Context similar to Staff, but specialized for guitar.
		 * - Lyrics - Holds words (syllables) for choir music. Every syllable is assigned to certain CANote.
		 * - Dynamics - Sometimes we want to have a separate context for the crescendo, rit., mf and other markings
		 */
		enum CAContextType {
			Staff,
			Tablature,
			Lyrics,
			Dynamics
		};
		
		const QString name() { return _name; } 

		/**
		 * Set the Context's name.
		 * 
		 * @param name Context's name in QString format
		 */
		void setName(const QString name) { _name = name; }
		virtual void clear() = 0;
		CAContextType contextType() { return _contextType; }
		
		/**
		 * Find the next music element to the given one and return its pointer.
		 * 
		 * @param elt Pointer to the music element which we seek its right neighbour.
		 * @return Pointer to the next element, null if the element doesn't have its right neighbour.
		 */
		virtual CAMusElement *findNextMusElement(CAMusElement *elt) = 0;

		/**
		 * Find the previous music element to the given one and return its pointer.
		 * 
		 * @param elt Pointer to the music element which we seek its left neighbour.
		 * @return Pointer to the previous element, null if the element doesn't have its left neighbour.
		 */
		virtual CAMusElement *findPrevMusElement(CAMusElement *elt) = 0;
		
		/**
		 * Remove a music element from the context.
		 * 
		 * @param elt Pointer to the element to be removed.
		 * @param cleanup Should the memory be freed as well or just remove a reference.
		 * @return True, if element was found and removed, False otherwise.
		 */
		virtual bool removeMusElement(CAMusElement *elt, bool cleanup = true) = 0;
		
		/**
		 * Return the CASheet which this Context belongs to.
		 * 
		 * @return Pointer to the CASheet which this Context belongs to.
		 */
		CASheet *sheet() { return _sheet; }
		
	protected:
		CASheet *_sheet;
		QString _name;
		CAContextType _contextType;
};

#endif /*CONTEXT_H_*/
