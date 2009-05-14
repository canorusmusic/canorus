/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%{
#include "score/document.h"
#include "score/sheet.h"

#include "score/context.h"
#include "score/staff.h"
#include "score/voice.h"
#include "score/functionmarkcontext.h"
#include "score/lyricscontext.h"

#include "score/playablelength.h"
#include "score/diatonicpitch.h"
#include "score/interval.h"
#include "score/diatonickey.h"
#include "core/transpose.h"

#include "score/muselement.h"
#include "score/playable.h"
#include "score/barline.h"
#include "score/clef.h"
#include "score/functionmark.h"
#include "score/keysignature.h"
#include "score/slur.h"
#include "score/tuplet.h"
#include "score/syllable.h"
#include "score/timesignature.h"
#include "score/note.h"
#include "score/rest.h"
#include "score/midinote.h"
%}

%include "score/document.h"
%include "score/sheet.h"

%include "score/context.h"
%include "score/staff.h"
%include "score/voice.h"
%include "score/functionmarkcontext.h"
%include "score/lyricscontext.h"

%include "score/playablelength.h"
%include "score/diatonicpitch.h"
%include "score/interval.h"
%include "score/diatonickey.h"
%include "core/transpose.h"

%include "score/muselement.h"
%include "score/playable.h"
%include "score/barline.h"
%include "score/clef.h"
%include "score/functionmark.h"
%include "score/keysignature.h"
%include "score/slur.h"
%include "score/tuplet.h"
%include "score/syllable.h"
%include "score/timesignature.h"
%include "score/note.h"
%include "score/rest.h"
%include "score/midinote.h"

%include "scripting/mark.i"
