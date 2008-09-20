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
#include "canorus.h"
 

/*!
	\class CAKeybdInput
	\brief Music input per midi non realtime
	This class adds a midi keyboard to the input devices to write a music score. It allows to use
	the computer mouse, computer keyboard and midi keyboard to input scores in non realtime.

	To activate midi keyboard input you have to select in canorus settings, readable devices,
	the alsa midi port of your midi keyboard. When in input mode, when a voice and a duration
	is selected, notes can be entered with the midi keyboard too.

	Key strockes within 100 ms will be combined into a chord.

	User selectable (to be implemented) midi pitches can be set to be interpreted as rest input,
	punctuation and so on.

	todo: accents according the current key pitch, auto barlines, automatic tracking of the scene ...
	
*/

CAKeybdInput::CAKeybdInput (CAMainWin *mw) {
	_mw = mw;
	// Initialize keyboad input chord timer
	_midiInChordTimer.stop();
	_midiInChordTimer.setSingleShot(true);
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
 
void CAKeybdInput::midiInEventToScore(CAScoreViewPort *v, QVector<unsigned char> m) {

	int i;
	CADiatonicPitch p = CAMidiDevice::midiPitchToDiatonicPitch( m[1] );
	// didn't help:  _mw->setCurrentViewPort( v );

	CAVoice *voice = _mw->currentVoice();
	if (voice) {

		// Default actual Key Signature is C
		_actualKeySignature = CADiatonicPitch ( CADiatonicPitch::C );
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
		CADiatonicPitch nonenharmonicPitch = matchPitchToKey( p );

		CANote *note = new CANote( nonenharmonicPitch, _mw->musElementFactory()->playableLength(), voice, 0 ); //voice->lastIimeEnd() );

		// Only for testing these functions, will be cleaned up!
		// It's still a work in progress.
		QList<CADrawableMusElement*> list = v->selection();
		CADrawableContext *drawableContext = v->currentContext();
		CAStaff *staff=0;
		CADrawableStaff *drawableStaff = 0;
		if (drawableContext) {
			drawableStaff = dynamic_cast<CADrawableStaff*>(drawableContext);
			staff = dynamic_cast<CAStaff*>(drawableContext->context());
		}

		QList<CAMusElement*> tupletList = voice->getPreviousByType( CAMusElement::Tuplet, 1000000 ); // voice->lastTimeEnd());
		// std::cout << " Tonartliste: " << keyList.size();
		// std::cout << "Tupletliste: " << tupletList.size() << std::endl;

		// if note come in sufficiently close together we make a chord of them
		bool appendToChord = _midiInChordTimer.isActive();
		if (!appendToChord)
			v->clearSelection();

		voice->append( note, appendToChord );
		voice->synchronizeMusElements();	// probably not needed
		v->addToSelection( (CADrawableMusElement*)(voice->lastMusElement()), true );	// todo: is not working as I expect it ...

		QRect scene = v->worldCoords();
		int xlast = v->timeToCoordsSimpleVersion( voice->lastTimeStart() );
		if ( ((xlast+50) > scene.right()) ) {	// the magic number 50 should be defined, ist the width of an element
			scene.translate( scene.width()/2, 0 );
			v->setWorldCoords(scene, false, true );
		}
		v->repaint();
		CACanorus::rebuildUI(_mw->document(), _mw->currentSheet());

		if ( !_midiInChordTimer.isActive() ) {
			_midiInChordTimer.start( 100 );		// Notes max 100 ms apart will form a chord
		}
	}
}


CADiatonicPitch CAKeybdInput::matchPitchToKey( CADiatonicPitch p ) {
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

