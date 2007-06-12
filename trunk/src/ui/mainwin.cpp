/*! 
	Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

//#include <Python.h> must be called before standard headers inclusion. See http://docs.python.org/api/includes.html
#ifdef USE_PYTHON
#include <Python.h>
#endif

#include <QtGui>
#include <QSlider>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>
#include <QKeyEvent>
#include <QString>
#include <QTextStream>
#include <QXmlInputSource>
#include <iostream>

#include "ui/mainwin.h"
#include "ui/keysigpsp.h"
#include "ui/timesigpsp.h"
#include "ui/midisetupdialog.h"

#include "interface/playback.h"
#include "interface/engraver.h"
#include "interface/pluginmanager.h"
#include "interface/mididevice.h"
#include "interface/rtmididevice.h"

#include "widgets/menutoolbutton.h"
#include "widgets/lcdnumber.h"

#include "widgets/viewport.h"
#include "widgets/viewportcontainer.h"
#include "widgets/scoreviewport.h"
#include "widgets/sourceviewport.h"

#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"
#include "drawable/drawablelyricscontext.h"
#include "drawable/drawablemuselement.h"
#include "drawable/drawablenote.h"

#include "core/canorus.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/functionmarkingcontext.h"
#include "core/lyricscontext.h"
#include "core/clef.h"
#include "core/keysignature.h"
#include "core/note.h"
#include "core/slur.h"
#include "core/rest.h"
#include "core/canorusml.h"
#include "core/voice.h"
#include "core/barline.h"
#include "core/timesignature.h"
#include "core/syllable.h"
#include "core/functionmarking.h"
#include "core/muselementfactory.h"

#include "scripting/swigruby.h"
#include "scripting/swigpython.h"

#include "export/lilypondexport.h"
#include "import/lilypondimport.h"

const QString CAMainWin::LILYPOND_FILTER = tr("LilyPond document (*.ly)");

/*!
	\class CAMainWin
	\brief Canorus main window
	Class CAMainWin represents Canorus main window.
	The core layout is generated using the Qt designer's ui/mainwin.ui file.
	Other widgets (specific toolbars, viewports, plugin menus) are generated manually in-code.
	
	Canorus supports multiple main windows pointing to the same document or separated document.
	
	Canorus uses multiple inheritance approach. See
	http://doc.trolltech.com/4.2/designer-using-a-component.html#the-multiple-inheritance-approach
	Class members having _ prefix are general private properties.
	Private attributes with ui prefix are GUI-only widgets objects created in Qt designer or manually. 
	
	\sa CAViewPort, CACanorus
*/

// Constructor
CAMainWin::CAMainWin(QMainWindow *oParent) : QMainWindow( oParent ) {
	// Locate resources (images, icons)
	QString currentPath = QDir::currentPath();
	
	QList<QString> resourcesLocations = CACanorus::locateResourceDir(QString("images"));
	if (!resourcesLocations.size()) // when Canorus not installed, search the source path
		resourcesLocations = CACanorus::locateResourceDir(QString("ui/images"));
		
	QDir::setCurrent( resourcesLocations[0] ); /// \todo Button and menu icons by default look at the current working directory as their resource path only. QResource::addSearchPath() doesn't work for external icons. Any other ideas? -Matevz
	// Initialize widgets
	setupUi( this );
	setupCustomUi();
	QDir::setCurrent( currentPath );

	// Explicitly initialize this so it isn't true sometimes
	setRebuildUILock( false );
	
	// Initialize import/export dialogs
	uiExportDialog = new QFileDialog(this);
	uiExportDialog->setFileMode(QFileDialog::AnyFile);
	uiExportDialog->setDirectory( QDir::current() );
	uiExportDialog->setAcceptMode( QFileDialog::AcceptSave );
	uiExportDialog->setFilter( LILYPOND_FILTER ); // add LilyPond filter defined in mainwin.h
  	
	uiImportDialog = new QFileDialog(this);
	uiImportDialog->setFileMode( QFileDialog::ExistingFile );
	uiImportDialog->setDirectory( QDir::current() );
	uiImportDialog->setAcceptMode( QFileDialog::AcceptOpen );
	
	// Initialize internal UI properties
	_mode = SelectMode;
	_playback = 0;
	_animatedScroll = true;
	_lockScrollPlayback = false;
	
	// Create plugins menus and toolbars in this main window
	CAPluginManager::enablePlugins(this);
	
	// Connects MIDI IN callback function to a local slot
	connect( CACanorus::midiDevice(), SIGNAL(midiInEvent( QVector<unsigned char> )), this, SLOT(on_midiInEvent( QVector<unsigned char> )) );
	
	// Connect QTimer so it increases the local document edited time every second
	connect( &_timeEditedTimer, SIGNAL(timeout()), this, SLOT(on_timeEditedTimer_timeout()) );
	_timeEditedTimer.start(1000);
	
	setDocument( 0 );
}

CAMainWin::~CAMainWin()  {
	CACanorus::removeMainWin(this);
	if (!CACanorus::mainWinCount(document())) {
		CACanorus::deleteUndoStack(document()); // delete undo stack when the last document deleted
		delete document();
	}
	
	delete _playback;
	
	// clear UI
	delete uiImportDialog; delete uiExportDialog;
	delete uiInsertToolBar; delete uiInsertGroup; delete uiContextType; delete uiSlurType; delete uiClefType; delete uiBarlineType;
	delete uiVoiceToolBar; delete uiVoiceNum; delete uiVoiceName; delete uiRemoveVoice; delete uiVoiceStemDirection; delete uiVoiceProperties;
	delete uiPlayableToolBar; delete uiPlayableLength; delete uiNoteAccs; delete uiNoteStemDirection; delete uiHiddenRest;
	delete uiKeySigToolBar; delete uiKeySigNumberOfAccs;
	delete uiTimeSigToolBar; delete uiTimeSigBeats; delete uiTimeSigSlash; delete uiTimeSigBeat;
	delete uiFMToolBar; delete uiFMFunction; delete uiFMChordArea; delete uiFMTonicDegree; delete uiFMEllipse;
}

/*!
	Creates more complex widgets and layouts that cannot be created using Qt Designer (like adding
	custom toolbars to main window, button boxes etc.).
*/
void CAMainWin::setupCustomUi() {
	_musElementFactory = new CAMusElementFactory();
	
	// Hide the specialized pre-created toolbars in Qt designer.
	/// \todo When Qt Designer have support for setting the visibility property, do this in Qt Designer already! -Matevz
	uiPrintToolBar->hide();
	uiFileToolBar->hide();
	uiStandardToolBar->updateGeometry();
	
	// Toolbars
	uiInsertToolBar = new QToolBar( tr("Insert ToolBar"), this );
		uiInsertToolBar->addAction( uiSelectMode );
		uiInsertToolBar->addSeparator();		
		uiInsertToolBar->addWidget( uiContextType = new CAMenuToolButton( tr("Select Context" ), 3, this ));
			uiContextType->addButton( QIcon("images/staffnew.svg"), CAContext::Staff, tr("New Staff") );
			uiContextType->addButton( QIcon("images/lyricscontextnew.svg"), CAContext::LyricsContext, tr("New Lyrics context") );
			uiContextType->addButton( QIcon("images/fmcontextnew.svg"), CAContext::FunctionMarkingContext, tr("New Function Marking context") );
			uiContextType->setCurrentId( CAContext::Staff );
			connect( uiContextType, SIGNAL( toggled(bool, int) ), this, SLOT( on_uiContextType_toggled(bool, int) ) );
			connect( uiNewContext, SIGNAL( triggered() ), uiContextType, SLOT( click() ) );
		uiInsertToolBar->addSeparator();
		uiInsertToolBar->addAction( uiInsertPlayable );
		uiInsertToolBar->addWidget( uiSlurType = new CAMenuToolButton( tr("Select Slur Type"), 3, this ) );
			connect( uiSlurType, SIGNAL(toggled(bool, int)), this, SLOT( on_uiSlurType_toggled(bool, int) ) );
			uiSlurType->addButton( QIcon("images/tie.svg"), CASlur::TieType, tr("Tie") );
			uiSlurType->addButton( QIcon("images/slur.svg"), CASlur::SlurType, tr("Slur") );
			uiSlurType->addButton( QIcon("images/phrasingslur.svg"), CASlur::PhrasingSlurType, tr("Phrasing Slur") );
			uiSlurType->setCurrentId( CASlur::TieType );
			uiSlurType->defaultAction()->setCheckable(false);
		uiInsertToolBar->addWidget(uiClefType = new CAMenuToolButton( tr("Select Clef"), 3, this ));
			uiClefType->addButton( QIcon("images/cleftreble.svg"), CAClef::Treble, tr("Treble Clef") );
			uiClefType->addButton( QIcon("images/clefbass.svg"), CAClef::Bass, tr("Bass Clef") );
			uiClefType->addButton( QIcon("images/clefalto.svg"), CAClef::Alto, tr("Alto Clef") );
			uiClefType->setCurrentId( CAClef::Treble );
			connect( uiClefType, SIGNAL( toggled(bool, int) ), this, SLOT( on_uiClefType_toggled(bool, int) ) );
			connect( uiInsertClef, SIGNAL( triggered() ), uiClefType, SLOT( click() ) );
		uiInsertToolBar->addAction( uiInsertKeySig );
		uiInsertToolBar->addWidget(uiTimeSigType = new CAMenuToolButton( tr("Select Time Signature" ), 3, this ));
			uiTimeSigType->addButton( QIcon("images/tsc.svg"), TS_44 );
			uiTimeSigType->addButton( QIcon("images/tsab.svg"), TS_22 );
			uiTimeSigType->addButton( QIcon("images/ts34.svg"), TS_34 );
			uiTimeSigType->addButton( QIcon("images/ts24.svg"), TS_24 );
			uiTimeSigType->addButton( QIcon("images/ts38.svg"), TS_38 );
			uiTimeSigType->addButton( QIcon("images/ts68.svg"), TS_68 );
			uiTimeSigType->addButton( QIcon("images/tscustom.svg"), TS_CUSTOM );			
			uiTimeSigType->setCurrentId( TS_44 );
			connect( uiTimeSigType, SIGNAL(toggled(bool, int)), this, SLOT(on_uiTimeSigType_toggled(bool, int)) );
			connect( uiInsertTimeSig, SIGNAL(triggered()), uiTimeSigType, SLOT(click()));
		uiInsertToolBar->addWidget( uiBarlineType = new CAMenuToolButton( tr("Select Barline" ), 4, this ));
			uiBarlineType->addButton( QIcon("images/barlinesingle.svg"), CABarline::Single, tr("Single Barline") );
			uiBarlineType->addButton( QIcon("images/barlinedouble.svg"), CABarline::Double, tr("Double Barline") );
			uiBarlineType->addButton( QIcon("images/barlineend.svg"), CABarline::End, tr("End Barline") );
			uiBarlineType->addButton( QIcon("images/barlinedotted.svg"), CABarline::Dotted, tr("Dotted Barline") );
			uiBarlineType->addButton( QIcon("images/barlinerepeatopen.svg"), CABarline::RepeatOpen, tr("Repeat Open") );
			uiBarlineType->addButton( QIcon("images/barlinerepeatclose.svg"), CABarline::RepeatClose, tr("Repeat Closed") );
			uiBarlineType->addButton( QIcon("images/barlinerepeatcloseopen.svg"), CABarline::RepeatCloseOpen, tr("Repeat Closed-Open") );
			uiBarlineType->setCurrentId( CABarline::Single );
			connect( uiBarlineType, SIGNAL(toggled(bool, int)), this, SLOT(on_uiBarlineType_toggled(bool, int)) );
			connect( uiInsertBarline, SIGNAL( triggered() ), uiBarlineType, SLOT( click() ) );
		uiInsertToolBar->addAction( uiInsertSyllable );
		uiInsertToolBar->addAction( uiInsertFM );
		addToolBar(Qt::LeftToolBarArea, uiInsertToolBar);
	
	uiSheetToolBar = new QToolBar( tr("Sheet ToolBar"), this );
		uiSheetToolBar->addAction( uiNewSheet );
		uiSheetToolBar->addWidget( uiSheetName = new QLineEdit(this) );
			connect( uiSheetName, SIGNAL( returnPressed() ), this, SLOT( on_uiSheetName_returnPressed() ) );
		uiSheetToolBar->addAction( uiRemoveSheet );
		uiSheetToolBar->addAction( uiSheetProperties );
		addToolBar(Qt::TopToolBarArea, uiSheetToolBar);
	
	uiContextToolBar = new QToolBar( tr("Context ToolBar"), this );
		uiContextToolBar->addWidget( uiContextName = new QLineEdit(this) );
			connect( uiContextName, SIGNAL( returnPressed() ), this, SLOT( on_uiContextName_returnPressed() ) );
			uiContextName->setToolTip(tr("Context name"));
		uiContextToolBar->addWidget( uiStaffNumberOfLines = new QSpinBox(this) );
			connect( uiStaffNumberOfLines, SIGNAL(valueChanged(int)), this, SLOT(on_uiStaffNumberOfLines_valueChanged(int)) );
			uiStaffNumberOfLines->setToolTip(tr("Number of lines"));
			uiStaffNumberOfLines->hide();
		uiContextToolBar->addWidget( uiStanzaNumber = new QSpinBox(this) );
			connect( uiStanzaNumber, SIGNAL(valueChanged(int)), this, SLOT(on_uiStanzaNumber_valueChanged(int)) );
			uiStanzaNumber->setToolTip(tr("Stanza number"));
			uiStanzaNumber->hide();
		uiContextToolBar->addWidget( uiAssociatedVoice = new QComboBox(this) );
			// Warning! disconnect and reconnect is also done in updateContextToolBar()!
			connect( uiAssociatedVoice, SIGNAL(currentIndexChanged(int)), this, SLOT(on_uiAssociatedVoice_currentIndexChanged(int)) );
			uiAssociatedVoice->setToolTip(tr("Associated voice"));
			uiAssociatedVoice->hide();
		uiContextToolBar->addAction( uiRemoveContext );
		uiContextToolBar->addAction( uiContextProperties );
		addToolBar(Qt::TopToolBarArea, uiContextToolBar);
	
	uiVoiceToolBar = new QToolBar( tr("Voice ToolBar"), this );
		uiVoiceToolBar->addAction( uiNewVoice );
		uiVoiceToolBar->addWidget( uiVoiceNum = new CALCDNumber( 0, 20, 0, "Voice number" ) );
			uiVoiceNum->setToolTip(tr("Current Voice number"));
			connect( uiVoiceNum, SIGNAL( valChanged( int ) ), this, SLOT(on_uiVoiceNum_valChanged( int ) ) );
		uiVoiceToolBar->addWidget( uiVoiceName = new QLineEdit( this ) );
			uiVoiceName->setToolTip(tr("Voice name"));
			connect( uiVoiceName, SIGNAL(returnPressed()), this, SLOT(on_uiVoiceName_returnPressed()) );			
		uiVoiceToolBar->addAction( uiRemoveVoice );
		uiVoiceToolBar->addWidget(uiVoiceStemDirection = new CAMenuToolButton( tr("Select Voice Stem Direction" ), 3, this ));
			connect( uiVoiceStemDirection, SIGNAL(toggled(bool, int)), this, SLOT(on_uiVoiceStemDirection_toggled(bool, int)) );
			uiVoiceStemDirection->setToolTip(tr("Voice stem direction"));
			uiVoiceStemDirection->addButton( QIcon("images/notestemneutral.svg"), CANote::StemNeutral, tr("Voice Stems Neutral") );
			uiVoiceStemDirection->addButton( QIcon("images/notestemup.svg"), CANote::StemUp, tr("Voice Stems Up") );
			uiVoiceStemDirection->addButton( QIcon("images/notestemdown.svg"), CANote::StemDown, tr("Voice Stems Down") );
			uiVoiceStemDirection->defaultAction()->setCheckable(false);
		uiVoiceToolBar->addAction( uiVoiceProperties );
		addToolBar(Qt::TopToolBarArea, uiVoiceToolBar);
		
	uiPlayableToolBar = new QToolBar( tr("Playable ToolBar"), this );
		uiPlayableToolBar->addWidget(uiPlayableLength = new CAMenuToolButton( tr("Select Length" ), 4, this ));
			connect( uiPlayableLength, SIGNAL(toggled(bool, int)), this, SLOT(on_uiPlayableLength_toggled(bool, int)) );
			uiPlayableLength->setToolTip(tr("Playable length"));
			uiPlayableLength->addButton( QIcon("images/n0.svg"), CANote::Breve, tr("Breve Length") );
			uiPlayableLength->addButton( QIcon("images/n1.svg"), CANote::Whole, tr("Whole Length") );
			uiPlayableLength->addButton( QIcon("images/n2.svg"), CANote::Half, tr("Half Length") );
			uiPlayableLength->addButton( QIcon("images/n4.svg"), CANote::Quarter, tr("Quarter Length") );
			uiPlayableLength->addButton( QIcon("images/n8.svg"), CANote::Eighth, tr("Eighth Length") );
			uiPlayableLength->addButton( QIcon("images/n16.svg"), CANote::Sixteenth, tr("Sixteenth Length") );
			uiPlayableLength->addButton( QIcon("images/n32.svg"), CANote::ThirtySecond, tr("ThirtySecond Length") );
			uiPlayableLength->addButton( QIcon("images/n64.svg"), CANote::SixtyFourth, tr("SixtyFourth Length") );
			uiPlayableLength->defaultAction()->setCheckable(false);
			uiPlayableLength->setCurrentId( CANote::Quarter );
		uiPlayableToolBar->addAction( uiAccsVisible );
		uiPlayableToolBar->addWidget(uiNoteStemDirection = new CAMenuToolButton( tr("Select Note Stem Direction" ), 4, this ));
			connect( uiNoteStemDirection, SIGNAL(toggled(bool, int)), this, SLOT(on_uiNoteStemDirection_toggled(bool, int)) );
			uiNoteStemDirection->setToolTip(tr("Note stem direction"));		
			uiNoteStemDirection->addButton( QIcon("images/notestemneutral.svg"), CANote::StemNeutral, tr("Note Stem Neutral") );
			uiNoteStemDirection->addButton( QIcon("images/notestemup.svg"), CANote::StemUp, tr("Note Stem Up") );
			uiNoteStemDirection->addButton( QIcon("images/notestemdown.svg"), CANote::StemDown, tr("Note Stem Down") );
			uiNoteStemDirection->addButton( QIcon("images/notestemvoice.svg"), CANote::StemPreferred, tr("Note Stem Preferred") );
			uiNoteStemDirection->defaultAction()->setCheckable(false);
			uiNoteStemDirection->setCurrentId( CANote::StemPreferred );
		uiPlayableToolBar->addAction( uiHiddenRest );
		addToolBar(Qt::TopToolBarArea, uiPlayableToolBar);
	
	uiKeySigToolBar = new QToolBar( tr("Key Signature ToolBar"), this );
		uiKeySigToolBar->addWidget( uiKeySigNumberOfAccs = new QSpinBox(this) );
		uiKeySigNumberOfAccs->setToolTip( tr("Number of Accidentals") );
		uiKeySigNumberOfAccs->setRange( -7, 7 );
		uiKeySigNumberOfAccs->setValue( 0 );
		connect( uiKeySigNumberOfAccs, SIGNAL(valueChanged(int)), this, SLOT(on_uiKeySigNumberOfAccs_valChanged(int)) );
		addToolBar(Qt::TopToolBarArea, uiKeySigToolBar);
	
	uiTimeSigToolBar = new QToolBar( tr("Time Signature ToolBar"), this );
		uiTimeSigToolBar->addWidget( uiTimeSigBeats = new QSpinBox(this) );
		uiTimeSigBeats->setMinimum( 1 );
		uiTimeSigBeats->setValue( 4 );
		uiTimeSigBeats->setToolTip( tr("Number of beats") );
		connect( uiTimeSigBeats, SIGNAL(valueChanged(int)), this, SLOT(on_uiTimeSigBeats_valChanged(int)) );
		uiTimeSigToolBar->addWidget( uiTimeSigSlash = new QLabel( "/", this ) );
		uiTimeSigToolBar->addWidget( uiTimeSigBeat = new QSpinBox(this) );
		uiTimeSigBeat->setMinimum( 1 );
		uiTimeSigBeat->setValue( 4 );
		uiTimeSigBeat->setToolTip( tr("Beat") );
		connect( uiTimeSigBeat, SIGNAL(valueChanged(int)), this, SLOT(on_uiTimeSigBeat_valChanged(int)) );
		addToolBar(Qt::TopToolBarArea, uiTimeSigToolBar);
	
	uiFMToolBar = new QToolBar( tr("Function marking ToolBar"), this );
		uiFMToolBar->addWidget( uiFMFunction = new CAMenuToolButton( tr("Select Function Name"), 8, this ) );
			connect( uiFMFunction, SIGNAL(toggled(bool, int)), this, SLOT(on_uiFMFunction_toggled(bool, int)) );
			uiFMFunction->addButton( QIcon("images/fmt.svg"), CAFunctionMarking::T, tr("Tonic") );
			uiFMFunction->addButton( QIcon("images/fms.svg"), CAFunctionMarking::S, tr("Subdominant") );
			uiFMFunction->addButton( QIcon("images/fmd.svg"), CAFunctionMarking::D, tr("Dominant") );
			uiFMFunction->addButton( QIcon("images/fmii.svg"), CAFunctionMarking::II, tr("II") );
			uiFMFunction->addButton( QIcon("images/fmiii.svg"), CAFunctionMarking::III, tr("III") );
			uiFMFunction->addButton( QIcon("images/fmvi.svg"), CAFunctionMarking::VI, tr("VI") );
			uiFMFunction->addButton( QIcon("images/fmvii.svg"), CAFunctionMarking::VII, tr("VII") );
			uiFMFunction->addButton( QIcon("images/fmk.svg"), CAFunctionMarking::K, tr("Cadenze") );
			uiFMFunction->addButton( QIcon("images/fmot.svg"), CAFunctionMarking::T*(-1), tr("minor Tonic") );
			uiFMFunction->addButton( QIcon("images/fmos.svg"), CAFunctionMarking::S*(-1), tr("minor Subdominant") );
			uiFMFunction->addButton( QIcon("images/fmn.svg"), CAFunctionMarking::N, tr("Napolitan") );
			uiFMFunction->addButton( QIcon("images/fmf.svg"), CAFunctionMarking::F, tr("Phrygian") );
			uiFMFunction->addButton( QIcon("images/fml.svg"), CAFunctionMarking::L, tr("Lydian") );
			uiFMFunction->addButton( QIcon("images/fmiv.svg"), CAFunctionMarking::IV, tr("IV") );
			uiFMFunction->addButton( QIcon("images/fmv.svg"), CAFunctionMarking::V, tr("V") );
			uiFMFunction->setCurrentId( CAFunctionMarking::T );
		uiFMToolBar->addWidget( uiFMChordArea = new CAMenuToolButton( tr("Select Chord Area"), 3, this ) );
			connect( uiFMChordArea, SIGNAL(toggled(bool, int)), this, SLOT(on_uiFMChordArea_toggled(bool, int)) );
			uiFMChordArea->addButton( QIcon("images/fmpt.svg"), CAFunctionMarking::T, tr("Tonic") );
			uiFMChordArea->addButton( QIcon("images/fmps.svg"), CAFunctionMarking::S, tr("Subdominant") );
			uiFMChordArea->addButton( QIcon("images/fmpd.svg"), CAFunctionMarking::D, tr("Dominant") );
			uiFMChordArea->addButton( QIcon("images/fmpot.svg"), CAFunctionMarking::T*(-1), tr("minor Tonic") );
			uiFMChordArea->addButton( QIcon("images/fmpos.svg"), CAFunctionMarking::S*(-1), tr("minor Subdominant") );
			uiFMChordArea->setCurrentId( CAFunctionMarking::T );
		uiFMToolBar->addWidget( uiFMTonicDegree = new CAMenuToolButton( tr("Select Tonic Degree"), 7, this ) );
			uiFMTonicDegree->defaultAction()->setCheckable( false );
			connect( uiFMTonicDegree, SIGNAL(toggled(bool, int)), this, SLOT(on_uiFMTonicDegree_toggled(bool, int)) );
			uiFMTonicDegree->addButton( QIcon("images/fmt.svg"), CAFunctionMarking::T, tr("Tonic") );
			uiFMTonicDegree->addButton( QIcon("images/fmot.svg"), CAFunctionMarking::T*(-1), tr("minor Tonic") );
			uiFMTonicDegree->addButton( QIcon("images/fmii.svg"), CAFunctionMarking::II, tr("II") );
			uiFMTonicDegree->addButton( QIcon("images/fmiii.svg"), CAFunctionMarking::III, tr("III") );
			uiFMTonicDegree->addButton( QIcon("images/fms.svg"), CAFunctionMarking::S, tr("Subdominant") );
			uiFMTonicDegree->addButton( QIcon("images/fmos.svg"), CAFunctionMarking::S*(-1), tr("minor Subdominant") );
			uiFMTonicDegree->addButton( QIcon("images/fmd.svg"), CAFunctionMarking::D, tr("Dominant") );
			uiFMTonicDegree->addButton( QIcon("images/fmvi.svg"), CAFunctionMarking::VI, tr("VI") );
			uiFMTonicDegree->addButton( QIcon("images/fmvii.svg"), CAFunctionMarking::VII, tr("VII") );
			uiFMTonicDegree->setCurrentId( CAFunctionMarking::T );
		uiFMToolBar->addAction( uiFMEllipse );
		addToolBar(Qt::TopToolBarArea, uiFMToolBar);
	
	// Mutual exclusive groups
	uiInsertGroup = new QActionGroup( this );
	uiInsertGroup->addAction( uiSelectMode );
	uiInsertGroup->addAction( uiNewContext );
	uiInsertGroup->addAction( uiContextType->defaultAction() );
	uiInsertGroup->addAction( uiInsertPlayable );
	uiInsertGroup->addAction( uiSlurType->defaultAction() );
	uiInsertGroup->addAction( uiInsertClef );
	uiInsertGroup->addAction( uiClefType->defaultAction() );
	uiInsertGroup->addAction( uiInsertTimeSig );
	uiInsertGroup->addAction( uiTimeSigType->defaultAction() );
	uiInsertGroup->addAction( uiInsertKeySig );
	uiInsertGroup->addAction( uiInsertBarline );
	uiInsertGroup->addAction( uiBarlineType->defaultAction() );
	uiInsertGroup->addAction( uiInsertSyllable );
	uiInsertGroup->addAction( uiInsertFM );
	uiInsertGroup->setExclusive( true );
	
	uiInsertToolBar->hide();
	uiSheetToolBar->hide();
	uiContextToolBar->hide();
	uiPlayableToolBar->hide();
	uiTimeSigToolBar->hide();
	uiKeySigToolBar->hide();
	uiFMToolBar->hide();
}

void CAMainWin::newDocument() {
	// clear GUI before clearing the data part!
	clearUI();
	
	// clear the data part
	if ( document() && (CACanorus::mainWinCount(document()) == 1) ) {
		CACanorus::deleteUndoStack( document() ); 
		delete document();
	}
	
	setDocument(new CADocument());
	CACanorus::setUndoStack( document(), new QUndoStack() );
	restartTimeEditedTime();
	
#ifdef USE_PYTHON
	QList<PyObject*> argsPython;
	argsPython << CASwigPython::toPythonObject(document(), CASwigPython::Document);
	CASwigPython::callFunction(CACanorus::locateResource("scripts/newdocument.py").at(0), "newDefaultDocument", argsPython);
#endif
	
	// call local rebuild only because no other main windows share the new document
	rebuildUI();
	updateToolBars();
}

/*!
	This adds a tab to tabWidget and creates a single score viewport of the sheet.
	It does not add the sheet to the document.
*/
void CAMainWin::addSheet(CASheet *s) {
	CAScoreViewPort *v = new CAScoreViewPort(s, 0);
	initViewPort( v );
	
	CAViewPortContainer *vpc = new CAViewPortContainer( 0 );
	vpc->addViewPort( v );
	_viewPortContainerList << vpc;
	_sheetMap[vpc] = s;
	
	uiTabWidget->addTab(vpc, s->name());
	uiTabWidget->setCurrentIndex(uiTabWidget->count()-1);
	setCurrentViewPortContainer( vpc );
	
	updateToolBars();
}

/*!
	Deletes all viewports (and their drawable content), disconnects all signals and resets all
	buttons and modes.
	
	This function deletes the current main window's GUI only (drawable elements). All the data
	classes (staffs, notes, rests) should stay intact. Use delete document() to free the data
	part of Canorus as well.
*/
void CAMainWin::clearUI() {
	if(_viewPortList.size() > 1)
		on_uiUnsplitAll_triggered();
	
	// Delete all view port containers
	while (uiTabWidget->count()) {
		CAViewPortContainer *vpc = static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget());
		uiTabWidget->removeTab( 0 );
		delete vpc;
	}
	
	// Delete all viewports
	while(!_viewPortList.isEmpty())
		delete _viewPortList.takeFirst();
	_viewPortList.clear();
	_sheetMap.clear();
	setCurrentViewPort( 0 );
	uiSelectMode->trigger(); // select mode
}

/*!
	Called when the current sheet is switched in the tab widget.
	\warning This method is only called when the index of the selected tab changes. If you remove the current tab and the next selected tab gets the same index, this slot isn't called!
*/
void CAMainWin::on_uiTabWidget_currentChanged(int idx) {
	setCurrentViewPortContainer( static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget()) );
	if (currentViewPortContainer())
		setCurrentViewPort( currentViewPortContainer()->currentViewPort() );
	
	updateToolBars();
}

void CAMainWin::on_uiFullscreen_toggled(bool checked) {
	if (checked)
		this->showFullScreen();
	else
		this->showNormal();
}

void CAMainWin::on_uiHiddenRest_toggled( bool checked ) {
	if ( mode()==InsertMode )
		_musElementFactory->setRestType( checked ? CARest::Hidden : CARest::Normal );
	else if (mode()==SelectMode || mode()==EditMode) {
		CAScoreViewPort *v = currentScoreViewPort();
		if ( v && v->selection().size() ) {
			CARest *rest = dynamic_cast<CARest*>(v->selection().at(0)->musElement());
			if ( rest ) {
				rest->setRestType( checked ? CARest::Hidden : CARest::Normal );
				CACanorus::rebuildUI( document(), currentSheet() );
			}
		}
	}}

void CAMainWin::on_uiSplitHorizontally_triggered() {
	CAViewPort *v = currentViewPortContainer()->splitHorizontally();
	if(!v)
		return;
	
	initViewPort( v );
	
	uiUnsplitAll->setEnabled(true);
	uiCloseCurrentView->setEnabled(true);
}

void CAMainWin::on_uiSplitVertically_triggered() {
	CAViewPort *v = currentViewPortContainer()->splitVertically();
	if(!v)
		return;
	
	initViewPort( v );
	
	uiUnsplitAll->setEnabled(true);
	uiCloseCurrentView->setEnabled(true);
}

void CAMainWin::doUnsplit(CAViewPort *v) {
	v = currentViewPortContainer()->unsplit(v);
	if (!v) return;
	
	if (currentViewPortContainer()->viewPortList().size() == 1)
	{
		uiCloseCurrentView->setEnabled(false);
		uiUnsplitAll->setEnabled(false);
	}
	setCurrentViewPort( currentViewPortContainer()->currentViewPort() );
}

void CAMainWin::on_uiUnsplitAll_triggered() {
	currentViewPortContainer()->unsplitAll();
	uiCloseCurrentView->setEnabled(false);
	uiUnsplitAll->setEnabled(false);
	setCurrentViewPort( currentViewPortContainer()->currentViewPort() );
}

void CAMainWin::on_uiCloseCurrentView_triggered() {
	if (currentViewPortContainer()->contains( currentViewPort() )) {
		doUnsplit();
	} else
		delete currentViewPort();
}

void CAMainWin::on_uiCloseDocument_triggered() {
	if ( CACanorus::mainWinCount(document()) == 1 ) {
			CACanorus::deleteUndoStack( document() ); 
			delete document();
	}
	setDocument( 0 );
	rebuildUI();
}

/*!
	Shows the current score in CanorusML syntax in a new or the current viewport.
*/
void CAMainWin::on_uiCanorusMLSource_triggered() {
	CASourceViewPort *v = new CASourceViewPort(document(), static_cast<QWidget*>(currentViewPort()->parent()));
	initViewPort( v );
	currentViewPortContainer()->addViewPort( v );
	
	uiUnsplitAll->setEnabled(true);
	uiCloseCurrentView->setEnabled(true);
}

void CAMainWin::on_uiNewViewport_triggered() {
	CAViewPort *v = currentViewPort()->clone( 0 );
	initViewPort( v );
	v->show();
	v->setGeometry(v->x(), v->y(), CAViewPort::DEFAULT_VIEWPORT_WIDTH, CAViewPort::DEFAULT_VIEWPORT_HEIGHT);
}

/*!
	Links the newly created viewport with the main window:
		- Adds the viewport to the viewport list
		- Connects its signals to main windows' slots.
		- Sets the icon, focus policy and sets the focus.
		- Sets the currentViewPort but not currentViewPortContainer
*/
void CAMainWin::initViewPort(CAViewPort *v) {
	_viewPortList << v;
	QList<QString> paths = CACanorus::locateResource("images/clogosm.png");
	if ( !paths.size() )
		paths = CACanorus::locateResource("ui/images/clogosm.png");
	
	if ( paths.size() )
		v->setWindowIcon(QIcon( paths[0] ));
	
	switch (v->viewPortType()) {
		case CAViewPort::ScoreViewPort: {
			connect( v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAScoreViewPort *)),
			         this, SLOT(scoreViewPortMousePress(QMouseEvent *, QPoint, CAScoreViewPort *)) );
			connect( v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAScoreViewPort *)),
			         this, SLOT(scoreViewPortMouseMove(QMouseEvent *, QPoint, CAScoreViewPort *)) );
			connect( v, SIGNAL(CAMouseReleaseEvent(QMouseEvent *, QPoint, CAScoreViewPort *)),
			         this, SLOT(scoreViewPortMouseRelease(QMouseEvent *, QPoint, CAScoreViewPort *)) );
			connect( v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAScoreViewPort *)),
			         this, SLOT(scoreViewPortWheel(QWheelEvent *, QPoint, CAScoreViewPort *)) );
			connect( v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAScoreViewPort *)),
			         this, SLOT(scoreViewPortKeyPress(QKeyEvent *, CAScoreViewPort *)) );
			connect( static_cast<CAScoreViewPort*>(v)->syllableEdit(), SIGNAL(CAKeyPressEvent(QKeyEvent*, CASyllableEdit*)),
			         this, SLOT(on_syllableEdit_keyPressEvent(QKeyEvent*, CASyllableEdit*)) );
			break;
		}
		case CAViewPort::SourceViewPort: {
			connect(v, SIGNAL(CACommit(QString, CASourceViewPort*)), this, SLOT(sourceViewPortCommit(QString, CASourceViewPort*)));
		}
	}
	
	v->setFocusPolicy(Qt::ClickFocus);
	v->setFocus();
	setCurrentViewPort(v);
	setMode(mode());	// updates the new viewport border settings	
}

/*!
	Returns the currently selected context in the current view port or 0 if no contexts are selected.
*/
CAContext *CAMainWin::currentContext() {
	if ( currentViewPort() &&
	     (currentViewPort()->viewPortType() == CAViewPort::ScoreViewPort) &&
	     (static_cast<CAScoreViewPort*>(currentViewPort())->currentContext())
	   ) {
		return static_cast<CAScoreViewPort*>(currentViewPort())->currentContext()->context();
	} else
		return 0;
}

/*!
	Returns the pointer to the currently active voice or 0, if All voices are selected or the current context is not a staff at all.
*/
CAVoice *CAMainWin::currentVoice() {
	CAStaff *staff = currentStaff();
	if (staff) {
		if ( uiVoiceNum->getRealValue() &&
		     uiVoiceNum->getRealValue() <= staff->voiceCount())
			return staff->voiceAt( uiVoiceNum->getRealValue() - 1); 
	}
	
	return 0;
}

/*!
	Creates a new main window sharing the current document.
*/
void CAMainWin::on_uiNewWindow_triggered() {
	CAMainWin *newMainWin = new CAMainWin();
	newMainWin->setDocument(document());
	newMainWin->rebuildUI();
	CACanorus::addMainWin(newMainWin);
}

void CAMainWin::on_uiNewDocument_triggered() {
	newDocument();
}

void CAMainWin::on_uiUndo_triggered() {
	if ( document() && CACanorus::undoStack( document() )->canUndo() ) {
		CACanorus::undoStack( document() )->undo();
		CACanorus::rebuildUI( document(), 0 );
		on_uiVoiceNum_valChanged( uiVoiceNum->getRealValue() ); // updates current voice in score viewport
	}
}

void CAMainWin::on_uiRedo_triggered() {
	if ( document() && CACanorus::undoStack( document() )->canRedo() ) {
		CACanorus::undoStack( document() )->redo();
		CACanorus::rebuildUI( document(), 0 );
		on_uiVoiceNum_valChanged( uiVoiceNum->getRealValue() ); // updates current voice in score viewport
	}
}

/*!
	Enables or Disabled undo/redo buttons if there are undo/redo commands on the undo stack.
	
	\sa CACanorus::undoStack()
*/
void CAMainWin::updateUndoRedoButtons() {
	if ( CACanorus::undoStack( document() ) && CACanorus::undoStack( document() )->canUndo() )
		uiUndo->setEnabled(true);
	else
		uiUndo->setEnabled(false);
	
	if ( CACanorus::undoStack( document() ) && CACanorus::undoStack( document() )->canRedo() )
		uiRedo->setEnabled(true);
	else
		uiRedo->setEnabled(false);
}

/*!
	Adds a new empty sheet.
*/
void CAMainWin::on_uiNewSheet_triggered() {
	CACanorus::createUndoCommand( document(), tr("new sheet", "undo") );
	document()->addSheet(tr("Sheet%1").arg(QString::number(document()->sheetCount()+1)));
	CACanorus::pushUndoCommand();
	CACanorus::rebuildUI(document());
}

/*!
	Adds a new voice to the staff.
*/
void CAMainWin::on_uiNewVoice_triggered() {
	CAStaff *staff = currentStaff();
	int voiceNumber = staff->voiceCount()+1;
	CANote::CAStemDirection stemDirection;
	if ( voiceNumber == 1 )
		stemDirection = CANote::StemNeutral;
	else {
		staff->voiceAt(0)->setStemDirection( CANote::StemUp );
		stemDirection = CANote::StemDown;
	}
	
	CACanorus::createUndoCommand( document(), tr("new voice", "undo") );
	if (staff)
		staff->addVoice(new CAVoice(staff, staff->name() + tr("Voice%1").arg( staff->voiceCount()+1 ), voiceNumber, stemDirection));
	
	CACanorus::pushUndoCommand();
	CACanorus::rebuildUI(document(), currentSheet());
	uiVoiceNum->setRealValue( staff->voiceCount() );
}

/*!
	Removes the current voice from the staff and deletes its contents.
*/
void CAMainWin::on_uiRemoveVoice_triggered() {
	CAVoice *voice = currentVoice();
	if (voice) {
		// Last voice cannot be deleted
		if (voice->staff()->voiceCount()==1) {
			int ret = QMessageBox::critical(
				this, tr("Canorus"),
				tr("Cannot delete the last voice in the staff!")
			);
			return;
		}
		
		int ret = QMessageBox::warning(
			this, tr("Canorus"),
			tr("Are you sure do you want to delete voice\n%1 and all its notes?").arg(voice->name()),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
		
		if (ret == QMessageBox::Yes) {
			CACanorus::createUndoCommand( document(), tr("voice removal", "undo") );
			currentScoreViewPort()->clearSelection();
			uiVoiceNum->setRealValue( voice->staff()->voiceCount()-1 );
			delete voice; // also removes voice from the staff
			CACanorus::pushUndoCommand();
			CACanorus::rebuildUI(document(), currentSheet());
		}
	}
}

/*!
	Removes the current context from the sheet and all its contents.
*/
void CAMainWin::on_uiRemoveContext_triggered() {
	CAContext *context = currentContext();
	if (context) {
		int ret = QMessageBox::warning(
			this, tr("Canorus"),
			tr("Are you sure do you want to delete context\n%1 and all its contents?").arg(context->name()),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
		
		if (ret == QMessageBox::Yes) {
			CACanorus::createUndoCommand( document(), tr("context removal", "undo") );
			CASheet *sheet = context->sheet();
			sheet->removeContext(context);
			CACanorus::pushUndoCommand();			
			CACanorus::rebuildUI(document(), currentSheet());
			delete context;
		}
	}
}

void CAMainWin::on_uiContextType_toggled(bool checked, int buttonId) {
	if (checked)
		setMode(InsertMode);
}

void CAMainWin::on_uiSelectMode_toggled(bool checked) {
	if (checked)
		setMode(SelectMode);
}

/*!
	Sets the current mode and updates the GUI and toolbars.
*/
void CAMainWin::setMode(CAMode mode) {
	_mode = mode;
	
	switch (mode) {
		case SelectMode: {
			for (int i=0; i<_viewPortList.size(); i++) {
				if (_viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort) {
					if (!((CAScoreViewPort*)_viewPortList[i])->playing())
						((CAScoreViewPort*)_viewPortList[i])->unsetBorder();
					((CAScoreViewPort*)_viewPortList[i])->setShadowNoteVisible(false);
					statusBar()->showMessage("");
					_musElementFactory->setMusElementType( CAMusElement::Undefined );
					uiVoiceNum->setRealValue( 0 );
					_viewPortList[i]->repaint();
				}
			}
			break;
		}
		case InsertMode: {
			QPen p;
			p.setColor(Qt::blue);
			p.setWidth(3);
			
			if ( _musElementFactory->musElementType() == CAMusElement::Note && currentScoreViewPort() )
				currentScoreViewPort()->setShadowNoteVisible(true);

			for (int i=0; i<_viewPortList.size(); i++) {
				if (_viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort) {
					if (!((CAScoreViewPort*)_viewPortList[i])->playing())
						((CAScoreViewPort*)_viewPortList[i])->setBorder(p);
					((CAScoreViewPort*)_viewPortList[i])->repaint();
				}
			}

			break;
		}
		case EditMode: {
			QPen p;
			p.setColor(Qt::red);
			p.setWidth(3);
			for (int i=0; i<_viewPortList.size(); i++) {
				if (_viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort) {
					if (!((CAScoreViewPort*)_viewPortList[i])->playing())
						((CAScoreViewPort*)_viewPortList[i])->setBorder(p);
					
					if (currentScoreViewPort()->selection().size() &&
					    currentScoreViewPort()->selection().front()->drawableMusElementType()==CADrawableMusElement::DrawableSyllable
					   ) {
						currentScoreViewPort()->createSyllableEdit(currentScoreViewPort()->selection().front());
					} else {
						currentScoreViewPort()->removeSyllableEdit();
					}
					
					_viewPortList[i]->repaint();
				}
			}
		}
	}	// switch (mode)
	updateToolBars();
	if ( currentScoreViewPort() && !currentScoreViewPort()->syllableEditVisible() ||
	     !currentScoreViewPort() && currentViewPort() )
		currentViewPort()->setFocus();
}

/*!
	Rebuilds the GUI from data.
	
	This method is called eg. when multiple viewports share the same data and a change has been made (eg. a
	note pitch has changed or a new element added). ViewPorts content is repositioned and redrawn (CAEngraver
	creates CADrawable elements for every score viewport, sources are updated in source viewports etc.).
	
	\a sheet argument is a pointer to the data sheet where the change occured. This way only viewports showing
	the given sheet are updated which speeds up the process.
	If \a sheet argument is null, all viewports are rebuilt, but the viewports contents, number and locations
	remain the same.
	
	If \a repaint is True (default) the rebuilt viewports are also repainted. If False, viewports content is
	only created but not yet drawn. This is useful when multiple operations which could potentially change the
	content are to happen and we want to actually draw it only at the end.
*/
void CAMainWin::rebuildUI(CASheet *sheet, bool repaint) {
	if (rebuildUILock()) return;
	
	setRebuildUILock( true );
	if (document()) {
		for (int i=0; i<_viewPortList.size(); i++) {
			if (sheet && _viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort &&
			    static_cast<CAScoreViewPort*>(_viewPortList[i])->sheet()!=sheet)
				continue;
			
			_viewPortList[i]->rebuild();
			
			if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
				static_cast<CAScoreViewPort*>(_viewPortList[i])->checkScrollBars();
				
			if (repaint)
				_viewPortList[i]->repaint();
		}
	} else {
		clearUI();
	}
	updateToolBars();
	setRebuildUILock( false );
}

/*!
	Rebuilds the GUI from data.
	
	This method is called eg. when multiple viewports share the same data and a change has been made (eg. a
	note pitch has changed or a new element added). ViewPorts content is repositioned and redrawn (CAEngraver
	creates CADrawable elements for every score viewport, sources are updated in source viewports etc.).
	
	This method in comparison to CAMainWin::rebuildUI(CASheet *s, bool repaint) rebuilds the whole GUI from
	scratch and creates new viewports for the sheets. This method is called for example when a new document
	is created or opened.
	
	If \a repaint is True (default) the rebuilt viewports are also repainted. If False, viewports content is
	only created but not yet drawn. This is useful when multiple operations which could potentially change the
	content are to happen and we want to actually draw it only at the end.
*/
void CAMainWin::rebuildUI(bool repaint) {
	if (rebuildUILock()) return;
	
	setRebuildUILock( true );
	if (document()) {
		// save the current state of viewports
		QList<QRect> worldCoordsList;
		for (int i=0; i<_viewPortList.size(); i++)
			if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
				worldCoordsList << static_cast<CAScoreViewPort*>(_viewPortList[i])->worldCoords();
		 
		clearUI();
		for (int i=0; i<document()->sheetCount(); i++) {
			addSheet(document()->sheetAt(i));
			
			// restore the current state of viewports
			if ( _viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort &&
			     i < worldCoordsList.size() )
				static_cast<CAScoreViewPort*>(_viewPortList[i])->setWorldCoords(worldCoordsList[i]);
		}
		
		for (int i=0; i<_viewPortList.size(); i++) {
			_viewPortList[i]->rebuild();
			
			if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
				static_cast<CAScoreViewPort*>(_viewPortList[i])->checkScrollBars();
				
			if (repaint)
				_viewPortList[i]->repaint();
		}
	} else {
		clearUI();
	}
	updateToolBars();
	setRebuildUILock( false );
}

/*!
	Processes the mouse press event \a e with world coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mousePressEvent(), scoreViewPortMouseMove(), scoreViewPortWheel(), scoreViewPortKeyPress()
*/
void CAMainWin::scoreViewPortMousePress(QMouseEvent *e, const QPoint coords, CAScoreViewPort *v) {
	setCurrentViewPort( v );
	currentViewPortContainer()->setCurrentViewPort( currentViewPort() );
	
	CADrawableContext *prevContext = v->currentContext();
	v->selectCElement(coords.x(), coords.y());
		
	QList<CADrawableMusElement*> l = v->musElementsAt( coords.x(), coords.y() );
	int idx=-1;
	
	if (l.size() > 0) { // multiple elements can share the same coordinates
		if ( (v->selection().size() > 0) && (!v->selection().contains(l.front())) ) {
			if (e->modifiers()!=Qt::ShiftModifier)
				v->clearSelection();
			v->addToSelection(l[0]);      // if the previous selection was not a single element or if the new list doesn't contain the selection set the first element in the available list to the selection
		} else {
			if (e->modifiers()==Qt::ShiftModifier && v->selection().size()) {
				v->removeFromSelection(l[0]); // shift used on an already selected element - toggle selection
			} else {
				idx = (v->selection().size()?l.indexOf(v->selection().front()):-1);
				v->clearSelection();
				v->addToSelection(l[((++idx < l.size()) ? idx : 0)]); // if there are two or more elements with the same coordinates, select the next one (behind it). This way, you can click multiple times on the same place and you'll always select the other element.
			}
		}
	} else if (e->modifiers()==Qt::NoModifier) { // no elements at that coordinates
		v->clearSelection();
	}
	
	// voice number widget
	if ( v->currentContext() && prevContext != v->currentContext() && mode()!=InsertMode) {	// new context was selected
		if (v->currentContext()->context()->contextType() == CAContext::Staff) {
			uiVoiceNum->setRealValue(0);
			uiVoiceNum->setMax(static_cast<CAStaff*>(v->currentContext()->context())->voiceCount());
		}
	} else if ( prevContext != v->currentContext() && uiInsertPlayable->isChecked() ) { // but insert playable mode is active and context should remain the same
		v->setCurrentContext( prevContext );
	}
	
	switch ( mode() ) {
		case SelectMode:
		case EditMode: {
			v->clearSelectionRegionList();
			
			if (v->selection().size()) {
				CADrawableMusElement *dElt = v->selection().front();
				CAMusElement *elt = dElt->musElement();
				if (!elt) break;
				
				// debug
				std::cout << "drawableMusElement: " << dElt << ", x,y=" << dElt->xPos() << "," << dElt->yPos() << ", w,h=" << dElt->width() << "," << dElt->height() << ", dContext=" << dElt->drawableContext() << std::endl;
				std::cout << "musElement: " << elt << ", timeStart=" << elt->timeStart() << ", timeEnd=" << elt->timeEnd() << ", dContext = " << v->selection().front()->drawableContext() << ", context=" << elt->context();
				if (elt->isPlayable()) {
					std::cout << ", voice=" << ((CAPlayable*)elt)->voice() << ", voiceNr=" << ((CAPlayable*)elt)->voice()->voiceNumber() << ", idxInVoice=" << ((CAPlayable*)elt)->voice()->indexOf(elt);
					std::cout << ", voiceStaff=" << ((CAPlayable*)elt)->voice()->staff();
					if (elt->musElementType()==CAMusElement::Note)
						std::cout << ", pitch=" << ((CANote*)elt)->pitch();
				}
				std::cout << std::endl;
			}
			
			break;
		}
		case InsertMode: {
			// Insert context
			if (uiContextType->isChecked()) {
				// Add new Context
				CAContext *newContext;
				CADrawableContext *dupContext = v->nearestUpContext(coords.x(), coords.y());
				switch(uiContextType->currentId()) {
					case CAContext::Staff: {
						CACanorus::createUndoCommand( document(), tr("new staff", "undo"));
						v->sheet()->insertContextAfter(
							dupContext?dupContext->context():0,
							newContext = new CAStaff(
								v->sheet(),
								tr("Staff%1").arg(v->sheet()->staffCount()+1)
							)
						);
						static_cast<CAStaff*>(newContext)->addVoice(new CAVoice(static_cast<CAStaff*>(newContext), newContext->name() + tr("Voice%1").arg(1), 1, CANote::StemNeutral));
						break;
					}
					case CAContext::LyricsContext: {
						CACanorus::createUndoCommand( document(), tr("new lyrics context", "undo"));
						v->sheet()->insertContextAfter(
							dupContext?dupContext->context():0,
							newContext = new CALyricsContext(
								1,
								(v->sheet()->voiceList().size()?v->sheet()->voiceList().at(0):0),
								v->sheet(),
								tr("LyricsContext%1").arg(v->sheet()->contextCount()+1)
							)
						);
						break;
					}
					case CAContext::FunctionMarkingContext: {
						CACanorus::createUndoCommand( document(), tr("new function marking context", "undo"));
						v->sheet()->insertContextAfter(
							dupContext?dupContext->context():0,
							newContext = new CAFunctionMarkingContext(
								v->sheet(),
								tr("FunctionMarkingContext%1").arg(v->sheet()->contextCount()+1)
							)
						);
						break;
					}
				}
				CACanorus::pushUndoCommand();
				CACanorus::rebuildUI(document(), v->sheet());
				
				v->selectContext(newContext);
				if (newContext->contextType()==CAContext::Staff) {
					uiVoiceNum->setMax( 1 );
					uiVoiceNum->setRealValue( 0 );
				}
				uiSelectMode->toggle();
				v->repaint();
				break;
			} else
			// Insert Syllable
			if (uiInsertSyllable->isChecked()) {
				if ( v->currentContext()->context()->contextType()==CAContext::LyricsContext && v->selection().size() && v->selection().front()->drawableMusElementType()==CADrawableMusElement::DrawableSyllable ) {
					int timeStart = 0, timeLength = 256;
					CADrawableLyricsContext *dlc = static_cast<CADrawableLyricsContext*>(v->currentContext());
					v->createSyllableEdit( v->selection().front() );
				} else {
					v->removeSyllableEdit();
				}
				
				break;
			} else
			// Insert music element
			if (uiInsertPlayable->isChecked()) {
				// Add Note/Rest
				if (e->button()==Qt::RightButton && _musElementFactory->musElementType()==CAMusElement::Note)
					// place a rest when using right mouse button and note insertion is selected
					_musElementFactory->setMusElementType( CAMusElement::Rest );
			}
			
			insertMusElementAt( coords, v );
			
			if (_musElementFactory->musElementType()==CAMusElement::Rest)
				_musElementFactory->setMusElementType( CAMusElement::Note );
			
			break;
		}
	}
	
	CAPluginManager::action("onScoreViewPortClick", document(), 0, 0, this);
	
	updateToolBars();
	v->repaint();
}

/*!
	Processes the mouse move event \a e with coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mouseMoveEvent(), scoreViewPortMousePress(), scoreViewPortWheel(), scoreViewPortKeyPress()
*/
void CAMainWin::scoreViewPortMouseMove(QMouseEvent *e, QPoint coords, CAScoreViewPort *c) {
	if ( (mode() == InsertMode && musElementFactory()->musElementType() == CAMusElement::Note) ) {
		CADrawableStaff *s;
		if (c->currentContext()?(c->currentContext()->drawableContextType() == CADrawableContext::DrawableStaff):0)
			s = (CADrawableStaff*)c->currentContext(); 
		else
			return;

		if ( musElementFactory()->musElementType() == CAMusElement::Note || 
             musElementFactory()->musElementType() == CAMusElement::Rest) {
			c->setShadowNoteVisible(true);
        }
        
		// calculate the musical pitch out of absolute world coordinates and the current clef
		int pitch = s->calculatePitch(coords.x(), coords.y());
		
		// write into the main window's status bar the note pitch name
		int iNoteAccs = s->getAccs(coords.x(), pitch)+_musElementFactory->noteExtraAccs();
		_musElementFactory->setNoteAccs( iNoteAccs );
		statusBar()->showMessage(CANote::generateNoteName(pitch, iNoteAccs));
		c->setShadowNoteAccs(iNoteAccs);
		c->repaint();
	} else
	if ( mode()!=InsertMode  && e->buttons()==Qt::LeftButton ) { // multiple selection
		c->clearSelectionRegionList();
		int x=c->lastMousePressCoords().x(), y=c->lastMousePressCoords().y(),
		    w=coords.x()-c->lastMousePressCoords().x(), h=coords.y()-c->lastMousePressCoords().y();
		if (w<0) { x+=w; w*=(-1); } // user selected from right to left
		if (h<0) { y+=h; h*=(-1); } // user selected from bottom to top
		QRect selectionRect( x, y, w, h );
		
		QList<CADrawableContext*> dcList = c->findContextsInRegion( selectionRect );
		for (int i=0; i<dcList.size(); i++) {
			QList<CADrawableMusElement*> musEltList = dcList[i]->findInRange( selectionRect.x(), selectionRect.x() + selectionRect.width() );
			for (int j=0; j<musEltList.size(); j++)
				if (musEltList[j]->drawableMusElementType()==CADrawableMusElement::DrawableSlur)
					musEltList.removeAt(j--);
			
			if (musEltList.size()) {
				c->addSelectionRegion( QRect(musEltList.front()->xPos(), dcList[i]->yPos(),
				                             musEltList.back()->xPos()+musEltList.back()->width()-musEltList.front()->xPos(), dcList[i]->height()) );
			}
		}
		c->repaint();
	}
}

/*!
	Processes the mouse move event \a e with coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mouseReleaseEvent(), scoreViewPortMousePress(), scoreViewPortMouseMove(), scoreViewPortWheel(), scoreViewPortKeyPress()
*/
void CAMainWin::scoreViewPortMouseRelease(QMouseEvent *e, QPoint coords, CAScoreViewPort *c) {
	if ( mode() != InsertMode  && c->lastMousePressCoords()!=coords ) { // area was selected
		c->clearSelectionRegionList();
		
		if (e->modifiers()==Qt::NoModifier)
			c->clearSelection();
		
		int x=c->lastMousePressCoords().x(), y=c->lastMousePressCoords().y(),
		    w=coords.x()-c->lastMousePressCoords().x(), h=coords.y()-c->lastMousePressCoords().y();
		if (w<0) { x+=w; w*=(-1); } // user selected from right to left
		if (h<0) { y+=h; h*=(-1); } // user selected from bottom to top
		QRect selectionRect( x, y, w, h );
		
		QList<CADrawableContext*> dcList = c->findContextsInRegion( selectionRect );
		for (int i=0; i<dcList.size(); i++) {
			QList<CADrawableMusElement*> musEltList = dcList[i]->findInRange( selectionRect.x(), selectionRect.x() + selectionRect.width() );
			if (c->selectedVoice() && dcList[i]->context()!=c->selectedVoice()->staff())
				continue;
			
			for (int j=0; j<musEltList.size(); j++)
				if (c->selectedVoice() && musEltList[j]->musElement()->isPlayable() && static_cast<CAPlayable*>(musEltList[j]->musElement())->voice()!=c->selectedVoice() ||
				    (!musEltList[j]->isSelectable()) ||
				    (musEltList[j]->drawableMusElementType()==CADrawableMusElement::DrawableSlur)
				)
					musEltList.removeAt(j--);
			c->addToSelection(musEltList);
		}
		c->repaint();
	}
}

/*!
	Processes the mouse wheel event \a e with coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::wheelEvent(), scoreViewPortMousePress(), scoreViewPortMouseMove(), scoreViewPortKeyPress()
*/
void CAMainWin::scoreViewPortWheel(QWheelEvent *e, QPoint coords, CAScoreViewPort *sv) {
	setCurrentViewPort( sv );
	
	int val;
	switch (e->modifiers()) {
		case Qt::NoModifier:			//scroll horizontally
			sv->setWorldX( sv->worldX() - (int)((0.5*e->delta()) / sv->zoom()), _animatedScroll );
			break;
		case Qt::AltModifier:			//scroll horizontally, fast
			sv->setWorldX( sv->worldX() - (int)(e->delta() / sv->zoom()), _animatedScroll );
			break;
		case Qt::ShiftModifier:			//scroll vertically
			sv->setWorldY( sv->worldY() - (int)((0.5*e->delta()) / sv->zoom()), _animatedScroll );
			break;
		case 0x0A000000://SHIFT+ALT		//scroll vertically, fast
			sv->setWorldY( sv->worldY() - (int)(e->delta() / sv->zoom()), _animatedScroll );
			break;
		case Qt::ControlModifier:		//zoom
			if (e->delta() > 0)
				sv->setZoom( sv->zoom()*1.1, coords.x(), coords.y(), _animatedScroll );
			else
				sv->setZoom( sv->zoom()/1.1, coords.x(), coords.y(), _animatedScroll );
			
			break;
	}
	
	sv->repaint();
}

/*!
	Processes the key press event \a e of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::keyPressEvent(), scoreViewPortMousePress(), scoreViewPortMouseMove(), scoreViewPortWheel()
*/
void CAMainWin::scoreViewPortKeyPress(QKeyEvent *e, CAScoreViewPort *v) {
	setCurrentViewPort( v );
	
	switch (e->key()) {
		//Music editing keys
		case Qt::Key_Right: {
			//select next music element
			v->selectNextMusElement();
			v->repaint();
			break;
		}
		
		case Qt::Key_Left: {
			//select previous music element
			v->selectPrevMusElement();
			v->repaint();
			break;
		}
		
		case Qt::Key_B: {
			//place a barline
			CADrawableContext *drawableContext;
			drawableContext = v->currentContext();
			
			if ( (!drawableContext) || (drawableContext->context()->contextType() != CAContext::Staff) )
				return;
		
			CAStaff *staff = (CAStaff*)drawableContext->context();
			CAMusElement *left = 0;
			if (!v->selection().isEmpty())
				left = v->selection().back()->musElement();
				
			CACanorus::createUndoCommand( document(), tr("insert barline", "undo") );
			CABarline *bar = new CABarline(
				CABarline::Single,
				staff,
				(left?left->timeEnd():staff->lastTimeEnd())
			);
			staff->insertSignAfter(bar, left, true);	//insert the barline in all the voices
			
			CACanorus::pushUndoCommand();
			CACanorus::rebuildUI(document(), v->sheet());
			v->selectMElement(bar);
			v->repaint();
			break;
		}
		
		case Qt::Key_Up: {
			if (mode() == SelectMode) {	//select the upper music element
				v->selectUpMusElement();
				v->repaint();
			} else if ((mode() == InsertMode) || (mode() == EditMode)) {
				if (!v->selection().isEmpty()) {
					CADrawableMusElement *elt =
						v->selection().back();
					
					// pitch note for one step higher
					if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
						CACanorus::createUndoCommand( document(), tr("rise note", "undo") );
						CANote *note = (CANote*)elt->musElement();
						note->setPitch(note->pitch()+1);
						CACanorus::pushUndoCommand();
						CACanorus::rebuildUI(document(), note->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Down: {
			if (mode() == SelectMode) {	//select the upper music element
				v->selectUpMusElement();
				v->repaint();
			} else if ((mode() == InsertMode) || (mode() == EditMode)) {
				if (!v->selection().isEmpty()) {
					CADrawableMusElement *elt =
						v->selection().back();
					
					// pitch note for one step higher
					if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
						CANote *note = (CANote*)elt->musElement();
						CACanorus::createUndoCommand( document(), tr("lower note", "undo") );
						note->setPitch(note->pitch()-1);
						CACanorus::pushUndoCommand();
						CACanorus::rebuildUI(document(), note->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Plus: {
			if (mode()==InsertMode) {
				musElementFactory()->addNoteExtraAccs(1); musElementFactory()->addNoteAccs(1);
				v->setDrawShadowNoteAccs(musElementFactory()->noteExtraAccs()!=0);
				v->setShadowNoteAccs(musElementFactory()->noteAccs());
				v->repaint();
			} else if (mode()==EditMode) {
				if (!v->selection().isEmpty()) {
					CAMusElement *elt = v->selection().front()->musElement();
					if (elt->musElementType()==CAMusElement::Note) {
						CACanorus::createUndoCommand( document(), tr("add sharp", "undo") );
						((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()+1);
						CACanorus::pushUndoCommand();
						CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Minus: {
			if (mode()==InsertMode) {
				musElementFactory()->subNoteExtraAccs(1); musElementFactory()->subNoteAccs(1);
				v->setDrawShadowNoteAccs(musElementFactory()->noteExtraAccs()!=0);
				v->setShadowNoteAccs(musElementFactory()->noteAccs());
				v->repaint();
			} else if (mode()==EditMode) {
				if (!v->selection().isEmpty()) {
					CAMusElement *elt = v->selection().front()->musElement();
					if (elt->musElementType()==CAMusElement::Note) {
						CACanorus::createUndoCommand( document(), tr("add flat", "undo") );
						((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()-1);
						CACanorus::pushUndoCommand();
						CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Period: {
			if (mode()==InsertMode) {
				_musElementFactory->addPlayableDotted( 1 );
				((CAScoreViewPort*)v)->setShadowNoteDotted(_musElementFactory->playableDotted());
				v->repaint();
			} else if (mode()==EditMode) {
				if (!((CAScoreViewPort*)v)->selection().isEmpty()) {
					CACanorus::createUndoCommand( document(), tr("set dotted", "undo") );
					CAMusElement *elt = ((CAScoreViewPort*)v)->selection().front()->musElement();
					
					if (elt->isPlayable()) {
						int diff;
						if (elt->musElementType()==CAMusElement::Note) {
							int i;
							for (i=0; i<((CANote*)elt)->chord().size(); i++) {
								diff = ((CANote*)elt)->chord().at(i)->setDotted((((CAPlayable*)elt)->dotted()+1)%4);
							}
							elt = ((CANote*)elt)->chord().last();
						} else if (elt->musElementType()==CAMusElement::Rest)
							diff = ((CAPlayable*)elt)->setDotted((((CAPlayable*)elt)->dotted()+1)%4);
						 
						((CAPlayable*)elt)->voice()->updateTimesAfter(elt, diff);
						CACanorus::pushUndoCommand();
						CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			if ( v->selection().size() ) {
				CACanorus::createUndoCommand( document(), tr("deletion of elements", "undo") );
				
				QSet<CAMusElement*> musElemSet;
				for (int i=0; i<v->selection().size(); i++)
					musElemSet << v->selection().at(i)->musElement();
				
				// cleans up the set - removes empty elements and elements which get deleted automatically (eg. slurs)
				for (QSet<CAMusElement*>::iterator i=musElemSet.begin(); i!=musElemSet.end();) {
					if (!(*i))
						i = musElemSet.erase(i);
					else if ((*i)->musElementType()==CAMusElement::Slur && musElemSet.contains(static_cast<CASlur*>(*i)->noteStart()))
						i = musElemSet.erase(i);
					else if ((*i)->musElementType()==CAMusElement::Slur && musElemSet.contains(static_cast<CASlur*>(*i)->noteEnd()))
						i = musElemSet.erase(i);
					else
						i++;
				}
				
				for (QSet<CAMusElement*>::const_iterator i=musElemSet.constBegin(); i!=musElemSet.constEnd(); i++) {
					if ((*i)->musElementType()==CAMusElement::Note) {
						CANote *note = static_cast<CANote*>(*i);
						if (!note->isPartOfTheChord()) {
							for (int j=0; j<note->voice()->lyricsContextList().size(); j++) {
								CASyllable *removedSyllable =
									note->voice()->lyricsContextList().at(j)->removeSyllableAtTimeStart(note->timeStart());
								musElemSet.remove(removedSyllable);
							}
						}
						(*i)->context()->removeMusElement(*i);
					} else if ((*i)->musElementType()==CAMusElement::Syllable) {
						if (e->modifiers()==Qt::ShiftModifier) {
							CALyricsContext *lc = static_cast<CALyricsContext*>((*i)->context()); 
							(*i)->context()->removeMusElement(*i);  // actually removes the syllable if SHIFT is pressed
							lc->repositSyllables();
						} else {
							static_cast<CASyllable*>(*i)->clear(); // only clears syllable's text
						}
					} else {
						(*i)->context()->removeMusElement(*i);
					}
				}			
				CACanorus::pushUndoCommand();
				v->clearSelection();
				CACanorus::rebuildUI(document(), v->sheet());
			}
			
			break;
		
		// Mode keys
		case Qt::Key_Escape:
			if (mode()==SelectMode) {
				v->clearSelection();
				v->setCurrentContext( 0 );
			}
			uiSelectMode->toggle();
			uiVoiceNum->setRealValue( 0 );
			//if (uiKeySigPSP)
			//	uiKeySigPSP->hide();
			break;
		case Qt::Key_I:
			musElementFactory()->setMusElementType( CAMusElement::Note );
			setMode(InsertMode);
			break;
		case Qt::Key_E:
			setMode(EditMode);
			break;
	}
	
	updateToolBars();
}

/*!
	This method places the currently prepared music element in CAMusElementFactory to the staff or
	voice, dependent on the music element type and the viewport coordinates.
*/
void CAMainWin::insertMusElementAt(const QPoint coords, CAScoreViewPort *v) {
	CADrawableContext *drawableContext = v->currentContext();
	
	CAStaff *staff=0;
	CADrawableStaff *drawableStaff = 0;
	if (drawableContext) {
		drawableStaff = dynamic_cast<CADrawableStaff*>(drawableContext);
		staff = dynamic_cast<CAStaff*>(drawableContext->context());
	}
	
	CADrawableMusElement *drawableLeft = v->nearestLeftElement(coords.x(), coords.y());
	
	CAMusElement *left=0;
	if ( drawableLeft )
		left = drawableLeft->musElement();
	
	bool success=false;
	bool repaint=false;
	int iPlayableDotted = 0;
	
	if (!drawableContext)
		return;
	
	CACanorus::createUndoCommand( document(), tr("insertion of music element", "undo") );
	
	switch ( _musElementFactory->musElementType() ) {
		case CAMusElement::Clef: {
			if (staff)
				success = _musElementFactory->configureClef( staff , left );
			break;
		}
		case CAMusElement::KeySignature: {
			if ( staff )
				success = _musElementFactory->configureKeySignature( staff, left );
			break;
		}
		case CAMusElement::TimeSignature: {
			if ( staff )
				success = _musElementFactory->configureTimeSignature( staff, left );
			break;
		}
		case CAMusElement::Barline: {
			if ( staff )
				success = _musElementFactory->configureBarline( staff, left );
			break;
		}
		case CAMusElement::Note: { // Do we really need to do all that here??
			int iVoiceNum = uiVoiceNum->getRealValue()-1<0?0:uiVoiceNum->getRealValue()-1;
			CAVoice *voice = 0;
			
			if ( staff )
				voice = staff->voiceAt( iVoiceNum );
			
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y(), voice);
			if ( voice && drawableStaff )
				success = musElementFactory()->configureNote( voice, coords, drawableStaff, left );
			if (success) {
				_musElementFactory->setNoteExtraAccs( 0 );
				v->setDrawShadowNoteAccs(false); 
				iPlayableDotted = ((CAPlayable *)(_musElementFactory->musElement()))->dotted();
			}
			break;
		}
		case CAMusElement::Rest: {
			int iVoiceNum = uiVoiceNum->getRealValue()-1<0?0:uiVoiceNum->getRealValue()-1;
			CAVoice *voice = 0;
			if (staff)
				voice = staff->voiceAt( iVoiceNum );
			if (voice)
				success = _musElementFactory->configureRest( voice, left );
			if (success)
				iPlayableDotted = ((CAPlayable *)(_musElementFactory->musElement()))->dotted();
			break;
		}
		case CAMusElement::Slur: {
			// Insert tie, slur or phrasing slur
			if ( v->selection().size() ) { // start note has to always be selected
				CANote *noteStart = (currentScoreViewPort()->selection().front()->musElement()?dynamic_cast<CANote*>(currentScoreViewPort()->selection().front()->musElement()):0);
				CANote *noteEnd = (currentScoreViewPort()->selection().back()->musElement()?dynamic_cast<CANote*>(currentScoreViewPort()->selection().back()->musElement()):0);
				
				if ( noteStart && musElementFactory()->slurType()==CASlur::TieType ) {
					noteEnd = 0; // find a fresh next note
					QList<CANote*> noteList = noteStart->voice()->noteList();
					
					if ( noteStart->tieStart() ) {
						break; // return, if the tie already exists
					} else {
						// create a new tie
						for (int i=0; i<noteList.count() && noteList[i]->timeStart()<=noteStart->timeEnd(); i++) {
							if ( noteList[i]->timeStart()==noteStart->timeEnd() && noteList[i]->pitch()==noteStart->pitch() ) {
								noteEnd = noteList[i];
								break;
							}
						}
					}
					success = musElementFactory()->configureSlur( staff, noteStart, noteEnd );
				} else
				if ( noteStart && noteEnd && noteStart != noteEnd && (musElementFactory()->slurType()==CASlur::SlurType || musElementFactory()->slurType()==CASlur::PhrasingSlurType) ) {
					if (noteStart->isPartOfTheChord()) noteStart = noteStart->chord().at(0);
					if (noteEnd->isPartOfTheChord()) noteEnd = noteEnd->chord().at(0);
					QList<CANote*> noteList = noteStart->voice()->noteList();
					int end = noteList.indexOf(noteEnd);
					for (int i=noteList.indexOf(noteStart); i<=end; i++)
						if ( musElementFactory()->slurType()==CASlur::SlurType && (noteList[i]->slurStart() || noteList[i]->slurEnd()) ||
						     musElementFactory()->slurType()==CASlur::PhrasingSlurType && (noteList[i]->phrasingSlurStart() || noteList[i]->phrasingSlurEnd()) )
							return;
					
					if (musElementFactory()->slurType()==CASlur::SlurType && (noteStart->slurStart() || noteEnd->slurEnd()) ||
					    musElementFactory()->slurType()==CASlur::PhrasingSlurType && (noteStart->phrasingSlurStart() || noteEnd->phrasingSlurEnd()))
						break; // return, if the slur already exist
					success = musElementFactory()->configureSlur( staff, noteStart, noteEnd );
				}
				
				if (success) repaint=true;
			}
			break;
		}
		case CAMusElement::FunctionMarking: {
			// Insert function marking
			if (drawableContext->context()->contextType()==CAContext::FunctionMarkingContext) {
				CAFunctionMarkingContext *fmc = static_cast<CAFunctionMarkingContext*>(drawableContext->context());
				CADrawableMusElement *dLeft = v->nearestLeftElement(coords.x(), coords.y(), false);
				int timeStart = 0;
				if ( dLeft ) // find the nearest left element from the cursor
					timeStart = dLeft->musElement()->timeStart();
				QList<CAPlayable*> chord = currentSheet()->getChord(timeStart);
				int timeLength = chord.size()?chord[0]->timeLength():256;
				for (int i=0; i<chord.size(); i++) // find the shortest note in the chord
					if (chord[i]->timeLength()-(timeStart-chord[i]->timeStart())<timeLength)
						timeLength = chord[i]->timeLength()-(timeStart-chord[i]->timeStart());
				
				success = _musElementFactory->configureFunctionMarking( fmc, timeStart, timeLength );
			}
			break;
		}
	}
	
	if (success) {
		CACanorus::pushUndoCommand();
		CACanorus::rebuildUI(document(), v->sheet());
		v->selectMElement( _musElementFactory->musElement() );
		v->setShadowNoteDotted(iPlayableDotted);
		musElementFactory()->cloneMusElem(); // Clones the current musElement so it doesn't conflict with the added musElement
	} else {
		musElementFactory()->removeMusElem( true );
		musElementFactory()->createMusElem(); // Factory always must have a valid element
		musElementFactory()->setNoteExtraAccs( 0 );
	}
	if (repaint)
		v->repaint();
}

/*!
	Main window's key press event.
	
	\sa viewPortKeyPressEvent()
*/
void CAMainWin::keyPressEvent(QKeyEvent *e) {
}

/*!
	Called every second when timeEditedTimer has timeout.
	Increases the locally stored time the document is being edited.
*/
void CAMainWin::on_timeEditedTimer_timeout() {
	_timeEditedTime.addSecs(1);
}

/*!
	Called when playback is finished or interrupted by the user.
	It stops the playback, closes ports etc.
*/
void CAMainWin::playbackFinished() {
	_playback->disconnect();
	//delete _playback;	/// \todo crashes on application close, if deleted! Is this ok? -Matevz
	uiPlayFromSelection->setChecked(false);
	
	_repaintTimer->stop();
	_repaintTimer->disconnect();	/// \todo crashes, if disconnected sometimes. -Matevz
	delete _repaintTimer;			/// \todo crashes, if deleted. -Matevz
	CACanorus::midiDevice()->closeOutputPort();
	
	setMode( mode() );
}

/*!
	Connected with the play button which starts the playback.
*/
void CAMainWin::on_uiPlayFromSelection_toggled(bool checked) {
	if (checked && currentScoreViewPort()) {
		CACanorus::midiDevice()->openOutputPort(CACanorus::midiOutPort());
		_repaintTimer = new QTimer();
		_repaintTimer->setInterval(100);
		_repaintTimer->start();
		//connect(_repaintTimer, SIGNAL(timeout()), this, SLOT(on_repaintTimer_timeout())); //TODO: timeout is connected directly to repaint() directly. This should be optimized in the future -Matevz
		connect(_repaintTimer, SIGNAL(timeout()), _currentViewPort, SLOT(repaint()));
		_playbackViewPort = currentViewPort();
		
		_playback = new CAPlayback(currentScoreViewPort(), CACanorus::midiDevice());
		if ( currentScoreViewPort()->selection().size() && currentScoreViewPort()->selection().at(0)->musElement() )
			_playback->setInitTimeStart( currentScoreViewPort()->selection().at(0)->musElement()->timeStart() );
		
		connect(_playback, SIGNAL(finished()), this, SLOT(playbackFinished()));
		_playback->start();
	} else {
		_playback->stop();
	}
}

/*void CAMainWin::on_repaintTimer_timeout() { //TODO: timeout is connected directly to repaint() currently
	if (_lockScrollPlayback && (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort))
		((CAScoreViewPort*)_currentViewPort)->zoomToSelection(_animatedScroll);
	_currentViewPort->repaint();
}*/

void CAMainWin::on_uiAnimatedScroll_toggled(bool val) {
	_animatedScroll = val;
}

void CAMainWin::on_uiLockScrollPlayback_toggled(bool val) {
	_lockScrollPlayback = val;
}

void CAMainWin::on_uiZoomToSelection_triggered() {
	if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_currentViewPort)->zoomToSelection(_animatedScroll);
}

void CAMainWin::on_uiZoomToFit_triggered() {
	if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_currentViewPort)->zoomToFit();
}

void CAMainWin::on_uiZoomToWidth_triggered() {
	if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_currentViewPort)->zoomToWidth();
}

void CAMainWin::on_uiZoomToHeight_triggered() {
	if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_currentViewPort)->zoomToHeight();
}

void CAMainWin::closeEvent(QCloseEvent *event) {	//TODO: Make the main window the main window of the application somehow - when it's closed, the destructor is also called. This way, this function will not be needed anymore. -Matevz
	clearUI();
}

void CAMainWin::on_uiOpenDocument_triggered() {
	QString s = QFileDialog::getOpenFileName(
	                this,
	                tr("Choose a file to open"),
	                "",
	                tr("Canorus document (*.xml)"));

	if (s.isEmpty())
		return;
	
	openDocument(s); 
}

void CAMainWin::on_uiSaveDocument_triggered() {
	QString s;
	if (document()) {
		if ((s=document()->fileName()).isEmpty())
			on_uiSaveDocumentAs_triggered();
		else
			saveDocument(s);
	}
}

void CAMainWin::on_uiSaveDocumentAs_triggered() {
	if (document()) {
		QString s = QFileDialog::getSaveFileName(
		                this,
		                tr("Choose a file to save"),
		                "",
		                tr("Canorus document (*.xml)"));
		
		if (s.isEmpty())
			return;
		
		// append the extension, if the last 4 characters don't already contain the dot
		int i;
		for (i=0; (i<4) && ((s.length()-i-1) > 0); i++) if (s[s.length()-i-1] == '.') break;
		if (i==4) s.append(".xml");
		
		saveDocument(s);
	}
}

/*!
	Opens a document with the given absolute fileName.
	Previous document will be lost.
*/
bool CAMainWin::openDocument(QString fileName) {
	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QXmlInputSource input(&file);
		CADocument *openedDoc = CACanorusML::openDocument(&input, this);
		if (openedDoc) {
			if (CACanorus::mainWinCount(document())==1) {
				CACanorus::deleteUndoStack( document() ); 
				delete document();
			}
			
			setDocument(openedDoc);
			CACanorus::setUndoStack( document(), new QUndoStack() );
			openedDoc->setFileName(fileName);
			rebuildUI(); // local rebuild only
			uiTabWidget->setCurrentIndex(0);
		}
		
		CACanorus::restartTimeEditedTimes( document() );
		file.close();
		
		return true;
	} else
		return false;
}

/*!
	Saves the current document to a given absolute fileName.
*/
bool CAMainWin::saveDocument(QString fileName) {
	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);
		document()->setTimeEdited( document()->timeEdited() + _timeEditedTime.elapsed()/1000 );
		document()->setDateLastModified( QDateTime::currentDateTime() );
		CACanorusML::saveDocument(document(), out);
		document()->setFileName(fileName);
		file.close();
		
		CACanorus::restartTimeEditedTimes( document() );
		
		return true;
	} else
		return false;
}

void CAMainWin::on_midiInEvent( QVector<unsigned char> m) {
	std::cout << "MidiInEvent: ";
	for (int i=0; i<m.size(); i++)
		std::cout << m[i] << " ";
	std::cout << std::endl;
}

/*!
	Called when File->Export is clicked.
*/
void CAMainWin::on_uiExportDocument_triggered() {
	QStringList fileNames;
	int ffound = uiExportDialog->exec();
	if (ffound)
		fileNames = uiExportDialog->selectedFiles();
	
	if (!ffound)
		return;
	
	QString s = fileNames[0];
	
	if (CAPluginManager::exportFilterExists(uiExportDialog->selectedFilter()))
		CAPluginManager::exportAction(uiExportDialog->selectedFilter(), document(), s);
	else {
		QFile file(s);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			
			if (exportDialog()->selectedFilter() == LILYPOND_FILTER) {
				// LilyPond
				CALilyPondExport(document(), &out);
			}
  	 		
			file.close();
		}
	}
}

/*!
	Called when File->Import is clicked.
*/
void CAMainWin::on_uiImportDocument_triggered() {
	QStringList fileNames;
	int ffound = uiImportDialog->exec();
	if (ffound)
		fileNames = uiImportDialog->selectedFiles();
	
	if (!ffound)
		return;
	
	QString s = fileNames[0];
	
	if (CAPluginManager::importFilterExists(uiImportDialog->selectedFilter()))
		CAPluginManager::importAction(uiImportDialog->selectedFilter(), document(), fileNames[0]);
	else {
		QFile file(s);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);
			/// \todo Call appropriate built-in import function here
			/// eg. CALilyImport::importDocument(in, document());
			file.close();
		}
	}              
}

/*!
	Called when a user changes the current voice number.
*/
void CAMainWin::on_uiVoiceNum_valChanged(int voiceNr) {
	updateVoiceToolBar();
	if ( currentScoreViewPort() ) {
		if ( voiceNr &&
		     currentScoreViewPort()->currentContext() &&
		     currentScoreViewPort()->currentContext()->context()->contextType() == CAContext::Staff
		   )
			currentScoreViewPort()->setSelectedVoice( static_cast<CAStaff*>(currentScoreViewPort()->currentContext()->context())->voiceAt(voiceNr-1) );
		else
			currentScoreViewPort()->setSelectedVoice(0);
		
		currentScoreViewPort()->repaint();
	}
}

/*!
	Changes the number of accidentals.
*/
void CAMainWin::on_uiKeySigNumberOfAccs_valChanged(int accs) {
	if (mode()==InsertMode)
		_musElementFactory->setKeySigNumberOfAccs( accs );
	else if ( mode()==EditMode ) {
		CAScoreViewPort *v = currentScoreViewPort();
		if ( v && v->selection().size() ) {
			CACanorus::createUndoCommand( document(), tr("change number of accidentals", "undo") );		
			CAKeySignature *keySig = dynamic_cast<CAKeySignature*>(v->selection().at(0)->musElement());
			if ( keySig ) {
				keySig->setKeySignatureType( CAKeySignature::MajorMinor, accs, CAKeySignature::Major );
				CACanorus::pushUndoCommand();
				CACanorus::rebuildUI(document(), currentSheet());
			}
		}
	}
}

/*!
	Gets the current voice and sets its name.
*/
void CAMainWin::on_uiVoiceName_returnPressed() {
	CAVoice *voice = currentVoice();
	if (voice) {
		CACanorus::createUndoCommand( document(), tr("change voice name", "undo") );
		CACanorus::pushUndoCommand();
		voice->setName(uiVoiceName->text());
	}
}

void CAMainWin::on_uiInsertPlayable_toggled(bool checked) {
	if (checked) {
		if (!uiVoiceNum->getRealValue())
			uiVoiceNum->setRealValue( 1 ); // select the first voice if none selected
		
		_musElementFactory->setMusElementType( CAMusElement::Note );
		setMode( InsertMode );
	}
}

void CAMainWin::on_uiInsertSyllable_toggled( bool checked ) {
	if (checked) {
		setMode( InsertMode );
	}
}

void CAMainWin::on_uiInsertFM_toggled(bool checked) {
	if (checked) {
		_musElementFactory->setMusElementType( CAMusElement::FunctionMarking );
		setMode( InsertMode );
	}
}

void CAMainWin::on_uiPlayableLength_toggled(bool checked, int buttonId) {
	// Read currently selected entry from tool button menu
	CAPlayable::CAPlayableLength length =
		static_cast<CAPlayable::CAPlayableLength>(buttonId);
		
	// New note length type
	_musElementFactory->setPlayableLength( length );
}

/*!
	Function called when user types the text of the syllable or hits control keys.
	
	The following behaviour possible:
		- alphanumeric keys are pressed - writes text
		- spacebar is pressed - creates the current syllable and jumps to the next syllable
		- return is pressed - creates the current syllable and hides the syllable edit widget
		- left key is pressed - if cursorPosition()==0, jumps to the previous syllable
		- right key is pressed - if cursorPosition()==length(), same as spacebar
		- escape key is pressed - hides the syllable edit and cancels any changes to syllable
	
*/
void CAMainWin::on_syllableEdit_keyPressEvent(QKeyEvent *e, CASyllableEdit *syllableEdit) {
	if (!currentScoreViewPort()) return;
	
	CAScoreViewPort *v = currentScoreViewPort();
	CADrawableContext *dContext = v->currentContext();
	if (!dContext || !dContext->context() || dContext->context()->contextType()!=CAContext::LyricsContext)
		return;
	
	CASyllable *syllable=0;
	if ( v->selection().size() && v->selection().front()->drawableMusElementType()==CADrawableMusElement::DrawableSyllable )
		syllable = static_cast<CASyllable*>(v->selection().front()->musElement());
	
	if (!syllable) v->removeSyllableEdit();
	
	QString text = syllableEdit->text().simplified(); // remove any trailing whitespaces
		
	bool hyphen = false;
	if (text.right(1)=="-") { hyphen = true; text.chop(1); }
		
	bool melisma = false;
	if (text.right(1)=="_") { melisma = true; text.chop(1); }
		
	CAVoice *voice = 0; /// \todo GUI for syllable specific associated voice - current is the default lyrics context's one
		
	CALyricsContext *context = static_cast<CALyricsContext*>(dContext->context());
	
	// create or edit syllable
	if ( e->key()==Qt::Key_Space  ||
	     e->key()==Qt::Key_Return ||
	     e->key()==Qt::Key_Right && syllableEdit->cursorPosition()==syllableEdit->text().size() ||
	     (e->key()==Qt::Key_Left || e->key()==Qt::Key_Backspace) && syllableEdit->cursorPosition()==0 ) {
		CACanorus::createUndoCommand( document(), tr("lyrics edit", "undo") );	     
		syllable->setText(text);
		syllable->setHyphenStart(hyphen);
		syllable->setMelismaStart(melisma);
		
		v->removeSyllableEdit();
		
		CAVoice *voice = (syllable->associatedVoice()?syllable->associatedVoice():context->associatedVoice());
		CAMusElement *nextSyllable = 0;
		if (syllable) {
			if (e->key()==Qt::Key_Space || e->key()==Qt::Key_Right || e->key()==Qt::Key_Return) // next right note
				nextSyllable = syllable->lyricsContext()->findNextMusElement(syllable);
			else  if (e->key()==Qt::Key_Left || e->key()==Qt::Key_Backspace)                    // next left note
				nextSyllable = syllable->lyricsContext()->findPrevMusElement(syllable);
			
			CACanorus::pushUndoCommand();
			CACanorus::rebuildUI( document(), currentSheet() );
			if (nextSyllable) {
				CADrawableMusElement *dNextSyllable = v->selectMElement(nextSyllable);
				v->createSyllableEdit( dNextSyllable );
			}
		}
	}
	
	// escape key - cancel
	if (e->key()==Qt::Key_Escape) {
		v->removeSyllableEdit();
	}
}

void CAMainWin::on_uiFMFunction_toggled(bool checked, int buttonId) {
	if (checked) {
		if (buttonId>0) {
			_musElementFactory->setFMFunction( static_cast<CAFunctionMarking::CAFunctionType>(buttonId) );
			_musElementFactory->setFMFunctionMinor( false );
		} else {
			_musElementFactory->setFMFunction( static_cast<CAFunctionMarking::CAFunctionType>(buttonId*(-1)) );
			_musElementFactory->setFMFunctionMinor( true );
		}
	} else
		_musElementFactory->setFMFunction( CAFunctionMarking::Undefined );
}

void CAMainWin::on_uiFMChordArea_toggled(bool checked, int buttonId) {
	if (checked) {
		if (buttonId>0) {
			_musElementFactory->setFMChordArea( static_cast<CAFunctionMarking::CAFunctionType>(buttonId) );
			_musElementFactory->setFMChordAreaMinor( false );
		} else {
			_musElementFactory->setFMChordArea( static_cast<CAFunctionMarking::CAFunctionType>(buttonId*(-1)) );
			_musElementFactory->setFMChordAreaMinor( true );
		}
	} else
		_musElementFactory->setFMChordArea( CAFunctionMarking::Undefined );
}

void CAMainWin::on_uiFMTonicDegree_toggled(bool checked, int buttonId) {
	if (buttonId>0) {
		_musElementFactory->setFMTonicDegree( static_cast<CAFunctionMarking::CAFunctionType>(buttonId) );
		_musElementFactory->setFMTonicDegreeMinor( false );
	} else {
		_musElementFactory->setFMTonicDegree( static_cast<CAFunctionMarking::CAFunctionType>(buttonId*(-1)) );
		_musElementFactory->setFMTonicDegreeMinor( true );
	}
}

void CAMainWin::on_uiFMEllipse_toggled(bool checked) {
	_musElementFactory->setFMEllipse( checked );
}


void CAMainWin::on_uiSlurType_toggled( bool checked, int buttonId ) {
	// Read currently selected entry from tool button menu
	CASlur::CASlurType slurType =
		static_cast<CASlur::CASlurType>(buttonId);
		
	_musElementFactory->setMusElementType( CAMusElement::Slur );
	
	// New clef type
	_musElementFactory->setSlurType( slurType );
	
	insertMusElementAt( QPoint(0,0), currentScoreViewPort() ); // inserts a slur or tie and quits the insert mode
}

void CAMainWin::on_uiClefType_toggled(bool checked, int buttonId) {
	if ( checked ) {
		// Read currently selected entry from tool button menu
		CAClef::CAClefType clefType =
			static_cast<CAClef::CAClefType>(buttonId);
			
		_musElementFactory->setMusElementType( CAMusElement::Clef );
		
		// New clef type
		_musElementFactory->setClef( clefType );
		
		setMode( InsertMode );
	}
}

void CAMainWin::on_uiTimeSigBeats_valChanged(int beats) {
	if (mode()==InsertMode) {
		_musElementFactory->setTimeSigBeats( beats );
		if (uiTimeSigBeats->value()==4 && uiTimeSigBeat->value()==4)
			uiTimeSigType->setCurrentId( TS_44 );
		else if (uiTimeSigBeats->value()==3 && uiTimeSigBeat->value()==4)
			uiTimeSigType->setCurrentId( TS_34 );
		else if (uiTimeSigBeats->value()==2 && uiTimeSigBeat->value()==4)
			uiTimeSigType->setCurrentId( TS_24 );
		else if (uiTimeSigBeats->value()==2 && uiTimeSigBeat->value()==2)
			uiTimeSigType->setCurrentId( TS_22 );
		else if (uiTimeSigBeats->value()==3 && uiTimeSigBeat->value()==8)
			uiTimeSigType->setCurrentId( TS_38 );
		else if (uiTimeSigBeats->value()==6 && uiTimeSigBeat->value()==8)
			uiTimeSigType->setCurrentId( TS_68 );
		else
			uiTimeSigType->setCurrentId( TS_CUSTOM );
	} else if ( mode()==EditMode ) {
		CAScoreViewPort *v = currentScoreViewPort();
		if ( v && v->selection().size() ) {
			CATimeSignature *timeSig = dynamic_cast<CATimeSignature*>(v->selection().at(0)->musElement());
			if ( timeSig ) {
				timeSig->setBeats( beats );
				CACanorus::rebuildUI(document(), currentSheet());
			}
		}
	}
}

void CAMainWin::on_uiTimeSigBeat_valChanged(int beat) {
	if (mode()==InsertMode) {
		_musElementFactory->setTimeSigBeat( beat );
		if (uiTimeSigBeats->value()==4 && uiTimeSigBeat->value()==4)
			uiTimeSigType->setCurrentId( TS_44 );
		else if (uiTimeSigBeats->value()==3 && uiTimeSigBeat->value()==4)
			uiTimeSigType->setCurrentId( TS_34 );
		else if (uiTimeSigBeats->value()==2 && uiTimeSigBeat->value()==4)
			uiTimeSigType->setCurrentId( TS_24 );
		else if (uiTimeSigBeats->value()==2 && uiTimeSigBeat->value()==2)
			uiTimeSigType->setCurrentId( TS_22 );
		else if (uiTimeSigBeats->value()==3 && uiTimeSigBeat->value()==8)
			uiTimeSigType->setCurrentId( TS_38 );
		else if (uiTimeSigBeats->value()==6 && uiTimeSigBeat->value()==8)
			uiTimeSigType->setCurrentId( TS_68 );
		else
			uiTimeSigType->setCurrentId( TS_CUSTOM );
	} else if ( mode()==EditMode ) {
		CAScoreViewPort *v = currentScoreViewPort();
		if ( v && v->selection().size() ) {
			CATimeSignature *timeSig = dynamic_cast<CATimeSignature*>(v->selection().at(0)->musElement());
			if ( timeSig ) {
				timeSig->setBeat( beat );
				CACanorus::rebuildUI(document(), currentSheet());
			}
		}
	}
}

void CAMainWin::on_uiTimeSigType_toggled(bool checked, int buttonId) {
	if (checked) {
		setMode( InsertMode );
		
		// Read currently selected entry from tool button menu
		CAFixedTimeSig type = static_cast<CAFixedTimeSig>(buttonId);
		
		// New (fixed) time signature
		switch( type ) {
			case TS_44:
			  uiTimeSigBeats->setValue( 4 );
			  uiTimeSigBeat->setValue( 4 );
			  break;
			case TS_22:
			  uiTimeSigBeats->setValue( 2 );
			  uiTimeSigBeat->setValue( 2 );
			  break;
			case TS_34:
			  uiTimeSigBeats->setValue( 3 );
			  uiTimeSigBeat->setValue( 4 );
			  break;
			case TS_24:
			  uiTimeSigBeats->setValue( 2 );
			  uiTimeSigBeat->setValue( 4 );
			  break;
			case TS_38:
			  uiTimeSigBeats->setValue( 3 );
			  uiTimeSigBeat->setValue( 8 );
			  break;
			case TS_68:
			  uiTimeSigBeats->setValue( 6 );
			  uiTimeSigBeat->setValue( 8 );
			  break;
			case TS_UNKNOWN:
			  break;
		}
		_musElementFactory->setTimeSigBeats( uiTimeSigBeats->value() );
		_musElementFactory->setTimeSigBeat( uiTimeSigBeat->value() );
		_musElementFactory->setMusElementType( CAMusElement::TimeSignature );
	}
}

void CAMainWin::on_uiInsertKeySig_toggled(bool checked) {
	if (checked) {
		_musElementFactory->setMusElementType( CAMusElement::KeySignature );
		setMode( InsertMode );
	}
}

void CAMainWin::on_uiBarlineType_toggled(bool checked, int buttonId) {
	if (checked) {
		setMode( InsertMode );
		_musElementFactory->setMusElementType( CAMusElement::Barline );
		_musElementFactory->setBarlineType( static_cast<CABarline::CABarlineType>(buttonId) );
	}
}

/*!
	Called when a user clicks "Commit" button in source viewport.
*/
void CAMainWin::sourceViewPortCommit(QString inputString, CASourceViewPort *v) {
	if (v->document()) {
		// CanorusML document source
		CACanorus::createUndoCommand( document(), tr("commit CanorusML source", "undo") );
		
		clearUI(); 	// clear GUI before clearing the data part!
		if ( document() )
			delete document();
		
		QXmlInputSource input;
		input.setData(inputString);
		CACanorus::pushUndoCommand();
		setDocument(CACanorusML::openDocument(&input, this));
		CACanorus::rebuildUI(document());
	} else
	if (v->voice()) {
		// LilyPond voice source
		CACanorus::createUndoCommand( document(), tr("commit LilyPond source", "undo") );
		
		v->voice()->clear(); // clearUI is not needed, because only voice content is changed
		
		CACanorus::pushUndoCommand();
		CALilyPondImport(inputString, v->voice());
		CACanorus::rebuildUI(document(), v->voice()->staff()->sheet());
	} else
	if (v->lyricsContext()) {
		// LilyPond lyrics source
		CACanorus::createUndoCommand( document(), tr("commit LilyPond source", "undo") );
		
		v->lyricsContext()->clear(); // clearUI is not needed, because only voice content is changed
		
		CACanorus::pushUndoCommand();
		CALilyPondImport(inputString, v->lyricsContext());
		CACanorus::rebuildUI(document(), v->lyricsContext()->sheet());
	}
	
	setCurrentViewPort( v );
}

void CAMainWin::on_uiAboutQt_triggered()
{
	QMessageBox::aboutQt( this, tr("About Qt") );
}

void CAMainWin::on_uiAboutCanorus_triggered()
{
	QMessageBox::about ( this, tr("About Canorus"),
	tr("Canorus - The next generation music score editor\n\n\
Version %1\n\
(C) 2006, 2007 Canorus Development team. All rights reserved.\n\
See the file AUTHORS for the list of Canorus developers\n\n\
This program is licensed under the GNU General Public License (GPL).\n\
See the file LICENSE.GPL for details.\n\n\
Homepage: http://www.canorus.org").arg(CANORUS_VERSION) );
}

void CAMainWin::on_uiSettings_triggered() {
	CAMidiSetupDialog(this);
}

void CAMainWin::on_uiLilyPondSource_triggered() {
	CAContext *context = currentContext();
	if ( context ) {
		CASourceViewPort *v=0;
		if (context->contextType()==CAContext::Staff) {
			CAStaff *staff = static_cast<CAStaff*>(static_cast<CAScoreViewPort*>(currentViewPort())->currentContext()->context());
			int voiceNum = uiVoiceNum->getRealValue()-1<0?0:uiVoiceNum->getRealValue()-1;
			CAVoice *voice = staff->voiceAt( voiceNum );
			v = new CASourceViewPort(voice, 0);
		} else
		if (context->contextType()==CAContext::LyricsContext) {
			v = new CASourceViewPort(static_cast<CALyricsContext*>(context), 0);
		}
		
		initViewPort( v );
		currentViewPortContainer()->addViewPort( v );
		
		uiUnsplitAll->setEnabled(true);
		uiCloseCurrentView->setEnabled(true);
	}
}

/*!
	Adds a new score viewport to default viewport container.
*/
void CAMainWin::on_uiScoreView_triggered() {
	CASheet* s = _sheetMap[currentViewPortContainer()];
	
	if ( currentViewPortContainer() && s ) {
		CAScoreViewPort *v = new CAScoreViewPort(s, 0);
		initViewPort( v );
		currentViewPortContainer()->addViewPort( v );
		v->rebuild();
		
		uiUnsplitAll->setEnabled(true);
		uiCloseCurrentView->setEnabled(true);
	}
}

/*!
	Removes the sheet, all its contents and rebuilds the GUI.
*/
void CAMainWin::on_uiRemoveSheet_triggered() {
	CASheet *sheet = currentSheet();
	if (sheet) {
		CACanorus::createUndoCommand( document(), tr("deletion of the sheet", "undo") );
		CACanorus::pushUndoCommand();
		document()->removeSheet(currentSheet());
		removeSheet(sheet);
		delete sheet;
	}
}

/*!
	Removes the sheet from the GUI and deletes the viewports and viewport containers and tabs
	pointing to the given \a sheet.
*/
void CAMainWin::removeSheet( CASheet *sheet ) {
	CAViewPortContainer *vpc = _sheetMap.key(sheet);
	_sheetMap.remove(vpc);
	
	// remove tab
	int idx = uiTabWidget->indexOf(vpc);
	uiTabWidget->removeTab( idx );
	delete vpc;
	if (idx < uiTabWidget->count())
		uiTabWidget->setCurrentIndex(idx);
	setCurrentViewPortContainer( static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget()) );
	setCurrentViewPort( static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget())?static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget())->currentViewPort():0 );
	
	// remove other viewports pointing to the sheet
	QList<CAViewPort*> vpl = viewPortList();
	for (int i=0; i<vpl.size(); i++) {
		switch (vpl[i]->viewPortType()) {
			case CAViewPort::ScoreViewPort: {
				if (static_cast<CAScoreViewPort*>(vpl[i])->sheet()==sheet)
					delete vpl[i];
				break;
			}
			case CAViewPort::SourceViewPort: {
				CASourceViewPort *sv = static_cast<CASourceViewPort*>(vpl[i]);
				if (sv->voice() && sv->voice()->staff()->sheet()==sheet)
					delete vpl[i];
				else
				if (sv->lyricsContext() && sv->lyricsContext()->sheet()==sheet)
					delete vpl[i];
				
				break;
			}
		}
	}
}

void CAMainWin::on_uiSheetName_returnPressed() {
	CASheet *sheet = currentSheet();
	if (sheet) {
		CACanorus::createUndoCommand( document(), tr("change sheet name", "undo") );
		CACanorus::pushUndoCommand();
		updateUndoRedoButtons();
		sheet->setName( uiSheetName->text() );
		uiTabWidget->setTabText(uiTabWidget->currentIndex(), sheet->name());
	}
}

/*!
	Sets the current context name.
*/
void CAMainWin::on_uiContextName_returnPressed() {
	CAContext *context = currentContext();
	if (context) {
		CACanorus::createUndoCommand( document(), tr("change context name", "undo") );
		CACanorus::pushUndoCommand();
		updateUndoRedoButtons();
		context->setName(uiContextName->text());
	}
}

/*!
	Sets the number of lines in the staff.
*/
void CAMainWin::on_uiStaffNumberOfLines_valueChanged(int lines) {
	CAStaff *staff = currentStaff();
	if (staff) {
		CACanorus::createUndoCommand( document(), tr("change number of staff lines", "undo") );
		if (staff->numberOfLines()!=lines)
			CACanorus::pushUndoCommand();
		staff->setNumberOfLines(lines);
		CACanorus::rebuildUI(document(), currentSheet());
	}
}

/*!
	Sets the stanza number of the current lyrics context.
*/
void CAMainWin::on_uiStanzaNumber_valueChanged(int stanzaNumber) {
	if (currentContext() && currentContext()->contextType()==CAContext::LyricsContext) {
		CACanorus::createUndoCommand( document(), tr("change stanza number", "undo") );
		if (static_cast<CALyricsContext*>(currentContext())->stanzaNumber()!=stanzaNumber)
			CACanorus::pushUndoCommand();
		static_cast<CALyricsContext*>(currentContext())->setStanzaNumber( stanzaNumber );
	}
}

/*!
	Sets the associated voice of the current lyrics context.
*/
void CAMainWin::on_uiAssociatedVoice_currentIndexChanged(int idx) {
	if (idx != -1 && currentContext() && currentContext()->contextType()==CAContext::LyricsContext) {
		CACanorus::createUndoCommand( document(), tr("change associated voice", "undo") );
		if (static_cast<CALyricsContext*>(currentContext())->associatedVoice()!=currentSheet()->voiceList().at( idx ))
			CACanorus::pushUndoCommand();
		static_cast<CALyricsContext*>(currentContext())->setAssociatedVoice( currentSheet()->voiceList().at( idx ) );
		CACanorus::rebuildUI( document(), currentSheet() ); // needs a rebuild if lyrics contexts are to be moved
	}
}

void CAMainWin::on_uiVoiceStemDirection_toggled(bool checked, int direction) {
	CAVoice *voice = currentVoice();
	if (voice) {
		CACanorus::createUndoCommand( document(), tr("change voice stem direction", "undo") );
		if (voice->stemDirection()!=static_cast<CANote::CAStemDirection>(direction))
			CACanorus::pushUndoCommand();
		CACanorus::pushUndoCommand();
		voice->setStemDirection(static_cast<CANote::CAStemDirection>(direction));
		CACanorus::rebuildUI(document(), currentSheet());
	}
}

/*!
	Sets the currently selected note stem direction if in insert/edit mode or the music elements factory note stem direction if in insert mode.
*/
void CAMainWin::on_uiNoteStemDirection_toggled(bool checked, int id) {
	CANote::CAStemDirection direction = static_cast<CANote::CAStemDirection>(id);
	if (mode()==InsertMode)
		_musElementFactory->setNoteStemDirection( direction );
	else if (mode()==SelectMode || mode()==EditMode) {
		CACanorus::createUndoCommand( document(), tr("change note stem direction", "undo") );
		CAScoreViewPort *v = currentScoreViewPort();
		bool changed=false;
		for (int i=0; v && i<v->selection().size(); i++) {
			CANote *note = dynamic_cast<CANote*>(v->selection().at(i)->musElement());
			if ( note ) {
				note->setStemDirection( direction );
				changed=true;
			}
		}
		if (changed) {
			CACanorus::pushUndoCommand();
			CACanorus::rebuildUI(document(), currentSheet());
		}
	}
}

/*!
	Updates all the toolbars according to the current state of the main window.
*/
void CAMainWin::updateToolBars() {
	updateInsertToolBar();
	updateSheetToolBar();
	updateContextToolBar();
	updateVoiceToolBar();
	updatePlayableToolBar();
	updateKeySigToolBar();
	updateTimeSigToolBar();
	updateFMToolBar();
	updateUndoRedoButtons();
}

/*!
	Shows sheet tool bar if nothing selected. Otherwise hides it.
*/
void CAMainWin::updateSheetToolBar() {
	CAScoreViewPort *v = currentScoreViewPort();
	if (v && v->selection().isEmpty() && (!v->currentContext())) {
		if (v->sheet())
			uiSheetName->setText(v->sheet()->name());
		uiSheetToolBar->show();
	} else
		uiSheetToolBar->hide();
}

/*!
	Shows/Hides the Voice properties tool bar according to the currently selected context and updates its properties.
*/
void CAMainWin::updateVoiceToolBar() {
	CAContext *context = currentContext();
	if ( context && context->contextType() == CAContext::Staff ) {
		CAStaff *staff = static_cast<CAStaff*>(context);
		uiNewVoice->setEnabled(true);
		if (staff->voiceCount()) {
			uiVoiceNum->setMax(staff->voiceCount());
			int voiceNr = uiVoiceNum->getRealValue();
			if (voiceNr) {
				CAVoice *curVoice = (voiceNr<=staff->voiceCount()?staff->voiceAt(voiceNr-1):staff->voiceAt(0));
				uiVoiceName->setText(curVoice->name());
				uiVoiceName->setEnabled(true);
				uiRemoveVoice->setEnabled(true);
				uiVoiceStemDirection->setCurrentId( curVoice->stemDirection() );
				uiVoiceStemDirection->setEnabled(true);
//				uiVoiceProperties->setEnabled(true);
			} else {
				uiVoiceName->setEnabled(false);
				uiRemoveVoice->setEnabled(false);
				uiVoiceStemDirection->setEnabled(false);
				uiVoiceProperties->setEnabled(false);
			}
		}
		
		uiVoiceToolBar->show();
	} else {
		uiNewVoice->setEnabled(false);
		uiVoiceToolBar->hide();
	}
}

/*!
	Shows/Hides context tool bar according to the selected context (if any) and hides/shows specific actions in the toolbar for the current context.
*/
void CAMainWin::updateContextToolBar() {
	CAContext *context = currentContext();
	if (mode()==SelectMode && context) {
		switch (context->contextType()) {
			case CAContext::Staff: {
				uiStaffNumberOfLines->setValue(static_cast<CAStaff*>(context)->numberOfLines());
				uiStaffNumberOfLines->setVisible(true);
				uiStanzaNumber->setVisible(false);
				uiAssociatedVoice->setVisible(false);
				break;
			}
			case CAContext::LyricsContext: {
				CALyricsContext *c = static_cast<CALyricsContext*>(context);				
				uiStanzaNumber->setValue(c->stanzaNumber());
				uiStanzaNumber->setVisible(true);
				
				uiAssociatedVoice->disconnect(); // avoid recursive rebuilds in widget's slot
				uiAssociatedVoice->clear();
				QList<CAVoice*> voiceList = currentSheet()->voiceList();
				int idx = voiceList.indexOf( c->associatedVoice() );
				for (int i=0; i<voiceList.count(); i++) uiAssociatedVoice->addItem(voiceList[i]->name());
				connect( uiAssociatedVoice, SIGNAL(currentIndexChanged(int)), this, SLOT(on_uiAssociatedVoice_currentIndexChanged(int)) );
				uiAssociatedVoice->setCurrentIndex( idx );
				uiAssociatedVoice->setVisible(true);
				
				uiStaffNumberOfLines->setVisible(false);
				break;
			}
			case CAContext::FunctionMarkingContext: {
				uiStanzaNumber->setVisible(false);
				uiAssociatedVoice->setVisible(false);
				uiStaffNumberOfLines->setVisible(false);
				break;
			}
		}
		uiContextName->setText(context->name());
		
		if (!uiInsertPlayable->isChecked())
			uiContextToolBar->show();
	} else
		uiContextToolBar->hide();
}

/*!
	Shows/Hides music elements which cannot be placed in the selected context.
*/
void CAMainWin::updateInsertToolBar() {
	if (currentSheet()) {
		uiNewContext->setVisible(true);
		if (mode()==EditMode)
			uiInsertToolBar->hide();
		else {
			uiInsertToolBar->show();
			CAContext *context = currentContext();
			if (context) {
				switch (context->contextType()) {
					case CAContext::Staff:
						// staff selected
						uiInsertPlayable->setVisible(true);
						uiSlurType->setVisible(true);
						uiInsertClef->setVisible(true); // menu
						uiInsertBarline->setVisible(true); // menu
						uiClefType->setVisible(true);
						uiTimeSigType->setVisible(true);
						uiInsertKeySig->setVisible(true);
						uiInsertTimeSig->setVisible(true);
						uiBarlineType->setVisible(true);
						uiInsertFM->setVisible(false);
						uiInsertSyllable->setVisible(false);
						break;
					case CAContext::FunctionMarkingContext:
						// function marking context selected
						uiInsertPlayable->setVisible(false);
						uiSlurType->setVisible(false);
						uiInsertClef->setVisible(false); // menu
						uiInsertBarline->setVisible(false); // menu
						uiClefType->setVisible(false);
						uiTimeSigType->setVisible(false);
						uiInsertKeySig->setVisible(false);
						uiInsertTimeSig->setVisible(false);
						uiBarlineType->setVisible(false);
						uiInsertFM->setVisible(true);
						uiInsertSyllable->setVisible(false);
						break;
					case CAContext::LyricsContext:
						// lyrics context selected
						uiInsertPlayable->setVisible(false);
						uiSlurType->setVisible(false);
						uiInsertClef->setVisible(false); // menu
						uiInsertBarline->setVisible(false); // menu
						uiClefType->setVisible(false);
						uiTimeSigType->setVisible(false);
						uiInsertKeySig->setVisible(false);
						uiInsertTimeSig->setVisible(false);
						uiBarlineType->setVisible(false);
						uiInsertFM->setVisible(false);
						uiInsertSyllable->setVisible(true);
						break;
				}
			} else {
				// no contexts selected
				uiInsertPlayable->setVisible(false);
				uiSlurType->setVisible(false);
				uiInsertClef->setVisible(false); // menu
				uiInsertBarline->setVisible(false); // menu
				uiClefType->setVisible(false);
				uiTimeSigType->setVisible(false);
				uiInsertKeySig->setVisible(false);
				uiInsertTimeSig->setVisible(false);
				uiBarlineType->setVisible(false);
				uiInsertFM->setVisible(false);
				uiInsertSyllable->setVisible(false);
			}
		}	
	} else {
		uiInsertToolBar->hide();
		uiNewContext->setVisible(false);
	}	
}

/*!
	Show/Hides the playable tool bar and its properties according to the current state.
*/
void CAMainWin::updatePlayableToolBar() {
	if ( uiInsertPlayable->isChecked() && mode()==InsertMode ) {
		uiPlayableLength->defaultAction()->setEnabled(true);
		uiPlayableLength->setCurrentId( _musElementFactory->playableLength() );
		uiNoteStemDirection->setCurrentId( _musElementFactory->noteStemDirection() );
		uiHiddenRest->setEnabled(true);
		uiHiddenRest->setChecked( _musElementFactory->restType()==CARest::Hidden );
		uiPlayableToolBar->show();
	} else if (mode()==EditMode) {
		CAScoreViewPort *v = currentScoreViewPort();
		if (v && v->selection().size()) {
			CAPlayable *playable = dynamic_cast<CAPlayable*>(v->selection().at(0)->musElement());
			if (playable) {
				uiPlayableLength->defaultAction()->setEnabled(false);
				uiPlayableLength->setCurrentId( playable->playableLength() );
				if (playable->musElementType()==CAMusElement::Note) {
					CANote *note = static_cast<CANote*>(playable);
					uiNoteStemDirection->setCurrentId( note->stemDirection() );
					uiHiddenRest->setEnabled(false);
				} else if (playable->musElementType()==CAMusElement::Rest) {
					CARest *rest = static_cast<CARest*>(playable);
					uiHiddenRest->setEnabled(true);
					uiHiddenRest->setChecked(rest->restType()==CARest::Hidden);
				}
				uiPlayableToolBar->show();
				uiVoiceNum->setRealValue( playable->voice()->voiceNumber() );
			} else {
				uiPlayableToolBar->hide();
				uiHiddenRest->setEnabled(false);
			}
		}
	} else
		uiPlayableToolBar->hide();
}

/*!
	Shows/Hides the time signature properties tool bar according to the current state.
*/
void CAMainWin::updateTimeSigToolBar() {
	if (uiTimeSigType->isChecked() && mode()==InsertMode) {
		uiTimeSigBeats->setValue( _musElementFactory->timeSigBeats() );
		uiTimeSigBeat->setValue( _musElementFactory->timeSigBeat() );
		uiTimeSigToolBar->show();
	} else if (mode()==EditMode) {
		CAScoreViewPort *v = currentScoreViewPort();
		if (v && v->selection().size()) {
			CATimeSignature *timeSig = dynamic_cast<CATimeSignature*>(v->selection().at(0)->musElement());
			if (timeSig) {
				uiTimeSigBeats->setValue( timeSig->beats() );
				uiTimeSigBeat->setValue( timeSig->beat() );
				uiTimeSigToolBar->show();
			} else
				uiTimeSigToolBar->hide();
		}	
	} else
		uiTimeSigToolBar->hide();
}

/*!
	Shows/Hides the key signature properties tool bar according to the current state.
*/
void CAMainWin::updateKeySigToolBar() {
	if (uiInsertKeySig->isChecked() && mode()==InsertMode) {
		uiKeySigNumberOfAccs->setValue( _musElementFactory->keySigNumberOfAccs() );
		uiKeySigToolBar->show();
	} else if (mode()==EditMode) {
		CAScoreViewPort *v = currentScoreViewPort();
		if (v && v->selection().size()) {
			CAKeySignature *keySig = dynamic_cast<CAKeySignature*>(v->selection().at(0)->musElement());
			if (keySig) {
				uiKeySigNumberOfAccs->setValue( keySig->numberOfAccidentals() );
				uiKeySigToolBar->show();
			} else
				uiKeySigToolBar->hide();
		}	
	} else
		uiKeySigToolBar->hide();
}

/*!
	Shows/Hides the function marking properties tool bar according to the current state.
*/
void CAMainWin::updateFMToolBar() {
	if (uiInsertFM->isChecked() && mode()==InsertMode) {
		if ( !_musElementFactory->fmFunction()==CAFunctionMarking::Undefined ) {
			uiFMFunction->defaultAction()->setChecked( false );
			uiFMFunction->setCurrentId( _musElementFactory->fmFunction()*(_musElementFactory->isFMFunctionMinor()?-1:1) );
			uiFMFunction->defaultAction()->toggle();
		} else {
			uiFMFunction->defaultAction()->setChecked( true );
			uiFMFunction->defaultAction()->toggle();
		}

		if ( !_musElementFactory->fmChordArea()==CAFunctionMarking::Undefined ) {
			uiFMChordArea->defaultAction()->setChecked( false );
			uiFMChordArea->setCurrentId( _musElementFactory->fmChordArea()*(_musElementFactory->isFMChordAreaMinor()?-1:1) );
			uiFMChordArea->defaultAction()->toggle();
		} else {
			uiFMChordArea->defaultAction()->setChecked( true );
			uiFMChordArea->defaultAction()->toggle();
		}
		
		if ( !_musElementFactory->fmTonicDegree()==CAFunctionMarking::Undefined ) {
			uiFMTonicDegree->setCurrentId( _musElementFactory->fmTonicDegree()*(_musElementFactory->isFMTonicDegreeMinor()?-1:1) );
			uiFMTonicDegree->defaultAction()->toggle();
		}
		
		if ( _musElementFactory->isFMEllipse() ) {
			uiFMEllipse->setChecked( false );
			uiFMEllipse->trigger();
		} else {
			uiFMEllipse->setChecked( true );
			uiFMEllipse->trigger();
		}
		
		uiFMToolBar->show();
	} else if (mode()==EditMode) {
		CAScoreViewPort *v = currentScoreViewPort();
		if (v && v->selection().size()) {
			CAFunctionMarking *fm = dynamic_cast<CAFunctionMarking*>(v->selection().at(0)->musElement());
			if (fm) {
				if ( !fm->function()==CAFunctionMarking::Undefined ) {
					uiFMFunction->defaultAction()->setChecked( false );
					uiFMFunction->setCurrentId( fm->function()*(fm->isMinor()?-1:1) );
					uiFMFunction->defaultAction()->toggle();
				} else {
					uiFMFunction->defaultAction()->setChecked( true );
					uiFMFunction->defaultAction()->toggle();
				}
		
				if ( !fm->chordArea()==CAFunctionMarking::Undefined ) {
					uiFMChordArea->defaultAction()->setChecked( false );
					uiFMChordArea->setCurrentId( fm->chordArea()*(fm->isChordAreaMinor()?-1:1) );
					uiFMChordArea->defaultAction()->toggle();
				} else {
					uiFMChordArea->defaultAction()->setChecked( true );
					uiFMChordArea->defaultAction()->toggle();
				}
				
				if ( !fm->tonicDegree()==CAFunctionMarking::Undefined ) {
					uiFMTonicDegree->setCurrentId( fm->tonicDegree()*(fm->isTonicDegreeMinor()?-1:1) );
					uiFMTonicDegree->defaultAction()->toggle();
				}
				
				if ( fm->isPartOfEllipse() ) {
					uiFMEllipse->setChecked( false );
					uiFMEllipse->trigger();
				} else {
					uiFMEllipse->setChecked( true );
					uiFMEllipse->trigger();
				}
				
				uiFMToolBar->show();
			} else
				uiFMToolBar->hide();
		}	
	} else
		uiFMToolBar->hide();
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
	\var QList<CAViewPort*> CAMainWin::_viewPortList
	List of all available viewports for any sheet in this main window.
*/

/*!
	\var CAViewPort* CACanorus::_currentViewPort
	Currently active viewport. Only one viewport per main window can be active.
*/

/*!
	\var bool CACanorus::_animatedScroll
	Animates mouse scroll/zoom. This gives a better user experience but slows down things.
*/

/*!
	\var bool CACanorus::_lockScrollPlayback
	If True, locks the scroll UI while playback.
	If False, user can scroll freely.
*/

/*!
	\var CAViewPort* CAMainWin::_playbackViewPort
	Viewport needed to be updated when playback is active.
*/

/*!
	\var QTimer* CACanorus::_repaintTimer
	Used when playback is active to repaint the playback viewport.
	\todo Viewport should be repainted only when needed, not constantly as now. This should result in much less resource hunger.
*/

/*!
	\var CAMusElementFactory *CACanorus::_musElementFactory
	Factory for creating/configuring music elements before actually placing them.
*/

/*!
	\var bool CAMainWin::_rebuildUILock
	Lock to avoid recursive rebuilds of the GUI viewports.
*/
