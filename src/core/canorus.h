/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/
 
#ifndef CANORUS_H_
#define CANORUS_H_

// Python.h needs to be loaded first!
#include "ui/mainwin.h"

#include <QString>
#include <QList>

class QSettings;
class CAMidiDevice;

class CACanorus {
public:
	static void initMain();
	static void initSettings();
	static void parseSettingsArguments(int argc, char *argv[]);
	static void initMidi();
	static void initScripting();
	static void parseOpenFileArguments(int argc, char *argv[]);
	
	static QList<QString> locateResource(const QString fileName);
	static QList<QString> locateResourceDir(const QString fileName);	
	
	inline static int mainWinCount() { return _mainWinList.size(); }
	static int mainWinCount(CADocument *);
	static QList<CAMainWin*> findMainWin(CADocument* document);
	inline static CAMainWin* mainWinAt(int idx) { return _mainWinList[idx]; }
	inline static void removeMainWin(CAMainWin *w) { _mainWinList.removeAll(w); }
	inline static void removeViewPort(CAViewPort *v) { for (int i=0; i<mainWinCount(); i++) _mainWinList[i]->removeViewPort(v); }
	inline static void addMainWin(CAMainWin *w, bool show=true) { _mainWinList << w; if (show) w->show(); }
	inline static void restartTimeEditedTimes(CADocument *doc) { for (int i=0; i<mainWinCount(); i++) if (mainWinAt(i)->document()==doc) mainWinAt(i)->restartTimeEditedTime(); }
	
	inline static QSettings *settings() { return _settings; }
	inline static CAMidiDevice *midiDevice() { return _midiDevice; }
	inline static void setMidiDevice(CAMidiDevice *d) { _midiDevice = d; }
	inline static int midiInPort() { return _midiInPort; }
	inline static int midiOutPort() { return _midiOutPort; }
	
	inline static void setMidiInPort(int in) { _midiInPort = in; }
	inline static void setMidiOutPort(int out) { _midiOutPort = out; }
	
	static void rebuildUI(CADocument *document=0, CASheet *sheet=0);
	
private:
	static QList<CAMainWin*> _mainWinList;
	static QSettings *_settings;
	
	/////////////////////
	// Playback output //
	/////////////////////
	static CAMidiDevice *_midiDevice;
	static int _midiOutPort; //-1 disabled, 0+ port number
	static int _midiInPort;  //-1 disabled, 0+ port number
};
#endif /* CANORUS_H_*/
