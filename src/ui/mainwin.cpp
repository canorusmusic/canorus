/*!
	Copyright (c) 2006-2020, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

//#include <Python.h> must be called before standard headers inclusion. See http://docs.python.org/api/includes.html
#ifdef USE_PYTHON
#include <Python.h>
#endif

#include <QCheckBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPoint>
#include <QSlider>
#include <QString>
#include <QTextStream>
#include <QThread>
#include <QToolBar>
#include <QWheelEvent>
#include <QXmlInputSource>
#include <QtGui>
#include <iostream>
#include <limits>

#include "ui/actionstorage.h"
#include "ui/jumptoview.h"
#include "ui/mainwin.h"
#include "ui/propertiesdialog.h"
#include "ui/settingsdialog.h"
#include "ui/transposeview.h"

#include "scoreui/keysignatureui.h"

#include "scorectl/keysignaturectl.h"

#include "control/helpctl.h"
#include "control/previewctl.h"
#include "control/printctl.h"
#include "control/resourcectl.h"

#include "interface/keybdinput.h"
#include "interface/mididevice.h"
#include "interface/playback.h"
#include "interface/pluginmanager.h"
#include "interface/rtmididevice.h"

#include "widgets/lcdnumber.h"
#include "widgets/menutoolbutton.h"
#include "widgets/midirecorderview.h"
#include "widgets/undotoolbutton.h"
#ifdef QT_WEBENGINEWIDGETS_LIB
#include "widgets/helpbrowser.h"
#endif

#include "widgets/pyconsole.h"
#include "widgets/scoreview.h"
#include "widgets/sourceview.h"
#include "widgets/view.h"
#include "widgets/viewcontainer.h"

#include "layout/drawablecontext.h"
#include "layout/drawablefiguredbassnumber.h"
#include "layout/drawablekeysignature.h"
#include "layout/drawablelyricscontext.h"
#include "layout/drawablemuselement.h"
#include "layout/drawablenote.h"
#include "layout/drawablestaff.h"
#include "layout/layoutengine.h"

#include "canorus.h"
#include "core/midirecorder.h"
#include "core/mimedata.h"
#include "core/muselementfactory.h"
#include "core/settings.h"
#include "core/undo.h"
#include "score/articulation.h"
#include "score/barline.h"
#include "score/bookmark.h"
#include "score/chordname.h"
#include "score/chordnamecontext.h"
#include "score/clef.h"
#include "score/dynamic.h"
#include "score/figuredbasscontext.h"
#include "score/figuredbassmark.h"
#include "score/fingering.h"
#include "score/functionmark.h"
#include "score/functionmarkcontext.h"
#include "score/instrumentchange.h"
#include "score/keysignature.h"
#include "score/lyricscontext.h"
#include "score/note.h"
#include "score/repeatmark.h"
#include "score/rest.h"
#include "score/ritardando.h"
#include "score/sheet.h"
#include "score/slur.h"
#include "score/staff.h"
#include "score/syllable.h"
#include "score/text.h"
#include "score/timesignature.h"
#include "score/voice.h"

#include "scripting/swigpython.h"
#include "scripting/swigruby.h"

#include "core/notechecker.h"
#include "export/canexport.h"
#include "export/canorusmlexport.h"
#include "export/export.h"
#include "export/lilypondexport.h"
#include "export/midiexport.h"
#include "export/musicxmlexport.h"
#include "export/pdfexport.h"
#include "export/svgexport.h"
#include "import/canimport.h"
#include "import/canorusmlimport.h"
#include "import/lilypondimport.h"
#include "import/midiimport.h"
#include "import/musicxmlimport.h"
#include "import/mxlimport.h"

/*!
	\class CAMainWin
	\brief Canorus main window
	Class CAMainWin represents Canorus main window.
	The core layout is generated using the Qt designer's ui/mainwin.ui file.
	Other widgets (specific toolbars, Views, plugin menus) are generated manually in-code.

	Canorus supports multiple main windows pointing to the same document or separated document.

	Canorus uses multiple inheritance approach. See
	http://doc.trolltech.com/4.2/designer-using-a-component.html#the-multiple-inheritance-approach
	Class members having _ prefix are general private properties.
	Private attributes with ui prefix are GUI-only widgets objects created in Qt designer or manually.

	\sa CAView, CACanorus
*/

/*!
	Default constructor.
	Creates Canorus main window with parent \a oParent. Parent is usually null.
*/
CAMainWin::CAMainWin(QMainWindow* oParent)
    : QMainWindow(oParent)
    , _mode(NoDocumentMode)
    , _mainWinProgressCtl(this)
    , _playbackView(nullptr)
    , _repaintTimer(nullptr)
    , _playback(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    _iNumAllowed = 1;

    // Create the GUI (actions, toolbars, menus etc.)
    createCustomActions();
    setupUi(this); // initialize elements created by Qt Designer
    actionStorage = new CAActionStorage(); // Shortcut System
    setupCustomUi();

    // Explicitly initialize this so it isn't true sometimes
    setRebuildUILock(false);

    // Initialize internal UI properties
    uiLockScrollPlayback->setChecked(CACanorus::settings()->lockScrollPlayback());

    // Create plugins menus and toolbars in this main window
    CAPluginManager::enablePlugins(this);

    // Connects MIDI IN callback function to a local slot. Not implemented yet.
    connect(CACanorus::midiDevice(), SIGNAL(midiInEvent(QVector<unsigned char>)), this, SLOT(onMidiInEvent(QVector<unsigned char>)));

    // Connect QTimer so it increases the local document edited time every second
    restartTimeEditedTime();
    connect(&_timeEditedTimer, SIGNAL(timeout()), this, SLOT(onTimeEditedTimerTimeout()));
    _timeEditedTimer.start(1000);

    // Setup the midi keyboad input processing object
    _keybdInput = new CAKeybdInput(this);

    _resourceView = new CAResourceView(nullptr, nullptr);
    _resourceView->hide();

    // Tools
    _midiRecorderView = nullptr;

    _transposeView = new CATransposeView(this);
    addDockWidget(Qt::RightDockWidgetArea, _transposeView);
    _transposeView->hide();

    _jumpToView = new CAJumpToView(this);

    _permanentStatusBar = statusBar();

    setDocument(nullptr);
    _poExp = nullptr;
    CACanorus::addMainWin(this);
}

CAMainWin::~CAMainWin()
{
    delete _musElementFactory;

    if (document() && CACanorus::mainWinCount(document()) == 1) {
        CACanorus::undo()->deleteUndoStack(document()); // delete undo stack when the last document deleted
        delete document();
    }

    CACanorus::removeMainWin(this); // must be called *after* CACanorus::deleteUndoStack()
    if (_playback)
        delete _playback;

    delete _poPrintCtl;
    delete _poPrintPreviewCtl;
    // clear UI
    delete uiInsertToolBar; // also deletes content of the toolbars
    delete uiInsertGroup;

    delete uiVoiceToolBar;
    delete uiPlayableToolBar;
    delete uiTimeSigToolBar;
    delete uiClefToolBar;
    delete uiFBMToolBar;
    delete uiFMToolBar;
    delete uiDynamicToolBar;
    delete uiInstrumentToolBar;
    delete uiTempoToolBar;
    delete uiFermataToolBar;
    delete uiRepeatMarkToolBar;

    delete _resourceView;
    delete _transposeView;

    if (_poExp)
        delete _poExp;

    if (_midiRecorderView)
        delete _midiRecorderView;

    if (!CACanorus::mainWinList().size()) // closing down
        CACanorus::cleanUp();
}

void CAMainWin::createCustomActions()
{
    // Toolbars
    uiUndo = new CAUndoToolButton(QIcon("images:general/editundo.png"), CAUndoToolButton::Undo, this);
    uiUndo->setObjectName("uiUndo");
    uiRedo = new CAUndoToolButton(QIcon("images:general/editredo.png"), CAUndoToolButton::Redo, this);
    uiRedo->setObjectName("uiRedo");

    uiInsertToolBar = new QToolBar(tr("Insert ToolBar"), this);
    uiContextType = new CAMenuToolButton(tr("Select Context"), 2, this);
    uiContextType->setObjectName("uiContextType");
    uiContextType->addButton(QIcon("images:document/staffnew.svg"), CAContext::Staff, tr("New Staff"));
    uiContextType->addButton(QIcon("images:document/lyricscontextnew.svg"), CAContext::LyricsContext, tr("New Lyrics context"));
    uiContextType->addButton(QIcon("images:document/chordnamecontextnew.svg"), CAContext::ChordNameContext, tr("New Chord Name context"));
    uiContextType->addButton(QIcon("images:document/fbcontextnew.svg"), CAContext::FiguredBassContext, tr("New Figured Bass context"));
    uiContextType->addButton(QIcon("images:document/fmcontextnew.svg"), CAContext::FunctionMarkContext, tr("New Function Mark context"));
    uiSlurType = new CAMenuToolButton(tr("Select Slur Type"), 3, this);
    uiSlurType->setObjectName("uiSlurType");
    uiSlurType->addButton(QIcon("images:slur/tie.svg"), CASlur::TieType, tr("Tie"));
    uiSlurType->addButton(QIcon("images:slur/slur.svg"), CASlur::SlurType, tr("Slur"));
    uiSlurType->addButton(QIcon("images:slur/phrasingslur.svg"), CASlur::PhrasingSlurType, tr("Phrasing Slur"));
    uiClefType = new CAMenuToolButton(tr("Select Clef"), 5, this);
    uiClefType->setObjectName("uiClefType");
    uiClefType->addButton(QIcon("images:clef/clefg.svg"), CAClef::Treble, tr("Treble Clef"));
    uiClefType->addButton(QIcon("images:clef/clefg.svg"), CAClef::French, tr("French Clef"));
    uiClefType->addButton(QIcon("images:clef/cleff.svg"), CAClef::Bass, tr("Bass Clef"));
    uiClefType->addButton(QIcon("images:clef/cleff.svg"), CAClef::Varbaritone, tr("Varbaritone Clef"));
    uiClefType->addButton(QIcon("images:clef/cleff.svg"), CAClef::Subbass, tr("Subbass Clef"));
    uiClefType->addButton(QIcon("images:clef/clefc.svg"), CAClef::Soprano, tr("Soprano Clef"));
    uiClefType->addButton(QIcon("images:clef/clefc.svg"), CAClef::Mezzosoprano, tr("Mezzosoprano Clef"));
    uiClefType->addButton(QIcon("images:clef/clefc.svg"), CAClef::Alto, tr("Alto Clef"));
    uiClefType->addButton(QIcon("images:clef/clefc.svg"), CAClef::Tenor, tr("Tenor Clef"));
    uiClefType->addButton(QIcon("images:clef/clefc.svg"), CAClef::Baritone, tr("Baritone Clef"));
    uiTimeSigType = new CAMenuToolButton(tr("Select Time Signature"), 3, this);
    uiTimeSigType->setObjectName("uiTimeSigType");
    uiTimeSigType->addButton(QIcon("images:timesig/tsc.svg"), 44);
    uiTimeSigType->addButton(QIcon("images:timesig/tsab.svg"), 22);
    uiTimeSigType->addButton(QIcon("images:timesig/ts34.svg"), 34);
    uiTimeSigType->addButton(QIcon("images:timesig/ts24.svg"), 24);
    uiTimeSigType->addButton(QIcon("images:timesig/ts38.svg"), 38);
    uiTimeSigType->addButton(QIcon("images:timesig/ts68.svg"), 68);
    uiTimeSigType->addButton(QIcon("images:timesig/tscustom.svg"), 0);
    uiBarlineType = new CAMenuToolButton(tr("Select Barline"), 4, this);
    uiBarlineType->setObjectName("uiBarlineType");
    uiBarlineType->addButton(QIcon("images:barline/barlinesingle.svg"), CABarline::Single, tr("Single Barline"));
    uiBarlineType->addButton(QIcon("images:barline/barlinedouble.svg"), CABarline::Double, tr("Double Barline"));
    uiBarlineType->addButton(QIcon("images:barline/barlineend.svg"), CABarline::End, tr("End Barline"));
    uiBarlineType->addButton(QIcon("images:barline/barlinedotted.svg"), CABarline::Dotted, tr("Dotted Barline"));
    uiBarlineType->addButton(QIcon("images:barline/barlinerepeatopen.svg"), CABarline::RepeatOpen, tr("Repeat Open"));
    uiBarlineType->addButton(QIcon("images:barline/barlinerepeatclose.svg"), CABarline::RepeatClose, tr("Repeat Closed"));
    uiBarlineType->addButton(QIcon("images:barline/barlinerepeatcloseopen.svg"), CABarline::RepeatCloseOpen, tr("Repeat Closed-Open"));
    uiMarkType = new CAMenuToolButton(tr("Select Mark"), 3, this);
    uiMarkType->setObjectName("uiMarkType");
    uiMarkType->addButton(QIcon("images:mark/tempo/tempo.svg"), CAMark::Tempo, tr("Tempo"));
    uiMarkType->addButton(QIcon("images:mark/tempo/rit.svg"), CAMark::Ritardando, tr("Ritardando"));
    uiMarkType->addButton(QIcon("images:mark/tempo/accel.svg"), CAMark::Ritardando * (-1), tr("Accellerando"));
    uiMarkType->addButton(QIcon("images:mark/dynamic/mf.svg"), CAMark::Dynamic, tr("Dynamic"));
    uiMarkType->addButton(QIcon("images:mark/dynamic/crescendo.svg"), CAMark::Crescendo, tr("Crescendo"));
    uiMarkType->addButton(QIcon("images:mark/dynamic/decrescendo.svg"), CAMark::Crescendo * (-1), tr("Decrescendo"));
    uiMarkType->addButton(QIcon("images:mark/fermata/normal.svg"), CAMark::Fermata, tr("Fermata"));
    uiMarkType->addButton(QIcon("images:mark/text.svg"), CAMark::Text, tr("Arbitrary Text"));
    uiMarkType->addButton(QIcon("images:mark/repeatmark/volta1.svg"), CAMark::RepeatMark, tr("Repeat Mark"));
    uiMarkType->addButton(QIcon("images:mark/pedal.svg"), CAMark::Pedal, tr("Pedal Mark"));
    uiMarkType->addButton(QIcon("images:mark/bookmark.svg"), CAMark::BookMark, tr("Bookmark"));
    uiMarkType->addButton(QIcon("images:mark/rehersalmark.svg"), CAMark::RehersalMark, tr("Rehersal Mark"));
    uiMarkType->addButton(QIcon("images:mark/fingering/fingering.svg"), CAMark::Fingering, tr("Fingering"));
    uiMarkType->addButton(QIcon("images:mark/instrumentchange.svg"), CAMark::InstrumentChange, tr("Instrument Change"));
    uiArticulationType = new CAMenuToolButton(tr("Articulation Mark"), 6, this);
    uiArticulationType->setObjectName("uiArticulationType");
    uiArticulationType->addButton(QIcon("images:mark/articulation/accent.svg"), CAArticulation::Accent, tr("Accent"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/marcato.svg"), CAArticulation::Marcato, tr("Marcato"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/staccatissimo.svg"), CAArticulation::Staccatissimo, tr("Stacatissimo"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/espressivo.svg"), CAArticulation::Espressivo, tr("Espressivo"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/staccato.svg"), CAArticulation::Staccato, tr("Staccato"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/tenuto.svg"), CAArticulation::Tenuto, tr("Tenuto"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/portato.svg"), CAArticulation::Portato, tr("Portato"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/breath.svg"), CAArticulation::Breath, tr("Breath"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/upbow.svg"), CAArticulation::UpBow, tr("UpBow"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/downbow.svg"), CAArticulation::DownBow, tr("DownBow"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/flageolet.svg"), CAArticulation::Flageolet, tr("Flageloet"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/open.svg"), CAArticulation::Open, tr("Open"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/stopped.svg"), CAArticulation::Stopped, tr("Stopped"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/turn.svg"), CAArticulation::Turn, tr("Turn"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/reverseturn.svg"), CAArticulation::ReverseTurn, tr("ReverseTurn"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/trill.svg"), CAArticulation::Trill, tr("Trill"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/prall.svg"), CAArticulation::Prall, tr("Prall"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/mordent.svg"), CAArticulation::Mordent, tr("Mordent"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/prallprall.svg"), CAArticulation::PrallPrall, tr("Prall-Prall"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/prallmordent.svg"), CAArticulation::PrallMordent, tr("Prall-Mordent"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/upprall.svg"), CAArticulation::UpPrall, tr("Up-Prall"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/downprall.svg"), CAArticulation::DownPrall, tr("Down-Prall"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/upmordent.svg"), CAArticulation::UpMordent, tr("Up-Mordent"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/downmordent.svg"), CAArticulation::DownMordent, tr("Down-Mordent"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/pralldown.svg"), CAArticulation::PrallDown, tr("Prall-Down"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/prallup.svg"), CAArticulation::PrallUp, tr("Prall-Up"));
    uiArticulationType->addButton(QIcon("images:mark/articulation/lineprall.svg"), CAArticulation::LinePrall, tr("Line-Prall"));

    uiSheetToolBar = new QToolBar(tr("Sheet ToolBar"), this);
    uiSheetName = new QLineEdit(this);
    uiSheetName->setObjectName("uiSheetName");

    uiContextToolBar = new QToolBar(tr("Context ToolBar"), this);
    uiContextName = new QLineEdit(this);
    uiContextName->setObjectName("uiContextName");
    uiContextName->setToolTip(tr("Context name"));
    uiStanzaNumber = new QSpinBox(this);
    uiStanzaNumber->setObjectName("uiStanzaNumber");
    uiStanzaNumber->setToolTip(tr("Stanza number"));
    uiStanzaNumber->setSpecialValueText(tr("none", "stanza number"));
    uiStanzaNumber->hide();
    uiAssociatedVoice = new QComboBox(this);
    // Warning! disconnect and reconnect is also done in updateContextToolBar()!
    uiAssociatedVoice->setObjectName("uiAssociatedVoice");
    uiAssociatedVoice->setToolTip(tr("Associated voice"));
    uiAssociatedVoice->hide();

    uiVoiceToolBar = new QToolBar(tr("Voice ToolBar"), this);
    uiVoiceNum = new CALCDNumber(0, 20, this, "Voice number");
    uiVoiceNum->setObjectName("uiVoiceNum");
    uiVoiceNum->setToolTip(tr("Current Voice number"));
    uiVoiceInstrument = new QComboBox(this);
    uiVoiceInstrument->setObjectName("uiVoiceInstrument");
    uiVoiceInstrument->setToolTip(tr("Voice instrument"));
    uiVoiceInstrument->addItems(CAMidiDevice::instrumentNames());
    uiVoiceName = new QLineEdit(this);
    uiVoiceName->setObjectName("uiVoiceName");
    uiVoiceName->setToolTip(tr("Voice name"));
    uiVoiceStemDirection = new CAMenuToolButton(tr("Select Voice Stem Direction"), 3, this);
    uiVoiceStemDirection->setObjectName("uiVoiceStemDirection");
    uiVoiceStemDirection->addButton(QIcon("images:playable/notestemneutral.svg"), CANote::StemNeutral, tr("Voice Stems Neutral"));
    uiVoiceStemDirection->addButton(QIcon("images:playable/notestemup.svg"), CANote::StemUp, tr("Voice Stems Up"));
    uiVoiceStemDirection->addButton(QIcon("images:playable/notestemdown.svg"), CANote::StemDown, tr("Voice Stems Down"));

    uiPlayableToolBar = new QToolBar(tr("Playable ToolBar"), this);
    uiPlayableLength = new CAMenuToolButton(tr("Select Length"), 3, this);
    uiPlayableLength->setObjectName("uiPlayableLength");
    uiPlayableLength->addButton(QIcon("images:playable/n0.svg"), CAPlayableLength::Breve, tr("Breve", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n1.svg"), CAPlayableLength::Whole, tr("Whole", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n2.svg"), CAPlayableLength::Half, tr("Half", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n4.svg"), CAPlayableLength::Quarter, tr("Quarter", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n8.svg"), CAPlayableLength::Eighth, tr("Eighth", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n16.svg"), CAPlayableLength::Sixteenth, tr("Sixteenth", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n32.svg"), CAPlayableLength::ThirtySecond, tr("ThirtySecond", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n64.svg"), CAPlayableLength::SixtyFourth, tr("SixtyFourth", "note"));
    uiPlayableLength->addButton(QIcon("images:playable/n128.svg"), CAPlayableLength::HundredTwentyEighth, tr("HundredTwentyEighth", "note"));
    uiNoteStemDirection = new CAMenuToolButton(tr("Select Note Stem Direction"), 4, this);
    uiNoteStemDirection->setObjectName("uiNoteStemDirection");
    uiNoteStemDirection->addButton(QIcon("images:playable/notestemneutral.svg"), CANote::StemNeutral, tr("Note Stem Neutral"));
    uiNoteStemDirection->addButton(QIcon("images:playable/notestemup.svg"), CANote::StemUp, tr("Note Stem Up"));
    uiNoteStemDirection->addButton(QIcon("images:playable/notestemdown.svg"), CANote::StemDown, tr("Note Stem Down"));
    uiNoteStemDirection->addButton(QIcon("images:playable/notestemvoice.svg"), CANote::StemPreferred, tr("Note Stem Preferred"));
    uiTupletType = new CAMenuToolButton(tr("Select Tuplet Type"), 2, this);
    uiTupletType->setObjectName("uiTupletType");
    uiTupletType->addButton(QIcon("images:tuplet/triplet.svg"), 0, tr("Triplet"));
    uiTupletType->addButton(QIcon("images:tuplet/tuplet.svg"), 1, tr("Tuplet"));
    uiTupletNumber = new QSpinBox(this);
    uiTupletNumber->setObjectName("uiTupletNumber");
    uiTupletNumber->setMinimum(1);
    uiTupletNumber->setValue(3);
    uiTupletNumber->setToolTip(tr("Number of notes"));
    uiTupletInsteadOf = new QLabel(tr("instead of"), this);
    uiTupletInsteadOf->setObjectName("uiTupletInsteadOf");
    uiTupletActualNumber = new QSpinBox(this);
    uiTupletActualNumber->setObjectName("uiTupletActualNumber");
    uiTupletActualNumber->setMinimum(1);
    uiTupletActualNumber->setValue(2);
    uiTupletActualNumber->setToolTip(tr("Actual number of notes"));

    uiTimeSigToolBar = new QToolBar(tr("Time Signature ToolBar"), this);
    uiTimeSigBeats = new QSpinBox(this);
    uiTimeSigBeats->setObjectName("uiTimeSigBeats");
    uiTimeSigBeats->setMinimum(1);
    uiTimeSigBeats->setValue(4);
    uiTimeSigBeats->setToolTip(tr("Number of beats"));
    uiTimeSigSlash = new QLabel("/", this);
    uiTimeSigSlash->setObjectName("uiTimeSigSlash");
    uiTimeSigBeat = new QSpinBox(this);
    uiTimeSigBeat->setObjectName("uiTimeSigBeat");
    uiTimeSigBeat->setMinimum(1);
    uiTimeSigBeat->setValue(4);
    uiTimeSigBeat->setToolTip(tr("Beat"));

    uiClefToolBar = new QToolBar(tr("Clef ToolBar"), this);
    oldUiClefOffsetValue = 0;
    uiClefOffset = new QSpinBox(this);
    uiClefOffset->setObjectName("uiClefOffset");
    uiClefOffset->setMinimum(-22);
    uiClefOffset->setMaximum(22);
    uiClefOffset->setValue(0);
    uiClefOffset->setToolTip(tr("Clef offset"));

    uiFBMToolBar = new QToolBar(tr("Figured bass ToolBar"), this);
    uiFBMNumber = new CAMenuToolButton(tr("Set/Unset Figured bass number"), 8, this);
    uiFBMNumber->setObjectName("uiFBMNumber");
    for (int i = 1; i <= 15; i++) {
        uiFBMNumber->addButton(QIcon(QString("images:numbers/") + QString::number(i) + ".svg"), i, "");
    }
    uiFBMAccs = new CAMenuToolButton(tr("Set/Unset Figured bass accidentals"), 5, this);
    uiFBMAccs->setObjectName("uiFBMAccs");
    uiFBMAccs->addButton(QIcon("images:accidental/doubleflat.svg"), 0, tr("Double flat"));
    uiFBMAccs->addButton(QIcon("images:accidental/flat.svg"), 1, tr("Flat"));
    uiFBMAccs->addButton(QIcon("images:accidental/neutral.svg"), 2, tr("Neutral"));
    uiFBMAccs->addButton(QIcon("images:accidental/sharp.svg"), 3, tr("Sharp"));
    uiFBMAccs->addButton(QIcon("images:accidental/doublesharp.svg"), 4, tr("Double sharp"));

    uiFMToolBar = new QToolBar(tr("Function mark ToolBar"), this);
    uiFMFunction = new CAMenuToolButton(tr("Select Function Name"), 8, this);
    uiFMFunction->setObjectName("uiFMFunction");
    uiFMFunction->addButton(QIcon("images:functionmark/fmt.svg"), CAFunctionMark::T, tr("Tonic"));
    uiFMFunction->addButton(QIcon("images:functionmark/fms.svg"), CAFunctionMark::S, tr("Subdominant"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmd.svg"), CAFunctionMark::D, tr("Dominant"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmii.svg"), CAFunctionMark::II, tr("II"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmiii.svg"), CAFunctionMark::III, tr("III"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmvi.svg"), CAFunctionMark::VI, tr("VI"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmvii.svg"), CAFunctionMark::VII, tr("VII"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmk.svg"), CAFunctionMark::K, tr("Cadenze"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmot.svg"), CAFunctionMark::T * (-1), tr("minor Tonic"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmos.svg"), CAFunctionMark::S * (-1), tr("minor Subdominant"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmn.svg"), CAFunctionMark::N, tr("Napolitan"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmf.svg"), CAFunctionMark::F, tr("Phrygian"));
    uiFMFunction->addButton(QIcon("images:functionmark/fml.svg"), CAFunctionMark::L, tr("Lydian"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmiv.svg"), CAFunctionMark::IV, tr("IV"));
    uiFMFunction->addButton(QIcon("images:functionmark/fmv.svg"), CAFunctionMark::V, tr("V"));
    uiFMFunction->addButton(QIcon("images:general/none.svg"), CAFunctionMark::Undefined, tr("None"));
    uiFMChordArea = new CAMenuToolButton(tr("Select Chord Area"), 3, this);
    uiFMChordArea->setObjectName("uiFMChordArea");
    uiFMChordArea->addButton(QIcon("images:functionmark/fmpt.svg"), CAFunctionMark::T, tr("Tonic"));
    uiFMChordArea->addButton(QIcon("images:functionmark/fmps.svg"), CAFunctionMark::S, tr("Subdominant"));
    uiFMChordArea->addButton(QIcon("images:functionmark/fmpd.svg"), CAFunctionMark::D, tr("Dominant"));
    uiFMChordArea->addButton(QIcon("images:functionmark/fmpot.svg"), CAFunctionMark::T * (-1), tr("minor Tonic"));
    uiFMChordArea->addButton(QIcon("images:functionmark/fmpos.svg"), CAFunctionMark::S * (-1), tr("minor Subdominant"));
    uiFMChordArea->addButton(QIcon("images:general/none.svg"), CAFunctionMark::Undefined, tr("None"));
    uiFMTonicDegree = new CAMenuToolButton(tr("Select Tonic Degree"), 5, this);
    uiFMTonicDegree->setObjectName("uiFMTonicDegree");
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmt.svg"), CAFunctionMark::T, tr("Tonic"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmot.svg"), CAFunctionMark::T * (-1), tr("minor Tonic"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fms.svg"), CAFunctionMark::S, tr("Subdominant"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmos.svg"), CAFunctionMark::S * (-1), tr("minor Subdominant"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmd.svg"), CAFunctionMark::D, tr("Dominant"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmii.svg"), CAFunctionMark::II, tr("II"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmiii.svg"), CAFunctionMark::III, tr("III"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmvi.svg"), CAFunctionMark::VI, tr("VI"));
    uiFMTonicDegree->addButton(QIcon("images:functionmark/fmvii.svg"), CAFunctionMark::VII, tr("VII"));
    uiFMKeySig = new QComboBox(this);
    uiFMKeySig->setObjectName("uiFMKeySig");
    CAKeySignatureUI::populateComboBox(uiFMKeySig);

    uiDynamicToolBar = new QToolBar(tr("Dynamic marks ToolBar"), this);
    uiDynamicText = new CAMenuToolButton(tr("Select Dynamic"), 5, this);
    uiDynamicText->setObjectName("uiDynamicText");
    uiDynamicText->addButton(QIcon("images:mark/dynamic/p.svg"), CADynamic::p, tr("Piano", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/pp.svg"), CADynamic::pp, tr("Pianissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/ppp.svg"), CADynamic::ppp, tr("Pianissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/pppp.svg"), CADynamic::pppp, tr("Pianissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/ppppp.svg"), CADynamic::ppppp, tr("Pianissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/f.svg"), CADynamic::f, tr("Forte", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/ff.svg"), CADynamic::ff, tr("Fortissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/fff.svg"), CADynamic::fff, tr("Fortissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/ffff.svg"), CADynamic::ffff, tr("Fortissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/fffff.svg"), CADynamic::fffff, tr("Fortissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/mf.svg"), CADynamic::mf, tr("Mezzo Forte", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/mp.svg"), CADynamic::mp, tr("Mezzo Piano", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/fp.svg"), CADynamic::fp, tr("Forte Piano", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/sf.svg"), CADynamic::sf, tr("Sforzando Forte", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/sp.svg"), CADynamic::sp, tr("Sforzando Piano", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/sfz.svg"), CADynamic::sfz, tr("Sforzando", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/rfz.svg"), CADynamic::rfz, tr("Rinforzando", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/spp.svg"), CADynamic::spp, tr("Sforzando Pianissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:mark/dynamic/sff.svg"), CADynamic::sff, tr("Sforzando Fortissimo", "dynamics"));
    uiDynamicText->addButton(QIcon("images:general/custom.svg"), CADynamic::Custom, tr("Custom", "dynamics"));
    uiDynamicVolume = new QSpinBox(this);
    uiDynamicVolume->setObjectName("uiDynamicVolume");
    uiDynamicVolume->setToolTip(tr("Playback Volume"));
    uiDynamicVolume->setMinimum(0);
    uiDynamicVolume->setMaximum(100);
    uiDynamicVolume->setSuffix(" %");
    uiDynamicCustomText = new QLineEdit(this);
    uiDynamicCustomText->setObjectName("uiDynamicCustomText");
    uiDynamicCustomText->setToolTip(tr("Dynamic mark text"));

    uiInstrumentToolBar = new QToolBar(tr("Instrument ToolBar"), this);
    uiInstrumentChange = new QComboBox(this);
    uiInstrumentChange->setObjectName("uiInstrumentChange");
    uiInstrumentChange->setToolTip(tr("Instrument Change"));
    uiInstrumentChange->addItems(CAMidiDevice::instrumentNames());

    uiTempoToolBar = new QToolBar(tr("Tempo ToolBar"), this);
    uiTempoBeat = new CAMenuToolButton(tr("Select Beat"), 3, this);
    uiTempoBeat->setObjectName("uiTempoBeat");
    uiTempoBeat->addButton(QIcon("images:playable/n4.svg"), CAPlayableLength::Quarter, tr("Quarter", "note"));
    uiTempoBeat->addButton(QIcon("images:playable/n2.svg"), CAPlayableLength::Half, tr("Half", "note"));
    uiTempoBeat->addButton(QIcon("images:playable/n8.svg"), CAPlayableLength::Eighth, tr("Eighth", "note"));
    uiTempoBeat->addButton(QIcon("images:playable/n4d.svg"), CAPlayableLength::Quarter * (-1), tr("Dotted Quarter", "note"));
    uiTempoBeat->addButton(QIcon("images:playable/n2d.svg"), CAPlayableLength::Half * (-1), tr("Dotted Half", "note"));
    uiTempoBeat->addButton(QIcon("images:playable/n8d.svg"), CAPlayableLength::Eighth * (-1), tr("Dotted Eighth", "note"));
    uiTempoEquals = new QLabel("=", this);
    uiTempoEquals->setObjectName("uiTempoEquals");
    uiTempoBpm = new QLineEdit(this);
    uiTempoBpm->setObjectName("uiTempoBpm");
    uiTempoBpm->setToolTip(tr("Beats per minute", "tempo"));

    uiFermataToolBar = new QToolBar(tr("Fermata ToolBar"), this);
    uiFermataType = new CAMenuToolButton(tr("Fermata Type"), 4, this);
    uiFermataType->setObjectName("uiFermataType");
    uiFermataType->addButton(QIcon("images:mark/fermata/short.svg"), CAFermata::ShortFermata, tr("Short", "fermata"));
    uiFermataType->addButton(QIcon("images:mark/fermata/normal.svg"), CAFermata::NormalFermata, tr("Normal", "fermata"));
    uiFermataType->addButton(QIcon("images:mark/fermata/long.svg"), CAFermata::LongFermata, tr("Long", "fermata"));
    uiFermataType->addButton(QIcon("images:mark/fermata/verylong.svg"), CAFermata::VeryLongFermata, tr("Very Long", "fermata"));

    uiRepeatMarkToolBar = new QToolBar(tr("Repeat Mark ToolBar"), this);
    uiRepeatMarkType = new CAMenuToolButton(tr("Repeat Mark Type"), 3, this);
    uiRepeatMarkType->setObjectName("uiRepeatMarkType");
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/volta1.svg"), -2, tr("Volta 1st", "repeat mark")); // -1 can't be used?!
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/volta2.svg"), -3, tr("Volta 2nd", "repeat mark"));
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/volta3.svg"), -4, tr("Volta 3rd", "repeat mark"));
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/segno.svg"), CARepeatMark::Segno, tr("Segno", "repeat mark"));
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/coda.svg"), CARepeatMark::Coda, tr("Coda", "repeat mark"));
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/varcoda.svg"), CARepeatMark::VarCoda, tr("VarCoda", "repeat mark"));
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/dalsegno.svg"), CARepeatMark::DalSegno, tr("Dal Segno", "repeat mark"));
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/dalcoda.svg"), CARepeatMark::DalCoda, tr("Dal Coda", "repeat mark"));
    uiRepeatMarkType->addButton(QIcon("images:mark/repeatmark/dalvarcoda.svg"), CARepeatMark::DalVarCoda, tr("Dal VarCoda", "repeat mark"));

    uiFingeringToolBar = new QToolBar(tr("Fingering ToolBar"), this);
    uiFinger = new CAMenuToolButton(tr("Finger"), 5, this);
    uiFinger->setObjectName("uiFinger");
    uiFinger->addButton(QIcon("images:mark/fingering/1.svg"), CAFingering::First, tr("First", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/2.svg"), CAFingering::Second, tr("Second", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/3.svg"), CAFingering::Third, tr("Third", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/4.svg"), CAFingering::Fourth, tr("Fourth", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/5.svg"), CAFingering::Fifth, tr("Fifth", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/0.svg"), CAFingering::Thumb, tr("Thumb", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/lheel.svg"), CAFingering::LHeel, tr("Left Heel", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/rheel.svg"), CAFingering::RHeel, tr("Right Heel", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/ltoe.svg"), CAFingering::LToe, tr("Left Toe", "fingering"));
    uiFinger->addButton(QIcon("images:mark/fingering/rtoe.svg"), CAFingering::RToe, tr("Right Toe", "fingering"));

    uiFingeringOriginal = new QCheckBox(tr("Original"), this);
    uiFingeringOriginal->setObjectName("uiFingeringOriginal");
    uiFingeringOriginal->setToolTip(tr("Is the fingering original by a composer (usually written italic)", "fingering original checkbox"));

    // User's guide and other Help
    uiHelpDock = new QDockWidget(tr("Help"), this);
    uiHelpDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    uiHelpDock->setMaximumWidth(400);
#ifdef QT_WEBENGINEWIDGETS_LIB
    uiHelpWidget = new CAHelpBrowser(uiHelpDock);
    uiHelpDock->setWidget(uiHelpWidget);
#endif

#ifdef USE_PYTHON
    uiPyConsoleDock = new QDockWidget(tr("Canorus console"), this);
    uiPyConsoleDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    pyConsole = new CAPyConsole(document(), uiPyConsoleDock);
    pyConsoleIface = new CAPyConsoleInterface(pyConsole);
    uiPyConsoleDock->setWidget(pyConsole);
#endif
}

/*!
	Creates more complex widgets and layouts that cannot be created using Qt Designer (like adding
	custom toolbars to main window, button boxes etc.).
*/
void CAMainWin::setupCustomUi()
{
    _musElementFactory = new CAMusElementFactory();

    _poPrintPreviewCtl = new CAPreviewCtl(this);
    connect(uiPrintPreview, SIGNAL(triggered()), _poPrintPreviewCtl, SLOT(on_uiPrintPreview_triggered()));
    _poPrintCtl = new CAPrintCtl(this);

    //uiPrint->setEnabled( false );
    // Standard Toolbar
    uiUndo->setDefaultAction(uiStandardToolBar->insertWidget(uiCut, uiUndo));
    uiUndo->defaultAction()->setText(tr("Undo"));
    uiUndo->defaultAction()->setShortcut(QApplication::translate("uiMainWindow", "Ctrl+Z", nullptr /*, QApplication::UnicodeUTF8*/));
    uiMenuEdit->insertAction(uiCut, uiUndo->defaultAction());
    QList<QKeySequence> redoShortcuts;
    redoShortcuts << QApplication::translate("uiMainWindow", "Ctrl+Y", nullptr /*, QApplication::UnicodeUTF8*/);
    redoShortcuts << QApplication::translate("uiMainWindow", "Ctrl+Shift+Z", nullptr /*, QApplication::UnicodeUTF8*/);
    uiRedo->setDefaultAction(uiStandardToolBar->insertWidget(uiCut, uiRedo));
    uiRedo->defaultAction()->setText(tr("Redo"));
    uiRedo->defaultAction()->setShortcuts(redoShortcuts);
    uiMenuEdit->insertAction(uiCut, uiRedo->defaultAction());

    // Hide the specialized pre-created toolbars in Qt designer.
    /// \todo When Qt Designer have support for setting the visibility property, do this in Qt Designer already! -Matevz
    uiPrintToolBar->hide();
    uiFileToolBar->hide();
    uiStandardToolBar->updateGeometry();

    // Insert Toolbar
    uiInsertToolBar->addAction(uiEditMode);
    uiInsertToolBar->addSeparator();
    uiContextType->setDefaultAction(uiInsertToolBar->addWidget(uiContextType));
    uiContextType->defaultAction()->setToolTip(tr("Insert context"));
    uiContextType->setCurrentId(CAContext::Staff);
    connect(uiNewContext, SIGNAL(triggered()), uiContextType, SLOT(click()));
    uiInsertToolBar->addSeparator();
    uiInsertToolBar->addAction(uiInsertPlayable);
    uiSlurType->setDefaultAction(uiInsertToolBar->addWidget(uiSlurType));
    uiSlurType->defaultAction()->setToolTip(tr("Insert slur"));
    uiSlurType->setCurrentId(CASlur::TieType);
    uiSlurType->defaultAction()->setCheckable(false);
    uiClefType->setDefaultAction(uiInsertToolBar->addWidget(uiClefType));
    uiClefType->defaultAction()->setToolTip(tr("Insert clef"));
    uiClefType->setCurrentId(CAClef::Treble);
    connect(uiInsertClef, SIGNAL(triggered()), uiClefType, SLOT(click()));
    uiInsertToolBar->addAction(uiInsertKeySig);
    uiTimeSigType->setDefaultAction(uiInsertToolBar->addWidget(uiTimeSigType));
    uiTimeSigType->defaultAction()->setToolTip(tr("Insert time signature"));
    uiTimeSigType->setCurrentId(44);
    connect(uiInsertTimeSig, SIGNAL(triggered()), uiTimeSigType, SLOT(click()));
    uiBarlineType->setDefaultAction(uiInsertToolBar->addWidget(uiBarlineType));
    uiBarlineType->defaultAction()->setToolTip(tr("Insert barline"));
    uiBarlineType->setCurrentId(CABarline::End);
    connect(uiInsertBarline, SIGNAL(triggered()), uiBarlineType, SLOT(click()));
    uiMarkType->setDefaultAction(uiInsertToolBar->addWidget(uiMarkType));
    uiMarkType->defaultAction()->setToolTip(tr("Insert mark"));
    uiMarkType->setCurrentId(CAMark::Dynamic);
    connect(uiInsertMark, SIGNAL(triggered()), uiMarkType, SLOT(click()));
    uiArticulationType->setDefaultAction(uiInsertToolBar->addWidget(uiArticulationType));
    uiArticulationType->defaultAction()->setToolTip(tr("Insert articulation mark"));
    uiArticulationType->setCurrentId(CAArticulation::Accent);
    connect(uiInsertArticulation, SIGNAL(triggered()), uiArticulationType, SLOT(click()));
    uiInsertToolBar->addAction(uiInsertSyllable);
    uiInsertToolBar->addAction(uiInsertFBM);
    uiInsertToolBar->addAction(uiInsertFM);
    uiInsertToolBar->addAction(uiInsertChordName);

    if (qApp->isRightToLeft())
        addToolBar(Qt::RightToolBarArea, uiInsertToolBar);
    else
        addToolBar(Qt::LeftToolBarArea, uiInsertToolBar);

    // Sheet Toolbar
    uiSheetToolBar->addAction(uiNewSheet);
    uiSheetToolBar->addWidget(uiSheetName);
    uiSheetToolBar->addAction(uiRemoveSheet);
    uiSheetToolBar->addAction(uiSheetProperties);
    addToolBar(Qt::TopToolBarArea, uiSheetToolBar);

    // Context Toolbar
    uiContextToolBar->addWidget(uiContextName);
    uiStanzaNumberAction = uiContextToolBar->addWidget(uiStanzaNumber);
    uiAssociatedVoiceAction = uiContextToolBar->addWidget(uiAssociatedVoice);
    uiContextToolBar->addAction(uiRemoveContext);
    uiContextToolBar->addAction(uiContextProperties);
    addToolBar(Qt::TopToolBarArea, uiContextToolBar);

    // Playable Toolbar
    uiPlayableLength->setDefaultAction(uiPlayableToolBar->addWidget(uiPlayableLength));
    uiPlayableLength->defaultAction()->setToolTip(tr("Playable length"));
    uiPlayableLength->defaultAction()->setCheckable(false);
    uiPlayableLength->setCurrentId(CAPlayableLength::Quarter);
    uiPlayableToolBar->addAction(uiAccsVisible);
    uiTupletType->setDefaultAction(uiPlayableToolBar->addWidget(uiTupletType));
    uiTupletType->defaultAction()->setToolTip(tr("Insert tuplet"));
    uiTupletType->setCurrentId(0);
    uiTupletNumberAction = uiPlayableToolBar->addWidget(uiTupletNumber);
    uiTupletInsteadOfAction = uiPlayableToolBar->addWidget(uiTupletInsteadOf);
    uiTupletActualNumberAction = uiPlayableToolBar->addWidget(uiTupletActualNumber);
    uiNoteStemDirection->setDefaultAction(uiPlayableToolBar->addWidget(uiNoteStemDirection));
    uiNoteStemDirection->defaultAction()->setToolTip(tr("Note stem direction"));
    uiNoteStemDirection->defaultAction()->setCheckable(false);
    uiNoteStemDirection->setCurrentId(CANote::StemPreferred);
    uiPlayableToolBar->addAction(uiHiddenRest);
    addToolBar(Qt::TopToolBarArea, uiPlayableToolBar);

    // Clef Toolbar
    uiClefToolBar->addWidget(uiClefOffset);
    addToolBar(Qt::TopToolBarArea, uiClefToolBar);

    // TimeSig Toolbar
    uiTimeSigToolBar->addWidget(uiTimeSigBeats);
    uiTimeSigToolBar->addWidget(uiTimeSigSlash);
    uiTimeSigToolBar->addWidget(uiTimeSigBeat);
    addToolBar(Qt::TopToolBarArea, uiTimeSigToolBar);

    // Voice Toolbar
    uiVoiceToolBar->addAction(uiNewVoice);
    uiVoiceToolBar->addWidget(uiVoiceNum);
    uiVoiceToolBar->addWidget(uiVoiceName);
    uiVoiceToolBar->addWidget(uiVoiceInstrument);
    uiVoiceToolBar->addAction(uiRemoveVoice);
    uiVoiceStemDirection->setDefaultAction(uiVoiceToolBar->addWidget(uiVoiceStemDirection));
    uiVoiceStemDirection->defaultAction()->setToolTip(tr("Voice stem direction"));
    uiVoiceStemDirection->defaultAction()->setCheckable(false);
    uiVoiceToolBar->addAction(uiVoiceProperties);
    addToolBar(Qt::TopToolBarArea, uiVoiceToolBar);

    // Figured bass Toolbar
    uiFBMNumber->setDefaultAction(uiFBMToolBar->addWidget(uiFBMNumber));
    uiFBMNumber->defaultAction()->setToolTip(tr("Figured bass number"));
    uiFBMNumber->setCurrentId(6);
    uiFBMNumber->defaultAction()->setCheckable(true);
    uiFBMNumber->defaultAction()->setChecked(true);
    uiFBMAccs->setDefaultAction(uiFBMToolBar->addWidget(uiFBMAccs));
    uiFBMAccs->defaultAction()->setToolTip(tr("Figured bass accidentals"));
    uiFBMAccs->setCurrentId(2);
    uiFBMAccs->defaultAction()->setCheckable(true);
    uiFBMAccs->defaultAction()->setChecked(false);
    addToolBar(Qt::TopToolBarArea, uiFBMToolBar);

    // Function mark Toolbar
    uiFMFunction->setDefaultAction(uiFMToolBar->addWidget(uiFMFunction));
    uiFMFunction->defaultAction()->setToolTip(tr("Function mark"));
    uiFMFunction->setCurrentId(CAFunctionMark::T);
    uiFMFunction->defaultAction()->setCheckable(false);
    uiFMChordArea->setDefaultAction(uiFMToolBar->addWidget(uiFMChordArea));
    uiFMChordArea->defaultAction()->setToolTip(tr("Function mark chord area"));
    uiFMChordArea->setCurrentId(CAFunctionMark::T);
    uiFMChordArea->defaultAction()->setCheckable(false);
    uiFMTonicDegree->setDefaultAction(uiFMToolBar->addWidget(uiFMTonicDegree));
    uiFMTonicDegree->defaultAction()->setCheckable(false);
    uiFMTonicDegree->defaultAction()->setToolTip(tr("Function mark tonic degree"));
    uiFMTonicDegree->setCurrentId(CAFunctionMark::T);
    uiFMToolBar->addAction(uiFMEllipse);
    uiFMToolBar->addWidget(uiFMKeySig);
    addToolBar(Qt::TopToolBarArea, uiFMToolBar);

    // Dynamic marks toolbar
    uiDynamicText->setDefaultAction(uiDynamicToolBar->addWidget(uiDynamicText));
    uiDynamicText->defaultAction()->setToolTip(tr("Predefined dynamic mark"));
    uiDynamicText->setCurrentId(CADynamic::mf);
    uiDynamicText->defaultAction()->setCheckable(false);
    uiDynamicToolBar->addWidget(uiDynamicVolume);
    uiDynamicToolBar->addWidget(uiDynamicCustomText);
    addToolBar(Qt::TopToolBarArea, uiDynamicToolBar);

    // Instrument tool bar
    uiInstrumentToolBar->addWidget(uiInstrumentChange);
    addToolBar(Qt::TopToolBarArea, uiInstrumentToolBar);

    // Tempo tool bar
    uiTempoBeat->setDefaultAction(uiTempoToolBar->addWidget(uiTempoBeat));
    uiTempoBeat->defaultAction()->setCheckable(false);
    uiTempoBeat->defaultAction()->setToolTip(tr("Beat", "tempo"));
    uiTempoToolBar->addWidget(uiTempoEquals);
    uiTempoToolBar->addWidget(uiTempoBpm);
    addToolBar(Qt::TopToolBarArea, uiTempoToolBar);

    // Fermata tool bar
    uiFermataType->setDefaultAction(uiFermataToolBar->addWidget(uiFermataType));
    uiFermataType->defaultAction()->setCheckable(false);
    uiFermataType->defaultAction()->setToolTip(tr("Fermata Type", "fermata"));
    addToolBar(Qt::TopToolBarArea, uiFermataToolBar);

    // Repeat Mark tool bar
    uiRepeatMarkType->setDefaultAction(uiRepeatMarkToolBar->addWidget(uiRepeatMarkType));
    uiRepeatMarkType->defaultAction()->setCheckable(false);
    uiRepeatMarkType->defaultAction()->setToolTip(tr("Repeat Mark Type", "repeat mark"));
    addToolBar(Qt::TopToolBarArea, uiRepeatMarkToolBar);

    // Fingering tool bar
    uiFinger->setDefaultAction(uiFingeringToolBar->addWidget(uiFinger));
    uiFinger->defaultAction()->setCheckable(false);
    uiFinger->defaultAction()->setToolTip(tr("Finger", "fingering"));
    uiFingeringOriginal->setChecked(false);
    uiFingeringToolBar->addWidget(uiFingeringOriginal);
    addToolBar(Qt::TopToolBarArea, uiFingeringToolBar);

#ifdef USE_PYTHON
    // Python console dock widget
    addDockWidget(Qt::BottomDockWidgetArea, uiPyConsoleDock);
#endif

    // View
    uiShowRuler->setChecked(CACanorus::settings()->showRuler());

    // Help
    addDockWidget((qApp->isLeftToRight()) ? Qt::RightDockWidgetArea : Qt::LeftDockWidgetArea, uiHelpDock);
    uiHelpDock->hide();

    // Score UI Interface
    _poKeySignatureUI = new CAKeySignatureUI(this, createModeHash()); // Control object is created here!
    connect(uiFMKeySig, SIGNAL(activated(int)), &_poKeySignatureUI->ctl(), SLOT(on_uiKeySig_activated(int)));

    // Mutual exclusive groups
    uiInsertGroup = new QActionGroup(this);
    uiInsertGroup->addAction(uiEditMode);
    uiInsertGroup->addAction(uiNewContext);
    uiInsertGroup->addAction(uiContextType->defaultAction());
    uiInsertGroup->addAction(uiInsertPlayable);
    uiInsertGroup->addAction(uiSlurType->defaultAction());
    uiInsertGroup->addAction(uiInsertClef);
    uiInsertGroup->addAction(uiClefType->defaultAction());
    uiInsertGroup->addAction(uiInsertTimeSig);
    uiInsertGroup->addAction(uiTimeSigType->defaultAction());
    uiInsertGroup->addAction(uiInsertKeySig);
    uiInsertGroup->addAction(uiInsertBarline);
    uiInsertGroup->addAction(uiBarlineType->defaultAction());
    uiInsertGroup->addAction(uiMarkType->defaultAction());
    uiInsertGroup->addAction(uiInsertMark);
    uiInsertGroup->addAction(uiArticulationType->defaultAction());
    uiInsertGroup->addAction(uiInsertArticulation);
    uiInsertGroup->addAction(uiInsertSyllable);
    uiInsertGroup->addAction(uiInsertFBM);
    uiInsertGroup->addAction(uiInsertFM);
    uiInsertGroup->addAction(uiInsertChordName);
    uiInsertGroup->setExclusive(true);

    uiInsertToolBar->hide();
    uiSheetToolBar->hide();
    uiContextToolBar->hide();
    uiPlayableToolBar->hide();
    uiTimeSigToolBar->hide();
    uiClefToolBar->hide();
    uiFBMToolBar->hide();
    uiFMToolBar->hide();
    uiDynamicToolBar->hide();
    uiInstrumentToolBar->hide();
    uiTempoToolBar->hide();
    uiFermataToolBar->hide();
    uiRepeatMarkToolBar->hide();
    uiFingeringToolBar->hide();

    actionStorage->storeActionsFromMainWindow(*this);
    actionStorage->addWinActions();
}

void CAMainWin::newDocument()
{
    stopPlayback();

    if (!handleUnsavedChanges()) {
        return;
    }

    // clear GUI before clearing the data part!
    clearUI();

    // clear the data part
    if (document() && (CACanorus::mainWinCount(document()) == 1)) {
        CACanorus::undo()->deleteUndoStack(document());
        delete document();
    }

    setDocument(new CADocument());
    setMode(EditMode);
    uiCloseDocument->setEnabled(true);
    CACanorus::undo()->createUndoStack(document());
    restartTimeEditedTime();

#ifdef USE_PYTHON
    QList<PyObject*> argsPython;
    PyEval_RestoreThread(CASwigPython::mainThreadState);
    argsPython << CASwigPython::toPythonObject(document(), CASwigPython::Document);
    PyEval_ReleaseThread(CASwigPython::mainThreadState);
    CASwigPython::callFunction(QFileInfo("scripts:newdocument.py").absoluteFilePath(), "newDefaultDocument", argsPython);
#else
    // fallback: add basic sheet with two staffs
    CASheet* sheet1 = document()->addSheet();
    CAStaff* staff1 = sheet1->addStaff();
    staff1->addVoice();
    staff1->voiceList()[0]->setStemDirection(CANote::StemUp);
    staff1->voiceList()[1]->setStemDirection(CANote::StemDown);
    staff1->voiceList()[0]->append(new CAClef(CAClef::Treble, staff1, 0));
    staff1->voiceList()[0]->append(new CATimeSignature(4, 4, staff1, 0));

    CAStaff* staff2 = sheet1->addStaff();
    staff2->addVoice();
    staff2->voiceList()[0]->setStemDirection(CANote::StemUp);
    staff2->voiceList()[1]->setStemDirection(CANote::StemDown);
    staff2->voiceList()[0]->append(new CAClef(CAClef::Bass, staff2, 0));
    staff2->voiceList()[0]->append(new CATimeSignature(4, 4, staff2, 0));

    staff1->synchronizeVoices();
    staff2->synchronizeVoices();
#endif

    // call local rebuild only because no other main windows share the new document
    rebuildUI();

    // select the first context automatically
    if (document()->sheetList().size() && document()->sheetList()[0]->contextList().size()) {
        currentScoreView()->selectContext(document()->sheetList()[0]->contextList()[0]);
    }

    setMode(EditMode);
}

/*!
	Checks for any unsaved modifications and returns True, to continue with closing the current document.
	Returns False only, if user clicks Cancel button.

	This method looks at CADocument::_modified property.
	The property is changed in undo/redo code.
 */
bool CAMainWin::handleUnsavedChanges()
{
    if (document()) {
        if (document()->isModified()) {
            QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Unsaved changes"), tr("Document \"%1\" was modified. Do you want to save the changes?").arg(document()->title().isEmpty() ? tr("Untitled") : document()->title()), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
            if (ret == QMessageBox::Yes) {
                return on_uiSaveDocument_triggered();
            } else if (ret == QMessageBox::No) {
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
    } else {
        return true;
    }
}

/*!
	This adds a tab to tabWidget and creates a single score View of the sheet.
	It does not add the sheet to the document.
*/
void CAMainWin::addSheet(CASheet* s)
{
    CAScoreView* v = new CAScoreView(s, nullptr);
    initView(v);

    CAViewContainer* vpc = new CAViewContainer(nullptr);
    vpc->addView(v);
    _viewContainerList << vpc;
    _sheetMap[vpc] = s;

    uiTabWidget->addTab(vpc, s->name());
    uiTabWidget->setCurrentIndex(uiTabWidget->count() - 1);
    setCurrentViewContainer(vpc);

    updateToolBars();
}

/*!
	Deletes all Views (and their drawable content), disconnects all signals and resets all
	buttons and modes.

	This function deletes the current main window's GUI only (drawable elements). All the data
	classes (staffs, notes, rests) should stay intact. Use delete document() to free the data
	part of Canorus as well.

	First call clearUI() and then delete model when clearing the document, because cleraUI()
	still needs the model alive.
*/
void CAMainWin::clearUI()
{
    setCurrentView(nullptr);

    // Delete all view port containers and view ports.
    while (uiTabWidget->count()) {
        CAViewContainer* vpc = static_cast<CAViewContainer*>(uiTabWidget->currentWidget());
        uiTabWidget->removeTab(uiTabWidget->currentIndex());
        delete vpc;
    }

    //delete floating Views
    while (!_viewList.isEmpty())
        delete _viewList.takeFirst();

    _sheetMap.clear();

    if (_midiRecorderView) {
        delete _midiRecorderView;
    }

    uiEditMode->trigger(); // select mode
}

/*!
	Called when the current sheet is switched in the tab widget.
	\warning This method is only called when the index of the selected tab changes. If you remove the current tab and the next selected tab gets the same index, this slot isn't called!
*/
void CAMainWin::on_uiTabWidget_currentChanged(int)
{
    setCurrentViewContainer(static_cast<CAViewContainer*>(uiTabWidget->currentWidget()));
    if (currentViewContainer())
        setCurrentView(currentViewContainer()->currentView());

    updateToolBars();
}

/*!
	Appends a new sheet to the document.
	This function is usually called when the user double clicks outside the tabs space.
*/
void CAMainWin::on_uiTabWidget_CANewTab()
{
    if (document()) {
        on_uiNewSheet_triggered();
    }
}

/*!
	Changes the sheet order in the document.
	This function is usually called when the user double clicks outside the tabs space.
*/
void CAMainWin::on_uiTabWidget_CAMoveTab(int from, int to)
{
    if (document() && document()->sheetList().count() >= 2) {
        CACanorus::undo()->createUndoCommand(document(), tr("change sheet order", "undo"));

        CASheet* s = document()->sheetList()[from];
        const_cast<QList<CASheet*>&>(document()->sheetList()).removeAt(from);
        const_cast<QList<CASheet*>&>(document()->sheetList()).insert(to, s);

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFullscreen_toggled(bool checked)
{
    if (checked)
        this->showFullScreen();
    else
        this->showNormal();
}

void CAMainWin::on_uiHiddenRest_toggled(bool checked)
{
    if (mode() == InsertMode) {
        musElementFactory()->setRestType(checked ? CARest::Hidden : CARest::Normal);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change hidden rest", "undo"));
        for (int i = 0; i < v->selection().size(); i++) {
            CARest* r = dynamic_cast<CARest*>(v->selection().at(i)->musElement());
            if (r) {
                r->setRestType(checked ? CARest::Hidden : CARest::Normal);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiSplitHorizontally_triggered()
{
    CAView* v = currentViewContainer()->splitHorizontally();
    if (!v)
        return;

    initView(v);

    uiUnsplitAll->setEnabled(true);
    uiCloseCurrentView->setEnabled(true);
}

void CAMainWin::on_uiSplitVertically_triggered()
{
    CAView* v = currentViewContainer()->splitVertically();
    if (!v)
        return;

    initView(v);

    uiUnsplitAll->setEnabled(true);
    uiCloseCurrentView->setEnabled(true);
}

void CAMainWin::on_uiUnsplitAll_triggered()
{
    QList<CAView*> list = currentViewContainer()->unsplitAll();
    for (CAView* vp : list)
        _viewList.removeAll(vp);
    uiCloseCurrentView->setEnabled(false);
    uiUnsplitAll->setEnabled(false);
    setCurrentView(currentViewContainer()->currentView());
}

void CAMainWin::on_uiCloseCurrentView_triggered()
{
    CAView* v = currentViewContainer()->unsplit();
    _viewList.removeAll(v);
    if (currentViewContainer()->viewList().size() == 1) {
        uiCloseCurrentView->setEnabled(false);
        uiUnsplitAll->setEnabled(false);
    }
    setCurrentView(currentViewContainer()->currentView());
}

void CAMainWin::on_uiCloseDocument_triggered()
{
    if (CACanorus::mainWinCount(document()) == 1) {
        if (!handleUnsavedChanges()) {
            return;
        }

        CACanorus::undo()->deleteUndoStack(document());
        clearUI();
        delete document();
    }
    setDocument(nullptr);
    uiCloseDocument->setEnabled(false);
    rebuildUI();
}

/*!
	Shows the current score in CanorusML syntax in a new or the current View.
*/
void CAMainWin::on_uiCanorusMLSource_triggered()
{
    CASourceView* v = new CASourceView(document(), nullptr);
    initView(v);
    currentViewContainer()->addView(v);

    uiUnsplitAll->setEnabled(true);
    uiCloseCurrentView->setEnabled(true);
}

/*!
	Toggles the visibility of the resource view window.
 */
void CAMainWin::on_uiResourceView_toggled(bool checked)
{
    if (checked) {
        _resourceView->show();
    } else {
        _resourceView->hide();
    }
}

/*!
	Toggles the visibility of the ruler.
 */
void CAMainWin::on_uiShowRuler_toggled(bool checked)
{
    if (checked) {
        CACanorus::settings()->setShowRuler(true);
    } else {
        CACanorus::settings()->setShowRuler(false);
    }
    CACanorus::settings()->writeSettings();
}

/*!
	Called when a floating view port is closed
*/
void CAMainWin::floatViewClosed(CAView* v)
{
    delete v;
    if (currentView() == v)
        setCurrentView(currentViewContainer()->currentView());
}

void CAMainWin::on_uiNewView_triggered()
{
    CAView* v = currentView()->clone(nullptr);
    initView(v);
    connect(v, SIGNAL(closed(CAView*)), this, SLOT(floatViewClosed(CAView*)));
    v->show();
    v->setGeometry(this->geometry().x(), this->geometry().y(), CAView::DEFAULT_VIEW_WIDTH, CAView::DEFAULT_VIEW_HEIGHT);
}

/*!
	Links the newly created View with the main window:
		- Adds the View to the View list
		- Connects its signals to main windows' slots.
		- Sets the icon, focus policy and sets the focus.
		- Sets the currentView but not currentViewContainer
*/
void CAMainWin::initView(CAView* v)
{
    _viewList << v;

    v->setWindowIcon(QIcon("images:clogosm.png"));

    connect(v, SIGNAL(clicked()), this, SLOT(viewClicked()));
    switch (v->viewType()) {
    case CAView::ScoreView: {
        connect(v, SIGNAL(CAMousePressEvent(QMouseEvent*, QPoint)),
            this, SLOT(scoreViewMousePress(QMouseEvent*, QPoint)));
        connect(v, SIGNAL(CAMouseMoveEvent(QMouseEvent*, QPoint)),
            this, SLOT(scoreViewMouseMove(QMouseEvent*, QPoint)));
        connect(v, SIGNAL(CAMouseReleaseEvent(QMouseEvent*, QPoint)),
            this, SLOT(scoreViewMouseRelease(QMouseEvent*, QPoint)));
        connect(v, SIGNAL(CADoubleClickEvent(QMouseEvent*, QPoint)),
            this, SLOT(scoreViewDoubleClick(QMouseEvent*, QPoint)));
        connect(v, SIGNAL(CATripleClickEvent(QMouseEvent*, QPoint)),
            this, SLOT(scoreViewTripleClick(QMouseEvent*, QPoint)));
        connect(v, SIGNAL(CAWheelEvent(QWheelEvent*, QPoint)),
            this, SLOT(scoreViewWheel(QWheelEvent*, QPoint)));
        connect(v, SIGNAL(CAKeyPressEvent(QKeyEvent*)),
            this, SLOT(scoreViewKeyPress(QKeyEvent*)));
        connect(static_cast<CAScoreView*>(v)->textEdit(), SIGNAL(CAKeyPressEvent(QKeyEvent*)),
            this, SLOT(onTextEditKeyPressEvent(QKeyEvent*)));
        connect(v, SIGNAL(selectionChanged()),
            this, SLOT(onScoreViewSelectionChanged()));
        break;
    }
    case CAView::SourceView: {
        connect(v, SIGNAL(CACommit(QString)), this, SLOT(sourceViewCommit(QString)));
        break;
    }
    }

    v->setFocusPolicy(Qt::ClickFocus);
    v->setFocus();
    setCurrentView(v);
    setMode(mode()); // updates the new View border settings
}

/*!
	Returns the currently selected sheet in the current view or 0, if no such view exists.
*/
CASheet* CAMainWin::currentSheet()
{
    if (!currentView()) {
        return nullptr;
    }

    switch (currentView()->viewType()) {
    case CAView::ScoreView: {
        CAScoreView* v = currentScoreView();
        if (v)
            return v->sheet();
        break;
    }
    case CAView::SourceView: {
        CASourceView* v = static_cast<CASourceView*>(currentView());
        if (v->voice() && v->voice()->staff()) {
            return v->voice()->staff()->sheet();
        } else if (v->lyricsContext()) {
            return v->lyricsContext()->sheet();
        }
        break;
    }
    }

    return nullptr;
}

/*!
	Returns the currently selected context in the current view port or 0 if no contexts are selected.
*/
CAContext* CAMainWin::currentContext()
{
    if (currentScoreView() && currentScoreView()->currentContext()) {
        return currentScoreView()->currentContext()->context();
    } else
        return nullptr;
}

/*!
	Returns the pointer to the currently active voice or 0, if All voices are selected or the current context is not a staff at all.
*/
CAVoice* CAMainWin::currentVoice()
{
    if (currentScoreView()) {
        return currentScoreView()->selectedVoice();
    }

    return nullptr;
}

/*!
	Sets the currently selected voice and update toolbars and helpers accordingly.
*/
void CAMainWin::setCurrentVoice(CAVoice* v)
{
    if (currentScoreView()) {
        if (v) {
            currentScoreView()->selectContext(v->staff());
        }
        currentScoreView()->setSelectedVoice(v);
        currentScoreView()->updateHelpers();

        updateVoiceToolBar();
        currentScoreView()->repaint();
    }
}

/*!
	Creates a new main window sharing the current document.
*/
void CAMainWin::on_uiNewWindow_triggered()
{
    CAMainWin* newMainWin = new CAMainWin();
    newMainWin->setDocument(document());
    newMainWin->rebuildUI();
    newMainWin->show();
}

void CAMainWin::on_uiNewDocument_triggered()
{
    newDocument();
}

void CAMainWin::on_uiUndo_toggled(bool, int row)
{
    stopPlayback();
    if (document()) {
        int curVoiceIdx = -1;
        if (currentVoice() && currentVoice()->staff() && currentVoice()->staff()->sheet()) {
            curVoiceIdx = currentVoice()->staff()->sheet()->voiceList().indexOf(currentVoice());
        }

        for (int i = 0; i <= row; i++) {
            CACanorus::undo()->undo(document());
        }

        if (CACanorus::settings()->useNoteChecker()) {
            for (int i = 0; i < document()->sheetList().size(); i++) {
                _noteChecker.checkSheet(document()->sheetList()[i]);
            }
        }

        CACanorus::rebuildUI(document());
        if (curVoiceIdx >= 0 && curVoiceIdx < currentSheet()->voiceList().size()) {
            setCurrentVoice(currentSheet()->voiceList()[curVoiceIdx]);
        }
    }
}

void CAMainWin::on_uiRedo_toggled(bool, int row)
{
    stopPlayback();
    if (document()) {
        int curVoiceIdx = -1;
        if (currentVoice() && currentVoice()->staff() && currentVoice()->staff()->sheet()) {
            curVoiceIdx = currentVoice()->staff()->sheet()->voiceList().indexOf(currentVoice());
        }

        for (int i = 0; i <= row; i++) {
            CACanorus::undo()->redo(document());
        }

        if (CACanorus::settings()->useNoteChecker()) {
            for (int i = 0; i < document()->sheetList().size(); i++) {
                _noteChecker.checkSheet(document()->sheetList()[i]);
            }
        }

        CACanorus::rebuildUI(document(), nullptr);

        if (curVoiceIdx >= 0 && curVoiceIdx < currentSheet()->voiceList().size()) {
            setCurrentVoice(currentSheet()->voiceList()[curVoiceIdx]);
        }
    }
}

/*!
	Enables or Disabled undo/redo buttons if there are undo/redo commands on the undo stack.

	\sa CACanorus::undoStack()
*/
void CAMainWin::updateUndoRedoButtons()
{
    if (CACanorus::undo()->canUndo(document()))
        uiUndo->defaultAction()->setEnabled(true);
    else
        uiUndo->defaultAction()->setEnabled(false);

    if (CACanorus::undo()->canRedo(document()))
        uiRedo->defaultAction()->setEnabled(true);
    else
        uiRedo->defaultAction()->setEnabled(false);
}

/*!
	Adds a new empty sheet.
*/
void CAMainWin::on_uiNewSheet_triggered()
{
    stopPlayback();
    CACanorus::undo()->createUndoCommand(document(), tr("new sheet", "undo"));
    document()->addSheet();
    CACanorus::undo()->pushUndoCommand();
    CACanorus::rebuildUI(document());
    uiTabWidget->setCurrentIndex(uiTabWidget->count() - 1);
}

/*!
	Adds a new voice to the staff.
*/
void CAMainWin::on_uiNewVoice_triggered()
{
    CAStaff* staff = currentStaff();
    int voiceNumber = staff->voiceList().size() + 1;
    CANote::CAStemDirection stemDirection;
    if (voiceNumber == 1)
        stemDirection = CANote::StemNeutral;
    else {
        staff->voiceList()[0]->setStemDirection(CANote::StemUp);
        stemDirection = CANote::StemDown;
    }

    CACanorus::undo()->createUndoCommand(document(), tr("new voice", "undo"));
    if (staff) {
        staff->addVoice(new CAVoice(staff->name() + tr("Voice%1").arg(staff->voiceList().size() + 1), staff, stemDirection));
        staff->synchronizeVoices();
    }

    CACanorus::undo()->pushUndoCommand();
    CACanorus::rebuildUI(document(), currentSheet());
    uiVoiceNum->setRealValue(staff->voiceList().size());
}

/*!
	Removes the current voice from the staff and deletes its contents.
*/
void CAMainWin::on_uiRemoveVoice_triggered()
{
    CAVoice* voice = currentVoice();
    if (voice) {
        // Last voice cannot be deleted
        if (voice->staff()->voiceList().size() == 1) {
            /*int ret =*/QMessageBox::critical(
                this, tr("Canorus"),
                tr("Cannot delete the last voice in the staff!"));
            return;
        }

        int ret = QMessageBox::warning(
            this, tr("Canorus"),
            tr("Are you sure do you want to delete voice\n%1 and all its notes?").arg(voice->name()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            stopPlayback();
            CACanorus::undo()->createUndoCommand(document(), tr("voice removal", "undo"));
            currentScoreView()->clearSelection();
            uiVoiceNum->setRealValue(voice->staff()->voiceList().size() - 1);

            voice->staff()->removeVoice(voice);
            CACanorus::undo()->pushUndoCommand();
            CACanorus::rebuildUI(document(), currentSheet());

            voice->staff()->addVoice(voice);
            delete voice; // also removes voice from the staff
        }
    }
}

/*!
	Removes the current context from the sheet and all its contents.
*/
void CAMainWin::on_uiRemoveContext_triggered()
{
    CAContext* context = currentContext();
    if (context) {
        int ret = QMessageBox::warning(
            this, tr("Canorus"),
            tr("Are you sure do you want to delete context\n%1 and all its contents?").arg(context->name()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            stopPlayback();
            CACanorus::undo()->createUndoCommand(document(), tr("context removal", "undo"));
            CASheet* sheet = context->sheet();
            sheet->removeContext(context);
            CACanorus::undo()->pushUndoCommand();
            CACanorus::rebuildUI(document(), currentSheet());
            delete context;
        }
    }
}

void CAMainWin::on_uiContextType_toggled(bool checked, int)
{
    if (checked) {
        musElementFactory()->setMusElementType(CAMusElement::Undefined);
        setMode(InsertMode);
    }
}

void CAMainWin::on_uiEditMode_toggled(bool checked)
{
    if (checked)
        setMode(EditMode);
}

/*!
	Allows to set the current mode from privileged (ui/ctl) objects 
 */
void CAMainWin::setMode(CAMode mode, const QString& oModeHash)
{
    int iAllowed = _modeHash[oModeHash];
    qWarning("Allowed %d, max allowed %d, modeHash %s", iAllowed, _iNumAllowed, oModeHash.toLatin1().constData());
    if (iAllowed > 0 && iAllowed < _iNumAllowed)
        setMode(mode);
}

/*!
	Sets the current mode and updates the GUI and toolbars.
*/
void CAMainWin::setMode(CAMode mode)
{
    _mode = mode;

    switch (mode) {
    case InsertMode: {
        QPen p;
        p.setColor(Qt::blue);
        p.setWidth(3);

        for (int i = 0; i < _viewList.size(); i++) {
            if (_viewList[i]->viewType() == CAView::ScoreView) {
                CAScoreView* v = static_cast<CAScoreView*>(_viewList[i]);
                if (!v->playing())
                    v->setBorder(p);
            }
        }

        if (currentScoreView()) {
            /// \todo Set other mouse cursors
            currentScoreView()->setShadowNoteVisible(musElementFactory()->musElementType() == CAMusElement::Note);
            currentScoreView()->repaint();
        }

        break;
    }
    case EditMode: {
        QPen p;
        p.setColor(Qt::red);
        p.setWidth(3);

        for (int i = 0; i < _viewList.size(); i++) {
            if (_viewList[i]->viewType() == CAView::ScoreView) {
                CAScoreView* sv = static_cast<CAScoreView*>(_viewList[i]);
                if (!sv->playing())
                    (sv->unsetBorder());

                sv->setShadowNoteVisible(false);
                sv->repaint();
            }
        }

        if (currentScoreView()) {
            if (!currentScoreView()->selection().size()) {
                musElementFactory()->setMusElementType(CAMusElement::Undefined);
            }
            uiVoiceNum->setRealValue(0);
        }

        break;
    }
    case ReadOnlyMode:
    case ProgressMode:
    case NoDocumentMode:
        fprintf(stderr, "Warning: CAMainWin::setMode - Unhandled mode %d\n", mode);
        break;
    } // switch (mode)
    updateToolBars();
    if ((currentScoreView() && !currentScoreView()->textEditVisible()) || (!currentScoreView() && currentView()))
        currentView()->setFocus();
}

/*!
	Create hash to allow changing the current mode from priviledged (ui/ctl) objects 
 */
QString CAMainWin::createModeHash()
{
    QString oHash = QUuid::createUuid().toString();
    _modeHash.insert(oHash, _iNumAllowed++);
    return oHash;
}

/*!
	Rebuilds the GUI from data.

	This method is called eg. when multiple Views share the same data and a change has been made (eg. a
	note pitch has changed or a new element added). Views content is repositioned and redrawn (CAEngraver
	creates CADrawable elements for every score View, sources are updated in source Views etc.).

	\a sheet argument is a pointer to the data sheet where the change occured. This way only Views showing
	the given sheet are updated which speeds up the process.
	If \a sheet argument is null, all Views are rebuilt, but the Views contents, number and locations
	remain the same.

	If \a repaint is True (default) the rebuilt Views are also repainted. If False, Views content is
	only created but not yet drawn. This is useful when multiple operations which could potentially change the
	content are to happen and we want to actually draw it only at the end.
*/
void CAMainWin::rebuildUI(CASheet* sheet, bool repaint)
{
    if (rebuildUILock())
        return;

    setRebuildUILock(true);
    if (document()) {
        // update views
        for (int i = 0; i < _viewList.size(); i++) {
            if (sheet && _viewList[i]->viewType() == CAView::ScoreView && static_cast<CAScoreView*>(_viewList[i])->sheet() != sheet)
                continue;

            _viewList[i]->rebuild();

            if (_viewList[i]->viewType() == CAView::ScoreView)
                static_cast<CAScoreView*>(_viewList[i])->checkScrollBars();

            if (repaint)
                _viewList[i]->repaint();
        }

        // update tab name
        for (int i = 0; i < uiTabWidget->count(); i++) {
            uiTabWidget->setTabText(i, document()->sheetList()[i]->name());
        }
    } else {
        clearUI();
    }

    if (_resourceView) {
        _resourceView->rebuildUi();
    }

    updateWindowTitle();
    updateToolBars();
    setRebuildUILock(false);
}

/*!
	Rebuilds the GUI from data.

	This method is called eg. when multiple Views share the same data and a change has been made (eg. a
	note pitch has changed or a new element added). Views content is repositioned and redrawn (CAEngraver
	creates CADrawable elements for every score View, sources are updated in source Views etc.).

	This method in comparison to CAMainWin::rebuildUI(CASheet *s, bool repaint) rebuilds the whole GUI from
	scratch and creates new Views for the sheets. This method is called for example when a new document
	is created or opened.

	If \a repaint is True (default) the rebuilt Views are also repainted. If False, Views content is
	only created but not yet drawn. This is useful when multiple operations which could potentially change the
	content are to happen and we want to actually draw it only at the end.
*/
void CAMainWin::rebuildUI(bool repaint)
{
    if (rebuildUILock())
        return;

    setRebuildUILock(true);
    if (document()) {
        int curIndex = uiTabWidget->currentIndex();

        // save the current state of Views
        QList<QRectF> worldCoordsList;
        for (int i = 0; i < _viewList.size(); i++)
            if (_viewList[i]->viewType() == CAView::ScoreView)
                worldCoordsList << static_cast<CAScoreView*>(_viewList[i])->worldCoords();

        clearUI();
        for (int i = 0; i < document()->sheetList().size(); i++) {
            addSheet(document()->sheetList()[i]);

            // restore the current state of Views
            if (_viewList[i]->viewType() == CAView::ScoreView && i < worldCoordsList.size())
                static_cast<CAScoreView*>(_viewList[i])->setWorldCoords(worldCoordsList[i]);
        }

        for (int i = 0; i < _viewList.size(); i++) {
            _viewList[i]->rebuild();

            if (_viewList[i]->viewType() == CAView::ScoreView)
                static_cast<CAScoreView*>(_viewList[i])->checkScrollBars();

            if (repaint)
                _viewList[i]->repaint();
        }

        if (curIndex < uiTabWidget->count())
            uiTabWidget->setCurrentIndex(curIndex);
    } else {
        clearUI();
    }

    if (_resourceView) {
        _resourceView->rebuildUi();
    }

    updateWindowTitle();
    updateToolBars();
    setRebuildUILock(false);
}

/*!
	Processes the mouse press event \a e with world coordinates \a coords.
	Any action happened in any of the Views are always linked to these main window slots.

	\sa CAScoreView::mousePressEvent(), scoreViewMouseMove(), scoreViewWheel(), scoreViewKeyPress()
*/
void CAMainWin::scoreViewMousePress(QMouseEvent* e, const QPoint coords)
{
    CAScoreView* v = static_cast<CAScoreView*>(sender());
    QList<CADrawableMusElement*> oldSelection = v->selection();

    CADrawableContext* prevContext = v->currentContext();
    v->selectCElement(coords.x(), coords.y());

    QList<CADrawableMusElement*> l = v->musElementsAt(coords.x(), coords.y());
    CADrawableMusElement* newlySelectedElement = nullptr;
    int idx = -1;

    if (l.size() > 0) { // multiple elements can share the same coordinates
        if ((v->selection().size() > 0) && (!v->selection().contains(l.front()))) {
            if (e->modifiers() != Qt::ShiftModifier)
                v->clearSelection();
            v->addToSelection(newlySelectedElement = l[0]); // if the previous selection was not a single element or if the new list doesn't contain the selection set the first element in the available list to the selection
        } else {
            if (e->modifiers() == Qt::ShiftModifier && v->selection().size()) {
                v->removeFromSelection(l[0]); // shift used on an already selected element - toggle selection
            } else {
                idx = (v->selection().size() ? l.indexOf(v->selection().front()) : -1);
                v->clearSelection();
                v->addToSelection(newlySelectedElement = l[((++idx < l.size()) ? idx : 0)]); // if there are two or more elements with the same coordinates, select the next one (behind it). This way, you can click multiple times on the same place and you'll always select the other element.
            }
        }
    } else if (e->modifiers() == Qt::NoModifier) { // no elements at that coordinates
        v->clearSelection();
    }

    // always select the context the current element belongs to
    if (newlySelectedElement && (mode() != InsertMode || !uiInsertPlayable->isChecked()))
        v->setCurrentContext(newlySelectedElement->drawableContext());

    if (v->currentContext() && prevContext != v->currentContext() && mode() != InsertMode) { // new context was selected
        // voice number widget
        if (v->currentContext()->context()->contextType() == CAContext::Staff) {
            uiVoiceNum->setRealValue(0);
            uiVoiceNum->setMax(static_cast<CAStaff*>(v->currentContext()->context())->voiceList().size());
        }
    } else if (prevContext != v->currentContext() && uiInsertPlayable->isChecked()) { // but insert playable mode is active and context should remain the same
        v->setCurrentContext(prevContext);
    }

    if (v->resizeDirection() != CADrawable::Undefined) {
        CACanorus::undo()->createUndoCommand(document(), tr("resize", "undo"));
    }

    switch (mode()) {
    case EditMode: {
        v->clearSelectionRegionList();

        CADrawableMusElement* dElt = nullptr;
        CAMusElement* elt = nullptr;

        if (v->selection().size()) {
            dElt = v->selection().front();
            elt = dElt->musElement();
            if (!elt)
                break;

            // debug
            QString debugStr;
            QTextStream outStr(&debugStr);
            outStr << "drawableMusElement: " << dElt << ", x,y=" << dElt->xPos() << "," << dElt->yPos() << ", w,h=" << dElt->width() << "," << dElt->height() << ", dContext=" << dElt->drawableContext() << endl;
            outStr << "musElement: " << elt << ", timeStart=" << elt->timeStart() << ", timeEnd=" << elt->timeEnd() << ", context=" << elt->context();
            if (elt->isPlayable()) {
                outStr << ", voice=" << (static_cast<CAPlayable*>(elt))->voice() << ", voiceNr=" << (static_cast<CAPlayable*>(elt))->voice()->voiceNumber() << ", idxInVoice=" << (static_cast<CAPlayable*>(elt))->voice()->musElementList().indexOf(elt);
                outStr << ", voiceStaff=" << (static_cast<CAPlayable*>(elt))->voice()->staff();

                if (static_cast<CAPlayable*>(elt)->tuplet()) {
                    outStr << ", tuplet=" << static_cast<CAPlayable*>(elt)->tuplet();
                }

                if (elt->musElementType() == CAMusElement::Note)
                    outStr << ", pitch=" << static_cast<CANote*>(elt)->diatonicPitch().noteName();
            }
            if (elt->musElementType() == CAMusElement::Slur) {
                outStr << "noteStart=" << static_cast<CASlur*>(elt)->noteStart() << ", noteEnd=" << static_cast<CASlur*>(elt)->noteStart();
            }
            outStr << endl;
            qDebug().noquote() << debugStr;
        }

        // lyrics, texts, bookmarks, chord names
        if (v->textEditVisible() && oldSelection.size() && oldSelection.front()->musElement()) {
            confirmTextEdit(v, v->textEdit(), oldSelection.front()->musElement());
        }

        break;
    }
    case InsertMode: {
        // Insert context
        if (uiContextType->isChecked()) {
            // Add new Context
            CAContext* newContext = nullptr;
            CADrawableContext* dupContext = v->nearestUpContext(coords.x(), coords.y());
            switch (uiContextType->currentId()) {
            case CAContext::Staff: {
                CACanorus::undo()->createUndoCommand(document(), tr("new staff", "undo"));
                QString name = v->sheet()->findUniqueContextName(tr("Staff%1"));
                v->sheet()->insertContextAfter(
                    dupContext ? dupContext->context() : nullptr,
                    newContext = new CAStaff(name, v->sheet()));
                static_cast<CAStaff*>(newContext)->addVoice();
                break;
            }
            case CAContext::LyricsContext: {
                CACanorus::undo()->createUndoCommand(document(), tr("new lyrics context", "undo"));
                QString name = v->sheet()->findUniqueContextName(tr("LyricsContext%1"));
                v->sheet()->insertContextAfter(
                    dupContext ? dupContext->context() : nullptr,
                    newContext = new CALyricsContext(
                        name,
                        0, // No stanza number by default.
                        (v->sheet()->voiceList().size() ? v->sheet()->voiceList().at(0) : nullptr)));
                break;
            }
            case CAContext::FiguredBassContext: {
                CACanorus::undo()->createUndoCommand(document(), tr("new figured bass context", "undo"));
                QString name = v->sheet()->findUniqueContextName(tr("FiguredBassContext%1"));
                v->sheet()->insertContextAfter(
                    dupContext ? dupContext->context() : nullptr,
                    newContext = new CAFiguredBassContext(name, v->sheet()));
                break;
            }
            case CAContext::FunctionMarkContext: {
                CACanorus::undo()->createUndoCommand(document(), tr("new function mark context", "undo"));
                QString name = v->sheet()->findUniqueContextName(tr("FunctionMarkContext%1"));
                v->sheet()->insertContextAfter(
                    dupContext ? dupContext->context() : nullptr,
                    newContext = new CAFunctionMarkContext(name, v->sheet()));
                break;
            }
            case CAContext::ChordNameContext: {
                CACanorus::undo()->createUndoCommand(document(), tr("new chord name context", "undo"));
                QString name = v->sheet()->findUniqueContextName(tr("ChordNameContext%1"));
                v->sheet()->insertContextAfter(
                    dupContext ? dupContext->context() : nullptr,
                    newContext = new CAChordNameContext(name, v->sheet()));
                break;
            }
            }
            CACanorus::undo()->pushUndoCommand();
            CACanorus::rebuildUI(document(), v->sheet());

            if (!newContext) {
                qDebug() << "Error: newContext empty";
                break;
            }

            v->selectContext(newContext);
            if (newContext->contextType() == CAContext::Staff) {
                uiVoiceNum->setMax(1);
                uiVoiceNum->setRealValue(0);
            }
            uiEditMode->toggle();
            v->repaint();
            break;
        } else
            // Insert music element
            if (uiInsertPlayable->isChecked()) {
            // Add Note/Rest
            if (e->button() == Qt::RightButton && musElementFactory()->musElementType() == CAMusElement::Note)
                // place a rest when using right mouse button and note insertion is selected
                musElementFactory()->setMusElementType(CAMusElement::Rest);
            // show the dotted shadow note
            currentScoreView()->setShadowNoteLength(musElementFactory()->playableLength());
            currentScoreView()->updateHelpers();
        }

        // Insert playable/music element
        bool success = insertMusElementAt(coords, v);

        if (musElementFactory()->musElementType() == CAMusElement::Rest)
            musElementFactory()->setMusElementType(CAMusElement::Note);

        // Insert Syllable, Text, or ChordName
        if (!v->selection().isEmpty() && (uiInsertSyllable->isChecked() || uiInsertChordName->isChecked() || (uiMarkType->isChecked() && success && (musElementFactory()->markType() == CAMark::Text || musElementFactory()->markType() == CAMark::BookMark)))) {
            v->createTextEdit(v->selection().front());
        } else {
            v->removeTextEdit();
        }

        break;
    }
    case ReadOnlyMode:
    case ProgressMode:
    case NoDocumentMode:
        fprintf(stderr, "Warning: CAMainWin::scoreViewMousePress - Unhandled mode %d\n", mode());
        break;
    }

    CAPluginManager::action("onScoreViewClick", document(), nullptr, nullptr, this);

    updateToolBars();
    v->repaint();
}

/*!
	General View mouse press event.
	Sets it as the current view port.

	\sa scoreViewMousePress()
*/
void CAMainWin::viewClicked()
{
    CAView* v = static_cast<CAView*>(sender());
    setCurrentView(v);
    if (currentView()->parent()) // not floating
        currentViewContainer()->setCurrentView(currentView());
}

/*!
	Processes the mouse move event \a e with coordinates \a coords.
	Any action happened in any of the Views are always linked to its main window slots.

	\sa CAScoreView::mouseMoveEvent(), scoreViewMousePress(), scoreViewWheel(), scoreViewKeyPress()
*/
void CAMainWin::scoreViewMouseMove(QMouseEvent* e, QPoint coords)
{
    CAScoreView* c = static_cast<CAScoreView*>(sender());
    c->setMouseTracking(false); // disable mouse move events until we finish with drawing

    if ((mode() == InsertMode && musElementFactory()->musElementType() == CAMusElement::Note)) {
        CADrawableStaff* s;
        if (c->currentContext() ? (c->currentContext()->drawableContextType() == CADrawableContext::DrawableStaff) : 0)
            s = static_cast<CADrawableStaff*>(c->currentContext());
        else
            return;

        if (musElementFactory()->musElementType() == CAMusElement::Note || musElementFactory()->musElementType() == CAMusElement::Rest) {
            c->setShadowNoteVisible(true);
        }

        // calculate the musical pitch out of absolute world coordinates and the current clef
        int pitch = s->calculatePitch(coords.x(), coords.y());

        // write into the main window's status bar the note pitch name
        int iNoteAccs = s->getAccs(coords.x(), pitch) + musElementFactory()->noteExtraAccs();
        musElementFactory()->setNoteAccs(iNoteAccs);
        c->setShadowNoteAccs(iNoteAccs);
        c->updateHelpers();
        c->repaint();
    } else if (mode() != InsertMode) {
        if (c->resizeDirection() != CADrawable::Undefined) {
            // resize element
            int time = c->coordsToTime(coords.x());
            time -= (time % CAPlayableLength::musicLengthToTimeLength(CAPlayableLength::Sixteenth)); // round timelength to eighth notes length
            if (c->resizeDirection() == CADrawable::Right && (time > c->selection().at(0)->musElement()->timeStart())) {
                c->selection().at(0)->musElement()->setTimeLength(time - c->selection().at(0)->musElement()->timeStart());
                c->selection().at(0)->setWidth(c->timeToCoords(time) - c->selection().at(0)->xPos());
                c->repaint();
            } else if (c->resizeDirection() == CADrawable::Left && (time < c->selection().at(0)->musElement()->timeEnd())) {
                c->selection().at(0)->musElement()->setTimeLength(c->selection().at(0)->musElement()->timeEnd() - time);
                c->selection().at(0)->musElement()->setTimeStart(time);
                c->selection().at(0)->setXPos(c->timeToCoords(time));
                c->selection().at(0)->setWidth(c->timeToCoords(c->selection().at(0)->musElement()->timeEnd()) - c->timeToCoords(time));
                c->repaint();
            }
        } else if (e->buttons() == Qt::LeftButton && c->mouseDragActivated()) {
            // multiple selection
            c->clearSelectionRegionList();
            int x = c->lastMousePressCoords().x(), y = c->lastMousePressCoords().y(),
                w = coords.x() - c->lastMousePressCoords().x(), h = coords.y() - c->lastMousePressCoords().y();
            if (w < 0) {
                x += w;
                w *= (-1);
            } // user selected from right to left
            if (h < 0) {
                y += h;
                h *= (-1);
            } // user selected from bottom to top
            QRect selectionRect(x, y, w, h);

            QList<CADrawableContext*> dcList = c->findContextsInRegion(selectionRect);
            for (int i = 0; i < dcList.size(); i++) {
                QList<CADrawableMusElement*> musEltList = dcList[i]->findInRange(selectionRect.x(), selectionRect.x() + selectionRect.width());
                for (int j = 0; j < musEltList.size(); j++)
                    if (musEltList[j]->drawableMusElementType() == CADrawableMusElement::DrawableSlur)
                        musEltList.removeAt(j--);

                if (musEltList.size()) {
                    c->addSelectionRegion(QRect(musEltList.front()->xPos(), dcList[i]->yPos(),
                        musEltList.back()->xPos() + musEltList.back()->width() - musEltList.front()->xPos(), dcList[i]->height()));
                }
            }
            c->repaint();
        }
    }
    c->setMouseTracking(true); // re-enable mouse move events, we finished rendering
}

/*!
	Processes the mouse double click event.
	Currently this selects the current bar.

	\sa CAScoreView::selectAllCurBar()
 */
void CAMainWin::scoreViewDoubleClick(QMouseEvent*, const QPoint)
{
    if (mode() == EditMode) {
        static_cast<CAScoreView*>(sender())->selectAllCurBar();
        static_cast<CAScoreView*>(sender())->repaint();
    }
}

/*!
	Processes the mouse triple click event.
	Currently this selects the current line.

	\sa CAScoreView::selectAllCurContext()
 */
void CAMainWin::scoreViewTripleClick(QMouseEvent*, const QPoint)
{
    if (mode() == EditMode) {
        static_cast<CAScoreView*>(sender())->selectAllCurContext();
        static_cast<CAScoreView*>(sender())->repaint();
    }
}

/*!
	Processes the mouse move event \a e with coordinates \a coords.
	Any action happened in any of the Views are always linked to its main window slots.

	\sa CAScoreView::mouseReleaseEvent(), scoreViewMousePress(), scoreViewMouseMove(), scoreViewWheel(), scoreViewKeyPress()
*/
void CAMainWin::scoreViewMouseRelease(QMouseEvent* e, QPoint coords)
{
    CAScoreView* v = static_cast<CAScoreView*>(sender());
    if (v->resizeDirection() != CADrawable::Undefined) {
        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), v->sheet());
    }

    if (mode() != InsertMode) {
        if (v->mouseDragActivated()) {
            // area was selected
            v->clearSelectionRegionList();

            if (e->modifiers() == Qt::NoModifier)
                v->clearSelection();

            int x = v->lastMousePressCoords().x(), y = v->lastMousePressCoords().y(),
                w = coords.x() - v->lastMousePressCoords().x(), h = coords.y() - v->lastMousePressCoords().y();
            if (w < 0) {
                x += w;
                w *= (-1);
            } // user selected from right to left
            if (h < 0) {
                y += h;
                h *= (-1);
            } // user selected from bottom to top
            QRect selectionRect(x, y, w, h);

            QList<CADrawableContext*> dcList = v->findContextsInRegion(selectionRect);
            for (int i = 0; i < dcList.size(); i++) {
                QList<CADrawableMusElement*> musEltList = dcList[i]->findInRange(selectionRect.x(), selectionRect.x() + selectionRect.width());
                if (v->selectedVoice() && dcList[i]->context() != v->selectedVoice()->staff())
                    continue;

                for (int j = 0; j < musEltList.size(); j++)
                    if ((!musEltList[j]->isSelectable()) || (v->selectedVoice() && musEltList[j]->musElement()->isPlayable() && static_cast<CAPlayable*>(musEltList[j]->musElement())->voice() != v->selectedVoice()) || (musEltList[j]->drawableMusElementType() == CADrawableMusElement::DrawableSlur))
                        musEltList.removeAt(j--);
                v->addToSelection(musEltList);
            }
        } else {
            // single element or none selected
            CADrawableMusElement* dElt = nullptr;
            CAMusElement* elt = nullptr;

            if (v->selection().size() == 1) {
                dElt = v->selection().front();
                elt = dElt->musElement();
            }

            if (elt && (elt->musElementType() == CAMusElement::Syllable || elt->musElementType() == CAMusElement::ChordName || (elt->musElementType() == CAMusElement::Mark && (static_cast<CAMark*>(elt)->markType() == CAMark::Text || static_cast<CAMark*>(elt)->markType() == CAMark::BookMark)))) {
                v->createTextEdit(dElt);
            }
        }
        v->repaint();
    }
}

/*!
	Processes the mouse wheel event \a e with coordinates \a coords.
	Any action happened in any of the Views are always linked to its main window slots.

	\sa CAScoreView::wheelEvent(), scoreViewMousePress(), scoreViewMouseMove(), scoreViewKeyPress()
*/
void CAMainWin::scoreViewWheel(QWheelEvent* e, QPoint coords)
{
    CAScoreView* sv = static_cast<CAScoreView*>(sender());
    setCurrentView(sv);

    //int val;
    switch (e->modifiers()) {
    case Qt::NoModifier: //scroll horizontally
        sv->setWorldX(sv->worldX() - (0.5 * e->delta()) / sv->zoom(), CACanorus::settings()->animatedScroll());
        break;
    case Qt::AltModifier: //scroll horizontally, fast
        sv->setWorldX(sv->worldX() - e->delta() / sv->zoom(), CACanorus::settings()->animatedScroll());
        break;
    case Qt::ShiftModifier: //scroll vertically
        sv->setWorldY(sv->worldY() - (0.5 * e->delta()) / sv->zoom(), CACanorus::settings()->animatedScroll());
        break;
    case 0x0A000000: //SHIFT+ALT		//scroll vertically, fast
        sv->setWorldY(sv->worldY() - e->delta() / sv->zoom(), CACanorus::settings()->animatedScroll());
        break;
    case Qt::ControlModifier: //zoom
        if (e->delta() > 0)
            sv->setZoom(sv->zoom() * 1.1, coords.x(), coords.y(), CACanorus::settings()->animatedScroll());
        else
            sv->setZoom(sv->zoom() / 1.1, coords.x(), coords.y(), CACanorus::settings()->animatedScroll());

        break;
    }

    sv->repaint();
}

/*!
	Processes the key press event \a e.
	Any action happened in any of the Views are always linked to its main window slots.

	\sa CAScoreView::keyPressEvent(), scoreViewMousePress(), scoreViewMouseMove(), scoreViewWheel()
*/
void CAMainWin::scoreViewKeyPress(QKeyEvent* e)
{
    CAScoreView* v = static_cast<CAScoreView*>(sender());
    setCurrentView(v);

    // go to Insert mode (if in Select mode) before changing note length
    if (e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9 && e->key() != Qt::Key_3) {
        if (currentScoreView()->currentContext() && currentScoreView()->currentContext()->context()->contextType() == CAContext::Staff && v->selection().size() == 0) {
            uiInsertPlayable->setChecked(true);
        }
    }

    switch (e->key()) {
    // Music editing keys
    case Qt::Key_Right: {
        // select next music element
        v->selectNextMusElement(e->modifiers() == Qt::ShiftModifier);
        v->repaint();
        break;
    }

    case Qt::Key_Left: {
        // select previous music element
        v->selectPrevMusElement(e->modifiers() == Qt::ShiftModifier);
        v->repaint();
        break;
    }

    case Qt::Key_B: {
        // place a barline
        CADrawableContext* drawableContext;
        drawableContext = v->currentContext();

        if ((!drawableContext) || (drawableContext->context()->contextType() != CAContext::Staff))
            return;

        CAStaff* staff = static_cast<CAStaff*>(drawableContext->context());
        CAMusElement* right = nullptr;
        if (!v->selection().isEmpty()) {
            CAMusElement* e = v->selection().back()->musElement();
            if (e->musElementType() == CAMusElement::Note) {
                right = staff->next(static_cast<CANote*>(e)->getChord().back());
            } else {
                right = staff->next(e);
            }
        }

        CACanorus::undo()->createUndoCommand(document(), tr("insert barline", "undo"));
        CABarline* bar = new CABarline(
            CABarline::Single,
            staff,
            0);

        if (currentVoice()) {
            currentVoice()->insert(right, bar); // insert the barline in all the voices, timeStart is set
        } else {
            if (right && right->isPlayable())
                static_cast<CAPlayable*>(right)->voice()->insert(right, bar);
            else
                staff->voiceList()[0]->insert(right, bar);
        }

        staff->synchronizeVoices();

        if (CACanorus::settings()->useNoteChecker()) {
            _noteChecker.checkSheet(v->sheet());
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), v->sheet());
        v->selectMElement(bar);
        v->repaint();
        break;
    }

    case Qt::Key_Up: {
        if ((mode() == InsertMode) || (mode() == EditMode)) {
            bool rebuild = false;
            if (v->selection().size())
                CACanorus::undo()->createUndoCommand(document(), tr("rise note", "undo"));

            QList<CAMusElement*> eltList;
            for (int i = 0; i < v->selection().size(); i++) {
                CADrawableMusElement* elt = v->selection().at(i);

                // pitch note for one step higher
                if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
                    CANote* note = static_cast<CANote*>(elt->musElement());
                    CADiatonicKey key;
                    if (note->voice()->getKeySig(note)) {
                        key = note->voice()->getKeySig(note)->diatonicKey();
                    }
                    CADiatonicPitch pitch(note->diatonicPitch().noteName() + 1, key.noteAccs(note->diatonicPitch().noteName() + 1));
                    note->setDiatonicPitch(pitch);
                    CACanorus::undo()->pushUndoCommand();
                    rebuild = true;
                    eltList << note;
                }
            }

            if (CACanorus::settings()->playInsertedNotes()) {
                playImmediately(eltList);
            }

            if (rebuild)
                CACanorus::rebuildUI(document(), currentSheet());
        }
        break;
    }

    case Qt::Key_Down: {
        if ((mode() == InsertMode) || (mode() == EditMode)) {
            //bool rebuild = false;
            if (v->selection().size())
                CACanorus::undo()->createUndoCommand(document(), tr("lower note", "undo"));

            QList<CAMusElement*> eltList;
            for (int i = 0; i < v->selection().size(); i++) {
                CADrawableMusElement* elt = v->selection().at(i);

                // pitch note for one step higher
                if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
                    CANote* note = static_cast<CANote*>(elt->musElement());
                    CADiatonicKey key;
                    if (note->voice()->getKeySig(note)) {
                        key = note->voice()->getKeySig(note)->diatonicKey();
                    }
                    CADiatonicPitch pitch(note->diatonicPitch().noteName() - 1, key.noteAccs(note->diatonicPitch().noteName() - 1));
                    note->setDiatonicPitch(pitch);
                    CACanorus::undo()->pushUndoCommand();
                    //rebuild = true;
                    eltList << note;
                }
            }

            if (CACanorus::settings()->playInsertedNotes()) {
                playImmediately(eltList);
            }

            CACanorus::rebuildUI(document(), currentSheet());
        }
        break;
    }

    case Qt::Key_PageDown: {
        v->setWorldX(v->worldX() + v->worldWidth(), CACanorus::settings()->animatedScroll());
        v->repaint();
        break;
    }

    case Qt::Key_PageUp: {
        v->setWorldX(v->worldX() - v->worldWidth(), CACanorus::settings()->animatedScroll());
        v->repaint();
        break;
    }

    case Qt::Key_End: {
        v->setWorldX(std::numeric_limits<double>::max(), CACanorus::settings()->animatedScroll());
        v->repaint();
        break;
    }

    case Qt::Key_Home: {
        v->setWorldX(0, CACanorus::settings()->animatedScroll());
        v->repaint();
        break;
    }

    case Qt::Key_Plus: {
        if (mode() == InsertMode) {
            musElementFactory()->addNoteExtraAccs(1);
            musElementFactory()->addNoteAccs(1);
            v->setDrawShadowNoteAccs(musElementFactory()->noteExtraAccs() != 0);
            v->setShadowNoteAccs(musElementFactory()->noteAccs());
            v->repaint();
        } else if (mode() == EditMode) {
            if (!v->selection().isEmpty()) {
                QList<CAMusElement*> eltList;
                CASheet* sheet = nullptr;
                for (CADrawableMusElement* dElt : v->selection()) {
                    CAMusElement* elt = dElt->musElement();
                    if (elt->musElementType() == CAMusElement::Note) {
                        if (!sheet) {
                            sheet = static_cast<CANote*>(elt)->voice()->staff()->sheet();
                            CACanorus::undo()->createUndoCommand(document(), tr("add sharp", "undo"));
                        }
                        if (static_cast<CANote*>(elt)->diatonicPitch().accs() < 2) // limit the amount of accidentals
                            static_cast<CANote*>(elt)->diatonicPitch().setAccs(static_cast<CANote*>(elt)->diatonicPitch().accs() + 1);
                    }
                    eltList << elt;
                }
                if (sheet) { // something's changed
                    CACanorus::undo()->pushUndoCommand();
                    CACanorus::rebuildUI(document(), sheet);
                    if (CACanorus::settings()->playInsertedNotes()) {
                        playImmediately(eltList);
                    }
                }
            }
        }
        break;
    }

    case Qt::Key_Minus: {
        if (mode() == InsertMode) {
            musElementFactory()->subNoteExtraAccs(1);
            musElementFactory()->subNoteAccs(1);
            v->setDrawShadowNoteAccs(musElementFactory()->noteExtraAccs() != 0);
            v->setShadowNoteAccs(musElementFactory()->noteAccs());
            v->repaint();
        } else if (mode() == EditMode) {
            if (!v->selection().isEmpty()) {
                QList<CAMusElement*> eltList;
                CASheet* sheet = nullptr;
                for (CADrawableMusElement* dElt : v->selection()) {
                    CAMusElement* elt = dElt->musElement();
                    if (elt->musElementType() == CAMusElement::Note) {
                        if (!sheet) {
                            sheet = static_cast<CANote*>(elt)->voice()->staff()->sheet();
                            CACanorus::undo()->createUndoCommand(document(), tr("add flat", "undo"));
                        }
                        if (static_cast<CANote*>(elt)->diatonicPitch().accs() > -2) // limit the amount of accidentals
                            static_cast<CANote*>(elt)->diatonicPitch().setAccs(static_cast<CANote*>(elt)->diatonicPitch().accs() - 1);
                    }
                    eltList << elt;
                }
                if (sheet) { // something's changed
                    CACanorus::undo()->pushUndoCommand();
                    CACanorus::rebuildUI(document(), sheet);
                    if (CACanorus::settings()->playInsertedNotes()) {
                        playImmediately(eltList);
                    }
                }
            }
        }
        break;
    }

    case Qt::Key_Period:
    case Qt::Key_Colon:
    case Qt::Key_Greater: {
        if (mode() == InsertMode) {
            musElementFactory()->addPlayableDotted(1, musElementFactory()->playableLength());
            currentScoreView()->setShadowNoteLength(musElementFactory()->playableLength());
            currentScoreView()->updateHelpers();
            v->repaint();
        } else if (mode() == EditMode) {
            if (!(static_cast<CAScoreView*>(v))->selection().isEmpty()) {
                CACanorus::undo()->createUndoCommand(document(), tr("set dotted", "undo"));
                CAPlayable* p = dynamic_cast<CAPlayable*>(currentScoreView()->selection().front()->musElement());

                if (p) {
                    CAMusElement* next = nullptr;
                    int oldLength = p->timeLength();
                    int dots = p->playableLength().dotted() + (e->modifiers() == Qt::ShiftModifier ? -1 : 1);
                    if (dots < 0) {
                        dots += 4;
                    } else {
                        dots %= 4;
                    }

                    if (p->musElementType() == CAMusElement::Note) { // change the length of the whole chord
                        QList<CANote*> chord = static_cast<CANote*>(p)->getChord();
                        for (int i = 0; i < chord.size(); i++) {
                            next = p->voice()->next(p);
                            p->voice()->remove(chord[i]);
                            chord[i]->playableLength().setDotted(dots);
                            chord[i]->calculateTimeLength();
                        }
                        p->voice()->insert(next, chord[0]);
                        for (int i = 1; i < chord.size(); i++) {
                            p->voice()->insert(chord[0], chord[i], true);
                        }
                    } else if (p->musElementType() == CAMusElement::Rest) {
                        next = p->voice()->next(p);
                        p->voice()->remove(p);
                        p->playableLength().setDotted(dots);
                        p->calculateTimeLength();
                        p->voice()->insert(next, p);
                    }

                    int newLength = p->timeLength();
                    if (newLength < oldLength) { // insert rests, if the new length is shorter to keep consistency
                        QList<CARest*> rests = CARest::composeRests(oldLength - newLength, p->timeStart() + p->timeLength(), p->voice(), CARest::Normal);
                        for (int i = rests.size() - 1; i >= 0; i--) {
                            p->voice()->insert(next, rests[i]); // insert rests from shortest to longest
                        }
                    } else {
                        p->staff()->synchronizeVoices();
                    }

                    for (int j = 0; j < p->voice()->lyricsContextList().size(); j++) { // reposit syllables
                        p->voice()->lyricsContextList().at(j)->repositSyllables();
                    }

                    if (CACanorus::settings()->useNoteChecker()) {
                        _noteChecker.checkSheet(v->sheet());
                    }

                    CACanorus::undo()->pushUndoCommand();
                    CACanorus::rebuildUI(document(), p->staff()->sheet());
                }
            }
        }
        break;
    }

    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        deleteSelection(v, e->modifiers() == Qt::ShiftModifier, e->modifiers() == Qt::ShiftModifier, true);
        break;

    // Mode keys
    case Qt::Key_Escape:
        if (mode() == EditMode) {
            if (v->selection().size()) {
                v->clearSelection();
            } else {
                v->setCurrentContext(nullptr);
            }
            v->repaint();
        }
        uiEditMode->trigger();
        break;
    case Qt::Key_I:
        uiInsertPlayable->trigger();
        break;
    case Qt::Key_E:
        uiEditMode->trigger();
        break;
    case Qt::Key_Space:
        uiPlayFromSelection->trigger();
        break;
    case Qt::Key_F5:
        CACanorus::rebuildUI(document());
        break;

    // Note length keys
    case Qt::Key_1:
        uiPlayableLength->setCurrentId(CAPlayableLength::Whole, true);
        break;
    case Qt::Key_2:
        uiPlayableLength->setCurrentId(CAPlayableLength::Half, true);
        break;
    case Qt::Key_4:
        uiPlayableLength->setCurrentId(CAPlayableLength::Quarter, true);
        break;
    case Qt::Key_5:
        uiPlayableLength->setCurrentId(CAPlayableLength::Eighth, true);
        break;
    case Qt::Key_6:
        uiPlayableLength->setCurrentId(CAPlayableLength::Sixteenth, true);
        break;
    case Qt::Key_7:
        uiPlayableLength->setCurrentId(CAPlayableLength::ThirtySecond, true);
        break;
    case Qt::Key_8:
        uiPlayableLength->setCurrentId(CAPlayableLength::SixtyFourth, true);
        break;
    case Qt::Key_9:
        uiPlayableLength->setCurrentId(CAPlayableLength::HundredTwentyEighth, true);
        break;
    case Qt::Key_0:
        uiPlayableLength->setCurrentId(CAPlayableLength::Breve, true);
        break;
    case Qt::Key_Tab:
    case Qt::Key_Backtab: {
        int idx = -1;
        if (e->modifiers() == Qt::ControlModifier) // Control tab has different use
            idx = uiTabWidget->currentIndex();
        else if (currentVoice())
            idx = currentSheet()->voiceList().indexOf(currentVoice());
        else
            idx = currentSheet()->contextList().indexOf(currentContext());

        if (e->key() == Qt::Key_Tab) {
            idx++;
        } else {
            idx--;
        }

        // Next/Previous sheet selection
        if (e->modifiers() == Qt::ControlModifier) {
            // Cycle if first or last sheet was reached
            if (idx >= uiTabWidget->count())
                idx = 0;
            else if (idx < 0)
                idx = uiTabWidget->count() - 1;
            //if( idx >=0 && idx < uiTabWidget->count() )
            uiTabWidget->setCurrentIndex(idx);
        } else // Next/Previous Voice selection
            if (currentVoice() && (mode() == InsertMode || mode() == EditMode)) {
            // Cycle if first or last voice number was reached
            if (idx >= currentSheet()->voiceList().size())
                idx = 0;
            else if (idx < 0)
                idx = currentSheet()->voiceList().size() - 1;

            CAVoice* v = currentSheet()->voiceList()[idx];
            currentScoreView()->selectContext(v->staff());
            uiVoiceNum->setRealValue(v->voiceNumber()); // also calls setCurrentVoice and updates the UI
        } else // Next/Previous Context, if selection is empty
            if (currentScoreView()->selection().size() == 0) {
            if (idx >= currentSheet()->contextList().size())
                idx = 0;
            else if (idx < 0)
                idx = currentSheet()->contextList().size() - 1;
            currentScoreView()->selectContext(currentSheet()->contextList()[idx]);
            updateToolBars();
            currentScoreView()->repaint();
        }
        break;
    }
    }

    if (e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9 && e->key() != Qt::Key_3) {
        musElementFactory()->playableLength().setDotted(0);
        v->setShadowNoteLength(musElementFactory()->playableLength());
        v->updateHelpers();
        v->repaint();
    }

    updateToolBars();
}

/*!
	This method places the currently prepared music element in CAMusElementFactory to the staff or
	voice, dependent on the music element type and the View coordinates.
	
	\return True, if a new element of any kind was inserted; False, if an
	element was just edited or not handled at all.
*/
bool CAMainWin::insertMusElementAt(const QPoint coords, CAScoreView* v)
{
    CADrawableContext* drawableContext = v->currentContext();

    CAStaff* staff = nullptr;
    CADrawableStaff* drawableStaff = nullptr;
    if (drawableContext) {
        drawableStaff = dynamic_cast<CADrawableStaff*>(drawableContext);
        staff = dynamic_cast<CAStaff*>(drawableContext->context());
    }

    CADrawableMusElement* drawableRight = v->nearestRightElement(coords.x(), coords.y(), v->currentContext());

    CAMusElement* right = nullptr;
    if (drawableRight)
        right = drawableRight->musElement();

    bool success = false;

    if (!drawableContext)
        return false;

    CACanorus::undo()->createUndoCommand(document(), tr("insertion of music element", "undo"));

    switch (musElementFactory()->musElementType()) {
    case CAMusElement::Clef: {
        if (staff)
            success = musElementFactory()->configureClef(staff, right);
        break;
    }
    case CAMusElement::KeySignature: {
        if (staff)
            success = musElementFactory()->configureKeySignature(staff, right);
        break;
    }
    case CAMusElement::TimeSignature: {
        if (staff)
            success = musElementFactory()->configureTimeSignature(staff, right);
        break;
    }
    case CAMusElement::Barline: {
        if (staff)
            success = musElementFactory()->configureBarline(staff, right);
        break;
    }
    case CAMusElement::Mark: {
        if (v->musElementsAt(coords.x(), coords.y()).size())
            success = musElementFactory()->configureMark(v->musElementsAt(coords.x(), coords.y())[0]->musElement());
        break;
    }
    case CAMusElement::Note: { // Do we really need to do all that here??
        CAVoice* voice = currentVoice();

        if (!voice)
            break;

        CADrawableMusElement* left = v->nearestLeftElement(coords.x(), coords.y(), voice); // use nearestLeft search because it searches left borders
        CADrawableMusElement* dright = v->nearestRightElement(coords.x(), coords.y(), voice);

        if (left && left->musElement() && left->musElement()->musElementType() == CAMusElement::Note && left->xPos() <= coords.x() && (left->width() + left->xPos() >= coords.x())) {

            // user clicked inside x borders of the note - add a note to the chord

            if (voice->containsPitch(drawableStaff->calculatePitch(coords.x(), coords.y()), left->musElement()->timeStart()))
                break; // user clicked on an already placed note or wanted to place illegal length (not the one the chord is of) - return and do nothing

            success = musElementFactory()->configureNote(drawableStaff->calculatePitch(coords.x(), coords.y()), voice, left->musElement(), true);
        } else if (left && left->musElement() && left->musElement()->musElementType() == CAMusElement::Rest && left->xPos() <= coords.x() && (left->width() + left->xPos() >= coords.x())) {

            // user clicked inside x borders of the rest - replace the rest/rests with the note
            // same code for the Rest insertion

            CATuplet* tuplet = static_cast<CAPlayable*>(left->musElement())->tuplet();
            QList<CAPlayable*> playableList;
            int number = 0;
            int actualNumber = 0;
            if (tuplet) {
                playableList = tuplet->noteList();
                number = tuplet->number();
                actualNumber = tuplet->actualNumber();
                delete tuplet;
            }

            int timeSum = left->musElement()->timeLength();
            int timeLength = CAPlayableLength::playableLengthToTimeLength(musElementFactory()->playableLength());

            CAMusElement* next = nullptr;
            while ((next = voice->next(left->musElement())) && next->musElementType() == CAMusElement::Rest && timeSum < timeLength) {
                voice->remove(next);
                playableList.removeAll(static_cast<CAPlayable*>(next));
                timeSum += next->timeLength();
            }

            int tupIndex = playableList.indexOf(static_cast<CAPlayable*>(left->musElement()));
            playableList.removeAll(static_cast<CAPlayable*>(left->musElement()));
            voice->remove(left->musElement());

            if (timeSum - timeLength > 0) {
                // we removed too many rests - insert the delta of the missing rests
                QList<CARest*> rests = CARest::composeRests(timeSum - timeLength, next ? next->timeStart() : voice->lastTimeEnd(), voice, CARest::Normal);
                for (int i = rests.size() - 1; i >= 0; i--) {
                    voice->insert(next, rests[i]);
                    playableList.insert(tupIndex, rests[i]);
                }
                next = rests.last();
            }

            success = musElementFactory()->configureNote(drawableStaff->calculatePitch(coords.x(), coords.y()), voice, next, false);

            if (success) {
                playableList.insert(tupIndex, static_cast<CAPlayable*>(musElementFactory()->musElement().get()));
            }

            if (success && tuplet) {
                new CATuplet(number, actualNumber, playableList);
            }

            if (success && CACanorus::settings()->autoBar()) {
                CAStaff::placeAutoBar(static_cast<CAPlayable*>(musElementFactory()->musElement().get()));
            }
        } else {

            // user clicked outside x borders of the note or rest

            if (dright && dright->musElement() && dright->musElement()->isPlayable() && static_cast<CAPlayable*>(dright->musElement())->tuplet() && !static_cast<CAPlayable*>(dright->musElement())->isFirstInTuplet()) {
                delete static_cast<CAPlayable*>(dright->musElement())->tuplet();
            }

            success = musElementFactory()->configureNote(drawableStaff->calculatePitch(coords.x(), coords.y()), voice, dright ? dright->musElement() : nullptr, false);
            if (success && CACanorus::settings()->autoBar())
                CAStaff::placeAutoBar(static_cast<CAPlayable*>(musElementFactory()->musElement().get()));

            if (success && uiTupletType->isChecked()) {
                QList<CAPlayable*> elements;
                elements << static_cast<CAPlayable*>(musElementFactory()->musElement().get());

                for (int i = 1; i < uiTupletNumber->value(); i++) {
                    musElementFactory()->configureRest(voice, dright ? dright->musElement() : nullptr);
                    elements << static_cast<CAPlayable*>(musElementFactory()->musElement().get());
                }
                musElementFactory()->setMusElement(std::shared_ptr<CAPlayable>(elements[0]));

                new CATuplet(uiTupletNumber->value(), uiTupletActualNumber->value(), elements);
            }
        }

        if (success) {
            if (musElementFactory()->musElement()->musElementType() == CAMusElement::Note && CACanorus::settings()->playInsertedNotes()) {
                playImmediately(QList<CAMusElement*>() << musElementFactory()->musElement().get());
            }

            musElementFactory()->setNoteExtraAccs(0);
            v->setDrawShadowNoteAccs(false);
            v->setShadowNoteLength(musElementFactory()->playableLength());
            v->updateHelpers();
        }
        break;
    }
    case CAMusElement::Rest: {
        CAVoice* voice = currentVoice();

        if (!voice)
            break;

        CADrawableMusElement* left = v->nearestLeftElement(coords.x(), coords.y(), voice); // use nearestLeft search because it searches left borders
        CADrawableMusElement* dright = v->nearestRightElement(coords.x(), coords.y(), voice);

        if (left && left->musElement() && left->musElement()->isPlayable() && left->xPos() <= coords.x() && (left->width() + left->xPos() >= coords.x())) {

            // user clicked inside x borders of the rest or note - replace the rest/rests with the rest
            // same code for the Note insertion

            CATuplet* tuplet = static_cast<CAPlayable*>(left->musElement())->tuplet();
            QList<CAPlayable*> playableList;
            int number = 0;
            int actualNumber = 0;
            if (tuplet) {
                playableList = tuplet->noteList();
                number = tuplet->number();
                actualNumber = tuplet->actualNumber();
                delete tuplet;
            }

            int timeSum = left->musElement()->timeLength();
            int timeLength = CAPlayableLength::playableLengthToTimeLength(musElementFactory()->playableLength());

            CAMusElement* next = nullptr;
            // collect all preceeding rests to merge them as one, if needed
            while ((next = voice->next(left->musElement())) && next->musElementType() == CAMusElement::Rest && timeSum < timeLength) {
                voice->remove(next);
                playableList.removeAll(static_cast<CAPlayable*>(next));
                timeSum += next->timeLength();
            }

            // remove all notes in the chord except the last
            if (left->musElement()->musElementType() == CAMusElement::Note && static_cast<CANote*>(left->musElement())->isPartOfChord()) {
                QList<CANote*> chordNotes = static_cast<CANote*>(left->musElement())->getChord();
                for (int i = 0; i < chordNotes.size(); i++) {
                    if (chordNotes[i] == left->musElement()) {
                        continue;
                    }

                    playableList.removeAll(chordNotes[i]);
                    voice->remove(chordNotes[i]);
                }

                // also update next
                next = voice->next(static_cast<CANote*>(left->musElement())->getChord().last());
            }

            CAPlayable* playableToReplace = static_cast<CAPlayable*>(left->musElement());
            int tupIndex = playableList.indexOf(playableToReplace);
            playableList.removeAll(playableToReplace);
            voice->remove(playableToReplace);

            if (timeSum - timeLength > 0) {
                // we removed too many rests - insert the delta of the missing rests
                QList<CARest*> rests = CARest::composeRests(timeSum - timeLength, next ? next->timeStart() : voice->lastTimeEnd(), voice, CARest::Normal);
                for (int i = rests.size() - 1; i >= 0; i--) {
                    voice->insert(next, rests[i]);
                    playableList.insert(tupIndex, rests[i]);
                }
                next = rests.last();
            }

            success = musElementFactory()->configureRest(voice, next);

            if (success) {
                playableList.insert(tupIndex, static_cast<CAPlayable*>(musElementFactory()->musElement().get()));
            }

            if (success && tuplet) {
                new CATuplet(number, actualNumber, playableList);
            }

            if (success && CACanorus::settings()->autoBar()) {
                CAStaff::placeAutoBar(static_cast<CAPlayable*>(musElementFactory()->musElement().get()));
            }
        } else {
            if (dright && dright->musElement() && dright->musElement()->isPlayable() && static_cast<CAPlayable*>(dright->musElement())->tuplet() && !static_cast<CAPlayable*>(dright->musElement())->isFirstInTuplet()) {
                delete static_cast<CAPlayable*>(dright->musElement())->tuplet();
            }

            success = musElementFactory()->configureRest(voice, dright ? dright->musElement() : nullptr);
            if (success && CACanorus::settings()->autoBar())
                CAStaff::placeAutoBar(static_cast<CAPlayable*>(musElementFactory()->musElement().get()));
        }

        if (success) {
            v->setShadowNoteLength(musElementFactory()->playableLength());
            v->updateHelpers();
        }

        break;
    }
    case CAMusElement::Slur: {
        // Insert tie, slur or phrasing slur
        if (v->selection().size()) { // start note has to always be selected
            CAMusElement* eltStart = currentScoreView()->selection().front()->musElement();
            CANote* noteStart = nullptr;
            if (eltStart->musElementType() == CAMusElement::Note) {
                noteStart = static_cast<CANote*>(eltStart);
            } else if (eltStart->musElementType() == CAMusElement::Mark) {
                noteStart = dynamic_cast<CANote*>(static_cast<CAMark*>(eltStart)->associatedElement());
            }

            CAMusElement* eltEnd = currentScoreView()->selection().back()->musElement();
            CANote* noteEnd = nullptr;
            if (eltEnd->musElementType() == CAMusElement::Note) {
                noteEnd = static_cast<CANote*>(eltEnd);
            } else if (eltEnd->musElementType() == CAMusElement::Mark) {
                noteEnd = dynamic_cast<CANote*>(static_cast<CAMark*>(eltEnd)->associatedElement());
            }

            // Insert Tie
            if (noteStart && musElementFactory()->slurType() == CASlur::TieType) {
                noteEnd = nullptr; // find a fresh next note
                QList<CANote*> noteList = noteStart->voice()->getNoteList();

                if (noteStart->tieStart()) {
                    break; // return, if the tie already exists
                } else {
                    // create a new tie
                    for (int i = 0; i < noteList.count() && noteList[i]->timeStart() <= noteStart->timeEnd(); i++) {
                        if (noteList[i]->timeStart() == noteStart->timeEnd() && noteList[i]->diatonicPitch() == noteStart->diatonicPitch()) {
                            noteEnd = noteList[i];
                            break;
                        }
                    }
                }
                success = musElementFactory()->configureSlur(staff, noteStart, noteEnd);
            } else
                // Insert slur or phrasing slur
                if (noteStart && noteEnd && noteStart != noteEnd && (musElementFactory()->slurType() == CASlur::SlurType || musElementFactory()->slurType() == CASlur::PhrasingSlurType)) {
                if (noteStart->isPartOfChord())
                    noteStart = noteStart->getChord().at(0);
                if (noteEnd->isPartOfChord())
                    noteEnd = noteEnd->getChord().at(0);
                QList<CANote*> noteList = noteStart->voice()->getNoteList();
                int end = noteList.indexOf(noteEnd);
                for (int i = noteList.indexOf(noteStart); i <= end; i++)
                    if ((musElementFactory()->slurType() == CASlur::SlurType && (noteList[i]->slurStart() || noteList[i]->slurEnd())) || (musElementFactory()->slurType() == CASlur::PhrasingSlurType && (noteList[i]->phrasingSlurStart() || noteList[i]->phrasingSlurEnd())))
                        return false;

                if (((musElementFactory()->slurType() == CASlur::SlurType && (noteStart->slurStart())) || noteEnd->slurEnd()) || (((musElementFactory()->slurType() == CASlur::PhrasingSlurType && (noteStart->phrasingSlurStart()))) || noteEnd->phrasingSlurEnd()))
                    break; // return, if the slur already exist
                success = musElementFactory()->configureSlur(staff, noteStart, noteEnd);
            }
        }
        break;
    }
    case CAMusElement::FiguredBassMark: {
        // Insert figured bass number
        CADrawableMusElement* left = v->nearestLeftElement(coords.x(), coords.y());
        int timeStart = ((left && left->musElement()) ? left->musElement()->timeStart() : 0);

        if (drawableContext->context()->contextType() == CAContext::FiguredBassContext) {
            CAFiguredBassContext* fbc = static_cast<CAFiguredBassContext*>(drawableContext->context());
            CAFiguredBassMark* fbm = fbc->figuredBassMarkAtTimeStart(timeStart);

            if (fbm) {
                success = musElementFactory()->configureFiguredBassNumber(fbm);
            }
        }
        break;
    }
    case CAMusElement::FunctionMark: {
        // Insert function mark
        if (drawableContext->context()->contextType() == CAContext::FunctionMarkContext) {
            CAFunctionMarkContext* fmc = static_cast<CAFunctionMarkContext*>(drawableContext->context());
            CADrawableMusElement* dLeft = v->nearestLeftElement(coords.x(), coords.y());
            int timeStart = 0;
            if (dLeft) // find the nearest left element from the cursor
                timeStart = dLeft->musElement()->timeStart();
            QList<CAPlayable*> chord = currentSheet()->getChord(timeStart);
            int timeLength = chord.size() ? chord[0]->timeLength() : 256;
            for (int i = 0; i < chord.size(); i++) // find the shortest note in the chord
                if (chord[i]->timeLength() - (timeStart - chord[i]->timeStart()) < timeLength)
                    timeLength = chord[i]->timeLength() - (timeStart - chord[i]->timeStart());

            success = musElementFactory()->configureFunctionMark(fmc, timeStart, timeLength);
        }
        break;
    }
    case CAMusElement::MidiNote:
    case CAMusElement::Syllable:
    case CAMusElement::Tuplet:
    case CAMusElement::ChordName:
    case CAMusElement::Undefined:
        qDebug() << "Warning: CAMainWin::insertMusElementAt - Unhandled Element" << musElementFactory()->musElementType();
        break;
    }

    if (success) {
        if (staff)
            staff->synchronizeVoices();

        CACanorus::undo()->pushUndoCommand();
        if (CACanorus::settings()->useNoteChecker()) {
            _noteChecker.checkSheet(v->sheet());
        }
        CACanorus::rebuildUI(document(), v->sheet());
        CADrawableMusElement* d = v->selectMElement(musElementFactory()->musElement().get());
        musElementFactory()->emptyMusElem();

        // move the view to the right, if the right border was hit
        if (d && (d->xPos() > v->worldX() + 0.85 * v->worldWidth())) {
            v->setWorldX(d->xPos() - v->worldWidth() / 2, CACanorus::settings()->animatedScroll());
        }
    }

    return success;
}

/*!
	Main window's key press event.

	\sa viewKeyPressEvent()
*/
void CAMainWin::keyPressEvent(QKeyEvent*)
{
}

/*!
	Called when selection in score Views is changed.
*/
void CAMainWin::onScoreViewSelectionChanged()
{
    if (static_cast<CAScoreView*>(sender())->selection().size()) {
        uiCopy->setEnabled(true);
        uiCut->setEnabled(true);
    } else {
        uiCopy->setEnabled(false);
        uiCut->setEnabled(false);
    }

    CAPluginManager::action("onSelectionChanged", document(), nullptr, nullptr, this);
}

/*!
	Called every second when timeEditedTimer has timeout.
	Increases the locally stored time the document is being edited.
*/
void CAMainWin::onTimeEditedTimerTimeout()
{
    _timeEditedTime++;
}

/*!
	Called when playback is finished or interrupted by the user.
	It stops the playback, closes ports etc.
*/
void CAMainWin::playbackFinished()
{
    delete _playback;
    _playback = nullptr;
    uiPlayFromSelection->setChecked(false);

    if (_playbackView) {
        static_cast<CAScoreView*>(_playbackView)->setPlaying(false);
    }

    if (_repaintTimer) {
        _repaintTimer->stop();
        /// \todo crashes, if disconnected sometimes. -Matevz
        //_repaintTimer->disconnect();
        /// \todo crashes, if deleted. -Matevz
        //delete _repaintTimer;
    }
    CACanorus::midiDevice()->closeOutputPort();

    if (_playbackView) {
        static_cast<CAScoreView*>(_playbackView)->clearSelection();
        static_cast<CAScoreView*>(_playbackView)->addToSelection(_prePlaybackSelection);
        static_cast<CAScoreView*>(_playbackView)->unsetBorder();
    }
    _prePlaybackSelection.clear();

    _playbackView = nullptr;
    setMode(mode());
}

/*!
	Connected with the play button which starts the playback.
*/
void CAMainWin::on_uiPlayFromSelection_toggled(bool checked)
{
    if (checked && currentScoreView() && !_playback) {
        /// \todo replace raw pointer with shared or unique pointer
        _repaintTimer = new QTimer();
        _repaintTimer->setInterval(100);
        _repaintTimer->start();
        //connect(_repaintTimer, SIGNAL(timeout()), this, SLOT(on_repaintTimer_timeout())); //TODO: timeout is connected directly to repaint() directly. This should be optimized in the future -Matevz
        connect(_repaintTimer, SIGNAL(timeout()), this, SLOT(onRepaintTimerTimeout()));

        CACanorus::midiDevice()->openOutputPort(CACanorus::settings()->midiOutPort());
        /// \todo replace raw pointer with shared or unique pointer
        _playback = new CAPlayback(currentSheet(), CACanorus::midiDevice());
        if (currentScoreView()->selection().size() && currentScoreView()->selection().at(0)->musElement())
            _playback->setInitTimeStart(currentScoreView()->selection().at(0)->musElement()->timeStart());

        connect(_playback, SIGNAL(playbackFinished()), this, SLOT(playbackFinished()));

        QPen p;
        p.setColor(Qt::green);
        p.setWidth(3);

        _playbackView = currentView();
        currentScoreView()->setBorder(p);
        currentScoreView()->setPlaying(true); // set the deadlock for borders

        // Remember old selection
        _prePlaybackSelection = currentScoreView()->selection();
        currentScoreView()->clearSelection();

        _playback->start();
    } else if (_playback) {
        _playback->stop();
    }
}

/*!
	Called every few miliseconds during playback to repaint score View as the GUI can
	only be repainted from the main thread.
*/
void CAMainWin::onRepaintTimerTimeout()
{
    CAScoreView* sv = static_cast<CAScoreView*>(_playbackView);
    sv->clearSelection();
    for (int i = 0; i < _playback->curPlaying().size(); i++) {
        if (_playback->curPlaying().at(i)->musElementType() == CAMusElement::Note) {
            CADrawableMusElement* elt = sv->addToSelection(_playback->curPlaying()[i]);
            if (CACanorus::settings()->lockScrollPlayback()) {
                if (elt && (elt->xPos() > (sv->worldX() + sv->worldWidth()) || elt->xPos() < sv->worldX())) {
                    sv->setWorldX(elt->xPos() - 50, CACanorus::settings()->animatedScroll());
                }
            }
        }
    }

    sv->repaint();
}

void CAMainWin::on_uiLockScrollPlayback_toggled(bool val)
{
    CACanorus::settings()->setLockScrollPlayback(val);
}

void CAMainWin::on_uiSelectAll_triggered()
{
    if (!currentView())
        return;
    if (currentView()->viewType() == CAView::ScoreView)
        static_cast<CAScoreView*>(currentView())->selectAll();
    else if (currentView()->viewType() == CAView::SourceView)
        static_cast<CASourceView*>(currentView())->selectAll();
    currentView()->repaint();
}

void CAMainWin::on_uiInvertSelection_triggered()
{
    if (currentView() && currentView()->viewType() == CAView::ScoreView) {
        static_cast<CAScoreView*>(currentView())->invertSelection();
        currentView()->repaint();
    }
}

void CAMainWin::on_uiZoomToSelection_triggered()
{
    if (_currentView->viewType() == CAView::ScoreView)
        (static_cast<CAScoreView*>(_currentView))->zoomToSelection(CACanorus::settings()->animatedScroll());
}

void CAMainWin::on_uiZoomToFit_triggered()
{
    if (_currentView->viewType() == CAView::ScoreView)
        (static_cast<CAScoreView*>(_currentView))->zoomToFit();
}

void CAMainWin::on_uiZoomToWidth_triggered()
{
    if (_currentView->viewType() == CAView::ScoreView)
        (static_cast<CAScoreView*>(_currentView))->zoomToWidth();
}

void CAMainWin::on_uiZoomToHeight_triggered()
{
    if (_currentView->viewType() == CAView::ScoreView)
        (static_cast<CAScoreView*>(_currentView))->zoomToHeight();
}

void CAMainWin::closeEvent(QCloseEvent* event)
{
    if (!handleUnsavedChanges()) {
        event->ignore();
    } else {
        clearUI();
        event->accept();
    }
}

void CAMainWin::on_uiOpenDocument_triggered()
{
    if (!handleUnsavedChanges()) {
        return;
    }

    if (CAMainWin::uiOpenDialog->exec() && CAMainWin::uiOpenDialog->selectedFiles().size()) {
        openDocument(CAMainWin::uiOpenDialog->selectedFiles().at(0));
    }
}

/**
	Calls the save document method or save as, if the document doesn't exist yet.
	Returns True, if the document was saved; False otherwise.
*/
bool CAMainWin::on_uiSaveDocument_triggered()
{
    QString s;
    if (document()) {
        if ((s = document()->fileName()).isEmpty()) {
            return on_uiSaveDocumentAs_triggered();
        } else {
            return saveDocument(s);
        }
    }
    return false;
}

/**
	Calls the save document method.
	Returns True, if the document was saved; False otherwise.
*/
bool CAMainWin::on_uiSaveDocumentAs_triggered()
{
    if (document()) {
        if (CAMainWin::uiSaveDialog) {
            CAMainWin::uiSaveDialog->exec();
            if (CAMainWin::uiSaveDialog->selectedFiles().size()) {
                QString s = CAMainWin::uiSaveDialog->selectedFiles().at(0);
                // append the extension, if the filename doesn't contain a dot
                //int i;
                if (!s.contains('.')) {
                    int left = uiSaveDialog->selectedNameFilter().indexOf("(*.") + 2;
                    int len = uiSaveDialog->selectedNameFilter().size() - left - 1;
                    s.append(uiSaveDialog->selectedNameFilter().mid(left, len));
                }

                return saveDocument(s);
            } else {
                qWarning() << "Save Document: No file selected.";
                return false;
            }
        } else {
            qWarning() << "Save Document: Save Dialog does not exist.";
            return false;
        }
    } else {
        qWarning() << "Save Document: Missing ressources to create Save Dialog.";
        return false;
    }
}

/*!
	Opens a document with the given absolute file name.
	The previous document will be lost.

	Returns a pointer to the opened document or null if opening the document has failed.
*/
CADocument* CAMainWin::openDocument(const QString& fileName)
{
    stopPlayback();

    if (_importFile) {
        _importFile.reset();
    }

    if (fileName.endsWith(".xml")) {
        _importFile = std::make_unique<CACanorusMLImport>();
        uiSaveDialog->selectNameFilter(CAFileFormats::CANORUSML_FILTER);
    } else if (fileName.endsWith(".can")) {
        _importFile = std::make_unique<CACanImport>();
        uiSaveDialog->selectNameFilter(CAFileFormats::CAN_FILTER);
    } else {
        return nullptr; // FIXME Failing quietly, add error message
    }

    connect(_importFile.get(), SIGNAL(importDone(int)), this, SLOT(onImportDone(int)));
    _importFile->setStreamFromFile(fileName);
    _importFile->importDocument();

    _mainWinProgressCtl.startProgress(*_importFile.get());
    return _importFile->importedDocument();
}

/*!
	Opens the given document.
	The previous document will be lost.

	Returns a pointer to the opened document or null if opening the document has failed.
*/
CADocument* CAMainWin::openDocument(CADocument* doc)
{
    stopPlayback();
    if (doc) {
        if (document() && CACanorus::mainWinCount(document()) == 1) {
            CACanorus::undo()->deleteUndoStack(document());
            clearUI();
            delete document();
        }

        setDocument(doc);
        if (!doc->fileName().isEmpty()) {
            CACanorus::insertRecentDocument(doc->fileName());

            QDir dir(QFileInfo(doc->fileName()).absoluteDir());
            uiOpenDialog->setDirectory(dir);
            uiSaveDialog->setDirectory(dir);
            uiExportDialog->setDirectory(dir);
            uiImportDialog->setDirectory(dir);
        }
        CACanorus::undo()->createUndoStack(document());

        uiCloseDocument->setEnabled(true);
        if (CACanorus::settings()->useNoteChecker()) {
            for (int i = 0; i < doc->sheetList().size(); i++) {
                _noteChecker.checkSheet(doc->sheetList()[i]);
            }
        }
        rebuildUI(); // local rebuild only
        if (doc->sheetList().size())
            uiTabWidget->setCurrentIndex(0);

        setMode(EditMode);

        // select the first context automatically
        if (document() && document()->sheetList().size() && document()->sheetList()[0]->contextList().size())
            currentScoreView()->selectContext(document()->sheetList()[0]->contextList()[0]);
        updateToolBars();

        return doc;
    }

    return nullptr;
}

/*!
	Saves the current document to a given absolute \a fileName.
	This function is usually called when the filename was entered and save document dialog
	successfully closed.
	If \a recovery is False (default), it changes the timeEdit, fileName and some other document
	properties before saving it.

	Returns True, if the save was complete; False otherwise.
*/
bool CAMainWin::saveDocument(QString fileName)
{
    document()->setTimeEdited(document()->timeEdited() + _timeEditedTime);
    document()->setDateLastModified(QDateTime::currentDateTime());
    CACanorus::restartTimeEditedTimes(document());

    CAExport* save = nullptr;
    if (fileName.endsWith(".xml")) { // check the filename extension directly without accessing the uiSaveDialog object due to a bug in Qt. -Matevz
        /// \todo replace raw pointer with shared or unique pointer
        save = new CACanorusMLExport();
    } else if (fileName.endsWith(".can")) {
        /// \todo replace raw pointer with shared or unique pointer
        save = new CACanExport();
    }

    if (save) {
        save->setStreamToFile(fileName);
        save->exportDocument(document());
        save->wait();

        if (save->status() == 0) {
            document()->setFileName(fileName);
            CACanorus::insertRecentDocument(fileName);
            delete save;

            QDir dir(QFileInfo(fileName).absoluteDir());
            uiOpenDialog->setDirectory(dir);
            uiSaveDialog->setDirectory(dir);
            uiExportDialog->setDirectory(dir);
            uiImportDialog->setDirectory(dir);

            document()->setModified(false);
            updateWindowTitle();
            return true;
        } else {
            QMessageBox::critical(
                this,
                tr("Error while saving document"),
                tr("The document was not saved!\nError number %1 %2.").arg(save->status()).arg(save->readableStatus()));

            delete save;
            return false;
        }
    }

    QMessageBox::critical(
        this,
        tr("Error while saving document"),
        tr("Unknown file format %1.").arg(uiSaveDialog->selectedNameFilter()));
    return false;
}

void CAMainWin::updateWindowTitle()
{
    if (document() && !document()->title().isEmpty()) {
        setWindowTitle(document()->title() + " - Canorus");
    } else if (document() && !document()->fileName().isEmpty()) {
        setWindowTitle(document()->fileName().right(document()->fileName().size() - document()->fileName().lastIndexOf('/') - 1) + " - Canorus");
    } else if (document()) {
        setWindowTitle(tr("Untitled") + " - Canorus");
    } else {
        setWindowTitle("Canorus");
    }

    if (document() && document()->isModified()) {
        setWindowTitle(windowTitle() + " " + tr("(modified)"));
    }
}

void CAMainWin::onMidiInEvent(QVector<unsigned char> m)
{
    _keybdInput->onMidiInEvent(m);
    return;
}

/*!
	Called when File->Export is clicked.
*/
void CAMainWin::on_uiExportDocument_triggered()
{
    QStringList fileNames;
    QString fileExtString;
    QStringList fileExtList;
    int ffound = uiExportDialog->exec();
    if (!ffound)
        return;

    // ! Warning: If there is still a running export instance
    // !               this will stop the old one (kill it actually)
    /// \todo: maybe block new export until the old is finished
    if (_poExp) // Delete old export instance
        delete _poExp;

    fileNames = uiExportDialog->selectedFiles();

    QString s = fileNames[0];
    if (s.isEmpty()) {
        QMessageBox::information(nullptr, tr("No file name"), tr("Warning: No file name for export specified."));
        return;
    }

    if (!s.contains('.')) {
        int left = uiExportDialog->selectedNameFilter().indexOf("(*.") + 2;
        int len = uiExportDialog->selectedNameFilter().size() - left - 1;
        fileExtString = uiExportDialog->selectedNameFilter().mid(left, len);
        // the default file extension is the first one:
        fileExtList = fileExtString.split(" ");
        s.append(fileExtList[0]);
    }

    if (CAPluginManager::exportFilterExists(uiExportDialog->selectedNameFilter())) {
        CAPluginManager::exportAction(uiExportDialog->selectedNameFilter(), document(), s);
    } else {
        if (uiExportDialog->selectedNameFilter() == CAFileFormats::MIDI_FILTER) {
            /// \todo replace raw pointer with shared or unique pointer
            CAMidiExport* pme = new CAMidiExport;
            _poExp = pme;
        } else if (uiExportDialog->selectedNameFilter() == CAFileFormats::LILYPOND_FILTER) {
            /// \todo replace raw pointer with shared or unique pointer
            CALilyPondExport* ple = new CALilyPondExport;
            _poExp = ple;
        } else if (uiExportDialog->selectedNameFilter() == CAFileFormats::MUSICXML_FILTER) {
            /// \todo replace raw pointer with shared or unique pointer
            CAMusicXmlExport* musicxml = new CAMusicXmlExport;
            _poExp = musicxml;
        } else if (uiExportDialog->selectedNameFilter() == CAFileFormats::PDF_FILTER) {
            /// \todo replace raw pointer with shared or unique pointer
            CAPDFExport* ppe = new CAPDFExport;
            _poExp = ppe;
        } else if (uiExportDialog->selectedNameFilter() == CAFileFormats::SVG_FILTER) {
            /// \todo replace raw pointer with shared or unique pointer
            CASVGExport* pse = new CASVGExport;
            _poExp = pse;
        } else {
            //TODO: unknown/unsupported format, raise an error
            return;
        }
        if (_poExp) {
            _poExp->setStreamToFile(s);
            // @ToDo Complete document export is not supported currently
            // Also when (context) to print current sheet and when the whole document ?
            //_poExp->exportDocument( document() );
            _poExp->exportSheet(currentSheet());
            _poExp->wait();
            //delete _poExp;
        }
    }
}

/*!
	Called when File->Import is clicked.
*/
void CAMainWin::on_uiImportDocument_triggered()
{
    if (!handleUnsavedChanges()) {
        return;
    }

    QStringList fileNames;
    int ffound = uiImportDialog->exec();
    if (ffound)
        fileNames = uiImportDialog->selectedFiles();

    if (!ffound)
        return;
    stopPlayback();

    QString s = fileNames[0];

    if (CAPluginManager::importFilterExists(uiImportDialog->selectedNameFilter())) {
        // Import done using a scripting engine
        /// \todo replace raw pointer with shared or unique pointer
        setDocument(new CADocument());
        CACanorus::undo()->createUndoStack(document());
        uiCloseDocument->setEnabled(true);

        CAPluginManager::importAction(uiImportDialog->selectedNameFilter(), document(), fileNames[0]);

        CACanorus::rebuildUI(document());
    } else {
        if (_importFile) {
            _importFile.reset();
        }

        if (uiImportDialog->selectedNameFilter() == CAFileFormats::MIDI_FILTER) {
            if (!document())
                newDocument();
            _importFile = std::make_unique<CAMidiImport>();
            if (_importFile) {
                _importFile->setStreamFromFile(s);
                connect(_importFile.get(), SIGNAL(importDone(int)), this, SLOT(onImportDone(int)));
                _importFile->importSheet();
            }
        } else if (uiImportDialog->selectedNameFilter() == CAFileFormats::LILYPOND_FILTER) {
            // activate this filter in src/canorus.cpp when sheet import is usable
            if (!document())
                newDocument();
            _importFile = std::make_unique<CALilyPondImport>(document());
            if (_importFile) {
                _importFile->setStreamFromFile(s);
                connect(_importFile.get(), SIGNAL(importDone(int)), this, SLOT(onImportDone(int)));
                _importFile->importSheet();
            }
        } else if (uiImportDialog->selectedNameFilter() == CAFileFormats::MUSICXML_FILTER) {
            _importFile = std::make_unique<CAMusicXmlImport>();
            if (_importFile) {
                _importFile->setStreamFromFile(s);
                connect(_importFile.get(), SIGNAL(importDone(int)), this, SLOT(onImportDone(int)));
                _importFile->importDocument();
            }
        } else if (uiImportDialog->selectedNameFilter() == CAFileFormats::MXL_FILTER) {
            _importFile = std::make_unique<CAMXLImport>();
            if (_importFile) {
                _importFile->setStreamFromFile(s);
                connect(_importFile.get(), SIGNAL(importDone(int)), this, SLOT(onImportDone(int)));
                _importFile->importDocument();
            }
        }
        if (_importFile) {
            _mainWinProgressCtl.startProgress(*_importFile.get());
        }
    }
}

void CAMainWin::onImportDone(int)
{
    CAImport* import = static_cast<CAImport*>(sender());

    if (!import) {
        return;
    }

    bool success = (import->status() == 0);

    if (success) {
        if (import->importedDocument()) {
            openDocument(import->importedDocument());
        } else {
            if (import->importedSheet()) {
                addSheet(import->importedSheet());
                document()->addSheet(import->importedSheet());
                if (CACanorus::settings()->useNoteChecker()) {
                    _noteChecker.checkSheet(import->importedSheet());
                }
                CACanorus::rebuildUI(document());
            }
        }

        // select the first context automatically
        if (document() && document()->sheetList().size() && document()->sheetList()[0]->contextList().size()) {
            currentScoreView()->selectContext(document()->sheetList()[0]->contextList()[0]);
        }

        updateToolBars();
    } else {
        QMessageBox::critical(this, tr("Canorus"),
            tr("Error while opening the file!\nError %1: ").arg(import->status()) + import->readableStatus());
    }
}

/*!
	Called when Export directly to PDF action is clicked.
*/
void CAMainWin::on_uiExportToPdf_triggered()
{
    uiExportDialog->setNameFilter(CAFileFormats::PDF_FILTER);
    on_uiExportDocument_triggered();
}

void CAMainWin::onExportDone(int)
{
}

/*!
	Called when a user changes the current voice number.
*/
void CAMainWin::on_uiVoiceNum_valChanged(int voiceNr)
{
    if (currentScoreView()) {
        if (voiceNr && currentScoreView()->currentContext() && currentScoreView()->currentContext()->context()->contextType() == CAContext::Staff && voiceNr <= static_cast<CAStaff*>(currentScoreView()->currentContext()->context())->voiceList().size()) {
            setCurrentVoice(static_cast<CAStaff*>(currentScoreView()->currentContext()->context())->voiceList()[voiceNr - 1]);
        } else {
            setCurrentVoice(nullptr);
        }
    }
}

/*!
	Brings up the properties dialog.
*/
void CAMainWin::on_uiVoiceProperties_triggered()
{
    CAPropertiesDialog::voiceProperties(currentVoice(), this);
}

/*!
	Changes the offset of the clef.
*/
void CAMainWin::on_uiClefOffset_valueChanged(int newOffset)
{
    if (oldUiClefOffsetValue == 0 && qAbs(newOffset) == 1) {
        uiClefOffset->setValue((newOffset / qAbs(newOffset)) * 2);
        return;
    } else if (qAbs(oldUiClefOffsetValue) == 2 && qAbs(newOffset) == 1) {
        uiClefOffset->setValue(0);
        return;
    }

    oldUiClefOffsetValue = newOffset;
    if (mode() == InsertMode) {
        musElementFactory()->setClefOffset(newOffset);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CACanorus::undo()->createUndoCommand(document(), tr("change clef offset", "undo"));
            CAClef* clef = dynamic_cast<CAClef*>(v->selection().at(0)->musElement());

            if (clef) {
                clef->setOffset(CAClef::offsetFromReadable(newOffset));
                CACanorus::undo()->pushUndoCommand();
                CACanorus::rebuildUI(document(), currentSheet());
            }
        }
    }
}

/*!
	Gets the current voice and sets its name.
*/
void CAMainWin::on_uiVoiceName_returnPressed()
{
    CAVoice* voice = currentVoice();
    if (voice) {
        CACanorus::undo()->createUndoCommand(document(), tr("change voice name", "undo"));
        CACanorus::undo()->pushUndoCommand();
        voice->setName(uiVoiceName->text());
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

/*!
	Changes voice instrument.
*/
void CAMainWin::on_uiVoiceInstrument_activated(int index)
{
    if (!currentVoice() || index < 0)
        return;

    CACanorus::undo()->createUndoCommand(document(), tr("change voice instrument", "undo"));
    CACanorus::undo()->pushUndoCommand();
    currentVoice()->setMidiProgram(static_cast<unsigned char>(index));
    CACanorus::rebuildUI(document(), currentSheet());
}

void CAMainWin::on_uiInsertPlayable_toggled(bool checked)
{
    if (checked) {
        if (!uiVoiceNum->getRealValue())
            uiVoiceNum->setRealValue(1); // select the first voice if none selected

        musElementFactory()->setMusElementType(CAMusElement::Note);
        setMode(InsertMode);

        on_uiPlayableLength_toggled(uiPlayableLength->isChecked(), uiPlayableLength->currentId());
    }
}

void CAMainWin::on_uiInsertSyllable_toggled(bool checked)
{
    if (checked) {
        setMode(InsertMode);
    }
}

void CAMainWin::on_uiInsertFBM_toggled(bool checked)
{
    if (checked) {
        musElementFactory()->setMusElementType(CAMusElement::FiguredBassMark);
        setMode(InsertMode);
    }
}

void CAMainWin::on_uiInsertFM_toggled(bool checked)
{
    if (checked) {
        musElementFactory()->setMusElementType(CAMusElement::FunctionMark);
        setMode(InsertMode);
    }
}

void CAMainWin::on_uiPlayableLength_toggled(bool, int buttonId)
{
    // Read currently selected entry from tool button menu
    CAPlayableLength length = CAPlayableLength(static_cast<CAPlayableLength::CAMusicLength>(buttonId));

    if (mode() == InsertMode) {
        // New note length type
        musElementFactory()->setPlayableLength(length);
        if (currentScoreView()) {
            currentScoreView()->setShadowNoteLength(musElementFactory()->playableLength());
            currentScoreView()->updateHelpers();
        }
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change playable length", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAPlayable* p = dynamic_cast<CAPlayable*>(v->selection().at(i)->musElement());

            if (p) {
                CAMusElement* next = nullptr;
                int oldLength = p->timeLength();
                int newLength = CAPlayableLength::playableLengthToTimeLength(length);
                if (p->musElementType() == CAMusElement::Note) { // change the length of the whole chord
                    QList<CANote*> chord = static_cast<CANote*>(p)->getChord();
                    next = p->voice()->next(chord.back());
                    for (int i = 0; i < chord.size(); i++) {
                        p->voice()->remove(chord[i]);
                        chord[i]->setPlayableLength(length);
                        chord[i]->calculateTimeLength();
                    }
                    p->voice()->insert(next, chord[0], false);
                    for (int i = 1; i < chord.size(); i++) {
                        p->voice()->insert(chord[0], chord[i], true);
                    }
                } else if (p->musElementType() == CAMusElement::Rest) {
                    next = p->voice()->next(p);
                    p->voice()->remove(p);
                    p->setPlayableLength(length);
                    p->calculateTimeLength();
                    p->voice()->insert(next, p);
                }

                if (newLength < oldLength) { // insert rests, if the new length is shorter to keep consistency
                    QList<CARest*> rests = CARest::composeRests(oldLength - newLength, p->timeStart() + p->timeLength(), p->voice(), CARest::Normal);
                    for (int i = rests.size() - 1; i >= 0; i--) {
                        p->voice()->insert(next, rests[i]); // insert rests from shortest to longest
                    }
                } else {
                    p->staff()->synchronizeVoices();
                }

                for (int j = 0; j < p->voice()->lyricsContextList().size(); j++) { // reposit syllables
                    p->voice()->lyricsContextList().at(j)->repositSyllables();
                }
            }
        }

        if (CACanorus::settings()->useNoteChecker()) {
            _noteChecker.checkSheet(v->sheet());
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

/*!
	Function called when user types the text of the syllable or chord name.

	The following behaviour is implemented:
		- alphanumeric keys are pressed - writes text
		- spacebar is pressed - creates the current syllable/chord name and jumps to the next syllable
		- return is pressed - creates the current syllable/chord name and hides the edit widget
		- left key is pressed - if cursorPosition()==0, jumps to the previous syllable/chord name
		- right key is pressed - if cursorPosition()==length(), same as spacebar
		- escape key is pressed - hides the edit widget and cancels any changes to syllable/chord name

*/
void CAMainWin::onTextEditKeyPressEvent(QKeyEvent* e)
{
    if (!currentScoreView())
        return;
    CATextEdit* textEdit = static_cast<CATextEdit*>(sender());

    CAScoreView* v = currentScoreView();
    CAMusElement* elt = (v->selection().size() ? v->selection().front()->musElement() : nullptr);

    if (!elt)
        return;

    switch (elt->musElementType()) {
    case CAMusElement::Syllable:
    case CAMusElement::ChordName: {
        if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return || (e->key() == Qt::Key_Right && textEdit->cursorPosition() == textEdit->text().size()) || ((e->key() == Qt::Key_Left || e->key() == Qt::Key_Backspace) && textEdit->cursorPosition() == 0) || (elt->musElementType() == CAMusElement::Syllable && CACanorus::settings()->finaleLyricsBehaviour() && e->key() == Qt::Key_Minus)) {
            // one of control keys were hit, create or edit syllable/chord name
            confirmTextEdit(currentScoreView(), textEdit, elt);

            CAMusElement* next = nullptr;
            if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Right || e->key() == Qt::Key_Return) {
                // move to the right neighbor
                next = elt->context()->next(elt);
            } else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Backspace) {
                // move to the left neighbor
                next = elt->context()->previous(elt);
            } else if (e->key() == Qt::Key_Minus && elt->musElementType() == CAMusElement::Syllable) {
                // move to the right neighbor + set hyphen
                static_cast<CASyllable*>(elt)->setHyphenStart(true);
                next = elt->context()->next(elt);
            }
            if (next) {
                CADrawableMusElement* dNext = v->selectMElement(next);
                v->createTextEdit(dNext);
                if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Right || e->key() == Qt::Key_Return) {
                    // edit widget cursor is at the end by default. go to the beginning, if moving to the right neighbor.
                    v->textEdit()->setCursorPosition(0);
                }

                if (dNext && (dNext->xPos() > v->worldX() + 0.85 * v->worldWidth())) {
                    v->setWorldX(dNext->xPos() - v->worldWidth() / 2, CACanorus::settings()->animatedScroll());
                }
            }
        }
        break;
    }
    case CAMusElement::Mark: {
        // CAMark::Text and CAMark::BookMark
        if (e->key() == Qt::Key_Return) {
            confirmTextEdit(currentScoreView(), textEdit, elt);
        }
        break;
    default:
        break;
    }
    }

    // escape key - cancel
    if (e->key() == Qt::Key_Escape) {
        v->removeTextEdit();
    }
}

void CAMainWin::confirmTextEdit(CAScoreView* v, CATextEdit* textEdit, CAMusElement* elt)
{
    switch (elt->musElementType()) {
    case CAMusElement::Syllable: {
        // create or edit syllable
        CASyllable* syllable = static_cast<CASyllable*>(elt);

        QString text = textEdit->text().simplified(); // remove any trailing whitespaces

        bool hyphen = false;
        if (text.right(1) == "-") {
            hyphen = true;
            text.chop(1);
        }

        bool melisma = false;
        if (text.right(1) == "_") {
            melisma = true;
            text.chop(1);
        }

        CACanorus::undo()->createUndoCommand(document(), tr("lyrics edit", "undo"));
        syllable->setText(text);
        syllable->setHyphenStart(hyphen);
        syllable->setMelismaStart(melisma);

        v->removeTextEdit();
        break;
    }
    case CAMusElement::Mark: {
        CAMark* mark = static_cast<CAMark*>(elt);
        if (!textEdit->text().isEmpty() || mark->markType() == CAMark::BookMark) {
            CACanorus::undo()->createUndoCommand(document(), tr("text edit", "undo"));
            if (mark->markType() == CAMark::Text) {
                static_cast<CAText*>(mark)->setText(textEdit->text());
            } else if (mark->markType() == CAMark::BookMark) {
                static_cast<CABookMark*>(mark)->setText(textEdit->text());
            }
            v->removeTextEdit();
        } else {
            // remove text sign with empty content, if it's not a bookmark
            CACanorus::undo()->createUndoCommand(document(), tr("text edit", "delete"));
            v->removeTextEdit();
            delete mark;
        }
        break;
    }
    case CAMusElement::ChordName: {
        // create or edit chord name
        CAChordName* cn = static_cast<CAChordName*>(elt);

        CACanorus::undo()->createUndoCommand(document(), tr("chord name edit", "undo"));

        QString text = textEdit->text().simplified(); // remove any trailing whitespaces
        cn->importFromString(text);
        _noteChecker.checkSheet(v->sheet());

        v->removeTextEdit();
        break;
    }
    case CAMusElement::Note:
    case CAMusElement::Rest:
    case CAMusElement::MidiNote:
    case CAMusElement::Barline:
    case CAMusElement::Clef:
    case CAMusElement::TimeSignature:
    case CAMusElement::KeySignature:
    case CAMusElement::Slur:
    case CAMusElement::Tuplet:
    case CAMusElement::FunctionMark:
    case CAMusElement::FiguredBassMark:
    case CAMusElement::Undefined:
        qDebug() << "Error: confirmTextEdit() called on element of type" << elt->musElementType();
        break;
    }

    CACanorus::undo()->pushUndoCommand();
    CACanorus::rebuildUI(document(), currentSheet());
}

void CAMainWin::on_uiFBMNumber_toggled(bool checked, int buttonId)
{
    if (!checked && !uiFBMAccs->isChecked()) {
        uiFBMNumber->setChecked(true);
        return;
    }

    if (mode() == InsertMode) {
        if (checked) {
            musElementFactory()->setFBMNumber(buttonId);
        } else {
            musElementFactory()->setFBMNumber(0);
        }
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change figured bass", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFiguredBassMark* fbm = dynamic_cast<CAFiguredBassMark*>(v->selection().at(i)->musElement());

            if (fbm) {
                int number = static_cast<CADrawableFiguredBassNumber*>(v->selection().at(i))->number();
                fbm->removeNumber(number);
                if (uiFBMAccs->isChecked()) {
                    fbm->addNumber((checked ? (buttonId) : 0), uiFBMAccs->currentId() - 2);
                } else {
                    fbm->addNumber((checked ? (buttonId) : 0));
                }
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFBMAccs_toggled(bool checked, int buttonId)
{
    if (!checked && !uiFBMNumber->isChecked()) {
        uiFBMAccs->setChecked(true);
        return;
    }

    if (mode() == InsertMode) {
        if (checked) {
            musElementFactory()->setFBMAccs(buttonId - 2);
            musElementFactory()->setFBMAccsVisible(true);
        } else {
            musElementFactory()->setFBMAccsVisible(false);
        }
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change figured bass", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFiguredBassMark* fbm = dynamic_cast<CAFiguredBassMark*>(v->selection().at(i)->musElement());

            if (fbm) {
                int number = static_cast<CADrawableFiguredBassNumber*>(v->selection().at(i))->number();
                fbm->removeNumber(number);
                if (checked) {
                    fbm->addNumber((uiFBMNumber->isChecked() ? (uiFBMNumber->currentId()) : 0), buttonId - 2);
                } else {
                    fbm->addNumber((uiFBMNumber->isChecked() ? (uiFBMNumber->currentId()) : 0));
                }
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFMFunction_toggled(bool, int buttonId)
{
    if (mode() == InsertMode) {
        musElementFactory()->setFMFunction(static_cast<CAFunctionMark::CAFunctionType>(buttonId * (buttonId < 0 ? -1 : 1)));
        musElementFactory()->setFMFunctionMinor(buttonId < 0);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change function", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFunctionMark* fm = dynamic_cast<CAFunctionMark*>(v->selection().at(i)->musElement());

            if (fm) {
                fm->setFunction(static_cast<CAFunctionMark::CAFunctionType>(buttonId * (buttonId < 0 ? -1 : 1)));
                fm->setMinor(buttonId < 0);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFMChordArea_toggled(bool, int buttonId)
{
    if (mode() == InsertMode) {
        musElementFactory()->setFMChordArea(static_cast<CAFunctionMark::CAFunctionType>(buttonId * (buttonId < 0 ? -1 : 1)));
        musElementFactory()->setFMChordAreaMinor(buttonId < 0);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change chord area", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFunctionMark* fm = dynamic_cast<CAFunctionMark*>(v->selection().at(i)->musElement());

            if (fm) {
                fm->setChordArea(static_cast<CAFunctionMark::CAFunctionType>(buttonId * (buttonId < 0 ? -1 : 1)));
                fm->setChordAreaMinor(buttonId < 0);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFMTonicDegree_toggled(bool, int buttonId)
{
    if (mode() == InsertMode) {
        musElementFactory()->setFMTonicDegree(static_cast<CAFunctionMark::CAFunctionType>(buttonId * (buttonId < 0 ? -1 : 1)));
        musElementFactory()->setFMTonicDegreeMinor(buttonId < 0);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change tonic degree", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFunctionMark* fm = dynamic_cast<CAFunctionMark*>(v->selection().at(i)->musElement());

            if (fm) {
                fm->setTonicDegree(static_cast<CAFunctionMark::CAFunctionType>(buttonId * (buttonId < 0 ? -1 : 1)));
                fm->setTonicDegreeMinor(buttonId < 0);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFMEllipse_toggled(bool checked)
{
    if (mode() == InsertMode) {
        musElementFactory()->setFMEllipse(checked);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("set/unset ellipse", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFunctionMark* fm = dynamic_cast<CAFunctionMark*>(v->selection().at(i)->musElement());

            if (fm) {
                fm->setEllipse(checked);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiSlurType_toggled(bool, int buttonId)
{
    // remember previous muselement type so we can return to previous state after
    CAMusElement::CAMusElementType prevMusEltType = musElementFactory()->musElementType();

    // Read currently selected entry from tool button menu
    CASlur::CASlurType slurType = static_cast<CASlur::CASlurType>(buttonId);

    musElementFactory()->setMusElementType(CAMusElement::Slur);

    // New clef type
    musElementFactory()->setSlurType(slurType);

    insertMusElementAt(QPoint(0, 0), currentScoreView()); // inserts a slur or tie and quits the insert mode

    musElementFactory()->setMusElementType(prevMusEltType);
}

void CAMainWin::on_uiClefType_toggled(bool checked, int buttonId)
{
    if (checked) {
        // Read currently selected entry from tool button menu
        CAClef::CAPredefinedClefType clefType = static_cast<CAClef::CAPredefinedClefType>(buttonId);

        musElementFactory()->setMusElementType(CAMusElement::Clef);

        // New clef type
        musElementFactory()->setClef(clefType);

        setMode(InsertMode);
    }
}

void CAMainWin::on_uiMarkType_toggled(bool checked, int buttonId)
{
    if (checked) {
        CAMark::CAMarkType markType;

        // Read currently selected entry from tool button menu
        if (buttonId == CAMark::Ritardando * (-1)) {
            markType = CAMark::Ritardando;
            musElementFactory()->setRitardandoType(CARitardando::Accellerando);
        } else if (buttonId == CAMark::Ritardando) {
            markType = CAMark::Ritardando;
            musElementFactory()->setRitardandoType(CARitardando::Ritardando);
        } else if (buttonId == CAMark::Crescendo * (-1)) {
            markType = CAMark::Crescendo;
            musElementFactory()->setCrescendoType(CACrescendo::Decrescendo);
        } else if (buttonId == CAMark::Crescendo) {
            markType = CAMark::Crescendo;
            musElementFactory()->setCrescendoType(CACrescendo::Crescendo);
        } else {
            markType = static_cast<CAMark::CAMarkType>(buttonId);
        }

        musElementFactory()->setMusElementType(CAMusElement::Mark);

        // New mark type
        musElementFactory()->setMarkType(markType);

        setMode(InsertMode);
    }
}

void CAMainWin::on_uiArticulationType_toggled(bool checked, int buttonId)
{
    if (checked) {
        // Read currently selected entry from tool button menu
        CAArticulation::CAArticulationType type = static_cast<CAArticulation::CAArticulationType>(buttonId);

        musElementFactory()->setMusElementType(CAMusElement::Mark);

        // New mark type
        musElementFactory()->setMarkType(CAMark::Articulation);
        musElementFactory()->setArticulationType(type);

        setMode(InsertMode);
    }
}

void CAMainWin::on_uiTimeSigBeats_valueChanged(int beats)
{
    if (mode() == InsertMode) {
        musElementFactory()->setTimeSigBeats(beats);
        if (uiTimeSigBeats->value() == 4 && uiTimeSigBeat->value() == 4)
            uiTimeSigType->setCurrentId(44);
        else if (uiTimeSigBeats->value() == 3 && uiTimeSigBeat->value() == 4)
            uiTimeSigType->setCurrentId(34);
        else if (uiTimeSigBeats->value() == 2 && uiTimeSigBeat->value() == 4)
            uiTimeSigType->setCurrentId(24);
        else if (uiTimeSigBeats->value() == 2 && uiTimeSigBeat->value() == 2)
            uiTimeSigType->setCurrentId(22);
        else if (uiTimeSigBeats->value() == 3 && uiTimeSigBeat->value() == 8)
            uiTimeSigType->setCurrentId(38);
        else if (uiTimeSigBeats->value() == 6 && uiTimeSigBeat->value() == 8)
            uiTimeSigType->setCurrentId(68);
        else
            uiTimeSigType->setCurrentId(0);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CATimeSignature* timeSig = dynamic_cast<CATimeSignature*>(v->selection().at(0)->musElement());
            if (timeSig) {
                timeSig->setBeats(beats);
                if (CACanorus::settings()->useNoteChecker()) {
                    _noteChecker.checkSheet(v->sheet());
                }

                CACanorus::rebuildUI(document(), currentSheet());
            }
        }
    }
}

void CAMainWin::on_uiTimeSigBeat_valueChanged(int beat)
{
    if (mode() == InsertMode) {
        musElementFactory()->setTimeSigBeat(beat);
        if (uiTimeSigBeats->value() == 4 && uiTimeSigBeat->value() == 4)
            uiTimeSigType->setCurrentId(44);
        else if (uiTimeSigBeats->value() == 3 && uiTimeSigBeat->value() == 4)
            uiTimeSigType->setCurrentId(34);
        else if (uiTimeSigBeats->value() == 2 && uiTimeSigBeat->value() == 4)
            uiTimeSigType->setCurrentId(24);
        else if (uiTimeSigBeats->value() == 2 && uiTimeSigBeat->value() == 2)
            uiTimeSigType->setCurrentId(22);
        else if (uiTimeSigBeats->value() == 3 && uiTimeSigBeat->value() == 8)
            uiTimeSigType->setCurrentId(38);
        else if (uiTimeSigBeats->value() == 6 && uiTimeSigBeat->value() == 8)
            uiTimeSigType->setCurrentId(68);
        else
            uiTimeSigType->setCurrentId(0);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CATimeSignature* timeSig = dynamic_cast<CATimeSignature*>(v->selection().at(0)->musElement());
            if (timeSig) {
                timeSig->setBeat(beat);
                CACanorus::rebuildUI(document(), currentSheet());
            }
        }
    }
}

void CAMainWin::on_uiTimeSigType_toggled(bool checked, int buttonId)
{
    if (checked) {
        switch (buttonId) {
        case 44:
            uiTimeSigBeats->setValue(4);
            uiTimeSigBeat->setValue(4);
            break;
        case 22:
            uiTimeSigBeats->setValue(2);
            uiTimeSigBeat->setValue(2);
            break;
        case 34:
            uiTimeSigBeats->setValue(3);
            uiTimeSigBeat->setValue(4);
            break;
        case 24:
            uiTimeSigBeats->setValue(2);
            uiTimeSigBeat->setValue(4);
            break;
        case 38:
            uiTimeSigBeats->setValue(3);
            uiTimeSigBeat->setValue(8);
            break;
        case 68:
            uiTimeSigBeats->setValue(6);
            uiTimeSigBeat->setValue(8);
            break;
        case 0:
            break;
        }
        musElementFactory()->setTimeSigBeats(uiTimeSigBeats->value());
        musElementFactory()->setTimeSigBeat(uiTimeSigBeat->value());
        musElementFactory()->setMusElementType(CAMusElement::TimeSignature);

        setMode(InsertMode);
    }
}

void CAMainWin::on_uiTupletType_toggled(bool checked, int type)
{
    if (checked) {
        if (mode() == EditMode && currentScoreView()->selection().size() > 1) {
            CACanorus::undo()->createUndoCommand(document(), tr("insert tuplet", "undo"));

            QList<CAPlayable*> playableList;
            bool wrongVoice = false; // all elements should belong to a single voice. If multiple voices detected, cancel the tuplet creation.
            CAVoice* tupletVoice = nullptr;
            for (int i = 0; i < currentScoreView()->selection().size(); i++) {
                if (currentScoreView()->selection()[i]->musElement() && currentScoreView()->selection()[i]->musElement()->isPlayable()) {
                    playableList << static_cast<CAPlayable*>(currentScoreView()->selection()[i]->musElement());
                    if (!tupletVoice || static_cast<CAPlayable*>(currentScoreView()->selection()[i]->musElement())->voice() == tupletVoice) {
                        tupletVoice = static_cast<CAPlayable*>(currentScoreView()->selection()[i]->musElement())->voice();
                    } else {
                        wrongVoice = true;
                        break;
                    }
                }
            }

            if (!wrongVoice) {
                if (type == 0) {
                    new CATuplet(3, 2, playableList);
                } else {
                    new CATuplet(uiTupletNumber->value(), uiTupletActualNumber->value(), playableList);
                }

                CACanorus::undo()->pushUndoCommand();
                CACanorus::rebuildUI(document(), currentSheet());
            }
        } else {
            switch (type) {
            case 0:
                uiTupletNumber->setValue(3);
                uiTupletActualNumber->setValue(2);
                break;
            }
        }
    }

    updatePlayableToolBar();
}

void CAMainWin::on_uiTupletNumber_valueChanged(int value)
{
    musElementFactory()->setTupletNumber(value);
}

void CAMainWin::on_uiTupletActualNumber_valueChanged(int value)
{
    musElementFactory()->setTupletActualNumber(value);
}

void CAMainWin::on_uiBarlineType_toggled(bool checked, int buttonId)
{
    if (checked) {
        musElementFactory()->setMusElementType(CAMusElement::Barline);
        musElementFactory()->setBarlineType(static_cast<CABarline::CABarlineType>(buttonId));
        setMode(InsertMode);
    }
}

/*!
	Called when a user clicks "Commit" button in source View.
*/
void CAMainWin::sourceViewCommit(QString inputString)
{
    CASourceView* v = static_cast<CASourceView*>(sender());

    stopPlayback();
    if (v->document()) {
        // CanorusML document source
        CACanorus::undo()->createUndoCommand(document(), tr("commit CanorusML source", "undo"));

        clearUI(); // clear GUI before clearing the data part!
        CADocument* oldDoc = document();

        CACanorusMLImport open(inputString);
        open.importDocument();
        open.wait();

        if (open.importedDocument()) {
            CACanorus::undo()->replaceDocument(document(), open.importedDocument());
            setDocument(open.importedDocument());
            // TODO UX: Set previous voice, set previous context.
        }

        if (CACanorus::settings()->useNoteChecker()) {
            for (int i = 0; i < document()->sheetList().size(); i++) {
                _noteChecker.checkSheet(document()->sheetList()[i]);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document());

        if (oldDoc) {
            delete oldDoc;
        }
    } else if (v->voice()) {
        // LilyPond voice source
        CACanorus::undo()->createUndoCommand(document(), tr("commit LilyPond source", "undo"));

        CALilyPondImport li(inputString);

        CAVoice* oldVoice = v->voice();

        QList<CAMusElement*> signList = oldVoice->getSignList();
        for (int i = 0; i < signList.size(); i++)
            oldVoice->remove(signList[i]); // removes signs from all voices

        li.setTemplateVoice(oldVoice); // copy properties
        li.importVoice();
        li.wait();

        CAVoice* newVoice = li.importedVoice();
        int voiceNumber = oldVoice->voiceNumber();
        oldVoice->staff()->removeVoice(oldVoice);

        newVoice->staff()->insertVoice(voiceNumber - 1, newVoice);
        newVoice->staff()->synchronizeVoices();

        // FIXME any way to avoid this?
        CAScoreView* scorevp;
        for (CAView* vp : _viewList) {
            if (vp->viewType() == CAView::ScoreView && (scorevp = static_cast<CAScoreView*>(vp))->selectedVoice() == oldVoice)
                scorevp->setSelectedVoice(newVoice);
        }

        v->setVoice(newVoice);

        if (CACanorus::settings()->useNoteChecker()) {
            _noteChecker.checkSheet(newVoice->staff()->sheet());
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), newVoice->staff()->sheet());
        setCurrentView(v);

        // oldVoice must be cleaned *after* rebuildUI(), because of shadow notes referencing it!
        delete oldVoice;
    } else if (v->lyricsContext()) {
        // LilyPond lyrics source
        CACanorus::undo()->createUndoCommand(document(), tr("commit LilyPond source", "undo"));

        CALilyPondImport li(inputString);

        li.importLyricsContext();
        li.wait();
        CALyricsContext* newLc = li.importedLyricsContext();
        CALyricsContext* oldLc = v->lyricsContext();
        newLc->cloneLyricsContextProperties(oldLc);
        if (newLc->associatedVoice()) {
            newLc->associatedVoice()->removeLyricsContext(oldLc);
            newLc->associatedVoice()->addLyricsContext(newLc);
        }
        newLc->sheet()->insertContextAfter(oldLc, newLc);
        newLc->sheet()->removeContext(oldLc);
        v->setLyricsContext(newLc);

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), v->lyricsContext()->sheet());
        setCurrentView(v);
        delete oldLc;
    }
}

void CAMainWin::on_uiUsersGuide_triggered()
{
#ifdef QT_WEBENGINEWIDGETS_LIB
    CACanorus::help()->showUsersGuide("playback", this);
#else
    CACanorus::help()->showUsersGuide("playback");
#endif
}

void CAMainWin::on_uiAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void CAMainWin::on_uiAboutCanorus_triggered()
{
    QString about = tr("<p><b>Canorus - The next generation music score editor</b></p>\
<p>Version %1<br>\
(C) 2006-2020 Canorus Development team. All rights reserved.<br>\
See the file AUTHORS for the list of Canorus developers<br><br>\
This program is licensed under the GNU General Public License (GPL).<br>\
See the file LICENSE.GPL for details.<br><br>\
Homepage: <a href=\"http://www.canorus.org\">http://www.canorus.org</a></p>")
                        .arg(CANORUS_VERSION);
#ifdef USE_PYTHON
    about += "<p>" + tr("Canorus is compiled with Python support.");
#endif
    QMessageBox::about(this, tr("About Canorus"), about);
}

void CAMainWin::on_uiSettings_triggered()
{
    CASettingsDialog(CASettingsDialog::EditorSettings, this);

    if (CACanorus::settings()->useNoteChecker()) {
        for (int i = 0; i < document()->sheetList().size(); i++) {
            _noteChecker.checkSheet(document()->sheetList()[i]);
        }
    } else {
        for (int i = 0; i < document()->sheetList().size(); i++) {
            document()->sheetList()[i]->clearNoteCheckerErrors();
        }
    }

    CACanorus::rebuildUI();
}

void CAMainWin::on_uiMidiRecorder_triggered()
{
    if (document()) {
        std::shared_ptr<CAResource> myMidiFile = CAResourceCtl::createEmptyResource(tr("Recorded Midi file"), document(), CAResource::Sound);

        if (_midiRecorderView) {
            delete _midiRecorderView;
        }

        _midiRecorderView = new CAMidiRecorderView(new CAMidiRecorder(myMidiFile, CACanorus::midiDevice()), this);
        addDockWidget(Qt::TopDockWidgetArea, _midiRecorderView);
        _midiRecorderView->show();

        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiTranspose_triggered()
{
    if (document()) {
        _transposeView->show();
    }
}

void CAMainWin::on_uiJumpTo_triggered()
{
    if (document() && currentSheet()) {
        _jumpToView->show();
    }
}

void CAMainWin::on_uiLilyPondSource_triggered()
{
    CAContext* context = currentContext();
    if (!context)
        return;

    CASourceView* v = nullptr;
    CAStaff* staff = currentStaff();
    if (staff) {
        int voiceNum = uiVoiceNum->getRealValue() - 1 < 0 ? 0 : uiVoiceNum->getRealValue() - 1;
        CAVoice* voice = staff->voiceList()[voiceNum];
        v = new CASourceView(voice, nullptr);
    } else if (context->contextType() == CAContext::LyricsContext) {
        v = new CASourceView(static_cast<CALyricsContext*>(context), nullptr);
    }

    initView(v);
    currentViewContainer()->addView(v);

    uiUnsplitAll->setEnabled(true);
    uiCloseCurrentView->setEnabled(true);
}

/*!
	Adds a new score View to default View container.
*/
void CAMainWin::on_uiScoreView_triggered()
{
    CASheet* s = _sheetMap[currentViewContainer()];

    if (currentViewContainer() && s) {
        CAScoreView* v = new CAScoreView(s, nullptr);
        initView(v);
        currentViewContainer()->addView(v);
        v->rebuild();

        uiUnsplitAll->setEnabled(true);
        uiCloseCurrentView->setEnabled(true);
    }
}

/*!
	Removes the sheet, all its contents and rebuilds the GUI.
*/
void CAMainWin::on_uiRemoveSheet_triggered()
{
    stopPlayback();
    CASheet* sheet = currentSheet();
    if (sheet) {
        CACanorus::undo()->createUndoCommand(document(), tr("deletion of the sheet", "undo"));
        CACanorus::undo()->pushUndoCommand();
        document()->removeSheet(currentSheet());
        removeSheet(sheet);
        delete sheet;
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

/*!
	Removes the sheet from the GUI and deletes the Views and View containers and tabs
	pointing to the given \a sheet.
*/
void CAMainWin::removeSheet(CASheet* sheet)
{
    CAViewContainer* vpc = _sheetMap.key(sheet);
    _sheetMap.remove(vpc);

    // remove tab
    int idx = uiTabWidget->indexOf(vpc);
    uiTabWidget->removeTab(idx);
    delete vpc;
    if (idx < uiTabWidget->count())
        uiTabWidget->setCurrentIndex(idx);
    setCurrentViewContainer(static_cast<CAViewContainer*>(uiTabWidget->currentWidget()));
    setCurrentView(static_cast<CAViewContainer*>(uiTabWidget->currentWidget()) ? static_cast<CAViewContainer*>(uiTabWidget->currentWidget())->currentView() : nullptr);

    // remove other Views pointing to the sheet
    QList<CAView*> vpl = viewList();
    for (int i = 0; i < vpl.size(); i++) {
        switch (vpl[i]->viewType()) {
        case CAView::ScoreView: {
            if (static_cast<CAScoreView*>(vpl[i])->sheet() == sheet)
                delete vpl[i];
            break;
        }
        case CAView::SourceView: {
            CASourceView* sv = static_cast<CASourceView*>(vpl[i]);
            if (sv->voice() && sv->voice()->staff()->sheet() == sheet)
                delete vpl[i];
            else if (sv->lyricsContext() && sv->lyricsContext()->sheet() == sheet)
                delete vpl[i];

            break;
        }
        }
    }
}

/*!
	Brings up the properties dialog.
*/
void CAMainWin::on_uiDocumentProperties_triggered()
{
    CAPropertiesDialog::documentProperties(document(), this);
}

void CAMainWin::on_uiSheetName_returnPressed()
{
    CASheet* sheet = currentSheet();
    if (sheet) {
        CACanorus::undo()->createUndoCommand(document(), tr("change sheet name", "undo"));
        CACanorus::undo()->pushUndoCommand();
        sheet->setName(uiSheetName->text());
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

/*!
	Brings up properties dialog.
*/
void CAMainWin::on_uiSheetProperties_triggered()
{
    CAPropertiesDialog::sheetProperties(currentSheet(), this);
}

/*!
	Sets the current context name.
*/
void CAMainWin::on_uiContextName_returnPressed()
{
    CAContext* context = currentContext();
    if (context) {
        CACanorus::undo()->createUndoCommand(document(), tr("change context name", "undo"));
        CACanorus::undo()->pushUndoCommand();
        context->setName(uiContextName->text());
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

/*!
	Brings up the properties dialog.
*/
void CAMainWin::on_uiContextProperties_triggered()
{
    CAPropertiesDialog::contextProperties(currentContext(), this);
}

/*!
	Sets the stanza number of the current lyrics context.
*/
void CAMainWin::on_uiStanzaNumber_valueChanged(int stanzaNumber)
{
    if (currentContext() && currentContext()->contextType() == CAContext::LyricsContext) {
        CACanorus::undo()->createUndoCommand(document(), tr("change stanza number", "undo"));
        if (static_cast<CALyricsContext*>(currentContext())->stanzaNumber() != stanzaNumber)
            CACanorus::undo()->pushUndoCommand();
        static_cast<CALyricsContext*>(currentContext())->setStanzaNumber(stanzaNumber);
    }
}

/*!
	Sets the associated voice of the current lyrics context.
*/
void CAMainWin::on_uiAssociatedVoice_activated(int idx)
{
    if (idx != -1 && currentContext() && currentContext()->contextType() == CAContext::LyricsContext) {
        CACanorus::undo()->createUndoCommand(document(), tr("change associated voice", "undo"));
        if (static_cast<CALyricsContext*>(currentContext())->associatedVoice() != currentSheet()->voiceList().at(idx))
            CACanorus::undo()->pushUndoCommand();
        static_cast<CALyricsContext*>(currentContext())->setAssociatedVoice(currentSheet()->voiceList().at(idx));
        CACanorus::rebuildUI(document(), currentSheet()); // needs a rebuild if lyrics contexts are to be moved
    }
}

void CAMainWin::on_uiVoiceStemDirection_toggled(bool, int direction)
{
    CAVoice* voice = currentVoice();
    if (voice) {
        CACanorus::undo()->createUndoCommand(document(), tr("change voice stem direction", "undo"));
        if (voice->stemDirection() != static_cast<CANote::CAStemDirection>(direction))
            CACanorus::undo()->pushUndoCommand();
        CACanorus::undo()->pushUndoCommand();
        voice->setStemDirection(static_cast<CANote::CAStemDirection>(direction));
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

/*!
	Sets the currently selected note stem direction if in insert/edit mode or the music elements factory note stem direction if in insert mode.
*/
void CAMainWin::on_uiNoteStemDirection_toggled(bool, int id)
{
    CANote::CAStemDirection direction = static_cast<CANote::CAStemDirection>(id);
    if (mode() == InsertMode)
        musElementFactory()->setNoteStemDirection(direction);
    else if (mode() == EditMode) {
        CACanorus::undo()->createUndoCommand(document(), tr("change note stem direction", "undo"));
        CAScoreView* v = currentScoreView();
        bool changed = false;
        for (int i = 0; v && i < v->selection().size(); i++) {
            CANote* note = dynamic_cast<CANote*>(v->selection().at(i)->musElement());
            if (note) {
                note->setStemDirection(direction);
                changed = true;
            }
        }
        if (changed) {
            CACanorus::undo()->pushUndoCommand();
            CACanorus::rebuildUI(document(), currentSheet());
        }
    }
}

/*!
	Updates all the toolbars according to the current state of the main window.
*/
void CAMainWin::updateToolBars()
{
    updateUndoRedoButtons();

    updateInsertToolBar();
    updateSheetToolBar();
    updateContextToolBar();
    updateVoiceToolBar();
    updatePlayableToolBar();
    _poKeySignatureUI->updateKeySigToolBar();
    updateTimeSigToolBar();
    updateClefToolBar();
    updateFBMToolBar();
    updateFMToolBar();
    updateDynamicToolBar();
    updateInstrumentToolBar();
    updateTempoToolBar();
    updateFermataToolBar();
    updateRepeatMarkToolBar();
    updateFingeringToolBar();

    if (document())
        uiNewSheet->setVisible(true);
    else
        uiNewSheet->setVisible(false);
}

/*!
	Shows sheet tool bar if nothing selected. Otherwise hides it.
*/
void CAMainWin::updateSheetToolBar()
{
    CAScoreView* v = currentScoreView();
    if (document()) {
        if (v) {
            if (v->sheet() && v->selection().isEmpty() && (!v->currentContext())) {
                // no track selected, show sheet actions
                uiSheetName->setText(v->sheet()->name());
                uiSheetName->setEnabled(true);
                uiRemoveSheet->setVisible(true);
                uiSheetProperties->setVisible(true);
                uiSheetToolBar->show();
            } else {
                // other track or elements selected, hide sheet actions
                uiSheetToolBar->hide();
            }
        } else {
            // no sheet exist: hide everything except the new sheet button
            uiSheetName->setEnabled(false);
            uiRemoveSheet->setVisible(false);
            uiSheetProperties->setVisible(false);
            uiSheetToolBar->show();
        }
    } else {
        // no document exists, hide sheet actions
        uiSheetToolBar->hide();
    }
}

/*!
	Shows/Hides the Voice properties tool bar according to the currently selected context and updates its properties.
*/
void CAMainWin::updateVoiceToolBar()
{
    CAContext* context = currentContext();
    if (context && context->contextType() == CAContext::Staff && currentScoreView() && ((mode() == EditMode && currentScoreView()->selection().size() == 0) || (mode() == InsertMode && uiInsertPlayable->isChecked()))) {
        CAStaff* staff = static_cast<CAStaff*>(context);
        uiNewVoice->setVisible(true);
        uiNewVoice->setEnabled(true);
        if (staff->voiceList().size()) {
            uiVoiceNum->setMax(staff->voiceList().size());
            int voiceNr = uiVoiceNum->getRealValue();
            if (currentVoice() && voiceNr) {
                CAVoice* curVoice = (voiceNr <= staff->voiceList().size() ? staff->voiceList()[voiceNr - 1] : staff->voiceList()[0]);
                uiVoiceName->setText(curVoice->name());
                uiVoiceName->setEnabled(true);
                uiVoiceInstrument->setEnabled(true);
                uiVoiceInstrument->setCurrentIndex(currentVoice()->midiProgram());
                uiRemoveVoice->setEnabled(true);
                uiVoiceStemDirection->setCurrentId(curVoice->stemDirection());
                uiVoiceStemDirection->setEnabled(true);
                uiVoiceProperties->setEnabled(true);
            } else {
                uiVoiceName->setEnabled(false);
                uiVoiceInstrument->setEnabled(false);
                uiRemoveVoice->setEnabled(false);
                uiVoiceStemDirection->setEnabled(false);
                uiVoiceProperties->setEnabled(false);
            }
        }

        uiVoiceToolBar->show();
    } else {
        uiNewVoice->setVisible(false);
        uiVoiceToolBar->hide();
    }
}

/*!
	Shows/Hides context tool bar according to the selected context (if any) and hides/shows specific actions in the toolbar for the current context.
*/
void CAMainWin::updateContextToolBar()
{
    CAContext* context = currentContext();
    if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() == 0 && context) {
        if (!uiInsertPlayable->isChecked())
            uiContextToolBar->show();

        switch (context->contextType()) {
        case CAContext::Staff: {
            uiStanzaNumberAction->setVisible(false);
            uiAssociatedVoiceAction->setVisible(false);
            break;
        }
        case CAContext::LyricsContext: {
            CALyricsContext* c = static_cast<CALyricsContext*>(context);
            uiStanzaNumber->setValue(c->stanzaNumber());
            uiStanzaNumberAction->setVisible(true);
            uiStanzaNumberAction->setEnabled(true);

            uiAssociatedVoice->clear();
            QList<CAVoice*> voiceList = currentSheet()->voiceList();
            for (int i = 0; i < voiceList.count(); i++)
                uiAssociatedVoice->addItem(voiceList[i]->name());
            uiAssociatedVoice->setCurrentIndex(voiceList.indexOf(c->associatedVoice()));
            uiAssociatedVoiceAction->setVisible(true);
            uiAssociatedVoiceAction->setEnabled(true);

            break;
        }
        case CAContext::FunctionMarkContext: {
            uiStanzaNumberAction->setVisible(false);
            uiAssociatedVoiceAction->setVisible(false);
            break;
        }
        case CAContext::FiguredBassContext:
        case CAContext::ChordNameContext: {
            uiStanzaNumberAction->setVisible(false);
            uiAssociatedVoiceAction->setVisible(false);
            break;
        }
        }
        uiContextName->setText(context->name());
    } else {
        uiContextToolBar->hide();
    }
}

/*!
	Shows/Hides music elements which cannot be placed in the selected context.
*/
void CAMainWin::updateInsertToolBar()
{
    if (currentSheet()) {
        uiNewContext->setVisible(true);
        uiInsertToolBar->show();
        CAContext* context = currentContext();
        if (context) {
            switch (context->contextType()) {
            case CAContext::Staff:
                // staff selected
                uiInsertPlayable->setVisible(true);
                uiSlurType->defaultAction()->setVisible(true);
                uiSlurType->defaultAction()->setEnabled(true);
                /// \todo This is needed in order for actions to hide?! -Matevz
                //uiSlurType->setVisible(true);
                uiInsertClef->setVisible(true); // menu
                uiInsertBarline->setVisible(true); // menu
                uiClefType->defaultAction()->setVisible(true);
                uiClefType->defaultAction()->setEnabled(true);
                uiTimeSigType->defaultAction()->setVisible(true);
                uiTimeSigType->defaultAction()->setEnabled(true);
                uiInsertKeySig->setVisible(true);
                uiMarkType->defaultAction()->setVisible(true);
                uiMarkType->defaultAction()->setEnabled(true);
                uiArticulationType->defaultAction()->setVisible(true);
                uiArticulationType->defaultAction()->setEnabled(true);
                uiInsertTimeSig->setVisible(true);
                uiBarlineType->defaultAction()->setVisible(true);
                uiBarlineType->defaultAction()->setEnabled(true);
                uiInsertFBM->setVisible(false);
                uiInsertFM->setVisible(false);
                uiInsertSyllable->setVisible(false);
                uiInsertChordName->setVisible(false);
                break;
            case CAContext::FunctionMarkContext:
                // function mark context selected
                uiInsertPlayable->setVisible(false);
                uiSlurType->defaultAction()->setVisible(false);
                uiInsertClef->setVisible(false); // menu
                uiInsertBarline->setVisible(false); // menu
                uiClefType->defaultAction()->setVisible(false);
                uiTimeSigType->defaultAction()->setVisible(false);
                uiInsertKeySig->setVisible(false);
                uiMarkType->defaultAction()->setVisible(false);
                uiArticulationType->defaultAction()->setVisible(false);
                uiInsertTimeSig->setVisible(false);
                uiBarlineType->defaultAction()->setVisible(false);
                uiInsertFBM->setVisible(false);
                uiInsertFM->setVisible(true);
                uiInsertSyllable->setVisible(false);
                uiInsertChordName->setVisible(false);
                break;
            case CAContext::LyricsContext:
                // lyrics context selected
                uiInsertPlayable->setVisible(false);
                uiSlurType->defaultAction()->setVisible(false);
                uiInsertClef->setVisible(false); // menu
                uiInsertBarline->setVisible(false); // menu
                uiClefType->defaultAction()->setVisible(false);
                uiTimeSigType->defaultAction()->setVisible(false);
                uiInsertKeySig->setVisible(false);
                uiMarkType->defaultAction()->setVisible(false);
                uiArticulationType->defaultAction()->setVisible(false);
                uiInsertTimeSig->setVisible(false);
                uiBarlineType->defaultAction()->setVisible(false);
                uiInsertFBM->setVisible(false);
                uiInsertFM->setVisible(false);
                uiInsertSyllable->setVisible(true);
                uiInsertChordName->setVisible(false);
                break;
            case CAContext::FiguredBassContext:
                // lyrics context selected
                uiInsertPlayable->setVisible(false);
                uiSlurType->defaultAction()->setVisible(false);
                uiInsertClef->setVisible(false); // menu
                uiInsertBarline->setVisible(false); // menu
                uiClefType->defaultAction()->setVisible(false);
                uiTimeSigType->defaultAction()->setVisible(false);
                uiInsertKeySig->setVisible(false);
                uiMarkType->defaultAction()->setVisible(false);
                uiArticulationType->defaultAction()->setVisible(false);
                uiInsertTimeSig->setVisible(false);
                uiBarlineType->defaultAction()->setVisible(false);
                uiInsertFBM->setVisible(true);
                uiInsertFM->setVisible(false);
                uiInsertSyllable->setVisible(false);
                uiInsertChordName->setVisible(false);
                break;
            case CAContext::ChordNameContext:
                // chord name context selected
                uiInsertPlayable->setVisible(false);
                uiSlurType->defaultAction()->setVisible(false);
                uiInsertClef->setVisible(false); // menu
                uiInsertBarline->setVisible(false); // menu
                uiClefType->defaultAction()->setVisible(false);
                uiTimeSigType->defaultAction()->setVisible(false);
                uiInsertKeySig->setVisible(false);
                uiMarkType->defaultAction()->setVisible(false);
                uiArticulationType->defaultAction()->setVisible(false);
                uiInsertTimeSig->setVisible(false);
                uiBarlineType->defaultAction()->setVisible(false);
                uiInsertFBM->setVisible(false);
                uiInsertFM->setVisible(false);
                uiInsertSyllable->setVisible(false);
                uiInsertChordName->setVisible(true);
                break;
            }
        } else {
            // no contexts selected
            uiInsertPlayable->setVisible(false);
            uiSlurType->defaultAction()->setVisible(false);
            uiInsertClef->setVisible(false); // menu
            uiInsertBarline->setVisible(false); // menu
            uiClefType->defaultAction()->setVisible(false);
            uiTimeSigType->defaultAction()->setVisible(false);
            uiInsertKeySig->setVisible(false);
            uiMarkType->defaultAction()->setVisible(false);
            uiArticulationType->defaultAction()->setVisible(false);
            uiInsertTimeSig->setVisible(false);
            uiBarlineType->defaultAction()->setVisible(false);
            uiInsertFBM->setVisible(false);
            uiInsertFM->setVisible(false);
            uiInsertSyllable->setVisible(false);
            uiInsertChordName->setVisible(false);
        }
    } else {
        uiInsertToolBar->hide();
        uiNewContext->setVisible(false);
        uiInsertPlayable->setVisible(false);
        uiSlurType->defaultAction()->setVisible(false);
        /// \todo This is needed in order for actions to hide?! -Matevz
        //uiSlurType->setVisible(false);
        uiInsertClef->setVisible(false); // menu
        uiInsertBarline->setVisible(false); // menu
        uiClefType->defaultAction()->setVisible(false);
        uiTimeSigType->defaultAction()->setVisible(false);
        uiInsertKeySig->setVisible(false);
        uiMarkType->defaultAction()->setVisible(false);
        uiArticulationType->defaultAction()->setVisible(false);
        uiInsertTimeSig->setVisible(false);
        uiBarlineType->defaultAction()->setVisible(false);
        uiInsertFBM->setVisible(false);
        uiInsertFM->setVisible(false);
        uiInsertSyllable->setVisible(false);
        uiInsertChordName->setVisible(false);
    }
}

/*!
	Show/Hides the playable tool bar and its properties according to the current state.
*/
void CAMainWin::updatePlayableToolBar()
{
    if (uiInsertPlayable->isChecked() && mode() == InsertMode) {
        uiPlayableLength->setCurrentId(musElementFactory()->playableLength().musicLength());
        uiNoteStemDirection->setCurrentId(musElementFactory()->noteStemDirection());
        uiTupletType->defaultAction()->setVisible(true);

        if (uiTupletType->isChecked() && uiTupletType->currentId() == 1) {
            uiTupletNumberAction->setVisible(true);
            uiTupletInsteadOfAction->setVisible(true);
            uiTupletActualNumberAction->setVisible(true);
        } else {
            uiTupletNumberAction->setVisible(false);
            uiTupletInsteadOfAction->setVisible(false);
            uiTupletActualNumberAction->setVisible(false);
        }

        uiHiddenRest->setEnabled(true);
        uiHiddenRest->setChecked(musElementFactory()->restType() == CARest::Hidden);
        uiPlayableToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CAPlayable*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CAPlayable* playable = dynamic_cast<CAPlayable*>(v->selection().at(0)->musElement());
            if (playable) {
                uiPlayableLength->setCurrentId(playable->playableLength().musicLength());
                if (playable->musElementType() == CAMusElement::Note) {
                    CANote* note = static_cast<CANote*>(playable);
                    uiNoteStemDirection->setCurrentId(note->stemDirection());
                    uiHiddenRest->setEnabled(false);
                } else if (playable->musElementType() == CAMusElement::Rest) {
                    CARest* rest = static_cast<CARest*>(playable);
                    uiHiddenRest->setEnabled(true);
                    uiHiddenRest->setChecked(rest->restType() == CARest::Hidden);
                }
                uiPlayableToolBar->show();
            } else {
                uiPlayableToolBar->hide();
                uiHiddenRest->setEnabled(false);
            }

            uiTupletType->defaultAction()->setVisible(false);
            uiTupletNumberAction->setVisible(false);
            uiTupletInsteadOfAction->setVisible(false);
            uiTupletActualNumberAction->setVisible(false);
        }
    } else {
        uiPlayableToolBar->hide();
    }
}

/*!
	Shows/Hides the time signature properties tool bar according to the current state.
*/
void CAMainWin::updateTimeSigToolBar()
{
    if (uiTimeSigType->isChecked() && mode() == InsertMode) {
        uiTimeSigBeats->setValue(musElementFactory()->timeSigBeats());
        uiTimeSigBeat->setValue(musElementFactory()->timeSigBeat());
        uiTimeSigToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CATimeSignature*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CATimeSignature* timeSig = dynamic_cast<CATimeSignature*>(v->selection().at(0)->musElement());
            if (timeSig) {
                uiTimeSigBeats->setValue(timeSig->beats());
                uiTimeSigBeat->setValue(timeSig->beat());
                uiTimeSigToolBar->show();
            } else
                uiTimeSigToolBar->hide();
        }
    } else
        uiTimeSigToolBar->hide();
}

/*!
	Shows/Hides the clef properties tool bar according to the current state.
*/
void CAMainWin::updateClefToolBar()
{
    if (uiClefType->isChecked() && mode() == InsertMode) {
        uiClefOffset->setValue(musElementFactory()->clefOffset());
        uiClefToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CAClef*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CAClef* clef = dynamic_cast<CAClef*>(v->selection().at(0)->musElement());
            if (clef) {
                uiClefOffset->setValue(CAClef::offsetToReadable(clef->offset()));
                uiClefToolBar->show();
            } else
                uiClefToolBar->hide();
        }
    } else
        uiClefToolBar->hide();
}

/*!
	Shows/Hides the figured bass mark properties tool bar according to the current state.
*/
void CAMainWin::updateFBMToolBar()
{
    if (uiInsertFBM->isChecked() && mode() == InsertMode) {
        if (musElementFactory()->fbmNumber()) {
            uiFBMNumber->setCurrentId(musElementFactory()->fbmNumber());
            uiFBMNumber->setChecked(true);
        } else {
            uiFBMNumber->setChecked(false);
        }

        uiFBMAccs->setCurrentId(musElementFactory()->fbmAccs() + 2);
        uiFBMAccs->setChecked(musElementFactory()->fbmAccsVisible());

        uiFBMToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CAFiguredBassMark*>(currentScoreView()->selection().at(0)->musElement())) {
        CAFiguredBassMark* fbm = dynamic_cast<CAFiguredBassMark*>(currentScoreView()->selection().at(0)->musElement());
        int number = static_cast<CADrawableFiguredBassNumber*>(currentScoreView()->selection().at(0))->number();

        if (number) {
            uiFBMNumber->setCurrentId(number);
            uiFBMNumber->setChecked(true);
        } else {
            uiFBMNumber->setChecked(false);
        }

        if (fbm->accs().contains(number)) {
            uiFBMAccs->setCurrentId(fbm->accs()[number] + 2);
            uiFBMAccs->setChecked(true);
        } else {
            uiFBMAccs->setChecked(false);
        }

        uiFBMToolBar->show();
    } else {
        uiFBMToolBar->hide();
    }
}

/*!
	Shows/Hides the function mark properties tool bar according to the current state.
*/
void CAMainWin::updateFMToolBar()
{
    if (uiInsertFM->isChecked() && mode() == InsertMode) {
        uiFMFunction->setCurrentId(musElementFactory()->fmFunction() * (musElementFactory()->isFMFunctionMinor() ? -1 : 1));
        uiFMChordArea->setCurrentId(musElementFactory()->fmChordArea() * (musElementFactory()->isFMChordAreaMinor() ? -1 : 1));
        uiFMTonicDegree->setCurrentId(musElementFactory()->fmTonicDegree() * (musElementFactory()->isFMTonicDegreeMinor() ? -1 : 1));
        uiFMEllipse->setChecked(musElementFactory()->isFMEllipse());

        uiFMKeySig->setCurrentIndex((musElementFactory()->diatonicKeyNumberOfAccs() + 7) * 2 + ((musElementFactory()->diatonicKeyGender() == CADiatonicKey::Minor) ? 1 : 0));

        uiFMToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CAFunctionMark*>(currentScoreView()->selection().at(0)->musElement())) {
        CAFunctionMark* fm = dynamic_cast<CAFunctionMark*>(currentScoreView()->selection().at(0)->musElement());
        uiFMFunction->setCurrentId(fm->function() * (fm->isMinor() ? -1 : 1));
        uiFMChordArea->setCurrentId(fm->chordArea() * (fm->isChordAreaMinor() ? -1 : 1));
        uiFMTonicDegree->setCurrentId(fm->tonicDegree() * (fm->isTonicDegreeMinor() ? -1 : 1));
        uiFMEllipse->setChecked(fm->isPartOfEllipse());

        uiFMKeySig->setCurrentIndex(
            uiFMKeySig->findData(
                CADiatonicKey::diatonicKeyToString(fm->key())));

        uiFMToolBar->show();
    } else {
        uiFMToolBar->hide();
    }
}

/*!
	Shows/Hides the dynamic marks properties tool bar according to the current state.
*/
void CAMainWin::updateDynamicToolBar()
{
    if (uiMarkType->isChecked() && uiMarkType->currentId() == CAMark::Dynamic && mode() == InsertMode) {
        uiDynamicText->setCurrentId(CADynamic::dynamicTextFromString(musElementFactory()->dynamicText()));
        uiDynamicVolume->setValue(musElementFactory()->dynamicVolume());
        uiDynamicCustomText->setText(musElementFactory()->dynamicText());
        uiDynamicToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CADynamic*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CADynamic* dynamic = dynamic_cast<CADynamic*>(v->selection().at(0)->musElement());
            if (dynamic) {
                uiDynamicText->setCurrentId(CADynamic::dynamicTextFromString(dynamic->text()));
                uiDynamicVolume->setValue(dynamic->volume());
                uiDynamicCustomText->setText(dynamic->text());
                uiDynamicToolBar->show();
            } else
                uiDynamicToolBar->hide();
        }
    } else
        uiDynamicToolBar->hide();
}

/*!
	Shows/Hides the fermata properties tool bar according to the current state.
*/
void CAMainWin::updateFermataToolBar()
{
    if (uiMarkType->isChecked() && uiMarkType->currentId() == CAMark::Fermata && mode() == InsertMode) {
        uiFermataType->setCurrentId(musElementFactory()->fermataType());
        uiFermataToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CAFermata*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CAFermata* f = dynamic_cast<CAFermata*>(v->selection().at(0)->musElement());
            if (f) {
                uiFermataType->setCurrentId(f->fermataType());
                uiFermataToolBar->show();
            } else
                uiFermataToolBar->hide();
        }
    } else
        uiFermataToolBar->hide();
}

/*!
	Shows/Hides the repeat mark properties tool bar according to the current state.
*/
void CAMainWin::updateRepeatMarkToolBar()
{
    if (uiMarkType->isChecked() && uiMarkType->currentId() == CAMark::RepeatMark && mode() == InsertMode) {
        if (musElementFactory()->repeatMarkType() == CARepeatMark::Volta)
            uiRepeatMarkType->setCurrentId(musElementFactory()->repeatMarkVoltaNumber() * (-1) - 1);
        else
            uiRepeatMarkType->setCurrentId(musElementFactory()->repeatMarkType());
        uiRepeatMarkToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CARepeatMark*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CARepeatMark* r = dynamic_cast<CARepeatMark*>(v->selection().at(0)->musElement());
            if (r) {
                if (r->repeatMarkType() == CARepeatMark::Volta)
                    uiRepeatMarkType->setCurrentId(r->voltaNumber() * (-1) - 1);
                else
                    uiRepeatMarkType->setCurrentId(r->repeatMarkType());
                uiRepeatMarkToolBar->show();
            } else
                uiRepeatMarkToolBar->hide();
        }
    } else
        uiRepeatMarkToolBar->hide();
}

/*!
	Shows/Hides the fingering properties tool bar according to the current state.
*/
void CAMainWin::updateFingeringToolBar()
{
    if (uiMarkType->isChecked() && uiMarkType->currentId() == CAMark::Fingering && mode() == InsertMode) {
        uiFinger->setCurrentId(musElementFactory()->fingeringFinger());
        uiFingeringOriginal->setChecked(musElementFactory()->isFingeringOriginal());
        uiFingeringToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CAFingering*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CAFingering* f = dynamic_cast<CAFingering*>(v->selection().at(0)->musElement());
            if (f) {
                uiFinger->setCurrentId(f->finger());
                uiFingeringOriginal->setChecked(f->isOriginal());
                uiFingeringToolBar->show();
            } else
                uiFingeringToolBar->hide();
        }
    } else
        uiFingeringToolBar->hide();
}

/*!
	Shows/Hides the tempo marks properties tool bar according to the current state.
*/
void CAMainWin::updateTempoToolBar()
{
    if (uiMarkType->isChecked() && uiMarkType->currentId() == CAMark::Tempo && mode() == InsertMode) {
        uiTempoBeat->setCurrentId(musElementFactory()->tempoBeat().musicLength() * (musElementFactory()->tempoBeat().dotted() ? -1 : 1));
        uiTempoBpm->setText(QString::number(musElementFactory()->tempoBpm()));
        uiTempoToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CATempo*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CATempo* tempo = dynamic_cast<CATempo*>(v->selection().at(0)->musElement());
            if (tempo) {
                uiTempoBeat->setCurrentId(tempo->beat().musicLength() * (tempo->beat().dotted() ? -1 : 1));
                uiTempoBpm->setText(QString::number(tempo->bpm()));
                uiTempoToolBar->show();
            } else
                uiTempoToolBar->hide();
        }
    } else
        uiTempoToolBar->hide();
}

/*!
	Shows/Hides the instrument marks properties tool bar according to the current state.
*/
void CAMainWin::updateInstrumentToolBar()
{
    if (uiMarkType->isChecked() && uiMarkType->currentId() == CAMark::InstrumentChange && mode() == InsertMode) {
        uiInstrumentChange->setCurrentIndex(musElementFactory()->instrument());
        uiInstrumentToolBar->show();
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size() && dynamic_cast<CAInstrumentChange*>(currentScoreView()->selection().at(0)->musElement())) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CAInstrumentChange* instrument = dynamic_cast<CAInstrumentChange*>(v->selection().at(0)->musElement());
            if (instrument) {
                uiInstrumentChange->setCurrentIndex(instrument->instrument());
                uiInstrumentToolBar->show();
            } else
                uiInstrumentToolBar->hide();
        }
    } else
        uiInstrumentToolBar->hide();
}

/*!
	Action on Edit->Copy.
*/
void CAMainWin::on_uiCopy_triggered()
{
    if (currentScoreView()) {
        copySelection(currentScoreView());
    }
}

/*!
	Action on Edit->Cut.
*/
void CAMainWin::on_uiCut_triggered()
{
    if (currentScoreView()) {
        stopPlayback();
        CACanorus::undo()->createUndoCommand(document(), tr("cut", "undo"));
        copySelection(currentScoreView());
        deleteSelection(currentScoreView(), false, true, false); // and don't make undo as we already make it
        CACanorus::undo()->pushUndoCommand();
    }
}

/*!
	Action on Edit->Paste.
*/
void CAMainWin::on_uiPaste_triggered()
{
    if (currentScoreView()) {
        pasteAt(currentScoreView()->lastMousePressCoords(), currentScoreView());
    }
}

/*!
	Backend for Edit->Copy.
*/
void CAMainWin::copySelection(CAScoreView* v)
{
    if (v->selection().size()) {
        CASheet* currentSheet = v->sheet();
        QHash<CAContext*, QList<CAMusElement*>> eltMap;
        QList<CAContext*> contexts;
        for (int i = 0; i < currentSheet->contextList().size(); i++)
            contexts << nullptr;

        for (CADrawableMusElement* drawable : v->selection()) {
            CAMusElement* elt;
            CAContext* context;
            if (!(elt = drawable->musElement()) || !(context = elt->context()))
                continue;
            if (elt->musElementType() == CAMusElement::Mark || elt->musElementType() == CAMusElement::Tuplet || elt->musElementType() == CAMusElement::Slur)
                continue; // marks are cloned together with their associated element, no need to clone them separately.
                    // tuplets and slurs cannot be inserted into a context or a voice
            if (context->contextType() != CAContext::Staff && context->contextType() != CAContext::LyricsContext)
                continue; // only these context types are impl'd. If we added anything else, we'd have old pointers in the context list.
            eltMap[context] << elt;
            int idx = currentSheet->contextList().indexOf(context);
            contexts[idx] = context;
        }
        contexts.removeAll(nullptr);
        // contexts now contains the contexts of the selected elements, in the correct order.

        // Copy staff elements
        QHash<CAVoice*, CAVoice*> voiceMap; // all voices in selection
        for (int i = 0; i < contexts.size(); i++) {
            CAContext* context = contexts[i];
            if (context->contextType() != CAContext::Staff)
                continue;
            CAStaff* staff = static_cast<CAStaff*>(context);
            CAStaff* newStaff = new CAStaff("", nullptr, staff->numberOfLines());
            contexts[i] = newStaff;

            QList<CAVoice*> voices;
            for (int i = 0; i < staff->voiceList().size(); i++)
                voices << nullptr;

            // create voices
            for (CAMusElement* elt : eltMap[context]) {
                if (!elt->isPlayable())
                    continue;
                CAVoice* voice = static_cast<CAPlayable*>(elt)->voice();
                int idx = staff->voiceList().indexOf(voice);
                if (!voices[idx])
                    voiceMap[voice] = voices[idx] = new CAVoice("", newStaff);
            }

            CAVoice* defaultVoice = nullptr; // for non-playable elements
            for (CAVoice* voice : voices) {
                if (voice) {
                    defaultVoice = voice;
                    break;
                }
            }
            if (!defaultVoice)
                defaultVoice = new CAVoice("", newStaff);
            // future FIXME (also in pasteAt): tuplets across staves (when cross-staff beam are impl'd GUI-wise).
            QHash<CATuplet*, QList<CAPlayable*>> tupletMap;
            QHash<CASlur*, CANote*> slurMap; //FIXME cross-staff slurs, when the crash is fixed (try cutting one)
            for (CAMusElement* elt : eltMap[context]) {
                if (elt->isPlayable()) {
                    CAPlayable* pl = static_cast<CAPlayable*>(elt);
                    CAVoice* voice = pl->voice();
                    int idx = staff->voiceList().indexOf(voice);
                    bool addToChord = false;
                    int eltidx = eltMap[context].indexOf(elt);
                    CANote* note = (pl->musElementType() == CAMusElement::Note) ? static_cast<CANote*>(pl) : nullptr;
                    if (note) {
                        CAMusElement* prev = nullptr;
                        for (int previdx = eltidx - 1; previdx >= 0; previdx--) {
                            if ((prev = eltMap[context][previdx]) && prev->musElementType() == CAMusElement::Note
                                && pl->voice() == static_cast<CAPlayable*>(prev)->voice()) {
                                //CANote *prevNote = static_cast<CANote*>(prev);
                                addToChord = prev->timeStart() == note->timeStart();
                                break;
                            }
                        }
                    }
                    CAPlayable* cloned = pl->clonePlayable(voices[idx]).get();
                    voices[idx]->append(cloned, addToChord);
                    if (pl->tuplet()) {
                        tupletMap[pl->tuplet()] << cloned;
                        if (tupletMap[pl->tuplet()].size() == pl->tuplet()->noteList().size())
                            pl->tuplet()->cloneTuplet(tupletMap[pl->tuplet()]).get();
                    }
                    if (note) {
                        QList<CASlur*> slurs;
                        slurs << note->tieStart() << note->tieEnd() << note->slurStart() << note->slurEnd() << note->phrasingSlurStart() << note->phrasingSlurEnd();
                        slurs.removeAll(nullptr);
                        for (CASlur* s : slurs) {
                            if (!slurMap.contains(s))
                                slurMap[s] = static_cast<CANote*>(cloned);
                            else {
                                CANote *noteStart = slurMap[s], *noteEnd = static_cast<CANote*>(cloned);
                                CASlur* newSlur = s->cloneSlur(noteStart->context(), noteStart, noteEnd).get();
                                switch (s->slurType()) {
                                case CASlur::TieType:
                                    noteStart->setTieStart(newSlur);
                                    noteEnd->setTieEnd(newSlur);
                                    break;
                                case CASlur::SlurType:
                                    noteStart->setSlurStart(newSlur);
                                    noteEnd->setSlurEnd(newSlur);
                                    break;
                                case CASlur::PhrasingSlurType:
                                    noteStart->setPhrasingSlurStart(newSlur);
                                    noteEnd->setPhrasingSlurEnd(newSlur);
                                    break;
                                }
                            }
                        }
                    }
                } else
                    defaultVoice->append(elt->cloneMusElement(newStaff).get());
            }

            voices.removeAll(nullptr);
            if (voices.isEmpty())
                voices << defaultVoice;

            //CAStaff *last = static_cast<CAStaff*>(currentSheet->contextList().last());
            for (CAVoice* voice : voices)
                newStaff->addVoice(voice);
        }

        // Copy lyrics
        for (int i = 0; i < contexts.size(); i++) {
            CAContext* context = contexts[i];
            if (context->contextType() != CAContext::LyricsContext)
                continue;
            CALyricsContext* lc = static_cast<CALyricsContext*>(context);
            CAVoice* associated = voiceMap[lc->associatedVoice()]; // init'd to 0 if map doesn't contain the voice.
            CALyricsContext* newLc = new CALyricsContext("", 0 /*FIXME*/, associated);
            contexts[i] = newLc;
            for (CAMusElement* elt : eltMap[context])
                newLc->addSyllable(static_cast<CASyllable*>(elt->cloneMusElement(newLc).get()), false);
        }

        QApplication::clipboard()->setMimeData(new CAMimeData(contexts));
    }
}

/*!
	Delete action.

	If \a deleteSyllables or \a deleteNotes is True, it completely removes the element.
	Otherwise it only replaces it with rest.

	If \a doUndo is True, it also creates undo. doUndo should be False when cutting.
*/
void CAMainWin::deleteSelection(CAScoreView* v, bool deleteSyllables, bool deleteNotes, bool doUndo)
{
    if (v->selection().size()) {
        if (doUndo)
            CACanorus::undo()->createUndoCommand(document(), tr("deletion of elements", "undo"));

        QSet<CAMusElement*> musElemSet;
        QHash<CAFiguredBassMark*, QList<int>> numbersToDelete;
        for (int i = 0; i < v->selection().size(); i++) {
            musElemSet << v->selection().at(i)->musElement();

            // gather numbers to delete of figured bass numbers
            if (dynamic_cast<CADrawableFiguredBassNumber*>(v->selection().at(i))) {
                CAFiguredBassMark* fbm = static_cast<CAFiguredBassMark*>(v->selection().at(i)->musElement());
                if (!numbersToDelete.contains(fbm)) {
                    numbersToDelete[fbm] = QList<int>();
                }

                numbersToDelete[fbm] << dynamic_cast<CADrawableFiguredBassNumber*>(v->selection().at(i))->number();
            }
        }

        // cleans up the set - removes empty elements and elements which get deleted automatically (eg. slurs, if both notes are deleted, marks)
        for (QSet<CAMusElement*>::iterator i = musElemSet.begin(); i != musElemSet.end();) {
            if (!(*i) || ((*i)->musElementType() == CAMusElement::Slur && musElemSet.contains(static_cast<CASlur*>(*i)->noteStart())) || ((*i)->musElementType() == CAMusElement::Slur && musElemSet.contains(static_cast<CASlur*>(*i)->noteEnd())) || ((*i)->musElementType() == CAMusElement::Mark && musElemSet.contains(static_cast<CAMark*>(*i)->associatedElement()))) {
                i = musElemSet.erase(i);
            } else {
                i++;
            }
        }

        for (QSet<CAMusElement*>::const_iterator i = musElemSet.constBegin(); i != musElemSet.constEnd(); i++) {
            if ((*i)->isPlayable()) {
                CAPlayable* p = static_cast<CAPlayable*>(*i);
                if ((p->musElementType() == CAMusElement::Rest) || (!static_cast<CANote*>(p)->isPartOfChord())) {
                    // find out the status of the rests in other voices
                    QList<CAPlayable*> chord = p->staff()->getChord(p->timeStart());
                    QList<CARest*> restsInOtherVoices;

                    // if deleting a rest, shift back by default
                    if (p->musElementType() == CAMusElement::Rest)
                        deleteNotes = true;

                    int chordIdx;
                    for (chordIdx = 0; chordIdx < chord.size(); chordIdx++) { // calculates chordIdx
                        if (chord[chordIdx]->voice() != p->voice()) {
                            CAPlayable* current = chord[chordIdx];
                            do {
                                if (current->musElementType() == CAMusElement::Rest)
                                    restsInOtherVoices << static_cast<CARest*>(current);
                                else if (!musElemSet.contains(current)) {
                                    deleteNotes = false; // note in other voice which is not going to be deleted, don't shift back
                                    break;
                                }
                                // loop over following playables while they start before p ends. i.e., not shifting back in situations such as: << { c1 } \ { r4 r g g } >>.
                            } while ((current = current->voice()->nextPlayable(current->timeStart())) && (current->timeStart() < p->timeEnd()));

                            if (!current && restsInOtherVoices.size() && (restsInOtherVoices.back()->voice() != chord[chordIdx]->voice() || (restsInOtherVoices.back()->voice() == chord[chordIdx]->voice() && restsInOtherVoices.back()->timeEnd() < p->timeEnd()))) {
                                deleteNotes = false;
                                break;
                            }
                        }
                    }
                    if (!deleteNotes) {
                        // replace note with rest

                        QList<CARest*> rests = CARest::composeRests(CAPlayableLength::playableLengthToTimeLength(p->playableLength()), p->timeStart(), p->voice(), CARest::Normal);
                        for (int j = 0; j < rests.size(); j++)
                            p->voice()->insert(p, rests[j]);

                        for (int j = 0; j < p->voice()->lyricsContextList().size(); j++) { // remove syllables
                            CASyllable* removedSyllable = p->voice()->lyricsContextList().at(j)->syllableAtTimeStart(p->timeStart());

                            musElemSet.remove(removedSyllable); // only remove syllables from the selection
                        }

                        if (p->tuplet()) { // remove the note from tuplet and add a rest
                            CATuplet* tuplet = p->tuplet();
                            tuplet->removeNote(p);
                            p->setTuplet(nullptr);
                            for (int j = 0; j < rests.size(); j++) {
                                tuplet->addNote(rests[j]);
                            }

                            p->voice()->remove(p, true);
                            tuplet->assignTimes();
                        }

                    } else {
                        // Actually remove the note or rest and shift other elements back if only rests in other voices present.
                        // This is allowed, if only rests are present below the deleted element in other voices.
                        // Example - the most comprehensive deletion:
                        // When deleting a half note in the first voice and there are two half rests in the second voice sticking
                        // together just in the middle of our half note, the deletion is made in 3 steps:
                        // 1. Convert the two rests below the deleted element to one quarter rest before the element, one half rest
                        //    just below the deleted element (its timeStart and timeLength are now equal to deleted elt) and one
                        //    quarter rest after the deleted element.
                        // 2. Do step 1 for all other voices (we only have 2 voices in our case).
                        // 3. Delete the newly generated rest right below the element in other voices and do not shift back
                        //    shared elements (shared elements are shifted back in the next step).
                        // 4. Delete the deleted element and shift back shared elements.

                        QList<CARest*> restRightBelowDeletedElement;
                        // remove any rests from other voices
                        for (int j = 0; j < restsInOtherVoices.size(); j++) {

                            // gather rests in the current voice
                            QList<CARest*> restsInCurVoice;
                            restsInCurVoice << restsInOtherVoices[j];
                            CAVoice* rVoice = restsInOtherVoices[j]->voice();
                            while ((++j < restsInOtherVoices.size()) && (restsInOtherVoices[j]->voice() == rVoice)) {
                                restsInCurVoice << restsInOtherVoices[j];
                            }

                            int firstTimeStart = restsInCurVoice.front()->timeStart();
                            int fullTimeLength = restsInCurVoice.back()->timeEnd() - restsInCurVoice.front()->timeStart();
                            int firstTimeLength = p->timeStart() - firstTimeStart;
                            CARest::CARestType firstRestType = restsInCurVoice.front()->restType();
                            CARest::CARestType lastRestType = restsInCurVoice.back()->restType();

                            CAMusElement* nextElt = rVoice->next(restsInCurVoice.back()); // needed later for insertion

                            // convert the rests
                            for (int k = 0; k < restsInCurVoice.size(); k++) {
                                if (restsInCurVoice[k]->tuplet()) {
                                    musElemSet.remove(restsInCurVoice[k]->tuplet());
                                }
                                musElemSet.remove(restsInCurVoice[k]);
                                rVoice->remove(restsInCurVoice[k], true);
                            }

                            // insert the first rests
                            QList<CARest*> firstRests = CARest::composeRests(firstTimeLength, firstTimeStart, rVoice, firstRestType);
                            for (int k = 0; k < firstRests.size(); k++) {
                                rVoice->insert(nextElt, firstRests[k]);
                            }

                            // insert the rests below the element - needed to correctly update timeStart of shared elements
                            QList<CARest*> restsRightBelow = CARest::composeRests(p->timeLength(), firstTimeStart + firstTimeLength, rVoice, CARest::Normal);
                            for (int k = 0; k < restsRightBelow.size(); k++) {
                                rVoice->insert(nextElt, restsRightBelow[k]);
                            }

                            // insert the rests after the element
                            QList<CARest*> lastRests = CARest::composeRests(fullTimeLength - (firstTimeLength + p->timeLength()), firstTimeStart + firstTimeLength + p->timeLength(), rVoice, lastRestType);
                            for (int k = 0; k < lastRests.size(); k++) {
                                rVoice->insert(nextElt, lastRests[k]);
                            }

                            // delete the rests below the element
                            for (int k = 0; k < restsRightBelow.size(); k++) {
                                delete restsRightBelow[k]; // do not shift back shared elements
                            }
                        }

                        for (int j = 0; j < p->voice()->lyricsContextList().size(); j++) // delete and shift syllables
                            musElemSet.remove(p->voice()->lyricsContextList().at(j)->removeSyllableAtTimeStart(p->timeStart()));

                        if (p->tuplet()) { // remove the tuplet from selection, if any, because it's deleted in playable destructor
                            musElemSet.remove(p->tuplet());
                        }
                    }
                }

                // stop the playback before deleting the note
                if (_playback && _playback->curPlaying().contains(p)) {
                    _playback->stopNow();
                }

                p->voice()->remove(p, true);
                for (int j = 0; j < p->voice()->lyricsContextList().size(); j++) {
                    p->voice()->lyricsContextList().at(j)->repositSyllables();
                }
                delete p;
            } else if ((*i)->musElementType() == CAMusElement::Syllable) {
                if (deleteSyllables) {
                    CALyricsContext* lc = static_cast<CALyricsContext*>((*i)->context());
                    (*i)->context()->remove(*i); // actually removes the syllable if SHIFT is pressed
                    lc->repositSyllables();
                } else {
                    static_cast<CASyllable*>(*i)->clear(); // only clears syllable's text
                }
            } else if ((*i)->musElementType() == CAMusElement::FiguredBassMark) {
                CAFiguredBassMark* fbm = static_cast<CAFiguredBassMark*>(*i);
                CAFiguredBassContext* fbc = static_cast<CAFiguredBassContext*>(fbm->context());

                if (deleteSyllables && fbm->numbers().size() == numbersToDelete[fbm].size()) {
                    (*i)->context()->remove(*i); // actually removes the function if SHIFT is pressed
                    fbc->repositFiguredBassMarks();
                } else {
                    for (int j = 0; j < numbersToDelete[fbm].size(); j++) {
                        fbm->removeNumber(numbersToDelete[fbm][j]);
                    }
                }
            } else if ((*i)->musElementType() == CAMusElement::FunctionMark) {
                if (deleteSyllables) {
                    CAFunctionMarkContext* fmc = static_cast<CAFunctionMarkContext*>((*i)->context());
                    (*i)->context()->remove(*i); // actually removes the function if SHIFT is pressed
                    fmc->repositFunctions();
                } else {
                    static_cast<CAFunctionMark*>(*i)->clear(); // only clears the function
                }
            } else if ((*i)->musElementType() == CAMusElement::Mark) {
                delete *i; // also removes itself from associated elements
            } else if ((*i)->musElementType() == CAMusElement::Slur) {
                delete *i; // also removes itself from associated elements
            } else if ((*i)->musElementType() == CAMusElement::Tuplet) {
                delete *i; // also removes itself from associated elements
            } else {
                (*i)->context()->remove(*i);
            }
        }
        if (doUndo)
            CACanorus::undo()->pushUndoCommand();

        if (CACanorus::settings()->useNoteChecker()) {
            _noteChecker.checkSheet(v->sheet());
        }

        v->clearSelection();
        CACanorus::rebuildUI(document(), v->sheet());
    }
}

/*!
	Backend for Edit->Paste.
*/
void CAMainWin::pasteAt(const QPoint coords, CAScoreView* v)
{
    if (QApplication::clipboard()->mimeData() && dynamic_cast<const CAMimeData*>(QApplication::clipboard()->mimeData()) && v->currentContext()) {
        CACanorus::undo()->createUndoCommand(document(), tr("paste", "undo"));

        CAContext* currentContext = v->currentContext()->context();
        CASheet* currentSheet = currentContext->sheet();

        QList<CAMusElement*> newEltList;
        QList<CAContext*> contexts = static_cast<const CAMimeData*>(QApplication::clipboard()->mimeData())->contexts();
        QHash<CAVoice*, CAVoice*> voiceMap; // MimeData -> paste
        CAContext* insertAfter = nullptr;
        for (CAContext* context : contexts) {
            // create a new context if there isn't one of the right type.
            // exception: if the context is a staff, skip lyrics contexts instead of inserting a staff before a lyrics context.
            if (context->contextType() == CAContext::Staff) {
                while (currentContext && currentContext->contextType() == CAContext::LyricsContext)
                    if (currentContext != currentSheet->contextList().last())
                        currentContext = currentSheet->contextList()[currentSheet->contextList().indexOf(currentContext) + 1];
                    else
                        currentContext = nullptr;
            }

            if (!currentContext || context->contextType() != currentContext->contextType()) {
                CAContext* newContext = nullptr;
                switch (context->contextType()) {
                case CAContext::Staff: {
                    CAStaff /* * s = static_cast<CAStaff*>(context),*/* newStaff;
                    newContext = newStaff = new CAStaff(tr("Staff%1").arg(v->sheet()->staffList().size() + 1), currentSheet);
                    break;
                }
                case CAContext::LyricsContext: {
                    /* Two cases:
						 * - Some notes were copied together with the lyrics below them: in this case the linked voice would've already been pasted (as the context list is ordered top to bottom), so we find the new voice using voiceMap.
						 * - Lyrics were copied without the notes. If currentContext is a staff, we'll use the current voice. Otherwise lyrics will not be pasted.
						 */
                    CAVoice* voice = voiceMap[static_cast<CALyricsContext*>(context)->associatedVoice()];
                    if (!voice && currentContext && currentContext->contextType() == CAContext::Staff)
                        voice = static_cast<CAStaff*>(currentContext)->voiceList()[(currentContext == v->currentContext()->context()) ? (uiVoiceNum->getRealValue() ? uiVoiceNum->getRealValue() - 1 : uiVoiceNum->getRealValue()) : 1]; // That is, if the currentContext is still the context that the user last clicked before pasting, use the current voice number. Otherwise, use the first voice.
                    if (!voice)
                        continue; // skipping lyrics - can't find a staff.

                    newContext = new CALyricsContext(tr("LyricsContext%1").arg(v->sheet()->contextList().size() + 1), 1, voice);
                    insertAfter = voice->staff();
                    break;
                }
                case CAContext::FunctionMarkContext: {
                    newContext = new CAFunctionMarkContext(tr("FunctionMarkContext%1").arg(v->sheet()->contextList().size() + 1), currentSheet);
                    break;
                }
                case CAContext::FiguredBassContext:
                    break;
                case CAContext::ChordNameContext:
                    newContext = new CAChordNameContext(tr("ChordNameContext%1").arg(v->sheet()->contextList().size() + 1), currentSheet);
                    break;
                }
                if (insertAfter) {
                    currentSheet->insertContextAfter(insertAfter, newContext);
                    insertAfter = nullptr;
                } else if (currentContext)
                    currentSheet->insertContextAfter(currentContext, newContext);
                else
                    currentSheet->addContext(newContext);
                currentContext = newContext;
            }
            if (context->contextType() == CAContext::Staff) {
                CAStaff *staff = static_cast<CAStaff*>(currentContext), *cbstaff = static_cast<CAStaff*>(context);
                int voice = uiVoiceNum->getRealValue() ? uiVoiceNum->getRealValue() - 1 : uiVoiceNum->getRealValue();
                for (int i = staff->voiceList().size() - 1; i < voice + cbstaff->voiceList().size() - 1; i++) {
                    staff->addVoice();
                }
                for (int i = voice; i < voice + cbstaff->voiceList().size(); i++) {
                    int cbi = i - voice;
                    CADrawableMusElement* drawable = v->nearestRightElement(coords.x(), coords.y(), staff->voiceList()[i]);
                    voiceMap[cbstaff->voiceList()[cbi]] = staff->voiceList()[i];
                    CAMusElement* right = (drawable) ? drawable->musElement() : nullptr;

                    // Can't have playables between two notes linked by a tie. Remove the tie in this case.
                    // FIXME this should be the behavior for insert as well.
                    CAMusElement* leftPl = right;
                    while ((leftPl = staff->voiceList()[i]->previous(leftPl)) && !leftPl->isPlayable())
                        ;
                    CANote* leftNote = (leftPl && leftPl->musElementType() == CAMusElement::Note) ? static_cast<CANote*>(leftPl) : nullptr;
                    CASlur* tie = leftNote ? leftNote->tieStart() : nullptr;

                    if (tie) {
                        if (tie->noteEnd() && staff->voiceList()[i]->musElementList().contains(tie->noteEnd()))
                            // pasting between two tied notes - remove tie
                            delete tie; // resets notes' tieStart/tieEnd;
                        else {
                            // pasting after an "open" tie - if the first paste element is a note, connect them. Otherwise delete the tie.
                            int idx = 0;
                            for (; idx < cbstaff->voiceList()[cbi]->musElementList().size() && !cbstaff->voiceList()[cbi]->musElementList()[idx]->isPlayable(); idx++)
                                ;
                            CAPlayable* first = (idx != cbstaff->voiceList()[cbi]->musElementList().size()) ? static_cast<CAPlayable*>(cbstaff->voiceList()[cbi]->musElementList()[idx]) : nullptr;
                            if (first && first->musElementType() == CAMusElement::Note)
                                static_cast<CANote*>(first)->setTieEnd(tie);
                            else
                                delete tie;
                        }
                    }

                    QHash<CATuplet*, QList<CAPlayable*>> tupletMap;
                    QHash<CASlur*, CANote*> slurMap;
                    for (CAMusElement* elt : cbstaff->voiceList()[cbi]->musElementList()) {
                        CAMusElement* cloned = (elt->isPlayable()) ? static_cast<CAPlayable*>(elt)->clonePlayable(staff->voiceList()[i]).get() : elt->cloneMusElement(staff).get();
                        CANote* n = (elt->musElementType() == CAMusElement::Note) ? static_cast<CANote*>(elt) : nullptr;
                        CAMusElement* prev = cbstaff->voiceList()[cbi]->previous(n);
                        CANote* prevNote = (prev && prev->musElementType() == CAMusElement::Note) ? static_cast<CANote*>(prev) : nullptr;
                        bool chord = n && prevNote && prevNote->timeStart() == n->timeStart();
                        if (n) {
                            QList<CASlur*> slurs;
                            slurs << n->tieStart() << n->tieEnd() << n->slurStart() << n->slurEnd() << n->phrasingSlurStart() << n->phrasingSlurEnd();
                            slurs.removeAll(nullptr);
                            for (CASlur* s : slurs) {
                                if (!slurMap.contains(s))
                                    slurMap[s] = static_cast<CANote*>(cloned);
                                else {
                                    CANote *noteStart = slurMap[s], *noteEnd = static_cast<CANote*>(cloned);
                                    CASlur* newSlur = s->cloneSlur(noteStart->context(), noteStart, noteEnd).get();
                                    switch (s->slurType()) {
                                    case CASlur::TieType:
                                        noteStart->setTieStart(newSlur);
                                        noteEnd->setTieEnd(newSlur);
                                        break;
                                    case CASlur::SlurType:
                                        noteStart->setSlurStart(newSlur);
                                        noteEnd->setSlurEnd(newSlur);
                                        break;
                                    case CASlur::PhrasingSlurType:
                                        noteStart->setPhrasingSlurStart(newSlur);
                                        noteEnd->setPhrasingSlurEnd(newSlur);
                                        break;
                                    }
                                }
                            }
                        }
                        staff->voiceList()[i]->insert(chord ? newEltList.last() : right, cloned, chord);
                        newEltList << cloned;
                        if (elt->isPlayable()) {
                            CAPlayable* pl = static_cast<CAPlayable*>(elt);
                            if (pl->tuplet()) {
                                tupletMap[pl->tuplet()] << static_cast<CAPlayable*>(cloned);
                                if (tupletMap[pl->tuplet()].size() == pl->tuplet()->noteList().size())
                                    pl->tuplet()->cloneTuplet(tupletMap[pl->tuplet()]);
                            }
                        }
                        // FIXME duplicated from CAMusElementFactory::configureNote.
                        if (n && staff->voiceList()[i]->lastNote() != static_cast<CANote*>(cloned)) {
                            for (CALyricsContext* context : staff->voiceList()[i]->lyricsContextList())
                                context->addEmptySyllable(cloned->timeStart(), cloned->timeLength());
                            for (CAContext* context : currentSheet->contextList())
                                if (context->contextType() == CAContext::FunctionMarkContext)
                                    static_cast<CAFunctionMarkContext*>(context)->addEmptyFunction(cloned->timeStart(), cloned->timeLength());
                        }
                    }
                    for (CALyricsContext* context : staff->voiceList()[i]->lyricsContextList())
                        context->repositSyllables();
                    for (CAContext* context : currentSheet->contextList())
                        if (context->contextType() == CAContext::FunctionMarkContext)
                            static_cast<CAFunctionMarkContext*>(context)->repositFunctions();
                }
                staff->synchronizeVoices();
            } else {
                /// \todo function mark copy&paste unimplemented
                if (context->contextType() == CAContext::LyricsContext) {
                    CALyricsContext* lc = static_cast<CALyricsContext*>(context);
                    CALyricsContext* currentLc = static_cast<CALyricsContext*>(currentContext);
                    CADrawableMusElement* drawable = nullptr;
                    if (currentContext == v->currentContext()->context()) // pasting where the user has clicked
                        drawable = v->nearestRightElement(coords.x(), coords.y(), v->currentContext());
                    int offset = lc->syllableList()[0]->timeStart() - (drawable ? drawable->musElement()->timeStart() : 0);
                    // Add syllables until there are no more notes, [popping existing syllables on the right end?].
                    for (CASyllable* syl : lc->syllableList()) {
                        CASyllable* clone = syl->cloneSyllable(currentLc).get();
                        clone->setTimeStart(clone->timeStart() - offset);
                        currentLc->addSyllable(clone, false);
                        newEltList << clone;
                    }
                }
            }
            int idx = currentSheet->contextList().indexOf(currentContext);
            currentContext = (idx + 1 < currentSheet->contextList().size()) ? currentSheet->contextList()[idx + 1] : nullptr;
        }

        if (CACanorus::settings()->useNoteChecker()) {
            _noteChecker.checkSheet(currentSheet);
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet);

        // select pasted elements
        currentScoreView()->clearSelection();
        for (int i = 0; i < newEltList.size(); i++)
            currentScoreView()->addToSelection(newEltList[i]);
        currentScoreView()->setLastMousePressCoordsAfter(newEltList);
        currentScoreView()->repaint();
    }
}

void CAMainWin::on_uiDynamicText_toggled(bool, int t)
{
    if (t == CADynamic::Custom)
        return;

    QString text = CADynamic::dynamicTextToString(static_cast<CADynamic::CADynamicText>(t));
    if (mode() == InsertMode) {
        musElementFactory()->setDynamicText(text);
        uiDynamicCustomText->setText(text);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CADynamic* dynamic = dynamic_cast<CADynamic*>(v->selection().at(0)->musElement());
            if (dynamic) {
                dynamic->setText(text);
                CACanorus::rebuildUI(document(), currentSheet());
            }
        }
    }
}

void CAMainWin::on_uiDynamicVolume_valueChanged(int vol)
{
    if (mode() == InsertMode) {
        musElementFactory()->setDynamicVolume(vol);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CADynamic* dynamic = dynamic_cast<CADynamic*>(v->selection().at(0)->musElement());
            if (dynamic) {
                dynamic->setVolume(vol);
                CACanorus::rebuildUI(document(), currentSheet());
            }
        }
    }
}

void CAMainWin::on_uiDynamicCustomText_returnPressed()
{
    QString text = uiDynamicCustomText->text();
    CADynamic::CADynamicText t = CADynamic::dynamicTextFromString(text);

    uiDynamicText->setCurrentId(t);
    if (mode() == InsertMode) {
        musElementFactory()->setDynamicText(text);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        if (v && v->selection().size()) {
            CADynamic* dynamic = dynamic_cast<CADynamic*>(v->selection().at(0)->musElement());
            if (dynamic) {
                dynamic->setText(text);
                CACanorus::rebuildUI(document(), currentSheet());
            }
        }
    }
}

void CAMainWin::on_uiInstrumentChange_activated(int index)
{
    if (mode() == InsertMode) {
        musElementFactory()->setInstrument(index);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change fermata type", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAInstrumentChange* instrument = dynamic_cast<CAInstrumentChange*>(v->selection().at(i)->musElement());

            if (instrument) {
                instrument->setInstrument(index);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFermataType_toggled(bool, int t)
{
    CAFermata::CAFermataType type = static_cast<CAFermata::CAFermataType>(t);

    if (mode() == InsertMode) {
        musElementFactory()->setFermataType(type);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change fermata type", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFermata* fm = dynamic_cast<CAFermata*>(v->selection().at(i)->musElement());

            if (fm) {
                fm->setFermataType(type);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFinger_toggled(bool, int t)
{
    CAFingering::CAFingerNumber type = static_cast<CAFingering::CAFingerNumber>(t);

    if (mode() == InsertMode) {
        musElementFactory()->setFingeringFinger(type);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change finger", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFingering* f = dynamic_cast<CAFingering*>(v->selection().at(i)->musElement());

            if (f) {
                f->setFinger(type);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiFingeringOriginal_toggled(bool checked)
{
    if (mode() == InsertMode) {
        musElementFactory()->setFingeringOriginal(checked);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change finger original property", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CAFingering* f = dynamic_cast<CAFingering*>(v->selection().at(i)->musElement());

            if (f) {
                f->setOriginal(checked);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiRepeatMarkType_toggled(bool, int t)
{
    CARepeatMark::CARepeatMarkType type;
    int voltaNumber;
    if (t >= 0) {
        type = static_cast<CARepeatMark::CARepeatMarkType>(t);
        voltaNumber = 0;
    } else {
        type = CARepeatMark::Volta;
        voltaNumber = t * (-1) - 1;
    }

    if (mode() == InsertMode) {
        musElementFactory()->setRepeatMarkType(type);
        musElementFactory()->setRepeatMarkVoltaNumber(voltaNumber);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change repeat mark", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CARepeatMark* r = dynamic_cast<CARepeatMark*>(v->selection().at(i)->musElement());

            if (r) {
                r->setRepeatMarkType(type);
                r->setVoltaNumber(voltaNumber);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiTempoBeat_toggled(bool, int t)
{
    CAPlayableLength length = CAPlayableLength(static_cast<CAPlayableLength::CAMusicLength>(t < 0 ? t * (-1) : t), t < 0 ? 1 : 0);

    if (mode() == InsertMode) {
        musElementFactory()->setTempoBeat(length);
    } else if (mode() == EditMode && currentScoreView() && currentScoreView()->selection().size()) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change tempo beat", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CATempo* tempo = dynamic_cast<CATempo*>(v->selection().at(i)->musElement());

            if (tempo) {
                tempo->setBeat(length);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiTempoBpm_returnPressed()
{
    QString text = uiTempoBpm->text();
    int bpm = text.toInt();

    if (mode() == InsertMode) {
        musElementFactory()->setTempoBpm(bpm);
    } else if (mode() == EditMode) {
        CAScoreView* v = currentScoreView();
        CACanorus::undo()->createUndoCommand(document(), tr("change tempo bpm", "undo"));

        for (int i = 0; i < v->selection().size(); i++) {
            CATempo* tempo = dynamic_cast<CATempo*>(v->selection().at(i)->musElement());

            if (tempo) {
                tempo->setBpm(bpm);
            }
        }

        CACanorus::undo()->pushUndoCommand();
        CACanorus::rebuildUI(document(), currentSheet());
    }
}

void CAMainWin::on_uiOpenRecent_aboutToShow()
{
    while (uiOpenRecent->actions().size())
        delete uiOpenRecent->actions().at(0);

    for (int i = 0; i < CACanorus::recentDocumentList().size(); i++) {
        QAction* a = new QAction(CACanorus::recentDocumentList()[i], this);
        uiOpenRecent->addAction(a);
        connect(a, SIGNAL(triggered()), this, SLOT(onUiOpenRecentDocumentTriggered()));
    }
}

void CAMainWin::onUiOpenRecentDocumentTriggered()
{
    if (!handleUnsavedChanges()) {
        return;
    }

    bool success = openDocument(CACanorus::recentDocumentList().at(
        uiOpenRecent->actions().indexOf(static_cast<QAction*>(sender()))));

    if (!success) {
        CACanorus::removeRecentDocument(CACanorus::recentDocumentList().at(
            uiOpenRecent->actions().indexOf(static_cast<QAction*>(sender()))));
    }
}

/*!
	Immediately plays the notes. This is usually called when inserting
	new notes or changing the pitch of existing notes.

	\sa CASettings::_playInsertedNotes
 */
void CAMainWin::playImmediately(QList<CAMusElement*> elements)
{
    if (!_playback) {
        _playback = new CAPlayback(CACanorus::midiDevice());
        connect(_playback, SIGNAL(playbackFinished()), this, SLOT(playbackFinished()));
    }

    _playback->playImmediately(elements, CACanorus::settings()->midiOutPort());
}

/*!
	\var CADocument *CAMainWin::_document
	Pointer to the main window's document it represents.
	Null if no document opened.

	\sa document()
*/

/*!
	\var CAMode CAMainWin::_mode
	Main window's current mode (Select mode, Edit mode, Playback mode, Insert mode etc.).

	\sa CAMode, mode(), setMode()
*/

/*!
	\var QList<CAView*> CAMainWin::_viewList
	List of all available Views for any sheet in this main window.
*/

/*!
	\var CAView* CACanorus::_currentView
	Currently active View. Only one View per main window can be active.
*/

/*!
	\var CAView* CAMainWin::_playbackView
	View needed to be updated when playback is active.
*/

/*!
	\var QTimer* CACanorus::_repaintTimer
	Used when playback is active to repaint the playback View.
	\todo View should be repainted only when needed, not constantly as now. This should result in much less resource hunger.
*/

/*!
	\var CAMusElementFactory *CACanorus::_musElementFactory
	Factory for creating/configuring music elements before actually placing them.
*/

/*!
	\var bool CAMainWin::_rebuildUILock
	Lock to avoid recursive rebuilds of the GUI Views.
*/
