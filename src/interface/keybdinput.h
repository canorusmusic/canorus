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

private:
	CAMainWin* _mw;
};

#endif /* KEYBDINPUT_H_ */
