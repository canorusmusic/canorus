/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef CANORUS_H_
#define CANORUS_H_

// Python.h needs to be loaded first!
#include "ui/settingsdialog.h"
#include "ui/mainwin.h"
#include "core/autorecovery.h"

#include <QString>
#include <QList>
#include <QUndoStack>
#include <QHash>

class CASettings;
class CAMidiDevice;
class CADocument;
class CAUndo;
class CAHelpCtl;

class CACanorus {
public:
	static void initMain( int argc=0, char *argv[]=0 );
	static CASettingsDialog::CASettingsPage initSettings();
	static void initCommonGUI();
	static void initPlayback();
	static bool parseSettingsArguments(int argc, char *argv[]);
	static void initScripting();
	static void initAutoRecovery();
	static void initUndo();
	static void initSearchPaths();
	static void initFonts();
	static void initHelp();
	static void parseOpenFileArguments(int argc, char *argv[]);
	static void cleanUp();

	static int  fetaCodepoint(const QString& name);

	inline static int mainWinCount() { return _mainWinList.size(); }
	static int mainWinCount(CADocument *);
	static QList<CAMainWin*> findMainWin(CADocument* document);
	inline static CAMainWin* mainWinAt(int idx) { return _mainWinList[idx]; }
	inline static void removeMainWin(CAMainWin *w) { _mainWinList.removeAll(w); }
	inline static void removeViewPort(CAViewPort *v) { for (int i=0; i<mainWinCount(); i++) _mainWinList[i]->removeViewPort(v); }
	inline static void addMainWin( CAMainWin *w ) { _mainWinList << w; }
	inline static void restartTimeEditedTimes(CADocument *doc) { for (int i=0; i<mainWinCount(); i++) if (mainWinAt(i)->document()==doc) mainWinAt(i)->restartTimeEditedTime(); }

	inline static CAUndo *undo() { return _undo; }

	static void addRecentDocument( QString );
	static void insertRecentDocument( QString );
	static void removeRecentDocument( QString );
	inline static QList<QString>& recentDocumentList() { return _recentDocumentList; }

	inline static CASettings *settings() { return _settings; }
	inline static CAAutoRecovery *autoRecovery() { return _autoRecovery; }
	inline static CAMidiDevice *midiDevice() { return _midiDevice; }
	inline static void setMidiDevice(CAMidiDevice *d) { _midiDevice = d; }

	inline static CAHelpCtl *help() { return _help; }

	static void rebuildUI( CADocument *document, CASheet *sheet );
	static void rebuildUI( CADocument *document=0 );

	// Our own slot connection method
	static void connectSlotsByName(QObject *pOS, const QObject *pOR);

private:
	static QList<CAMainWin*> _mainWinList;
	static CASettings *_settings;
	static CAUndo *_undo;
	static QList<QString> _recentDocumentList;
	static QHash<QString, int> _fetaMap;

	// Playback output
	static CAMidiDevice *_midiDevice;

	// Auto recovery
	static CAAutoRecovery *_autoRecovery;

	// Help
	static CAHelpCtl *_help;
};
#endif /* CANORUS_H_ */
