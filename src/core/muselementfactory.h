/*
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

// Music element factory (creation, removal, configuration)

#ifndef MUSELEMENTFACTORY_H_
#define MUSELEMENTFACTORY_H_

class CAMusElement;

class CAMusElementFactory
{
public:
	CAMusElementFactory();
	~CAMusElementFactory();
	
	CAMusElement *createMusElem();
	
	void removeMusElem( bool bReallyRemove = false );
	
	void configureMusElem( CAMusElement &roMusElement );
	
	inline CAMusElement *getMusElement() { return mpoMusElement; };
	
	bool configureClef( CADrawableContext *context, 
	                    CADrawableMusElement *left );
	
	bool configureKeySignature( int iKeySignature,
	                            CADrawableContext *context, 
	                            CADrawableMusElement *left );
	
	bool configureTimeSignature( CADrawableContext *context, 
	                             CADrawableMusElement *left );
	
	bool configureRest( CAVoice *voice,
	                    const QPoint coords,
	                    CADrawableContext *context, 
	                    CADrawableMusElement *left );
	
	
	bool configureNote( CAVoice *voice,
	                    const QPoint coords,
	                    CADrawableContext *context, 
	                    CADrawableMusElement *left );
	
	inline CAMusElement::CAMusElementType musElementType()
	{ return mpoMusElement->musElementType(); };
	
	void setMusElementType( CAMusElement::CAMusElementType eMEType );
	
	inline void setPlayableLength( CAPlayable::CAPlayableLength ePlayableLength )
	{ _ePlayableLength = ePlayableLength; };
	
	inline int  playableDotted() { return _iPlayableDotted; };
	
	inline void setPlayableDotted( int iPlaybleDotted )
	{ _iPlayableDotted = iPlaybleDotted; };
	
	inline void addPlayableDotted( int iAdd )
	{ _iPlayableDotted = (_iPlayableDotted+iAdd)%4; };
	
	inline int  noteAccs() { return _iNoteAccs; };
	
	inline void setNoteAccs( int iNoteAccs )
	{ _iNoteAccs = iNoteAccs; };
	
	inline void addNoteAccs( int iAdd )
	{ _iNoteAccs+= iAdd; };
	
	inline void subNoteAccs( int iSub )
	{ _iNoteAccs-= iSub; };
	
	inline int  noteExtraAccs() { return _iNoteExtraAccs; };
	
	inline void setNoteExtraAccs( int iNoteExtraAccs )
	{ _iNoteExtraAccs = iNoteExtraAccs; };
	
	inline void addNoteExtraAccs( int iAdd )
	{ _iNoteExtraAccs += iAdd; };
	
	inline void subNoteExtraAccs( int iSub )
	{ _iNoteExtraAccs -= iSub; };
	
	inline void setTimeSigBeats( int iTimeSigBeats )
	{ _iTimeSigBeats = iTimeSigBeats; };
	
	inline void setTimeSigBeat( int iTimeSigBeat )
	{ _iTimeSigBeat = iTimeSigBeat; };
	
	inline void setClef( CAClef::CAClefType eClefType )
	{ _eClef = eClefType; };
	
private:
	CAMusElement *mpoMusElement;     // newly created music element itself
	/////////////////////////////////
	// Element creation parameters //
	/////////////////////////////////
	CAPlayable::CAPlayableLength _ePlayableLength; // Length of note/rest to be added
	int _iPlayableDotted;	   // Number of dots to be inserted for the note/rest
	int _iNoteExtraAccs;	   // Extra note accidentals for new notes which user adds/removes with +/- keys
	int _iNoteAccs;	         // Note accidentals at specific coordinates updated regularily when in insert mode
	int _iTimeSigBeats;      // Time signature number of beats to be inserted
	int _iTimeSigBeat;       // Time signature beat to be inserted
	CAClef::CAClefType _eClef; // Type of the clef to be inserted
};
#endif // MUSELEMENTFACTORY_H_
