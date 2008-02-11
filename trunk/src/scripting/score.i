/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%{
#include "core/document.h"
#include "core/sheet.h"

#include "core/context.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/functionmarkcontext.h"
#include "core/lyricscontext.h"

#include "core/playablelength.h"
#include "core/diatonicpitch.h"
#include "core/interval.h"
#include "core/diatonickey.h"

#include "core/muselement.h"
#include "core/playable.h"
#include "core/barline.h"
#include "core/clef.h"
#include "core/functionmark.h"
#include "core/keysignature.h"
#include "core/slur.h"
#include "core/syllable.h"
#include "core/timesignature.h"
#include "core/note.h"
#include "core/rest.h"
%}

%include "core/document.h"
%include "core/sheet.h"

%include "core/context.h"
%include "core/staff.h"
%include "core/voice.h"
%include "core/functionmarkcontext.h"
%include "core/lyricscontext.h"

%include "core/playablelength.h"
%include "core/diatonicpitch.h"
%include "core/interval.h"
%include "core/diatonickey.h"

%include "core/muselement.h"
%include "core/playable.h"
%include "core/barline.h"
%include "core/clef.h"
%include "core/functionmark.h"
%include "core/keysignature.h"
%include "core/slur.h"
%include "core/syllable.h"
%include "core/timesignature.h"
%include "core/note.h"
%include "core/rest.h"

%include "scripting/mark.i"
