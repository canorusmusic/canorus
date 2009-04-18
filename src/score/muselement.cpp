/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/muselement.h"
#include "score/context.h"
#include "score/staff.h"
#include "score/playable.h"
#include "score/mark.h"
#include "score/articulation.h"

/*!
	\class CAMusElement
	\brief An abstract class which represents every music element in the score.

	This class is a base class for every music element in the score.
	Music elements can be of various types, note, rest, barline, clef, lyrics syllable,
	function mark, figured bass mark etc.
	See CAMusElementType for details.

	Every music element belongs to a so called parent area in the score called context.
	See CAContext for details.

	Since Canorus tends to be built in Model-View-Controller style, every music element
	has one or more of its drawable instances. These classes are named CADrawableClassName,
	where ClassName is type of the music element. eg. CADrawableClef, CADrawableBarline etc.

	\sa CAMusElementType, CAContext, CADrawableMusElement
*/

/*!
	Constructs a music element with parent context (staff, lyrics, functionmarks) \a context,
	start time \a time and length \a length.
*/
CAMusElement::CAMusElement(CAContext *context, int time, int length) {
	_context = context;
	_timeStart = time;
	_timeLength = length;
	_musElementType = CAMusElement::Undefined;
	_visible = true;
	_color = QColor( 0, 0, 0, 0 );
}

/*!
	Destroys a music element.
	This removes the music element from the parent context as well!
*/
CAMusElement::~CAMusElement() {
	while(!_markList.isEmpty()) {
		if ( !_markList.first()->isCommon() ||
		     musElementType()!=CAMusElement::Note ) {
			delete _markList.takeFirst();
		} else {
			_markList.takeFirst();
		}
	}

	// needed when removing a shared-voice music element - when an instance is removed, it should be removed from all the voices as well! -Matevz
	if( context() && !isPlayable() )
		context()->remove( this );
}

/*!
	Returns true, if the current element is playable; otherwise false.
	Playable elements are music elements with _timeLength variable greater
	than 0 (notes, rests). They inherit CAPlayable.

	\sa _timeLength, CAPlayable
*/
bool CAMusElement::isPlayable()
{
	return (musElementType()==Note || musElementType()==Rest);//dynamic_cast<CAPlayable*>(this);
}

/*!
	Converts a music element \a type to QString.

	\sa CAMusElementType, musElementTypeFromString()
*/
const QString CAMusElement::musElementTypeToString(CAMusElement::CAMusElementType type) {
	switch ( type ) {
		case (Undefined): return "undefined"; break;
		case (Note): return "note"; break;
		case (Rest): return "rest"; break;
		case (Barline): return "barline"; break;
		case (Clef): return "clef"; break;
		case (TimeSignature): return "time-signature"; break;
		case (KeySignature): return "key-signature"; break;
		case (Slur): return "slur"; break;
		case (FunctionMark): return "function-mark"; break;
		case (Syllable): return "syllable"; break;
	}
}

/*!
	Converts QString \a type to music element type.

	\sa CAMusElementType, musElementTypeToString()
*/
CAMusElement::CAMusElementType CAMusElement::musElementTypeFromString(const QString type) {
	if ( type=="undefined" ) return Undefined;
	if ( type=="note" ) return Note;
	if ( type=="rest" ) return Rest;
	if ( type=="barline" ) return Barline;
	if ( type=="clef" ) return Clef;
	if ( type=="time-signature" ) return TimeSignature;
	if ( type=="key-signature" ) return KeySignature;
	if ( type=="slur" ) return Slur;
	if ( type=="function-mark" ) return FunctionMark;
	if ( type=="syllable" ) return Syllable;
}

/*!
	Adds a \a mark to the mark list in correct order.
*/
void CAMusElement::addMark( CAMark *mark ) {
	if ( !mark || _markList.contains(mark) )
		return;

	int l;
	for ( l=0; l<markList().size() && mark->markType() < markList()[l]->markType(); l++ ); // marks should be sorted by their mark type
	if ( mark->markType()==CAMark::Articulation ) {
		for ( ; l<markList().size() &&
		        markList()[l]->markType()==CAMark::Articulation &&
		        static_cast<CAArticulation*>(mark)->articulationType() < static_cast<CAArticulation*>(markList()[l])->articulationType(); l++ ); // marks should be sorted by their mark type
	}

	_markList.insert( l, mark );
}

/*!
	Adds a list of marks to the mark list in correct order.
*/
void CAMusElement::addMarks( QList<CAMark*> marks ) {
	for (int i=0; i<marks.size(); i++)
		addMark( marks[i] );
}

/*!
	\enum CAMusElement::CAMusElementType

	Includes different types for describing the CAMusElement:
		- Note - A music element which represents CANote.
		- NoteBracket - A music element which represents CANoteBracket (the bracket which connects the stems).
		- Chord - A virtual music element which represents CAChord.
		- Rest - A music element which represents CARest.
		- BarLine - A music elemnet which represents CABarLine.
		- Clef - A music element which represents CAClef.
		- TimeSignature - A music element which represents CATimeSignature.
		- KeySignature - A music element which represents CAKeySignature.
		- Slur - A music element which represents CASlur.
		- Tie - A music element which represents CATie.
		- PhrazingSlur - A music element which represents CAPhrazingSlur.
		- ExpressionMark - A music element which represents any technical text marks about how the score should be played - CAExpressionMark (eg. Legato)
		- VolumeSign - A music element which represents any volue sign (forte, piano etc.).
		- Text - A music element which represents any text notes and authors additions to the score. (eg. These 3 measures still need to be fixed)

	\sa musElementType()
*/

/*!
	\fn CAMusElement::musElementType()
	Returns the music element type.

	\sa CAMusElementType
*/

/*!
	\fn CAMusElement::context()
	Returns pointer to the CAContext which the music element belongs to.

	\sa CAContext
*/

/*!
	\fn CAMusElement::timeStart()
	Returns the time in the score when the music element appears in time.
	The returned time is in absolute time units.

	\sa _timeStart, setTimeStart()
*/

/*!
 	\fn CAMusElement::setTimeStart(int time)
	Sets the time in the score when the music element appears for this music element to \a time.
	The given time is in absolute time units.

	\sa _timeStart, timeStart()
*/

/*!
	\fn CAMusElement::timeLength()
	Returns the time how long the music element lasts in the score.
	The returned time is in absolute time units.

	\sa _timeLength, setTimeLength(), timeEnd()
*/

/*!
	\fn CAMusElement::setTimeLength(int length)
	Sets the length in the score for this music element to \a time.
	The given time is in absolute time units.

	\sa _timeLength, timeLength()
*/

/*!
	\fn CAMusElement::timeEnd()
	Returns the time when the music element stops playing.
	This is always the sum of _timeStart + _timeLength.
	The returned time is in absolute time units.

	\sa _timeStart, _timeLength
*/

/*!
	\fn CAMusElement::name()
	Returns the name of the music element.
*/

/*!
	\fn CAMusElement::setName(QString name)
	Sets the name of the music element to \a name.

	\sa _name, name()
*/

/*!
	\fn CAMusElement::clone()
	Clones a music element with exact properties including the context.
*/

/*!
	\fn CAMusElement::compare(CAMusElement *elt)
	Compares the music element with the given \a elt and returns number of
	differences in their properties.
	Returns 0, if the music elements are exact; -1 if the music element type differs;
	otherwise number greater than 0.

	This method is usually used when opening a score document where music elements are
	written in various voices (eg. barlines), but are eventually merged and written only
	once per staff.
*/

/*!
	\var CAMusElement::_musElementType
	Stores the type of the music element.

	\sa CAMusElementType
*/

/*!
	\var CAMusElement::_context
	Pointer to the context which the music element belongs to.

	\sa context()
*/

/*!
	\var CAMusElement::_timeStart
	Where does the music element starts in time.
	Time is stored in absolute time units and is not affected by different tempos or
	other expressions.

	\sa timeStart(), setTimeStart()
*/

/*!
	\var CAMusElement::_timeLength
	How long does this music element lasts.
	Time is stored in absolute time units and is not affected by different tempos or
	other expressions.
	Non-playable elements (barlines, clefs, key signatures etc.) have this time always 0.
	Playable elements (notes, rests) have this time always greater than 0.

	\sa timeLength(), setTimeLength(), CAPlayable::CAPlayableLength
*/

/*!
	\var CAMusElement::_name
	Specific name of the music element in QString.
	Names are optional and are not necessary unique.

	\sa name(), setName()
*/
