/*!
	Copyright (c) 2006-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLESTAFF_H_
#define DRAWABLESTAFF_H_

#include "drawable/drawablecontext.h"
#include "core/staff.h"

class CANote;
class CAClef;
class CAKeySignature;
class CATimeSignature;
class CADrawableClef;
class CADrawableKeySignature;
class CADrawableTimeSignature;

class CADrawableStaff : public CADrawableContext {
public:
	CADrawableStaff(CAStaff* staff, double x, double y);
	void draw(QPainter *, const CADrawSettings s);
	CADrawableStaff *clone();
	inline CAStaff *staff() { return static_cast<CAStaff*>(_context); }

	inline double lineSpace() { return (staff()->numberOfLines()?height()/(staff()->numberOfLines()-1):0); }

	double calculateCenterYCoord(int pitch, CAClef *clef);
	double calculateCenterYCoord(CANote *note, CAClef *clef);
	double calculateCenterYCoord(CANote *note, double x);
	double calculateCenterYCoord(int pitch, double x);
	double calculateCenterYCoord(double y);

	int calculatePitch(double x, double y);

	void addClef(CADrawableClef *clef);
	void addKeySignature(CADrawableKeySignature *keySig);
	void addTimeSignature(CADrawableTimeSignature *keySig);

	bool removeClef(CADrawableClef *clef);
	bool removeKeySignature(CADrawableKeySignature *keySig);
	bool removeTimeSignature(CADrawableTimeSignature *keySig);

	CAClef *getClef(double x);
	CAKeySignature *getKeySignature(double x);
	CATimeSignature *getTimeSignature(double x);

	int getAccs(double x, int pitch);
	void addMElement(CADrawableMusElement *elt);
	int removeMElement(CADrawableMusElement* elt);

private:
	QList<CADrawableClef *> _drawableClefList;                   // List of all the drawable clefs. Used for fast look-up with the given key - X-coordinate usually.
	QList<CADrawableKeySignature *> _drawableKeySignatureList;   // List of all the drawable key signatures. Used for fast look-up with the given key - X-coordinate usually.
	QList<CADrawableTimeSignature *> _drawableTimeSignatureList; // List of all the drawable time signatures. Used for fast look-up with the given key - X-coordinate usually.
	static const double STAFFLINE_WIDTH;                         // Width of the staffs' lines. Defined in drawablestaff.cpp
};

#endif /* DRAWABLESTAFF_H_ */
