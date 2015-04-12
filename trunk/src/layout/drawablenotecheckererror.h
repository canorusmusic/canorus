/*!
	Copyright (c) 2015, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLENOTECHECKERERROR_H_
#define DRAWABLENOTECHECKERERROR_H_

#include "layout/drawable.h"

class CANoteCheckerError;

class CADrawableNoteCheckerError : public CADrawable {
public:
	CADrawableNoteCheckerError(CANoteCheckerError *nce, CADrawable *dTarget);
	void draw(QPainter *p, const CADrawSettings s);
	CADrawable *clone();
	
private:
	CANoteCheckerError *_noteCheckerError;
};

#endif /* DRAWABLECONTEXT_H_ */
