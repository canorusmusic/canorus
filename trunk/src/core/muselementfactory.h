/*!
	Copyright (c) 2006, Reinhard Katzmann, Canorus development team
	              2007, Matevž Jekovec, Canorus development team
	
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef MUSELEMENTFACTORY_H_
#define MUSELEMENTFACTORY_H_
#include "core/barline.h"

#include "core/staff.h"
#include "core/muselement.h"
#include "core/keysignature.h"
#include "core/timesignature.h"
#include "core/voice.h"
#include "core/rest.h"
#include "core/slur.h"
#include "core/functionmark.h"
#include "core/lyricscontext.h"
#include "core/syllable.h"
#include "core/mark.h"
#include "core/articulation.h"
#include "core/fermata.h"
#include "core/tempo.h"
#include "core/ritardando.h"
#include "core/crescendo.h"
#include "core/repeatmark.h"
#include "core/fingering.h"
#include "core/playablelength.h"
#include "core/diatonickey.h"

class CAMusElement;

class CAMusElementFactory {
public:
	CAMusElementFactory();
	~CAMusElementFactory();
	
	CAMusElement *createMusElem();
	
	void removeMusElem( bool bReallyRemove = false );
	
	void configureMusElem( CAMusElement &roMusElement );
	
	inline CAMusElement *musElement() { return mpoMusElement; };
	
	inline void cloneMusElem() { mpoMusElement = mpoMusElement->clone(); }
	inline void emptyMusElem() { mpoMusElement = mpoEmpty; }
	
	bool configureClef( CAStaff *staff, 
	                    CAMusElement *right );
	
	bool configureKeySignature( CAStaff *staff, 
	                            CAMusElement *right );
	
	bool configureTimeSignature( CAStaff *staff, 
	                             CAMusElement *right );
	
	bool configureBarline( CAStaff *staff, 
	                       CAMusElement *right );
	
	bool configureRest( CAVoice *voice,
	                    CAMusElement *right );
	
	
	bool configureNote( int pitch,
	                    CAVoice *voice,
	                    CAMusElement *right,
	                    bool addToChord
	                  );
	
	bool configureSlur( CAStaff *staff,
	                    CANote *noteStart, CANote *noteEnd );
	
	bool configureMark( CAMusElement *elt );
	
	bool configureFunctionMark( CAFunctionMarkContext *fmc,
	                               int timeStart, int timeLength );
	
	inline CAMusElement::CAMusElementType musElementType() { return _musElementType; }
	void setMusElementType( CAMusElement::CAMusElementType eMEType ) { _musElementType = eMEType; } 
	
	inline CAPlayableLength& playableLength() { return _playableLength; }
	
	inline void setPlayableLength( CAPlayableLength& playableLength )
	{ _playableLength = playableLength; };
	
	void addPlayableDotted( int add, CAPlayableLength curLength );
	
	inline CANote::CAStemDirection noteStemDirection() { return _eNoteStemDirection; }
	
	inline void setNoteStemDirection( CANote::CAStemDirection eDir )
	{ _eNoteStemDirection = eDir; }
	
	inline int  noteAccs() { return _iNoteAccs; };
	
	inline void setNoteAccs( int iNoteAccs )
	{ _iNoteAccs = iNoteAccs; };
	
	inline void addNoteAccs( int iAdd )
	{ _iNoteAccs+= iAdd; };
	
	inline void subNoteAccs( int iSub )
	{ _iNoteAccs-= iSub; };
	
	inline int diatonicKeyNumberOfAccs() { return _diatonicKeyNumberOfAccs; }
	inline void setDiatonicKeyNumberOfAccs(int accs) { _diatonicKeyNumberOfAccs = accs; }
	inline CADiatonicKey::CAGender diatonicKeyGender() { return _diatonicKeyGender; }
	inline void setDiatonicKeyGender( CADiatonicKey::CAGender g ) { _diatonicKeyGender = g; }
	
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
	
	inline CAClef::CAPredefinedClefType clef() { return _eClef; }
	inline void setClef( CAClef::CAPredefinedClefType eClefType )
	{ _eClef = eClefType; };
	
	inline int clefOffset() { return _iClefOffset; } // readable offset interval, not internal offset
	inline void setClefOffset( int offset )
	{ _iClefOffset= offset; };
	
	inline CABarline::CABarlineType barlineType() { return _eBarlineType; }
	inline void setBarlineType( CABarline::CABarlineType type)
	{ _eBarlineType = type; }
	
	inline CASlur::CASlurType slurType() { return _eSlurType; }
	inline void setSlurType( CASlur::CASlurType type ) { _eSlurType = type; }
	
	inline CASlur::CASlurStyle slurStyle() { return _slurStyle; }
	inline void setSlurStyle( CASlur::CASlurStyle style ) { _slurStyle = style; }
	
	inline CAMark::CAMarkType markType() { return _markType; }
	inline void setMarkType( CAMark::CAMarkType t ) { _markType = t; }
	
	inline CAArticulation::CAArticulationType articulationType() { return _articulationType; }
	inline void setArticulationType( CAArticulation::CAArticulationType t ) { _articulationType = t; }
	
	inline CAFunctionMark::CAFunctionType fmFunction() { return _fmFunction; }
	inline void setFMFunction( CAFunctionMark::CAFunctionType f ) { _fmFunction = f; }
	
	inline CAFunctionMark::CAFunctionType fmChordArea() { return _fmChordArea; }
	inline void setFMChordArea( CAFunctionMark::CAFunctionType c ) { _fmChordArea = c; }
	
	inline CAFunctionMark::CAFunctionType fmTonicDegree() { return _fmTonicDegree; }
	inline void setFMTonicDegree( CAFunctionMark::CAFunctionType td ) { _fmTonicDegree = td; }
	
	inline bool isFMFunctionMinor() { return _fmFunctionMinor; }
	inline void setFMFunctionMinor( bool m ) { _fmFunctionMinor = m; }
	
	inline bool isFMTonicDegreeMinor() { return _fmTonicDegreeMinor; }
	inline void setFMTonicDegreeMinor( bool m ) { _fmTonicDegreeMinor = m; }
	
	inline bool isFMChordAreaMinor() { return _fmChordAreaMinor; }
	inline void setFMChordAreaMinor( bool m ) { _fmChordAreaMinor = m; }
	
	inline bool isFMEllipse() { return _fmEllipse; }
	inline void setFMEllipse( bool e ) { _fmEllipse = e; }
	
	inline const QString dynamicText() { return _dynamicText; }
	inline void setDynamicText( const QString t ) { _dynamicText = t; }
	
	inline const int dynamicVolume() { return _dynamicVolume; }
	inline void setDynamicVolume( const int vol ) { _dynamicVolume = vol; }
	
	inline const int instrument() { return _instrument; }
	inline void setInstrument( const int instrument ) { _instrument = instrument; }
	
	inline const CAFermata::CAFermataType fermataType() { return _fermataType; }
	inline void setFermataType( const CAFermata::CAFermataType type ) { _fermataType = type; }
	
	inline const int tempoBpm() { return _tempoBpm; }
	inline void setTempoBpm( const int tempoBpm ) { _tempoBpm = tempoBpm; }
	
	inline CAPlayableLength& tempoBeat() { return _tempoBeat; }
	inline void setTempoBeat( CAPlayableLength& length ) { _tempoBeat = length; }
	
	inline const CARitardando::CARitardandoType ritardandoType() { return _ritardandoType; }
	inline void setRitardandoType( CARitardando::CARitardandoType t ) { _ritardandoType = t; }
	
	inline const int crescendoFinalVolume() { return _crescendoFinalVolume; }
	inline void setCrescendoFinalVolume( const int v ) { _crescendoFinalVolume = v; }
	
	inline const CACrescendo::CACrescendoType crescendoType() { return _crescendoType; }
	inline void setCrescendoType( const CACrescendo::CACrescendoType t ) { _crescendoType = t; }
	
	inline const CARepeatMark::CARepeatMarkType repeatMarkType() { return _repeatMarkType; }
	inline void setRepeatMarkType( const CARepeatMark::CARepeatMarkType t ) { _repeatMarkType = t; }
	
	inline const int repeatMarkVoltaNumber() { return _repeatMarkVoltaNumber; }
	inline void setRepeatMarkVoltaNumber( const int n ) { _repeatMarkVoltaNumber = n; }
	
	inline const CAFingering::CAFingerNumber fingeringFinger() { return _fingeringFinger; }
	inline void setFingeringFinger( const CAFingering::CAFingerNumber f ) { _fingeringFinger = f; }
	
	inline const bool isFingeringOriginal() { return _fingeringOriginal; }
	inline void setFingeringOriginal( const int o ) { _fingeringOriginal = o; }
private:
	CAMusElement *mpoMusElement;                    // Newly created music element itself
	CAMusElement *mpoEmpty;                         // An empty (dummy) element.
	
	/////////////////////////////////
	// Element creation parameters //
	/////////////////////////////////
	CAMusElement::CAMusElementType _musElementType; // Music element type
	
	// Staff music elements
	CAPlayableLength _playableLength; // Length of note/rest to be added
	CANote::CAStemDirection _eNoteStemDirection;   // Note stem direction to be inserted
	CASlur::CASlurType _eSlurType;                 // Slur type to be placed
	int _iPlayableDotted;                          // Number of dots to be inserted for the note/rest
	int _iNoteExtraAccs;                           // Extra note accidentals for new notes which user adds/removes with +/- keys
	int _iNoteAccs;                                // Note accidentals at specific coordinates updated regularily when in insert mode
	CARest::CARestType _eRestType;                 // Hidden/Normal rest
	int _diatonicKeyNumberOfAccs;                  // Key signature number of accidentals
	CADiatonicKey::CAGender _diatonicKeyGender;    // Major/Minor gender of the key signature
	int _iTimeSigBeats;                            // Time signature number of beats to be inserted
	int _iTimeSigBeat;                             // Time signature beat to be inserted
	CAClef::CAPredefinedClefType _eClef;           // Type of the clef to be inserted
	int                          _iClefOffset;     // Interval offset for the clef
	CABarline::CABarlineType _eBarlineType;        // Type of the barline
	CAMark::CAMarkType _markType;                  // Type of the mark
	CAArticulation::CAArticulationType _articulationType; // Type of the articulation mark
	CASlur::CASlurStyle _slurStyle;                // Style of the slur (solid, dotted)
	
	// Function Mark
	CAFunctionMark::CAFunctionType _fmFunction;    // Name of the function
	CAFunctionMark::CAFunctionType _fmChordArea;   // Chord area of the function
	CAFunctionMark::CAFunctionType _fmTonicDegree; // Tonic degree of the function
	bool _fmFunctionMinor;
	bool _fmChordAreaMinor;
	bool _fmTonicDegreeMinor;
	bool _fmEllipse;
	
	// Marks
	QString _dynamicText;
	int _dynamicVolume;
	int _instrument;
	CAFermata::CAFermataType _fermataType;
	CAPlayableLength _tempoBeat;
	int _tempoBpm;
	CARitardando::CARitardandoType _ritardandoType;
	int _crescendoFinalVolume;
	CACrescendo::CACrescendoType _crescendoType;
	CARepeatMark::CARepeatMarkType _repeatMarkType;
	int _repeatMarkVoltaNumber;
	CAFingering::CAFingerNumber _fingeringFinger;
	int _fingeringOriginal;
};
#endif // MUSELEMENTFACTORY_H_
