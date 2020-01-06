/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef CANORUS_H_
#define CANORUS_H_

// std::unique_ptr for old Qt LTS 5.9.x
// Do not move below mainwin!
#include <iostream> // verbose stuff
#include <memory>

// Python.h needs to be loaded first!
#include "core/autorecovery.h"
#include "ui/mainwin.h"
#include "ui/settingsdialog.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QUndoStack>
#include <memory>

//Duma leads to a crash on libfontconfig with Ubuntu (10.04/12.04)
//#include "duma.h"

class CASettings;
class CAMidiDevice;
class CADocument;
class CAUndo;
class CAHelpCtl;

class CACanorus {
public:
    static void initMain(int argc = 0, char* argv[] = nullptr);
    static CASettingsDialog::CASettingsPage initSettings();
    static void initTranslations();
    static void initCommonGUI(std::unique_ptr<QFileDialog>& uiSaveDialog,
        std::unique_ptr<QFileDialog>& uiOpenDialog,
        std::unique_ptr<QFileDialog>& uiExportDialog,
        std::unique_ptr<QFileDialog>& uiImportDialog);
    static void initPlayback();
    static bool parseSettingsArguments(int argc, char* argv[]);
    static void initScripting();
    static void initAutoRecovery();
    static void initUndo();
    static void initSearchPaths();
    static void initFonts();
    static void initHelp();
    static void parseOpenFileArguments(int argc, char* argv[]);
    static void cleanUp();

    static int fetaCodepoint(const QString& name);

    inline static const QList<CAMainWin*>& mainWinList() { return _mainWinList; }
    inline static void addMainWin(CAMainWin* w) { _mainWinList << w; }
    inline static void removeMainWin(CAMainWin* w) { _mainWinList.removeAll(w); }
    static int mainWinCount(CADocument*);
    static QList<CAMainWin*> findMainWin(CADocument* document);

    inline static void removeView(CAView* v)
    {
        for (int i = 0; i < mainWinList().size(); i++)
            _mainWinList[i]->removeView(v);
    }
    inline static void restartTimeEditedTimes(CADocument* doc)
    {
        for (int i = 0; i < mainWinList().size(); i++)
            if (mainWinList()[i]->document() == doc)
                mainWinList()[i]->restartTimeEditedTime();
    }

    inline static CAUndo* undo() { return _undo; }

    static void addRecentDocument(QString);
    static void insertRecentDocument(QString);
    static void removeRecentDocument(QString);
    inline static QList<QString>& recentDocumentList() { return _recentDocumentList; }

    inline static CASettings* settings() { return _settings; }
    inline static CAAutoRecovery* autoRecovery() { return _autoRecovery; }
    inline static CAMidiDevice* midiDevice() { return _midiDevice; }
    inline static void setMidiDevice(CAMidiDevice* d) { _midiDevice = d; }

    inline static CAHelpCtl* help() { return _help; }

    static void rebuildUI(CADocument* document, CASheet* sheet);
    static void rebuildUI(CADocument* document = nullptr);
    static void repaintUI();

    // Our own slot connection method
    static void connectSlotsByName(QObject* pOS, const QObject* pOR);

    // Canorus specific names of const properties for actions
    static const char* propCommand() { return "Command"; }
    static const char* propContext() { return "Context"; }
    static const char* propDescription() { return "Description"; }
    static const char* propShortCut() { return "ShortCut"; }
    static const char* propMidiCommand() { return "MidiCommand"; }
    static const char* propConflicts() { return "Conflicts"; }

private:
    static QList<CAMainWin*> _mainWinList;
    static CASettings* _settings;
    static CAUndo* _undo;
    static QList<QString> _recentDocumentList;
    static QHash<QString, int> _fetaMap;

    // Playback output
    static CAMidiDevice* _midiDevice;

    // Auto recovery
    static CAAutoRecovery* _autoRecovery;

    // Help
    static CAHelpCtl* _help;
};
#endif /* CANORUS_H_ */
