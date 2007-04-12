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
 * Copyright (c) 2006, Reinhard Katzmann, Matevž Jekovec, Canorus development team           
 * All Rights Reserved. See AUTHORS for a complete list of authors.          
 *                                                                           
 */

// Music element factory (creation, removal, configuration)

#ifndef MUSELEMENTFACTORY_H_
#define MUSELEMENTFACTORY_H_
#include "core/barline.h"

#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"

#include "core/staff.h"
#include "core/muselement.h"
#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/voice.h"
#include "core/rest.h"
#include "core/slur.h"
#include "core/functionmarking.h"
#include "core/lyricscontext.h"
#include "core/syllable.h"

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
	
	bool configureClef( CAStaff *staff, 
	                    CAMusElement *left );
	
	bool configureKeySignature( CAStaff *staff, 
	                            CAMusElement *left );
	
	bool configureTimeSignature( CAStaff *staff, 
	                             CAMusElement *left );
	
	bool configureBarline( CAStaff *staff, 
	                       CAMusElement *left );
	
	bool configureRest( CAVoice *voice,
	                    CAMusElement *left );
	
	
	bool configureNote( CAVoice *voice,
	                    const QPoint coords,
	                    CADrawableStaff *staff,  // needed because of the note collision determination
	                    CADrawableMusElement *left // needed because of the note collision determination
	                  );
	
	bool configureSlur( CAStaff *staff,
	                    CANote *noteStart, CANote *noteEnd );
	
	bool configureFunctionMarking( CAFunctionMarkingContext *fmc,
	                               int timeStart, int timeLength );
	
	bool configureSyllable( QString text,
	                        int stanzaNumber,
	                        bool hyphen,
	                        bool melisma,
	                        CAVoice *voice,
	                        CALyricsContext *context
	                      );
	
	inline CAMusElement::CAMusElementType musElementType()
	{ return mpoMusElement->musElementType(); };
	
	void setMusElementType( CAMusElement::CAMusElementType eMEType );
	
	inline void setTimeStart(int timeStart) { _timeStart = timeStart; }
	inline int timeStart() { return _timeStart; }
	
	inline void setTimeLength(int timeLength) { _timeLength = timeLength; }
	inline int timeLength() { return _timeLength; }
	
	inline CAPlayable::CAPlayableLength playableLength() { return _ePlayableLength; }
	
	inline void setPlayableLength( CAPlayable::CAPlayableLength ePlayableLength )
	{ _ePlayableLength = ePlayableLength; };
	
	inline int  playableDotted() { return _iPlayableDotted; };
	
	inline void setPlayableDotted( int iPlaybleDotted )
	{ _iPlayableDotted = iPlaybleDotted; };
	
	inline CANote::CAStemDirection noteStemDirection() { return _eNoteStemDirection; }
	
	inline void setNoteStemDirection( CANote::CAStemDirection eDir )
	{ _eNoteStemDirection = eDir; }
	
	inline void addPlayableDotted( int iAdd )
	{ _iPlayableDotted = (_iPlayableDotted+iAdd)%4; };
	
	inline int  noteAccs() { return _iNoteAccs; };
	
	inline void setNoteAccs( int iNoteAccs )
	{ _iNoteAccs = iNoteAccs; };
	
	inline void addNoteAccs( int iAdd )
	{ _iNoteAccs+= iAdd; };
	
	inline void subNoteAccs( int iSub )
	{ _iNoteAccs-= iSub; };
	
	inline int keySigNumberOfAccs() { return _iKeySigNumberOfAccs; }
	inline void setKeySigNumberOfAccs(int accs) { _iKeySigNumberOfAccs = accs; }
	
	inline int  noteExtraAccs() { return _iNoteExtraAccs; };
	
	inline void setNoteExtraAccs( int iNoteExtraAccs )
	{ _iNoteExtraAccs = iNoteExtraAccs; };
	
	inline void addNoteExtraAccs( int iAdd )
	{ _iNoteExtraAccs += iAdd; };
	
	inline void subNoteExtraAccs( int iSub )
	{ _iNoteExtraAccs -= iSub; };
	
	inline CARest::CARestType restType() { return _eRestType; }
	
	inline void setRestType(CARest::CARestType eType)
	{ _eRestType = eType; }
	
	inline int timeSigBeats() { return _iTimeSigBeats; }
	
	inline void setTimeSigBeats( int iTimeSigBeats )
	{ _iTimeSigBeats = iTimeSigBeats; };
	
	inline int timeSigBeat() { return _iTimeSigBeat; }
	
	inline void setTimeSigBeat( int iTimeSigBeat )
	{ _iTimeSigBeat = iTimeSigBeat; };
	
	inline void setClef( CAClef::CAClefType eClefType )
	{ _eClef = eClefType; };
	
	inline CABarline::CABarlineType barlineType() { return _eBarlineType; }
	inline void setBarlineType( CABarline::CABarlineType type)
	{ _eBarlineType = type; }
	
	inline CASlur::CASlurType slurType() { return _eSlurType; }
	inline void setSlurType( CASlur::CASlurType type ) { _eSlurType = type; }
	
	inline CASlur::CASlurStyle slurStyle() { return _slurStyle; }
	inline void setSlurStyle( CASlur::CASlurStyle style ) { _slurStyle = style; }
	
	inline CAFunctionMarking::CAFunctionType fmFunction() { return _fmFunction; }
	inline void setFMFunction( CAFunctionMarking::CAFunctionType f ) { _fmFunction = f; }
	
	inline CAFunctionMarking::CAFunctionType fmChordArea() { return _fmChordArea; }
	inline void setFMChordArea( CAFunctionMarking::CAFunctionType c ) { _fmChordArea = c; }
	
	inline CAFunctionMarking::CAFunctionType fmTonicDegree() { return _fmTonicDegree; }
	inline void setFMTonicDegree( CAFunctionMarking::CAFunctionType td ) { _fmTonicDegree = td; }
	
	inline bool isFMFunctionMinor() { return _fmFunctionMinor; }
	inline void setFMFunctionMinor( bool m ) { _fmFunctionMinor = m; }
	
	inline bool isFMTonicDegreeMinor() { return _fmTonicDegreeMinor; }
	inline void setFMTonicDegreeMinor( bool m ) { _fmTonicDegreeMinor = m; }
	
	inline bool isFMChordAreaMinor() { return _fmChordAreaMinor; }
	inline void setFMChordAreaMinor( bool m ) { _fmChordAreaMinor = m; }
	
	inline bool isFMEllipse() { return _fmEllipse; }
	inline void setFMEllipse( bool e ) { _fmEllipse = e; }	
	
	inline QString syllableText() { return _syllableText; }
	inline void setSyllableText( QString text ) { _syllableText = text; }
	
private:
	CAMusElement *mpoMusElement;     // newly created music element itself
	/////////////////////////////////
	// Element creation parameters //
	/////////////////////////////////
	int _timeStart;
	int _timeLength;
	
	// Staff music elements
	CAPlayable::CAPlayableLength _ePlayableLength; // Length of note/rest to be added
	CANote::CAStemDirection _eNoteStemDirection; // Note stem direction to be inserted
	CASlur::CASlurType _eSlurType;  // Slur type to be placed
	int _iPlayableDotted;	   // Number of dots to be inserted for the note/rest
	int _iNoteExtraAccs;	   // Extra note accidentals for new notes which user adds/removes with +/- keys
	int _iNoteAccs;	         // Note accidentals at specific coordinates updated regularily when in insert mode
	CARest::CARestType _eRestType; // Hidden/Normal rest
	int _iKeySigNumberOfAccs; // Key signature number of accidentals
	int _iTimeSigBeats;      // Time signature number of beats to be inserted
	int _iTimeSigBeat;       // Time signature beat to be inserted
	CAClef::CAClefType _eClef; // Type of the clef to be inserted
	CABarline::CABarlineType _eBarlineType; // Type of the barline
	CASlur::CASlurStyle _slurStyle; // Style of the slur (solid, dotted)
	
	// Function Marking
	CAFunctionMarking::CAFunctionType _fmFunction; // Name of the function
	CAFunctionMarking::CAFunctionType _fmChordArea; // Chord area of the function
	CAFunctionMarking::CAFunctionType _fmTonicDegree; // Tonic degree of the function
	bool _fmFunctionMinor;
	bool _fmChordAreaMinor;
	bool _fmTonicDegreeMinor;
	bool _fmEllipse;
	
	// Lyrics
	QString _syllableText;
};
#endif // MUSELEMENTFACTORY_H_
