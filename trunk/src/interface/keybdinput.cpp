/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	Copyright (c) 2008, Georg Rudolph
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "interface/keybdinput.h"

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
	
*/

CAKeybdInput::CAKeybdInput (CAMainWin *mw) {
	_mw = mw;
}

/*!
	Destructor deletes the created arrays.
*/
CAKeybdInput::~CAKeybdInput() {
}

