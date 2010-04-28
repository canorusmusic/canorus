/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEMIDINOTE_H_
#define DRAWABLEMIDINOTE_H_

#include "layout/drawablemuselement.h"

class CAMidiNote;
class CADrawableStaff;

class CADrawableMidiNote: public CADrawableMusElement {
public:
	CADrawableMidiNote( CAMidiNote *midiNote, CADrawableStaff* c );
	virtual ~CADrawableMidiNote();
};

#endif /* DRAWABLEMIDINOTE_H_ */
