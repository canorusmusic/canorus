/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/playable.h"
#include "core/voice.h"
#include "core/staff.h"

/*!
	\class CAPlayable
	\brief Playable instances of music elements.
	
	CAPlayable class represents a base class for all the music elements which are
	playable (timeLength property is greater than 0). It also adds other properties
	like the music length (whole, half, quarter etc.), number of dots and instead
	of contexts, playable elements voices for their parent objects.
	
	Notes and rests inherit this class.
	
	\sa CAMusElement, CAPlayableLength
*/

/*!
	Creates a new playable element with playable length \a length, \a voice, \a timeStart
	and number of dots \a dotted.
	
	\sa CAPlayableLength, CAVoice, CAMusElement
*/
CAPlayable::CAPlayable( CAPlayableLength length, CAVoice *voice, int timeStart )
 : CAMusElement(voice?voice->staff():0, timeStart, 0) {
	setVoice( voice );
	setPlayableLength( length );	
}

/*!
	Destroys the playable element.
	
	The element is removed from any voice, if part of.
	
	\note Non-playable signs are not shifted back when removing the element from the voice. 
*/
CAPlayable::~CAPlayable() {
	if (voice())
		voice()->remove( this, false );
}

void CAPlayable::setVoice(CAVoice *voice) {
	_voice = voice; _context = voice?voice->staff():0;
}

/*!
	Calculates the new timeLength depending on the playableLength and dotted properties.
	
	This function is usually automatically called when changing these properties.
	
	\sa playableLength()
*/
void CAPlayable::calculateTimeLength() {
	setTimeLength( CAPlayableLength::playableLengthToTimeLength(playableLength()) );
}
