/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPen>
#include <QPainter>
#include <QBrush>

#include "layout/drawablemidinote.h"
#include "layout/drawablestaff.h"
#include "score/midinote.h"

CADrawableMidiNote::CADrawableMidiNote( CAMidiNote *midiNote, CADrawableStaff* c, double x, double y )
 : CADrawableMusElement( midiNote, c, x, y) {
	setDrawableMusElementType( DrawableMidiNote );
	setNeededSpaceWidth( midiNote->timeLength()/8.0 );
	setNeededSpaceHeight( c->lineSpace() );
}

CADrawableMidiNote::~CADrawableMidiNote() {
}

void CADrawableMidiNote::draw(QPainter *p, CADrawSettings s) {
	QBrush brush(s.color);
	p->fillRect( s.x, s.y, qRound(neededSpaceWidth()*s.z), qRound(neededSpaceHeight()*s.z), brush );
}

CADrawableMidiNote* CADrawableMidiNote::clone(CADrawableContext* newContext) {
	return new CADrawableMidiNote(static_cast<CAMidiNote*>(_musElement), static_cast<CADrawableStaff*>((newContext)?newContext:_drawableContext), _xPos, _yPos);
}
