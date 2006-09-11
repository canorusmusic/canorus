/** @file drawablestaff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLESTAFF_H_
#define DRAWABLESTAFF_H_

#include "drawable/drawablecontext.h"

class CAStaff;
class CANote;
class CAClef;
class CAKeySignature;
class CATimeSignature;
class CADrawableClef;
class CADrawableKeySignature;
class CADrawableTimeSignature;

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
		 * Calculates the highest possible Y coordinate of the given pitch in this drawable staff without ledger lines.
		 * Useful for calculating the Y-coordinate of the sharp key signatures for every sharp.
		 * 
		 * @param pitch Pitch in logical units from 0 (C) to 6 (B)
		 * @param x X coordinate of the note.
		 */
		int calculateHighestCenterYCoord(int pitch, int x);
		
		/**
		 * Calculates the lowest possible Y coordinate of the given pitch in this drawable staff without ledger lines.
		 * Useful for calculating the Y-coordinate of the sharp key signatures for every sharp.
		 * 
		 * @param pitch Pitch in logical units from 0 (C) to 6 (B)
		 * @param x X coordinate of the note.
		 */
		int calculateLowestCenterYCoord(int pitch, int x);

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
		
		void addClef(CADrawableClef *clef);	///add the clef to the clef list for faster search of the current clef
		void addKeySignature(CADrawableKeySignature *keySig);	///add the key signature to the key signature list for faster search of the current key signature
		void addTimeSignature(CADrawableTimeSignature *keySig);	///add the time signature to the time signature list for faster search of the current time signature
		bool removeClef(CADrawableClef *clef); ///return true, if clef deleted, false otherwise
		bool removeKeySignature(CADrawableKeySignature *keySig); ///return true, if clef deleted, false otherwise
		bool removeTimeSignature(CADrawableTimeSignature *keySig); ///return true, if clef deleted, false otherwise
		CAClef *getClef(int x);	///return pointer to the clef on the given X-coordinate
		CAKeySignature *getKeySignature(int x);	///return pointer to the key signature on the given X-coordinate
		CATimeSignature *getTimeSignature(int x);	///return pointer to the time signature on the given X-coordinate
		
	private:
		QList<CADrawableClef *> _drawableClefList;	///List of all the drawable clefs. Used for fast look-up with the given key - X-coordinate usually.
		QList<CADrawableKeySignature *> _drawableKeySignatureList;	///List of all the drawable key signatures. Used for fast look-up with the given key - X-coordinate usually.
		QList<CADrawableTimeSignature *> _drawableTimeSignatureList;	///List of all the drawable time signatures. Used for fast look-up with the given key - X-coordinate usually.
		static const float STAFFLINE_WIDTH = 0.8;	///Width of the staffs' lines
};

#endif /*DRAWABLESTAFF_H_*/
