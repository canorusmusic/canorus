/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/rest.h"
#include "core/staff.h"

/*!
	\class CARest
	\brief Represents a rest in the score.

	This class represents every rest in the score. It inherits the base class CAPlayable.
*/

/*!
	Creates a new rest with given \a type, playable length \a length in voice \a voice with starting time in the score \a timeStart and number of dots \a dotted.
	timeLength is calculated automatically from the playable length.

	\sa CARestType, CAPlayableLength, CAPlayable, CAVoice
*/
CARest::CARest(CARestType type, CAPlayableLength length, CAVoice *voice, int timeStart, int dotted)
 : CAPlayable(length, voice, timeStart, dotted) {
 	_musElementType = CAMusElement::Rest;
 	_restType = type;
}

/*!
	Destroys the rest.
*/
CARest::~CARest() {
}

CARest *CARest::clone( CAVoice *voice ) {
	return new CARest(_restType, _playableLength, voice, _timeStart, _dotted);
}

int CARest::compare(CAMusElement *elt) {
	if (elt->musElementType()!=CAMusElement::Rest)
		return -1;

	int diffs=0;
	if (_playableLength!=((CAPlayable*)elt)->playableLength()) diffs++;
	if (timeLength()!=((CAPlayable*)elt)->timeLength()) diffs++;

	return diffs;
}

/*!
	Converts rest type CARestType to QString.
	This is usually used when saving the score.

	\sa CARestType, CACanorusML
*/
const QString CARest::restTypeToString(CARestType type) {
	switch (type) {
		case Normal: return "normal";
		case Hidden: return "hidden";
		default: return "";
	}
}

/*!
	Converts rest type from QString to CARestType.
	This is usually used when loading the score.

	\sa CARestType, CACanorusML
*/
CARest::CARestType CARest::restTypeFromString(const QString type) {
	if (type=="hidden") {
		return Hidden;
	} else
		return Normal;
}

/*!
	Generates a list of new rests in the total length of \a timeLength.
	Rests are sorted from the shortes to the longest one.
	The first rest has the given \a timeStart.
	Passing voice and restType is optional.
	
	This function is usually called when a gap between two voices with shared elements
	appear in one voice and the gap with custom length needs to be filled with rests.
	
	\note Only non-dotted rests are generated.
*/
QList<CARest*> CARest::composeRests(int timeLength, int timeStart, CAVoice* voice, CARestType type ) {
	QList<CARest*> list;
	
	// 2048 is the longest rest (breve)
	for ( ; timeLength > 2048; timeLength-= 2048, timeStart+=2048 )
		list << new CARest( type, static_cast<CAPlayableLength>(0), voice, timeStart );
	
	for ( int i = 0, TL=2048; i<256; (i?i*=2:i++), TL/=2 ) {
		if ( TL <= timeLength) {
			list.prepend( new CARest( type, static_cast<CAPlayableLength>( i ), voice, timeStart ) );
			timeLength -= TL;
			timeStart += TL;
		}
	}
	
	return list;
}

/*!
	\var CARest::_restType
	Type of the rest.

	\sa CARestType, restType()
*/
