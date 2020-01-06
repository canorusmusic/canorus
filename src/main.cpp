/*!
	Copyright (c) 2006-2019, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QSplashScreen>

// Python.h needs to be loaded first!
#include "canorus.h"
#include "core/settings.h"
#include "interface/pluginmanager.h"
#include "ui/mainwin.h"
#include "ui/settingsdialog.h"

#include <iostream>

#ifndef Q_OS_WIN
#include <signal.h>

//Duma leads to a crash on libfontconfig with Ubuntu (10.04/12.04)
//#include "duma.h"

void catch_sig(int)
{
    qApp->quit();
}
#endif

/*!
	Main function. This is the first function called when Canorus is run.
	It initializes CACanorus class and creates the main window.
*/
int main(int argc, char* argv[])
{
    QApplication mainApp(argc, argv);

#ifdef Q_WS_X11
    signal(SIGINT, catch_sig);
    signal(SIGQUIT, catch_sig);
#endif

    CACanorus::initSearchPaths();

    QPixmap splashPixmap(400, 300);
    splashPixmap = QPixmap("images:splash.png");

    QSplashScreen splash(splashPixmap);
    QFont font("Century Schoolbook L");
    font.setPixelSize(17);
    splash.setFont(font);
    mainApp.processEvents();

    // Set main application properties
    CACanorus::initMain();

    // Parse switch and settings command line arguments
    if (!CACanorus::parseSettingsArguments(argc, argv))
        return 0;

    splash.show();

    // Load system translation if found
    CACanorus::initTranslations();

    // Init MIDI devices
    CACanorus::initPlayback();

    // Load config file
    bool firstTime = !QFile::exists(CASettings::defaultSettingsPath() + "/canorus.ini");
    CASettingsDialog::CASettingsPage showSettingsPage = CACanorus::initSettings();

    // Enable scripting and plugins subsystem
    splash.showMessage(QObject::tr("Initializing Scripting engine", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CACanorus::initScripting();

    // Finds all the plugins
    splash.showMessage(QObject::tr("Reading Plugins", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CAPluginManager::readPlugins();

    // Initialize autosave
    splash.showMessage(QObject::tr("Initializing Automatic recovery", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CACanorus::initAutoRecovery();

    // Initialize help
    splash.showMessage(QObject::tr("Initializing Help", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CACanorus::initHelp();

    // Initialize undo/redo stacks
    splash.showMessage(QObject::tr("Initializing Undo/Redo framework", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CACanorus::initUndo();

    // Load bundled fonts
    splash.showMessage(QObject::tr("Loading fonts", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CACanorus::initFonts();

    // Check for any crashed Canorus sessions and open the recovery files
    splash.showMessage(QObject::tr("Searching for recovery documents", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CACanorus::autoRecovery()->openRecovery();

    // Creates a main window of a document to open if passed in command line
    splash.showMessage(QObject::tr("Initializing Main window", "splashScreen"), Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    mainApp.processEvents();
    CACanorus::parseOpenFileArguments(argc, argv);

    // If no file to open is passed in command line, create a new default main window. It's shown automatically by CACanorus::addMainWin().
    if (!CACanorus::mainWinList().size()) {
        CAMainWin* mainWin = new CAMainWin();

        // Init dialogs etc.
        CACanorus::initCommonGUI(mainWin->uiSaveDialog,
            mainWin->uiOpenDialog,
            mainWin->uiExportDialog,
            mainWin->uiImportDialog);

        mainWin->newDocument();
        mainWin->show();

        // Init dialogs etc.
        CACanorus::initCommonGUI(mainWin->uiSaveDialog,
            mainWin->uiOpenDialog,
            mainWin->uiExportDialog,
            mainWin->uiImportDialog);

        if (firstTime) {
            mainWin->on_uiUsersGuide_triggered();
        }
    }
    splash.close();

    // Show settings dialog, if needed (eg. MIDI setup when running Canorus for the first time)
    if (showSettingsPage != CASettingsDialog::UndefinedSettings) {
        CASettingsDialog(showSettingsPage, CACanorus::mainWinList()[0]);
    }

    return mainApp.exec();
}
