class QThread {
public:
	bool isRunning();
	bool wait ( unsigned long time = ULONG_MAX );
protected:
	void run()=0;
};

class QXmlDefaultHandler {
};

class QObject {
};

class QSettings {
};

// resources
%include "scripting/resources.i"

// score
%include "scripting/score.i"

// playback
%include "scripting/playback.i"
%include "scripting/mididevice.i"
%include "scripting/rtmididevice.i"

// io
%include "scripting/importexport.i"

// console
%include "scripting/pyconsoleinterface.i"
