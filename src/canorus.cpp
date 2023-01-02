/*!
    Copyright (c) 2007-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Python.h needs to be loaded first!
#include "scripting/swigpython.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QLocale>
#include <QMetaMethod>
#include <QTextCodec>
#include <QTranslator>

#include "canorus.h"
#include "control/helpctl.h"
#include "core/settings.h"
#include "core/undo.h"
#include "interface/rtmididevice.h"
#include "score/sheet.h"
#include "scripting/swigruby.h"
#include "ui/settingsdialog.h"

// define private static members
QList<CAMainWin*> CACanorus::_mainWinList;
CASettings* CACanorus::_settings;
CAAutoRecovery* CACanorus::_autoRecovery;
CAMidiDevice* CACanorus::_midiDevice;
CAUndo* CACanorus::_undo;
CAHelpCtl* CACanorus::_help;
QList<QString> CACanorus::_recentDocumentList;
QHash<QString, int> CACanorus::_fetaMap;
std::unique_ptr<QTranslator> CACanorus::_translator;

/*!
	Add all search paths.
	The search order is the following:
	- TODO passed path as an argument to exe
	- TODO path in user's config file
	- current dir
	- exe dir
	- DEFAULT_DATA_DIR set by compiler
*/

void CACanorus::initSearchPaths()
{
    QStringList categories, paths;
    categories << "images"
               << "fonts"
               << "plugins"
               << "scripts"
               << "doc"
               << "lang"
               << "base";
    for (QString cat: categories) {
        // The "base" prefix is used by CASwigPython and CASwigRuby to find files which are not in a subdir.
        QString dirname = (cat == "base") ? "" : cat;

        // When debugging, current directory may be used for resources.
        if (QDir(QDir::currentPath() + "/" + dirname).exists()) {
            QDir::addSearchPath(cat, QDir::currentPath() + "/" + dirname);
        }

        // Expected paths when running from canorus source.
        if (cat == "images" && QDir(qApp->applicationDirPath() + "/../../src/ui/images/").exists()) {
            QDir::addSearchPath(cat, qApp->applicationDirPath() + "/../../src/ui/images/");
        }
        if (cat == "fonts" && QDir(qApp->applicationDirPath() + "/../../src/fonts/").exists()) {
            QDir::addSearchPath(cat, qApp->applicationDirPath() + "/../../src/fonts/");
        }
        if (cat == "scripts" && QDir(qApp->applicationDirPath() + "/../../src/scripts/").exists()) {
            QDir::addSearchPath(cat, qApp->applicationDirPath() + "/../../src/scripts/");
        }
        if (cat == "doc" && QDir(qApp->applicationDirPath()).exists("/../../doc/")) {
            QDir::addSearchPath(cat, qApp->applicationDirPath() + "/../../doc");
        }

        // Expected paths when running from installation directory.
        if (QDir(qApp->applicationDirPath() + "/" + dirname).exists()) {
            QDir::addSearchPath(cat, qApp->applicationDirPath() + "/" + dirname);
        }
#ifdef DEFAULT_DATA_DIR
        if (QDir(DEFAULT_DATA_DIR + ("/" + dirname)).exists()) {
            QDir::addSearchPath(cat, DEFAULT_DATA_DIR + ("/" + dirname));
        }
#endif
    }
}

/*!
	Initializes application properties like application name, home page etc.
*/
void CACanorus::initMain(int, char* [])
{
    _autoRecovery = nullptr;

    // Init main application properties
    QCoreApplication::setOrganizationName("Canorus");
    QCoreApplication::setOrganizationDomain("canorus.org");
    QCoreApplication::setApplicationName("Canorus");
}

/*!
	Initializes language specific settings like the translation file for the GUI,
	text flow (left-to-right or right-to-left), default string encoding etc.
 */
void CACanorus::initTranslations()
{
    QString translationFile = "lang:" + QLocale::system().name() + ".qm"; // load language_COUNTRY.qm
    if (!QFileInfo(translationFile).exists())
        translationFile = "lang:" + QLocale::system().name().left(2) + ".qm"; // if not found, load language.qm

    if (QFileInfo(translationFile).exists()) {
        CACanorus::_translator = std::unique_ptr<QTranslator>(new QTranslator);
        CACanorus::_translator->load(QFileInfo(translationFile).absoluteFilePath());
        static_cast<QApplication*>(QApplication::instance())->installTranslator(CACanorus::_translator.get());
    }

    if (QLocale::system().language() == QLocale::Hebrew) { /// \todo add Arabic, etc.
        static_cast<QApplication*>(QApplication::instance())->setLayoutDirection(Qt::RightToLeft);
    }
}

void CACanorus::initCommonGUI(std::unique_ptr<QFileDialog>& uiSaveDialog,
    std::unique_ptr<QFileDialog>& uiOpenDialog,
    std::unique_ptr<QFileDialog>& uiExportDialog,
    std::unique_ptr<QFileDialog>& uiImportDialog)
{
    // Initialize main window's load/save/import/export dialogs
    qInfo() << "Entered initCommonGUI";
    uiSaveDialog = std::make_unique<QFileDialog>(nullptr, QObject::tr("Choose a file to save"), settings()->documentsDirectory().absolutePath());
    uiSaveDialog->setFileMode(QFileDialog::AnyFile);
    uiSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    uiSaveDialog->setNameFilters(QStringList() << CAFileFormats::CANORUSML_FILTER);
    uiSaveDialog->setNameFilters(uiSaveDialog->nameFilters() << CAFileFormats::CAN_FILTER);
    uiSaveDialog->selectNameFilter(CAFileFormats::getFilter(settings()->defaultSaveFormat()));

    uiOpenDialog = std::make_unique<QFileDialog>(nullptr, QObject::tr("Choose a file to open"), settings()->documentsDirectory().absolutePath());
    uiOpenDialog->setFileMode(QFileDialog::ExistingFile);
    uiOpenDialog->setAcceptMode(QFileDialog::AcceptOpen);
    uiOpenDialog->setNameFilters(QStringList() << CAFileFormats::CANORUSML_FILTER); // clear the * filter
    uiOpenDialog->setNameFilters(uiOpenDialog->nameFilters() << CAFileFormats::CAN_FILTER);
    QString allFilters; // generate list of all files
    for (int i = 0; i < uiOpenDialog->nameFilters().size(); i++) {
        QString curFilter = uiOpenDialog->nameFilters()[i];
        int left = curFilter.indexOf('(') + 1;
        allFilters += curFilter.mid(left, curFilter.size() - left - 1) + " ";
    }
    allFilters.chop(1);
    uiOpenDialog->setNameFilters(QStringList() << QString(QObject::tr("All supported formats (%1)").arg(allFilters)) << uiOpenDialog->nameFilters());

    uiExportDialog = std::make_unique<QFileDialog>(nullptr, QObject::tr("Choose a file to export"), settings()->documentsDirectory().absolutePath());
    uiExportDialog->setFileMode(QFileDialog::AnyFile);
    uiExportDialog->setAcceptMode(QFileDialog::AcceptSave);
    uiExportDialog->setNameFilters(QStringList() << CAFileFormats::LILYPOND_FILTER);
    uiExportDialog->setNameFilters(uiExportDialog->nameFilters() << CAFileFormats::MUSICXML_FILTER);
    uiExportDialog->setNameFilters(uiExportDialog->nameFilters() << CAFileFormats::MIDI_FILTER);
    uiExportDialog->setNameFilters(uiExportDialog->nameFilters() << CAFileFormats::PDF_FILTER);
    uiExportDialog->setNameFilters(uiExportDialog->nameFilters() << CAFileFormats::SVG_FILTER);

    uiImportDialog = std::make_unique<QFileDialog>(nullptr, QObject::tr("Choose a file to import"), settings()->documentsDirectory().absolutePath());
    uiImportDialog->setFileMode(QFileDialog::ExistingFile);
    uiImportDialog->setAcceptMode(QFileDialog::AcceptOpen);
    uiImportDialog->setNameFilters(QStringList() << CAFileFormats::MUSICXML_FILTER);
    uiImportDialog->setNameFilters(uiImportDialog->nameFilters() << CAFileFormats::MXL_FILTER);
    uiImportDialog->setNameFilters(uiImportDialog->nameFilters() << CAFileFormats::MIDI_FILTER);
    // uiImportDialog->setNameFilters( uiImportDialog->nameFilters() << CAFileFormats::LILYPOND_FILTER ); // activate when usable
}

/*!
	Initializes playback devices.
*/
void CACanorus::initPlayback()
{
    qRegisterMetaType<QVector<unsigned char>>("QVector< unsigned char >");
    setMidiDevice(new CARtMidiDevice());
}

/*!
	Opens Canorus config file and loads the settings.
	Config file is always INI file in user's home directory.
	No native formats are used (Windows registry etc.) - this is provided for easier transition of settings between the platforms.

	\sa settings()
*/
CASettingsDialog::CASettingsPage CACanorus::initSettings()
{
    _settings = new CASettings();

    switch (_settings->readSettings()) {
    case -1:
        return CASettingsDialog::PlaybackSettings;
    default:
        return CASettingsDialog::UndefinedSettings;
    }
}

/*!
	Initializes scripting and plugins subsystem.
*/
void CACanorus::initScripting()
{
#ifdef USE_RUBY
    CASwigRuby::init();
#endif
#ifdef USE_PYTHON
    CASwigPython::init();
#endif
}

/*!
	Initializes recovery saving.
*/
void CACanorus::initAutoRecovery()
{
    _autoRecovery = new CAAutoRecovery();
}

void CACanorus::initUndo()
{
    _undo = new CAUndo();
}

void CACanorus::initFonts()
{
    // addApplicationFont doesn't understand prefix: paths.
    QFontDatabase::addApplicationFont(QFileInfo("fonts:CenturySchL-Roma.ttf").absoluteFilePath());
    QFontDatabase::addApplicationFont(QFileInfo("fonts:CenturySchL-Ital.ttf").absoluteFilePath());
    QFontDatabase::addApplicationFont(QFileInfo("fonts:CenturySchL-Bold.ttf").absoluteFilePath());
    QFontDatabase::addApplicationFont(QFileInfo("fonts:CenturySchL-BoldItal.ttf").absoluteFilePath());
    QFontDatabase::addApplicationFont(QFileInfo("fonts:FreeSans.ttf").absoluteFilePath());
    QFontDatabase::addApplicationFont(QFileInfo("fonts:Emmentaler-14.ttf").absoluteFilePath());
// populate glyph->codepoint map using generated list
#include "fonts/fetaList.cxx"
}

/*!
	Returns codepoint for an Feta (Emmentaler) glyph by its name.
 */
int CACanorus::fetaCodepoint(const QString& name)
{
    return _fetaMap[name];
}

void CACanorus::initHelp()
{
    _help = new CAHelpCtl();
}

void CACanorus::insertRecentDocument(QString filename)
{
    if (recentDocumentList().contains(filename))
        removeRecentDocument(filename);

    recentDocumentList().prepend(filename);

    if (recentDocumentList().size() > settings()->maxRecentDocuments())
        recentDocumentList().removeLast();

    settings()->writeSettings();
}

void CACanorus::addRecentDocument(QString filename)
{
    recentDocumentList() << filename;
}

void CACanorus::removeRecentDocument(QString filename)
{
    recentDocumentList().removeAll(filename);
}

/*!
	Free resources before quitting
*/
void CACanorus::cleanUp()
{
    delete _settings;
    delete _midiDevice;
    autoRecovery()->cleanupRecovery();
    delete _autoRecovery;
    delete _undo;
}

/*!
	Parses the switches and settings command line arguments to application.
	This function sets any settings passed in command line.

	Returns True, if application should resume with loading or False, if such
	a switch was passed.

	\sa parseOpenFileArguments()
*/
bool CACanorus::parseSettingsArguments(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        if (QString(argv[i]) == "--version") {
            std::cout << "Canorus - The next generation music score editor" << std::endl
                      << "Version " << CANORUS_VERSION << std::endl;

            return false;
        }
    }

    return true;
}

/*!
	This function parses any arguments which doesn't look like switch or a setting.
	It creates a new main window and opens a file if a file is passed in the command line.
*/
void CACanorus::parseOpenFileArguments(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') { // automatically treat any argument which doesn't start with '-' to be a file name - \todo
            // passed is not the switch but a file name
            QFileInfo file(argv[i]);

            if (!file.exists())
                continue;

            std::unique_ptr<CAMainWin> mainWin = std::make_unique<CAMainWin>();
            mainWin->openDocument(file.absoluteFilePath());
            mainWin->show();
        }
    }
}

/*!
	\fn int CACanorus::mainWinCount()

	Returns the number of all main windows.
*/

/*!
	Returns number of main windows which have the given document opened.
*/
int CACanorus::mainWinCount(CADocument* doc)
{
    int count = 0;
    for (int i = 0; i < _mainWinList.size(); i++)
        if (_mainWinList[i]->document() == doc)
            count++;

    return count;
}

/*!
	Rebuilds main windows with the given \a document and its views showing the given \a sheet.

	\sa rebuildUI(CADocument*), CAMainWin::rebuildUI()
*/
void CACanorus::rebuildUI(CADocument* document, CASheet* sheet)
{
    for (int i = 0; i < mainWinList().size(); i++)
        if (mainWinList()[i]->document() == document)
            mainWinList()[i]->rebuildUI(sheet);
}

/*!
	Rebuilds main windows with the given \a document.
	Rebuilds all main windows, if \a document is not given or null.

	\sa rebuildUI(CADocument*, CASheet*), CAMainWin::rebuildUI()
*/
void CACanorus::rebuildUI(CADocument* document)
{
    for (int i = 0; i < mainWinList().size(); i++) {
        if (document && mainWinList()[i]->document() == document) {
            mainWinList()[i]->rebuildUI();
        } else if (!document)
            mainWinList()[i]->rebuildUI();
    }
}

/*!
	Repaints all main window.
	This is useful for example if only selection was changed by external event (eg. plugin)
	and the GUI should be repainted, but not rebuilt.
 */
void CACanorus::repaintUI()
{
    for (int i = 0; i < mainWinList().size(); i++) {
        mainWinList()[i]->repaint();
    }
}

/*!
	Finds and returns a list of main windows containing the given document.
*/
QList<CAMainWin*> CACanorus::findMainWin(CADocument* document)
{
    QList<CAMainWin*> mainWinList;
    for (int i = 0; i < CACanorus::mainWinList().size(); i++)
        if (CACanorus::mainWinList()[i]->document() == document)
            mainWinList << CACanorus::mainWinList()[i];

    return mainWinList;
}

/*!
	\function void CACanorus::addMainWin( CAMainWin *w )
	Adds an already created main window to the global main window list.

	\sa removeMainWin(), mainWinAt()
*/

/*!
	Searches recursively for all child objects of the given object, and connects matching signals from them to slots of object that follow the following form:
	void on_<widget name>_<signal name>(<signal parameters>);
	Let's assume our object has a child object of type QPushButton with the object name 	button1. The slot to catch the button's clicked() signal would be:
	void on_button1_clicked();

	This enhanced function allows to precisely define in which object the signal (pOS)
	is defined and in which object the slot (poR) is defined.

	\sa QObject::setObjectName()
	\sa QMetaObject::connectSlotsByName(QObject *o)
*/
void CACanorus::connectSlotsByName(QObject* pOS, const QObject* pOR)
{
    if (!pOS || !pOR)
        return;
    const QMetaObject* mo = pOR->metaObject();
    Q_ASSERT(mo);
    const QObjectList list = pOS->findChildren<QObject*>(QString());
    for (int i = 0; i < mo->methodCount(); ++i) {
        const char* slot = mo->method(i).methodSignature().data();
        Q_ASSERT(slot);
        if (slot[0] != 'o' || slot[1] != 'n' || slot[2] != '_')
            continue;
        bool foundIt = false, foundObj = false;
        if (list.isEmpty())
            qWarning("CACanorus::connectSlotsByName: Object list empty!");
        for (int j = 0; j < list.count(); ++j) {
            const QObject* co = list.at(j);
            QByteArray objName = co->objectName().toLatin1();
            unsigned int len = static_cast<unsigned int>(objName.length());
            if (!len || qstrncmp(slot + 3, objName.data(), len) || slot[len + 3] != '_')
                continue;
            foundObj = true;
            const QMetaObject* smo = co->metaObject();
            int sigIndex = smo->indexOfMethod(slot + len + 4);
            if (sigIndex < 0) { // search for compatible signals
                unsigned int slotlen = qstrlen(slot + len + 4) - 1;
                for (int k = 0; k < co->metaObject()->methodCount(); ++k) {
                    if (smo->method(k).methodType() != QMetaMethod::Signal)
                        continue;

                    if (!qstrncmp(smo->method(k).methodSignature().data(), slot + len + 4, slotlen)) {
                        sigIndex = k;
                        break;
                    }
                }
            }
            if (sigIndex < 0)
                continue;
            if (QMetaObject::connect(co, sigIndex, pOR, i)) {
                foundIt = true;
                break;
            }
        }
        if (foundIt) {
            // we found our slot, now skip all overloads
            while (mo->method(i + 1).attributes() & QMetaMethod::Cloned)
                ++i;
        } else if (!(mo->method(i).attributes() & QMetaMethod::Cloned)) {
            if (foundObj)
                qWarning("CACanorus::connectSlotsByName: No matching signal for %s", slot);
            else
                qWarning("CACanorus::connectSlotsByName: No matching object for %s", slot);
        }
    }
}
