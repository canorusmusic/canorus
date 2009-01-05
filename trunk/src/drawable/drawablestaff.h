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
	CADrawableStaff(CAStaff* staff, int x, int y);
	void draw(QPainter *, const CADrawSettings s);
	CADrawableStaff *clone();
	inline CAStaff *staff() { return (CAStaff*)_context; }
	
	inline float lineSpaceF() { return (staff()->numberOfLines()?(float)_height/(staff()->numberOfLines()-1):0); }
	inline float lineSpace() { return qRound(lineSpaceF()); }
	
	int calculateCenterYCoord(int pitch, CAClef *clef);
	int calculateCenterYCoord(CANote *note, CAClef *clef);
	int calculateCenterYCoord(CANote *note, int x);
	int calculateCenterYCoord(int pitch, int x);
	int calculateCenterYCoord(int y);
	
	int calculatePitch(int x, int y);
	
	void addClef(CADrawableClef *clef);
	void addKeySignature(CADrawableKeySignature *keySig);
	void addTimeSignature(CADrawableTimeSignature *keySig);
	
	bool removeClef(CADrawableClef *clef);
	bool removeKeySignature(CADrawableKeySignature *keySig);
	bool removeTimeSignature(CADrawableTimeSignature *keySig);
	
	CAClef *getClef(int x);
	CAKeySignature *getKeySignature(int x);
	CATimeSignature *getTimeSignature(int x);
	
	int getAccs(int x, int pitch);
	void addMElement(CADrawableMusElement *elt);
	int removeMElement(CADrawableMusElement* elt);
	
private:
	QList<CADrawableClef *> _drawableClefList;                   // List of all the drawable clefs. Used for fast look-up with the given key - X-coordinate usually.
	QList<CADrawableKeySignature *> _drawableKeySignatureList;   // List of all the drawable key signatures. Used for fast look-up with the given key - X-coordinate usually.
	QList<CADrawableTimeSignature *> _drawableTimeSignatureList; // List of all the drawable time signatures. Used for fast look-up with the given key - X-coordinate usually.
	static const float STAFFLINE_WIDTH;                          // Width of the staffs' lines. Defined in drawablestaff.cpp
};

#endif /* DRAWABLESTAFF_H_ */
