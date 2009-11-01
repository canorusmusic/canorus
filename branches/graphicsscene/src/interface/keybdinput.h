/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	Copyright (c) 2008, Georg Rudolph
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef KEYBDINPUT_H_
#define KEYBDINPUT_H_

#include <QThread>
#include <QList>

#include "ui/mainwin.h"

class CAMainWin;

class CAKeybdInput {
public:
	CAKeybdInput( CAMainWin* m);
	~CAKeybdInput();
	void onMidiInEvent( QVector<unsigned char> m );

private:
	CAMainWin* _mw;
	void midiInEventToScore(CAScoreView *v, QVector<unsigned char> m);
	QTimer _midiInChordTimer;
	CASheet *_lastMidiInSheet;
	CAStaff *_lastMidiInStaff;
	CAVoice *_lastMidiInVoice;
	CADiatonicPitch _actualKeySignature;
	signed char _actualKeySignatureAccs[7];
	int _actualKeyAccidentalsSum;
	CADiatonicPitch matchPitchToKey( CAVoice *voice, CADiatonicPitch p );

	CAPlayable* _tupPla;
	CATuplet* _tup;
	QList<CAMusElement*> _noteLayout;

};

#endif /* KEYBDINPUT_H_ */
