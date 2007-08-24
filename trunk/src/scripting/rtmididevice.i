/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%{
#include "interface/rtmididevice.h"
%}

%rename(RtMidiDevice) CARtMidiDevice;
%include "interface/rtmididevice.h"
