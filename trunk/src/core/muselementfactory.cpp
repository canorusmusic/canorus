/** @file muselementfactory.cpp
 *
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
 * Copyright (c) 2006, Reinhard Katzmann, Canorus development team           
 * All Rights Reserved. See AUTHORS for a complete list of authors.          
 *                                                                           
 */

#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"
#include "core/staff.h"
#include "core/muselement.h"
#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/voice.h"
#include "core/rest.h"
#include "core/muselementfactory.h"

// For comparing undefined music elements
#define UNDEFNUM 36720165

// Class for undefined music elements, not to be known outside of this class
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

CAMusElementFactory::CAMusElementFactory()
{
	_ePlayableLength = CAPlayable::Quarter;
	_iPlayableDotted = 0;
	_iTimeSigBeats = 4;
	_iTimeSigBeat = 4;
	_eClef = CAClef::Treble;
	_iNoteAccs = 0;
	_iNoteExtraAccs = 0;
	createMusElem();
}

CAMusElementFactory::~CAMusElementFactory()
{
	removeMusElem();
}

CAMusElement *CAMusElementFactory::createMusElem()
{
	mpoMusElement = new CAUndefMusElem( 0, 0, 0 );
	((CAUndefMusElem *)mpoMusElement)->setMusElementType( CAMusElement::Undefined );
	return mpoMusElement;
}

void CAMusElementFactory::removeMusElem( bool bReallyRemove /* = false */ )
{
	CAUndefMusElem oUMElem( 0, 0, 0 );
	if( mpoMusElement && 
	    ( bReallyRemove || oUMElem.compare( mpoMusElement ) <= 0 ) )
		delete mpoMusElement;
	mpoMusElement = 0;
}

void CAMusElementFactory::configureMusElem( CAMusElement &roMusElement )
{
	if( mpoMusElement )
		delete mpoMusElement;
	mpoMusElement = roMusElement.clone();
}

bool CAMusElementFactory::configureClef( CADrawableContext *context, 
                                         CADrawableMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if ( (context) && 
	     (context->context()->contextType() == CAContext::Staff) )
	{
		CAStaff *staff = (CAStaff*)context->context();
		mpoMusElement = new CAClef(_eClef, staff, (left?left->musElement()->timeEnd():0));
		bSuccess = staff->insertSignAfter(mpoMusElement, left?left->musElement():0, true);
	}
	return bSuccess;
}

bool CAMusElementFactory::configureKeySignature( int iKeySignature,
                                                 CADrawableContext *context, 
                                                 CADrawableMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if ( (context) &&
	     (context->context()->contextType() == CAContext::Staff) )
	{		
		CAStaff *staff = (CAStaff*)context->context();
		mpoMusElement = new CAKeySignature(CAKeySignature::MajorMinor, 
			                           iKeySignature,
			                           CAKeySignature::Major, staff,
			                           (left?left->musElement()->timeEnd():0));
		bSuccess = staff->insertSignAfter(mpoMusElement, left?left->musElement():0, true);
	}
	return bSuccess;
}

bool CAMusElementFactory::configureTimeSignature( CADrawableContext *context, 
                                                  CADrawableMusElement *left )
{
	bool bSuccess = false;
	removeMusElem();
	if ( (context) &&
	     (context->context()->contextType() == CAContext::Staff) )
	{
		CAStaff *staff = (CAStaff*)context->context();
		mpoMusElement = new CATimeSignature( _iTimeSigBeats, _iTimeSigBeat,
			                             staff,
			                             (left?left->musElement()->timeEnd():0));
		bSuccess = staff->insertSignAfter(mpoMusElement, left?left->musElement():0, true);
	}
	return bSuccess;
}

bool CAMusElementFactory::configureNote( CAVoice *voice,
                                         const QPoint coords,
                                         CADrawableContext *context,
                                         CADrawableMusElement *left )
{
	bool bSuccess = false;
        CADrawableStaff *drawableStaff = (CADrawableStaff *)context;
	removeMusElem();
	if ( (context) &&
	     (context->context()->contextType() == CAContext::Staff) )
	{
		//did a user click on the note or before/after it? In first case, add a note to a chord, in latter case, insert a new note.
		CADrawableMusElement *followingNote;			
		if ( left && (left->musElement()->musElementType() == CAMusElement::Note) && (left->xPos() <= coords.x()) && (left->width() + left->xPos() >= coords.x()) ) {
			//user clicked inside x borders of the note - add a note to the chord
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
			//user clicked outside x borders of the note - add a new note
			int pitch;
			mpoMusElement = new CANote(_ePlayableLength,
			          voice,
			          pitch = drawableStaff->calculatePitch(coords.x(), coords.y()),
			          _iNoteExtraAccs,
			          (left?left->musElement()->timeEnd():0),
			          _iPlayableDotted
			       );
			if (left)	//left element exists
				bSuccess = voice->insertMusElementAfter(mpoMusElement, left->musElement());
			else		//left element doesn't exist, prepend the new music element
				bSuccess = voice->prependMusElement(mpoMusElement);
			}
	}
	return bSuccess;
}

bool CAMusElementFactory::configureRest( CAVoice *voice,
                                         const QPoint coords,
                                         CADrawableContext *context,
                                         CADrawableMusElement *left )
{
	bool bSuccess = false;
        CADrawableStaff *drawableStaff = (CADrawableStaff *)context;
	removeMusElem();
	if ( (context) &&
	     (context->context()->contextType() == CAContext::Staff) )
	{
		mpoMusElement = new CARest(CARest::Normal,
				_ePlayableLength,
				voice,
				(left?left->musElement()->timeEnd():0),
				_iPlayableDotted
				);
		bSuccess = voice->insertMusElementAfter(mpoMusElement, left?left->musElement():0);
	}
	return bSuccess;
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
