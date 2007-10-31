class QThread {
public:
	bool isRunning();
protected:
	void run()=0;
};

class QXmlDefaultHandler {
};

class QObject {
};

%include "scripting/context.i"
%include "scripting/document.i"
%include "scripting/muselement.i"
%include "scripting/sheet.i"
%include "scripting/voice.i"
%include "scripting/playable.i"
%include "scripting/staff.i"
%include "scripting/note.i"
%include "scripting/rest.i"
%include "scripting/keysignature.i"
%include "scripting/timesignature.i"
%include "scripting/clef.i"
%include "scripting/barline.i"
%include "scripting/lyricscontext.i"
%include "scripting/syllable.i"
%include "scripting/slur.i"
%include "scripting/functionmarking.i"
%include "scripting/functionmarkingcontext.i"

%include "scripting/importexport.i"

%include "scripting/playback.i"
%include "scripting/mididevice.i"
%include "scripting/rtmididevice.i"
