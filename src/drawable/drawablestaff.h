/** @file drawablestaff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLESTAFF_H_
#define DRAWABLESTAFF_H_

#include "drawablecontext.h"

class CAStaff;
class CANote;
class CAClef;
class CADrawableClef;

class CADrawableStaff : public CADrawableContext {
	public:
		CADrawableStaff(CAStaff* staff, int x, int y);
		void draw(QPainter *, const CADrawSettings s);
		CADrawableStaff *clone();
		inline CAStaff *staff() { return (CAStaff*)_context; }
		
		/**
		 * Return the center Y coordinate of the given note in this staff.
		 * 
		 * @param pitch Note pitch which the following coordinates are being calculated for.
		 * @param clef Corresponding clef.
		 * @return Center of a space/line of a staff in absolute world units.
		 */
		int calculateCenterYCoord(int pitch, CAClef *clef);

		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Return the center Y coordinate of the given note in this staff.
		 * 
		 * @param note Note which the following coordinates are being calculated.
		 * @param clef Corresponding clef.
		 * @return Center of a space/line of a staff in absolute world units.
		 */
		int calculateCenterYCoord(CANote *note, CAClef *clef);
		
		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Return the center Y coordinate of the given note in this staff.
		 * 
		 * @param note Note which the following coordinates are being calculated.
		 * @param x X coordinate of the note.
		 * @return Center of a space/line of a staff in absolute world units.
		 */
		int calculateCenterYCoord(CANote *note, int x);
		
		/**
		 * This is an overloaded member function, provided for convenience.
		 * 
		 * Return the center Y coordinate of the given note in this staff.
		 * 
		 * @param pitch Note pitch which the following coordinates are being calculated for.
		 * @param x X coordinate of the note.
		 * @return Center of a space/line of a staff in absolute world units.
		 */
		int calculateCenterYCoord(int pitch, int x);

		/**
		 * Round the given Y coordinate to the nearest one so it fits a line or a space (ledger lines too, if needed) in a staff.
		 * 
		 * @return Center of the nearest space/line of a staff, whichever is closer in absolute world units.
		 */
		int calculateCenterYCoord(int y);
		
		/**
		 * Calculate the note pitch on the given clef and absolute world Y coordinate.
		 * 
		 * @param x X coordinate in absolute world units.
		 * @param y Y coordinate in absolute world units.
		 * @return Note pitch in logical units. 
		 */
		int calculatePitch(int x, int y);
		
		void addClef(CADrawableClef *clef);	//add the clef to the clef list for faster search of the current clef
		bool removeClef(CADrawableClef *clef); //returns true, if clef deleted, false otherwise
		CAClef *getClef(int x);
		
	private:
		QList<CADrawableClef *> _drawableClefList;	///List of all the drawable clefs. Used for fast look-up with the given key - X-coordinate usually.
};

#endif /*DRAWABLESTAFF_H_*/
