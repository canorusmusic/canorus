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

		autoBarInsert();

		int cpitch = m[1];
		switch (cpitch) {
		case 37:	std::cout << "  Pause" << std::endl;
					break;
		default:	;
		}

		CADiatonicPitch nonenharmonicPitch = matchPitchToKey( voice, p );

		CADrawableMusElement *left = (CADrawableMusElement*)(voice->lastMusElement());
		// Problem: zeigt auf die KeySignatur
		CAMusElement* guck = voice->musElementAt(voice->musElementList().count()-1);
		CAPlayable* pla;
		CATuplet* tup;
		std::cout<<"tu-Config: "<<_mw->uiTupletType->isChecked()
					<<"/"<<_mw->uiTupletNumber->value()
					<<"/"<<_mw->uiTupletActualNumber->value()<<std::endl;
		for(i= voice->musElementList().count()-1; i>=0; i--) {
			guck = voice->musElementAt(i);
			pla = dynamic_cast<CAPlayable*>(guck);
			if (pla) {
				tup = pla->tuplet();
				//tup = static_cast<CAPlayable*>(guck)->tuplet();
				if (tup) {
					std::cout<<"   "<<i<<"      Tuplet gefunden: "<<tup->actualNumber()<<"/"<<tup->number()
																	<<" "<<(pla==tup->firstNote())
																	<<" "<<tup->containsNote(pla)
																	<<" "<<(pla==tup->lastNote())
																	<<" "<<pla->timeStart()
																	<<std::endl;
				} else {
					std::cout<<"   "<<i<<"      Note"<<std::endl;
				}
			} else {
					std::cout<<"   "<<i<<"      ????"<<std::endl;
			}
		}

		if (left && ((CAMusElement*)(left))->musElementType()==CAMusElement::Tuplet ) {
			CATuplet *tuplet = static_cast<CAPlayable*>(left->musElement())->tuplet();
			QList<CAPlayable*> noteList; int number; int actualNumber;
			std::cout<<" Tuplet "<<tuplet<<" Liste mit "<<noteList.size()<<" number "<<number<<" actnume "
					<<actualNumber<<std::endl;
			if ( tuplet  ) {
				noteList = tuplet->noteList();
				number = tuplet->number();
				actualNumber = tuplet->actualNumber();
				std::cout<<" Tuplet "<<tuplet<<" Liste mit "<<noteList.size()<<" number "<<number<<" actnume "
						<<actualNumber<<std::endl;
				delete tuplet;
			}
			int timeSum = left->musElement()->timeLength();
			int timeLength = CAPlayableLength::playableLengthToTimeLength( _mw->musElementFactory()->playableLength() );
		}



		CANote *note = new CANote( nonenharmonicPitch, _mw->musElementFactory()->playableLength(), voice, -1 );

		// Only for testing these functions, will be cleaned up!
		// It's still a work in progress.
		CADrawableContext *drawableContext = v->currentContext();
		CAStaff *staff=0;
		CADrawableStaff *drawableStaff = 0;
		if (drawableContext) {
			drawableStaff = dynamic_cast<CADrawableStaff*>(drawableContext);
			staff = dynamic_cast<CAStaff*>(drawableContext->context());
		}

		// QList<CAMusElement*> tupletList = voice->getPreviousByType( CAMusElement::Tuplet, 1000000 ); // voice->lastTimeEnd());
		// std::cout << " Tonartliste: " << keyList.size();
		// std::cout << "Tupletliste: " << tupletList.size() << std::endl;

		// if note come in sufficiently close together we make a chord of them
		bool appendToChord = _midiInChordTimer.isActive();
		if (!appendToChord) {
			v->clearSelection();
			_mw->currentScoreViewPort()->clearSelection();
		}


		// from mainwin.cpp, ca. line 2241
		if ( _mw->uiTupletType->isChecked() ) {
			// insert a tuplet, first is the note, tuplet is filled with rests
			QList<CAPlayable*> elements;
			elements << static_cast<CAPlayable*>(note);
			for (int i=1; i<_mw->uiTupletNumber->value(); i++) {
				_mw->musElementFactory()->configureRest( voice, 0 );
				elements << static_cast<CAPlayable*>(_mw->musElementFactory()->musElement());
			}
			new CATuplet( _mw->uiTupletNumber->value(), _mw->uiTupletActualNumber->value(), elements );

		} else {
			// insert just a note
			voice->append( note, appendToChord );
		}


		voice->synchronizeMusElements();	// probably not needed
		v->addToSelection( (CADrawableMusElement*)(voice->lastMusElement()), true );	// todo: is not working as I expect it ...

		// QList<CAMusElement*> newEltList;
		// select paste elements
		// for (int i=0; i<newEltList.size(); i++)
		// 	currentScoreViewPort()->addToSelection( newEltList[i] );

		// verkraftet er nicht:  _mw->currentScoreViewPort()->addToSelection( voice->lastMusElement() );
		_mw->currentScoreViewPort()->repaint();

		QList<CADrawableMusElement*> list = v->selection();
		std::cout << " Selektierte Elemente: " << list.size() << " Stück" << std::endl;

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

