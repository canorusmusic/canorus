/*!
	Copyright (c) 2006, Reinhard Katzmann, Canorus development team
	              2007, Matevž Jekovec, Canorus development team
	
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/muselementfactory.h"
#include "core/functionmarkingcontext.h"
#include "core/sheet.h"
#include "core/text.h"
#include "core/dynamic.h"
#include "core/articulation.h"

/*!
	\class CAMusElementFactory
	\brief creation, removal, configuration of music elements
	
	This class is used when creating a new music element, settings its values, but not having it
	placed inside the score (eg. music element still in the GUI, but user didn't clicked on the score
	to place it yet).
	
	The factory contains the methods to set properties of all the available music elements ranging from
	playable elements to slurs and lyrics. The actual music element is created and added to the scene
	when one of the configure functions are called. These functions return True, if the element was
	successfully created or False otherwise.
	
	This class should be typically used in the following order:
	  1) User selects a music element to place. CAMusElementFactory::setMusElementType() is called.
	  2) GUI for music element properties is updated according to the current factory values.
	  3) User sets music element properties he wants (eg. number of accidentals in key signature).
	     CAMusElementFactory::setSomeMusicElementProperty( music element property value ) is called
	     (eg. CAMusElementFactory::setKeySigNumberOfAccs(3) for A-Major/fis-minor).
	  4) User places the music element. CAMusElementFactory::configureSomeMusicElement()
	     (eg. CAMusElementFactory::configureKeySignature(currentStaff, prevElement)).
	  5) If the configure function returns True, the GUI is refreshed and the note is drawn.
	
	Changing the properties of already placed elements (eg. in edit mode) is done without using the factory.
	
	\sa CAMainWin, CAMusElement
*/


/*!
	Creates an empty, defeault music elements factory.
*/
CAMusElementFactory::CAMusElementFactory() {
	_ePlayableLength = CAPlayable::Quarter;
	_eNoteStemDirection = CANote::StemPreferred;
	_iPlayableDotted = 0;
	_eRestType = CARest::Normal;
	_iTimeSigBeats = 4;
	_iTimeSigBeat = 4;
	_iKeySigNumberOfAccs = 0;
	_eKeySigGender = CAKeySignature::Major;
	_eClef = CAClef::Treble;
	_iClefOffset = 0;
	_iNoteAccs = 0;
	_iNoteExtraAccs = 0;
	_eBarlineType = CABarline::Single;
	_eSlurType = CASlur::TieType;
	_slurStyle = CASlur::SlurSolid;

	_fmFunction = CAFunctionMarking::T; // Name of the function
	_fmChordArea = CAFunctionMarking::Undefined; // Chord area of the function
	_fmTonicDegree = CAFunctionMarking::T; // Tonic degree of the function
	_fmFunctionMinor = false;
	_fmChordAreaMinor = false;
	_fmTonicDegreeMinor = false;
	_fmEllipse = false;
	mpoMusElement = 0;
	_musElementType = CAMusElement::Undefined;
	
	_dynamicText = "mf";
	_dynamicVolume = 80;
}

/*!
	Destroys the music elements factory.
*/
CAMusElementFactory::~CAMusElementFactory() {
}

/*!
	Removes the current music element.
	Destroys the music element, if \a bReallyRemove is true (default is false).
*/
void CAMusElementFactory::removeMusElem( bool bReallyRemove /* = false */ ) {
	if( mpoMusElement && bReallyRemove )
		delete mpoMusElement;
	
	mpoMusElement = 0;
}

/*!
	Configures a new clef music element in \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureClef( CAStaff *staff, 
                                         CAMusElement *right )
{
	bool success = false;
	if ( staff && staff->voiceCount() ) {		
		mpoMusElement = new CAClef( _eClef, staff, 0, _iClefOffset );
		success = staff->voiceAt(0)->insert( right, mpoMusElement );
		if (!success)
			removeMusElem( true );
	}
	return success;
}

/*!
	Configures a new key signature music element with \a iKeySignature accidentals, \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureKeySignature( CAStaff *staff, 
                                                 CAMusElement *right )
{
	bool success = false;
	if ( staff && staff->voiceCount() ) {		
		mpoMusElement = new CAKeySignature(CAKeySignature::MajorMinor, 
			                           _iKeySigNumberOfAccs,
			                           _eKeySigGender, staff,
			                           0);
		success = staff->voiceAt(0)->insert( right, mpoMusElement );
		if (!success)
			removeMusElem( true );
	}
	return success;
}

/*!
	Configures a new time signature music element with \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureTimeSignature( CAStaff *staff, 
                                                  CAMusElement *right )
{
	bool success = false;
	if ( staff && staff->voiceCount() ) {		
		mpoMusElement = new CATimeSignature( _iTimeSigBeats, _iTimeSigBeat,
			                             staff,
			                             0);
		success = staff->voiceAt(0)->insert( right, mpoMusElement );
		if (!success)
			removeMusElem( true );
	}
	return success;
}

/*!
	Configures a new barline with \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureBarline( CAStaff *staff, 
                                            CAMusElement *right )
{
	bool success = false;
	if ( staff && staff->voiceCount() ) {		
		mpoMusElement = new CABarline( _eBarlineType,
			                             staff,
			                             0);
		success = staff->voiceAt(0)->insert( right, mpoMusElement );
		if (!success)
			removeMusElem( true );
	}
	return success;
}

/*!
	Configures new note music element in the \a voice before element \a right.
	If \a addToChord is true and right is note, the element is added to the note instead of inserted.
*/
bool CAMusElementFactory::configureNote( int pitch,
                                         CAVoice *voice,
                                         CAMusElement *right,
                                         bool addToChord )
{
	bool bSuccess = false;
	removeMusElem();
	
	if ( right && addToChord ) {
		mpoMusElement = new CANote( static_cast<CANote*>(right)->playableLength(),
                                    voice,
                                    pitch,
	                                _iNoteAccs,
	                                0, // timeStart is set when inserting to voice
	                                0  // dotted is set when inserting to voice
	   );
		
		bSuccess = voice->insert( right, mpoMusElement, true );
	} else {
		mpoMusElement = new CANote( _ePlayableLength,
		                            voice,
		                            pitch,
		                            _iNoteAccs,
		                            0,
		                            _iPlayableDotted
		);
		
		// add an empty syllable or reposit syllables
		static_cast<CANote*>(mpoMusElement)->setStemDirection( _eNoteStemDirection );
		bSuccess = voice->insert( right, mpoMusElement, false );
		
		// adds empty syllables, if syllable below the note doesn't exist or repositions the syllables, if it exists
		if (voice->lastNote()==mpoMusElement) {
			for (int i=0; i<voice->lyricsContextList().size(); i++) {
				voice->lyricsContextList().at(i)->repositSyllables(); // adds an empty syllable or assigns the already placed at the end if it exists
			}
			for (int i=0; i<voice->staff()->sheet()->contextCount(); i++) {
				if (voice->staff()->sheet()->contextAt(i)->contextType()==CAContext::FunctionMarkingContext)
					static_cast<CAFunctionMarkingContext*>(voice->staff()->sheet()->contextAt(i))->repositFunctions();
			}
		} else {
			for (int i=0; i<voice->lyricsContextList().size(); i++) {
				voice->lyricsContextList().at(i)->addEmptySyllable(
					mpoMusElement->timeStart(), mpoMusElement->timeLength()
				);
			}
			for (int i=0; i<voice->staff()->sheet()->contextCount(); i++) {
				if (voice->staff()->sheet()->contextAt(i)->contextType()==CAContext::FunctionMarkingContext)
					static_cast<CAFunctionMarkingContext*>(voice->staff()->sheet()->contextAt(i))->addEmptyFunction(
						mpoMusElement->timeStart(), mpoMusElement->timeLength()
					);
			}
		}

	}
	
	if (bSuccess) {
		static_cast<CANote*>(mpoMusElement)->updateTies();
	}
	else
		removeMusElem( true );
	
	return bSuccess;
}

/*!
	Configures the new tie, slur or phrasing slur for the notes \a noteStart and \a noteEnd.
*/
bool CAMusElementFactory::configureSlur( CAStaff *staff,
                                         CANote *noteStart, CANote *noteEnd )
{
	bool success=false;
	removeMusElem();
	CASlur *slur = new CASlur( slurType(), CASlur::SlurPreferred, staff, noteStart, noteEnd );
	mpoMusElement = slur;
	
	slur->setSlurStyle( slurStyle() );
	switch (slurType()) {
		case CASlur::TieType:
			noteStart->setTieStart( slur );
			if (noteEnd) noteEnd->setTieEnd( slur );
			success=true;
			break;
		case CASlur::SlurType:
			noteStart->setSlurStart( slur );
			if (noteEnd) noteEnd->setSlurEnd( slur );
			success=true;
			break;
		case CASlur::PhrasingSlurType:
			noteStart->setPhrasingSlurStart( slur );
			if (noteEnd) noteEnd->setPhrasingSlurEnd( slur );
			success=true;
			break;
	}
	
	if (!success)
		removeMusElem( true );
	
	return success;
}

/*!
	Configures the new mark and adds it to the \a elt.
*/
bool CAMusElementFactory::configureMark( CAMusElement *elt ) {
	bool success = false;
	
	switch ( markType() ) {
	case CAMark::Dynamic: {
		if ( elt->musElementType()==CAMusElement::Note ) {
			mpoMusElement = new CADynamic( dynamicText(), dynamicVolume(), static_cast<CANote*>(elt) );
			success = true;
		}
		break;
	}
	case CAMark::Text: {
		mpoMusElement = new CAText( "test", elt );
		success = true;
		break;
	}
	case CAMark::Articulation: {
		if ( elt->musElementType()==CAMusElement::Note ) {
			mpoMusElement = new CAArticulation( articulationType(), static_cast<CANote*>(elt) );
			success = true;
		}
		break;
	}
	}
	
	if (success) {
		elt->addMark( static_cast<CAMark*>(mpoMusElement) );
		return true;
	}
	
	return false;
}

/*!
	Configures a new rest music element in voice \a voice before element \a right.
*/
bool CAMusElementFactory::configureRest( CAVoice *voice, CAMusElement *right ) {
	bool success = false;
	if ( voice ) {
		mpoMusElement = new CARest(restType(),
				_ePlayableLength,
				voice,
				0,
				_iPlayableDotted
				);
		success = voice->insert( right, mpoMusElement );
		if (!success)
			removeMusElem(true);
	}
	return success;
}

/*!
	Configures a new function marking with \a timeStart and \a timeLength in context \a fmc.
*/
bool CAMusElementFactory::configureFunctionMarking( CAFunctionMarkingContext *fmc, int timeStart, int timeLength ) {
	CAFunctionMarking *fm = new CAFunctionMarking(
		fmFunction(), isFMFunctionMinor(),
		CAKeySignature::keySignatureToString( _iKeySigNumberOfAccs, _eKeySigGender ),
		fmc, timeStart, timeLength,
		fmChordArea(), isFMChordAreaMinor(),
		fmTonicDegree(), isFMTonicDegreeMinor(),
		"", /// \todo Function marking altered/added degrees
		isFMEllipse()
	);
	
	fmc->addFunctionMarking(fm);
	mpoMusElement = fm;
	
	return true;
}

/*!
	\fn CAMusElementFactory::musElement()
	Reads the current music element and returns its pointer.
*/

/*!
	\fn CAMusElementFactory::musElementType()
	Returns the current music element type.
*/

/*!
	\fn CAMusElementFactory::setMusElementType(CAMusElement::CAMusElementType eMEType)
	Sets the new current music element type \a eMEType, does not create a new element of this type!
*/

/*!
	\var CAMusElementFactory::mpoMusElement
	Newly created music element itself.

	\sa musElement()
*/

/*!
	\var CAMusElementFactory::_ePlayableLength
	Length of note/rest to be added.
*/

/*!
	\var CAMusElementFactory::_iPlayableDotted
	Number of dots to be inserted for the note/rest.
*/

/*!
	\var CAMusElementFactory::_iNoteExtraAccs
	Extra note accidentals for new notes which user adds/removes with +/- keys.
*/

/*!
	\var CAMusElementFactory::_iNoteAccs
	Note accidentals at specific coordinates updated regularily when in insert mode.
*/

/*!
	\var CAMusElementFactory::_iTimeSigBeats
	Time signature number of beats to be inserted.
*/

/*!
	\var CAMusElementFactory::_iTimeSigBeat
	Time signature beat to be inserted.
*/

/*!
	\var CAMusElementFactory::_eClef
	Type of the clef to be inserted.
*/
