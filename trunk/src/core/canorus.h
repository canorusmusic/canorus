/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/
 
#ifndef CANORUS_H_
#define CANORUS_H_

// Python.h needs to be loaded first!
#include "ui/mainwin.h"
#include "ui/settingsdialog.h"
#include "core/undocommand.h"
#include "core/autorecovery.h"

#include <QString>
#include <QList>
#include <QUndoStack>

class CASettings;
class CAMidiDevice;
class CADocument;

class CACanorus {
public:
	static void initMain();
	static CASettingsDialog::CASettingsPage initSettings();
	static void initCommonGUI();
	static void parseSettingsArguments(int argc, char *argv[]);
	static void initScripting();
	static void initAutoRecovery();
	static void parseOpenFileArguments(int argc, char *argv[]);
	
	static QList<QString> locateResource(const QString fileName);
	static QList<QString> locateResourceDir(const QString fileName);	
	
	inline static int mainWinCount() { return _mainWinList.size(); }
	static int mainWinCount(CADocument *);
	static QList<CAMainWin*> findMainWin(CADocument* document);
	inline static CAMainWin* mainWinAt(int idx) { return _mainWinList[idx]; }
	inline static void removeMainWin(CAMainWin *w) { _mainWinList.removeAll(w); }
	inline static void removeViewPort(CAViewPort *v) { for (int i=0; i<mainWinCount(); i++) _mainWinList[i]->removeViewPort(v); }
	static void addMainWin(CAMainWin *w, bool show=true);
	inline static void restartTimeEditedTimes(CADocument *doc) { for (int i=0; i<mainWinCount(); i++) if (mainWinAt(i)->document()==doc) mainWinAt(i)->restartTimeEditedTime(); }
	
	// Undo
	inline static bool containsUndoStack( CADocument *d ) { return _undoStack.contains(d); }
	inline static void setUndoStack( CADocument *d, QUndoStack *s ) { _undoStack[d]=s; }
	inline static QUndoStack *undoStack( CADocument* d ) { return _undoStack[d]; }
	inline static void removeUndoStack( CADocument *d ) { _undoStack.remove(d); }
	static void deleteUndoStack( CADocument *doc );
	static void createUndoCommand( CADocument *d, QString text );
	static void pushUndoCommand();
	inline static CAUndoCommand *lastUndoCommand( CADocument *d ) { return static_cast<CAUndoCommand*>(_lastUndoCommand[_undoStack[d]]); }
	static void updateLastUndoCommand( CAUndoCommand *c );
	
	inline static CASettings *settings() { return _settings; }
	inline static CAAutoRecovery *autoRecovery() { return _autoRecovery; }
	inline static QString settingsPath() { return _settingsPath; }
	inline static CAMidiDevice *midiDevice() { return _midiDevice; }
	inline static void setMidiDevice(CAMidiDevice *d) { _midiDevice = d; }
	
	static void rebuildUI( CADocument *document, CASheet *sheet );
	static void rebuildUI( CADocument *document=0 );
	
private:
	static QList<CAMainWin*> _mainWinList;
	static CASettings *_settings;
	static QString _settingsPath;
	
	// Undo
	static void clearUndoCommand();
	static QHash< CADocument*, QUndoStack* > _undoStack;
	static QHash< QUndoStack*, CAUndoCommand* > _lastUndoCommand;     // We need to relink the commands when pushing them, but there is no function in QUndoStack to get the topmost command on the stack.
	static QHash< CAUndoCommand*, CAUndoCommand* > _prevUndoCommands; // These two hashes are here to find the topmost commands when relinking.
	static CAUndoCommand *_undoCommand; // current undo command created to be put on the undo stack
	
	// Playback output
	static CAMidiDevice *_midiDevice;
	
	// Auto recovery
	static CAAutoRecovery *_autoRecovery;
};
#endif /* CANORUS_H_ */
