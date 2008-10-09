/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	Copyright (c) 2008, Georg Rudolph
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "interface/keybdinput.h"
#include "interface/mididevice.h"
#include "core/muselementfactory.h"
#include "core/interval.h"
#include "drawable/drawablestaff.h"
#include "widgets/menutoolbutton.h"
#include "canorus.h"

class CAMenuToolButton;
 

/*!
	\class CAKeybdInput
	\brief Music input per midi non realtime
	This class adds a midi keyboard to the input devices to write a music score. It allows to use
	a midi keyboard, the computer mouse and the computer keyboard to input scores in non realtime.

	To activate midi keyboard input you have to select in canorus settings, readable devices,
	the alsa midi port of your midi keyboard. When in input mode, when a voice and a duration
	is selected, notes can be entered with the midi keyboard too.

	Key strockes within 100 ms will be combined into a chord.

	Accents are set according the current key pitch. Automatic tracking of the scene is done.

	todo: User selectable (to be implemented) midi pitches can be set to be interpreted as rest input,
	punctuation and so on.

	todo: applying auto barlines, possibly inserting at the currently selected note, configuration of
	selectable midi pitches to 
	
*/

CAKeybdInput::CAKeybdInput (CAMainWin *mw) {
	_mw = mw;
	// Initialize keyboad input chord timer
	_midiInChordTimer.stop();
	_midiInChordTimer.setSingleShot(true);
	_tupPla = 0;
	_tup = 0;
}

/*!
	Destructor deletes the created arrays.
*/
CAKeybdInput::~CAKeybdInput() {
}

void CAKeybdInput::onMidiInEvent( QVector<unsigned char> m ) {
	unsigned char event, velocity;
 	std::cout << "MidiInEvent: ";
 	for (int i=0; i<m.size(); i++)
 		std::cout << (int)m[i] << " ";
 	std::cout << std::endl;
	event = m[0];
	velocity = m[2];
	if ( event == CAMidiDevice::Midi_Note_On && velocity !=0 ) {
		CADiatonicPitch x = CAMidiDevice::midiPitchToDiatonicPitch( m[1] );
		midiInEventToScore( _mw->currentScoreViewPort(), m );
	}
}
 
/*!
	This is the entry point the midi input device. All note on events are passed over here.
*/
void CAKeybdInput::midiInEventToScore(CAScoreViewPort *v, QVector<unsigned char> m) {

	int i;
	CADiatonicPitch p = CAMidiDevice::midiPitchToDiatonicPitch( m[1] );
	CADiatonicPitch nonenharmonicPitch;

	CAVoice *voice = _mw->currentVoice();
	if (voice) {

		int cpitch = m[1];
/*
		// will publish this only when it's configurable. Habe only a four octave keyboard ...

		CAPlayableLength plength = CAPlayableLength::Undefined;
		switch (cpitch) {
		case 39:	plength = CAPlayableLength::Whole;		break;
		case 40:	plength = CAPlayableLength::Half;		break;
		case 41:	plength = CAPlayableLength::Quarter;	break;
		case 42:	plength = CAPlayableLength::Eighth;		break;
		case 43:	plength = CAPlayableLength::Sixteenth;	break;
		default:	;
		}
		if (plength !=  CAPlayableLength::Undefined) {
			_mw->uiPlayableLength->setCurrentId( plength.musicLength(), true );
			_mw->musElementFactory()->playableLength().setDotted( 0 );
			v->setShadowNoteLength( _mw->musElementFactory()->playableLength() );
			v->updateHelpers();
			v->repaint();
			return;
		}

		switch (cpitch) {
		case 37:	std::cout << "  Pause" << std::endl;
					break;
		case 38:	_mw->uiTupletType->defaultAction()->setChecked( !_mw->uiTupletType->isChecked() );
					return;
		default:	nonenharmonicPitch = matchPitchToKey( voice, p );
		}
*/

		nonenharmonicPitch = matchPitchToKey( voice, p );
		CANote *note = new CANote( nonenharmonicPitch, _mw->musElementFactory()->playableLength(), voice, -1 );

		// if notes come in sufficiently close together we make a chord of them
		bool appendToChord = _midiInChordTimer.isActive();

		// Where to put the note? When in a tuplet, do a chord in the tuplet or the nex not in the tuplet.
		if ( _tupPla &&!appendToChord ) {
			_tupPla = _tup->nextTimed( _tupPla );
		}

		if ( !_tupPla && !appendToChord ) {
			//	try to apply autobar, still crashing:
			//	if ( CACanorus::settings()->autoBar() )
			//  	CAMusElementFactory::placeAutoBar( static_cast<CAPlayable*>(_mw->musElementFactory()->musElement()) );
		}

		if ( _tupPla ) {
			// next note in tuplet
			_tupPla = voice->insertInTupletAndVoiceAt( _tupPla, note );
			_tup = _tupPla->tuplet();
		} else if ( _mw->uiTupletType->isChecked() ) {
			// start a new tuplet
			QList<CAPlayable*> elements;
			elements << static_cast<CAPlayable*>(note);
			for (int i=1; i<_mw->uiTupletNumber->value(); i++) {
				_mw->musElementFactory()->configureRest( voice, 0 );
				elements << static_cast<CAPlayable*>(_mw->musElementFactory()->musElement());
			}
			_tup = new CATuplet( _mw->uiTupletNumber->value(), _mw->uiTupletActualNumber->value(), elements );
			_tupPla = _tup->firstNote();
		} else {
			// insert just a note
			voice->append( note, appendToChord );
		}

		voice->synchronizeMusElements();	// probably not needed

		// now we try to highlight the inserted note/chord by selection:
		if (!appendToChord) {
			v->clearSelection();	// remove old note/chord from selection
		}
		QList<CAPlayable*> lp = voice->getChord(voice->lastMusElement()->timeStart());
		QList<CAMusElement*> lme;
		for (int i=0;i<lp.size();i++) lme << static_cast<CAMusElement*>(lp[i]);
		v->addToSelection(lme);

		// When I looking the last appended note is note showing up. Where goes it missing?
		// Still without a clue. georg
		// QList<CADrawableMusElement*> list = v->selection();
		// std::cout << " Selektierte Elemente: " << list.size() << " Stück, oben "<<lp.size()<<" und "<<lme.size() << std::endl;

		// scene tracking
		QRect scene = v->worldCoords();
		int xlast = v->timeToCoordsSimpleVersion( voice->lastTimeStart() );
		if ( ((xlast+50) > scene.right()) ) {	// the magic number 50 should be defined, ist the width of an element
			scene.translate( scene.width()/2, 0 );
			v->setWorldCoords(scene, false, true );
		}
		v->updateHelpers();
		v->repaint();
		CACanorus::rebuildUI(_mw->document(), _mw->currentSheet());

		// start timer eventually for chord detection
		if ( !_midiInChordTimer.isActive() ) {
			_midiInChordTimer.start( 100 );		// Notes max 100 ms apart will form a chord
		}
	}
}


/*!
	This function looks up the current key signiture. Then it computes the proper accidentials
	for the note.

	This function should be somewhere else, maybe in \a CADiatonicPitch ?
*/
CADiatonicPitch CAKeybdInput::matchPitchToKey( CAVoice* voice, CADiatonicPitch p ) {

	// Default actual Key Signature is C
	_actualKeySignature = CADiatonicPitch ( CADiatonicPitch::C );

	int i;
	for(i=0;i<7;i++) _actualKeySignatureAccs[i] = 0;
	_actualKeyAccidentalsSum = 0;

	// Trace which Key Signature might be in effect.
	// We make a local copy for later optimisation by only updating at a non
	// linear input
	QList<CAMusElement*> keyList = voice->getPreviousByType(
							CAMusElement::KeySignature, voice->lastTimeEnd());
	if (keyList.size()) {
		// set the note name and its accidental and the accidentals of the scale
		CAKeySignature* effSig = (CAKeySignature*) keyList.last();
		_actualKeySignature = effSig->diatonicKey().diatonicPitch();
		_actualKeyAccidentalsSum = 0;
		for(i=0;i<7;i++) {
			_actualKeySignatureAccs[i] = (effSig->accidentals())[i];
			_actualKeyAccidentalsSum += _actualKeySignatureAccs[i];
		}
	}

	// f and s (flat/sharp) are enharmonic pitches for p
	CADiatonicPitch f = p + CAInterval( CAInterval::Diminished, CAInterval::Second );
	CADiatonicPitch s = p - CAInterval( CAInterval::Diminished, CAInterval::Second );

	// If the pitch appears in the keys scale we are done
	if (p.accs() == _actualKeySignatureAccs[p.noteName()%7] ) {
		return p;
	}
	// When the key is with flats we don't want sharps
	if (_actualKeyAccidentalsSum < 0) {
		if (f.accs() == _actualKeySignatureAccs[f.noteName()%7] )
			return f;
 		if (p.accs() > 0)
			return f;
	}
	// When the key is with sharps we don't want flats
	if (_actualKeyAccidentalsSum > 0) {
		if (s.accs() == _actualKeySignatureAccs[s.noteName()%7] ) {
			return s;
		}
 		if (p.accs() < 0) {
			return s;
		}
	}
	return p;
}

/*!
	Not in use currently, will be superseeded by autoBar() hopefully
*/
CADiatonicPitch CAKeybdInput::autoBarInsert( void ) {
	// Trace which Time Signature might be in effect.
	// We make a local copy for later optimisation by only updating at a non
	// linear input
	CAVoice *voice = _mw->currentVoice();
	if (voice) {
		QList<CAMusElement*> tSigList = voice->getPreviousByType(
				CAMusElement::TimeSignature, voice->lastTimeEnd());
		int beats, beat = 0;
		if ( tSigList.size() ) {
			beats = ((CATimeSignature*)(tSigList.last()))->beats();
			beat = ((CATimeSignature*)(tSigList.last()))->beat();
		} else {
			beats = beat = 4;
		}
		std::cout << "TimeSig: "<<beats<<"/"<<beat<<std::endl;
	}
}

