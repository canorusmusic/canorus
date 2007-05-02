/*!
 * This program is free software; you can redistribute it and/or modify it   
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.                       
 *                                                                           
 * This program is distributed in the hope that it will be useful, but       
 * WITHOUT ANY WARRANTY; without even the implied warranty of               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  
 * Public License for more details.                                          
 *                                                                           
 * You should have received a copy of the GNU General Public License along   
 * with this program; (See "LICENSE.GPL"). If not, write to the Free         
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA        
 * 02111-1307, USA.                                                          
 *                                                                           
 *---------------------------------------------------------------------------
 *                                                                           
 * Copyright (c) 2006, Reinhard Katzmann, Matevž Jekovec, Canorus development team           
 * All Rights Reserved. See AUTHORS for a complete list of authors.          
 *                                                                           
 */

#include "core/muselementfactory.h"

#include "core/functionmarkingcontext.h"

// For comparing undefined music elements
#define UNDEFNUM 36720165

/*!
	Class for undefined music elements, not to be known outside of this class
*/
class CAUndefMusElem : public CAMusElement
{
public:
	CAUndefMusElem(CAContext *poContext, int iTime, int iLength=0) : CAMusElement( poContext, iTime, iLength )
	{ mpoContext = poContext; miTime = iTime; miLength = iLength; miUndefined = 0; };
	virtual ~CAUndefMusElem() { miUndefined = 0; };
	
	CAUndefMusElem* clone() 
	{ return new CAUndefMusElem( mpoContext, miTime, miLength); };
	int compare(CAMusElement *elt) 
	{
		int iRet = -1;
		if (elt->musElementType()==CAMusElement::Undefined)
			iRet = 0;
		if( ((CAUndefMusElem *)elt)->undefined() != UNDEFNUM )
			iRet = 1;
		//printf( "Undef: %d, %d\n",((CAUndefMusElem *)elt)->undefined(), miUndefined ); fflush( stdout );
		return iRet;
	};

	void setMusElementType( CAMusElement::CAMusElementType eMEType )
	{ _musElementType = eMEType; miUndefined = UNDEFNUM; };

	inline int undefined() { return miUndefined; };

private:
	CAContext *mpoContext;
	int miTime;
	int miLength;
	int miUndefined;
};

/*!
	\class CAMusElementFactory
	\brief creation, removal, configuration of music elements
	
	This class is used when creating a new music element, settings its values, but not having it
	placed inside the score (eg. music element still in the GUI, but user didn't clicked on the score
	to place it yet).
	
	The factory can contain only a single music element in "construction".
	
	\sa CAMainWin, CAMusElement
*/


/*!
	Creates an empty, defeault music elements factory.
*/
CAMusElementFactory::CAMusElementFactory()
{
	_ePlayableLength = CAPlayable::Quarter;
	_eNoteStemDirection = CANote::StemPreferred;
	_iPlayableDotted = 0;
	_eRestType = CARest::Normal;
	_iTimeSigBeats = 4;
	_iTimeSigBeat = 4;
	_iKeySigNumberOfAccs = 0;
	_eClef = CAClef::Treble;
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
	
	createMusElem();
}

/*!
	Destroys the music elements factory.
*/
CAMusElementFactory::~CAMusElementFactory()
{
	removeMusElem();
}

/*!
	Creates a placebo music element (type: undefined)
*/
CAMusElement *CAMusElementFactory::createMusElem()
{
	mpoMusElement = new CAUndefMusElem( 0, 0, 0 );
	((CAUndefMusElem *)mpoMusElement)->setMusElementType( CAMusElement::Undefined );
	return mpoMusElement;
}

/*!
	Removes the current music element.
	Destroys the music element, if \a bReallyRemove is true (default is false).
*/
void CAMusElementFactory::removeMusElem( bool bReallyRemove /* = false */ )
{
	CAUndefMusElem oUMElem( 0, 0, 0 );
	if( mpoMusElement && 
	    ( bReallyRemove || oUMElem.compare( mpoMusElement ) <= 0 ) )
		delete mpoMusElement;
	mpoMusElement = 0;
}

/*!
	Configures music element \a roMusElement by cloning it and moving it to the factory.
	Destroys the original.
*/
void CAMusElementFactory::configureMusElem( CAMusElement &roMusElement )
{
	if( mpoMusElement )
		delete mpoMusElement;
	mpoMusElement = roMusElement.clone();
}

/*!
	Configures a new clef music element in \a context and right next to the \a left element.
*/
bool CAMusElementFactory::configureClef( CAStaff *staff, 
                                         CAMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if ( staff ) {
		mpoMusElement = new CAClef(_eClef, staff, (left?left->timeEnd():0));
		bSuccess = staff->insertSignAfter(mpoMusElement, left, true);
	}
	return bSuccess;
}

/*!
	Configures a new key signature music element with \a iKeySignature accidentals, \a context and right next to the \a left element.
*/
bool CAMusElementFactory::configureKeySignature( CAStaff *staff, 
                                                 CAMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if (staff) {		
		mpoMusElement = new CAKeySignature(CAKeySignature::MajorMinor, 
			                           _iKeySigNumberOfAccs,
			                           CAKeySignature::Major, staff,
			                           (left?left->timeEnd():0));
		bSuccess = staff->insertSignAfter(mpoMusElement, left, true);
	}
	return bSuccess;
}

/*!
	Configures a new time signature music element with \a context and right next to the \a left element.
*/
bool CAMusElementFactory::configureTimeSignature( CAStaff *staff, 
                                                  CAMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if (staff) {
		mpoMusElement = new CATimeSignature( _iTimeSigBeats, _iTimeSigBeat,
			                             staff,
			                             (left?left->timeEnd():0));
		bSuccess = staff->insertSignAfter(mpoMusElement, left, true);
	}
	return bSuccess;
}

/*!
	Configures a new barline with \a context and right next to the \a left element.
*/
bool CAMusElementFactory::configureBarline( CAStaff *staff, 
                                            CAMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if (staff) {
		mpoMusElement = new CABarline( _eBarlineType,
			                             staff,
			                             (left?left->timeEnd():0));
		bSuccess = staff->insertSignAfter(mpoMusElement, left, true);
	}
	return bSuccess;
}

/*!
	Configures new note music element.
	
	\param iVoiceNum        voice number where new note is inserted
	\param voice            voice where new note is inserted
	\param coords           mouse position where the new note is inserted
	\param context          context within the new note is inserted
	\param left             music element left of new note
*/
bool CAMusElementFactory::configureNote( CAVoice *voice,
                                         const QPoint coords,
                                         CADrawableStaff *drawableStaff,
                                         CADrawableMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if ( drawableStaff ) {
		// did a user click on the note or before/after it? In first case, add a note to a chord, in latter case, insert a new note.
		CADrawableMusElement *followingNote;			
		if ( left && (left->musElement()->musElementType() == CAMusElement::Note) && (left->xPos() <= coords.x()) && (left->width() + left->xPos() >= coords.x()) ) {
			// user clicked inside x borders of the note - add a note to the chord
			if (voice->containsPitch(drawableStaff->calculatePitch(coords.x(), coords.y()), left->musElement()->timeStart()))
				return false;	//user clicked on an already placed note or wanted to place illegal length (not the one the chord is of) - return and do nothing
				
			int pitch;
			mpoMusElement = new CANote(((CANote*)left->musElement())->playableLength(),
		                  voice,
		                  pitch = drawableStaff->calculatePitch(coords.x(), coords.y()),
			          _iNoteExtraAccs,
			          left->musElement()->timeStart(),
			          ((CANote*)left->musElement())->dotted()
			        );
			bSuccess = voice->addNoteToChord((CANote*)mpoMusElement, (CANote*)left->musElement());
		} else {
			// user clicked outside x borders of the note - add a new note
			int pitch;
			mpoMusElement = new CANote(_ePlayableLength,
			          voice,
			          pitch = drawableStaff->calculatePitch(coords.x(), coords.y()),
			          _iNoteExtraAccs,
			          (left?left->musElement()->timeEnd():0),
			          _iPlayableDotted
			       );
			// add an empty syllable or reposit syllables
			static_cast<CANote*>(mpoMusElement)->setStemDirection( _eNoteStemDirection );
			if (left)	//left element exists
				bSuccess = voice->insertMusElementAfter(mpoMusElement, left->musElement());
			else		//left element doesn't exist, prepend the new music element
				bSuccess = voice->prependMusElement(mpoMusElement);
			
			// adds empty syllables, if syllable below the note doesn't exist or repositions the syllables, if it exists
			if (voice->lastNote()==mpoMusElement) {
				for (int i=0; i<voice->lyricsContextList().size(); i++) {
					voice->lyricsContextList().at(i)->repositSyllables(); // adds an empty syllable or assigns the already placed at the end if it exists
				}
			} else {
				for (int i=0; i<voice->lyricsContextList().size(); i++) {
					voice->lyricsContextList().at(i)->addEmptySyllable(
						mpoMusElement->timeStart(), mpoMusElement->timeLength()
					);
				}
			}
		}
	}
	
	if (bSuccess)
		static_cast<CANote*>(mpoMusElement)->updateTies();
	
	return bSuccess;
}

bool CAMusElementFactory::configureSlur( CAStaff *staff,
                                         CANote *noteStart, CANote *noteEnd )
{
	bool success=false;
	removeMusElem();
	CASlur *slur = new CASlur( slurType(), CASlur::SlurPreferred, staff, noteStart, noteEnd );
	
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
	slur->setSlurStyle( slurStyle() );
	mpoMusElement = slur;
	
	return success;
}

/*!
	Configures a new rest music element.
	
	\param voice      voice where new rest is inserted
	\param left       music element left of new rest
*/
bool CAMusElementFactory::configureRest( CAVoice *voice, CAMusElement *left ) {
	bool bSuccess = false;
	removeMusElem();
	if ( voice ) {
		mpoMusElement = new CARest(restType(),
				_ePlayableLength,
				voice,
				(left?left->timeEnd():0),
				_iPlayableDotted
				);
		bSuccess = voice->insertMusElementAfter(mpoMusElement, left);
	}
	return bSuccess;
}

bool CAMusElementFactory::configureFunctionMarking( CAFunctionMarkingContext *fmc, int timeStart, int timeLength ) {
	removeMusElem();
	CAFunctionMarking *fm = new CAFunctionMarking(
		fmFunction(), isFMFunctionMinor(),
		"C", /// \todo
		fmc, timeStart, timeLength,
		fmChordArea(), isFMChordAreaMinor(),
		fmTonicDegree(), isFMTonicDegreeMinor(),
		"", /// \todo
		isFMEllipse()
	);
	
	fmc->addFunctionMarking(fm);
	
	mpoMusElement = fm;
	
	return true;
}

void CAMusElementFactory::setMusElementType( CAMusElement::CAMusElementType eMEType )
{
	CAUndefMusElem oUMElem( 0, 0, 0 );
	// Check if music element is undefined
	removeMusElem();
	// We create now an instance of our CAUndefMusElem class
	CAUndefMusElem *poUnDefElem = (CAUndefMusElem *)createMusElem();
	poUnDefElem->setMusElementType( eMEType );
};

/*!
	\fn CAMusElementFactory::getMusElement()
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

	\sa getMusElement()
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
