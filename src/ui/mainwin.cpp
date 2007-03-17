/** \file ui/mainwin.cpp
 * 
 * Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
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

#include "widgets/menutoolbutton.h"
#include "widgets/lcdnumber.h"

#include "widgets/viewport.h"
#include "widgets/viewportcontainer.h"
#include "widgets/scoreviewport.h"
#include "widgets/sourceviewport.h"

#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"
#include "drawable/drawablemuselement.h"
#include "drawable/drawablenote.h"
#include "drawable/drawableaccidental.h"	//DEBUG, this isn't needed though

#include "core/canorus.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/functionmarkingcontext.h"
#include "core/clef.h"
#include "core/keysignature.h"
#include "core/note.h"
#include "core/rest.h"
#include "core/canorusml.h"
#include "core/voice.h"
#include "core/barline.h"
#include "core/timesignature.h"
#include "core/muselementfactory.h"

#include "scripting/swigruby.h"
#include "scripting/swigpython.h"

#include "export/lilypondexport.h"
#include "import/lilypondimport.h"

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
	// Initialize widgets
	setupUi( this );
	setupCustomUi();
	
	// Initialize import/export dialogs
	uiExportDialog = new QFileDialog(this);
	uiExportDialog->setFileMode(QFileDialog::AnyFile);
	uiExportDialog->setDirectory( QDir::current() );
	uiImportDialog = new QFileDialog(this);
	uiImportDialog->setFileMode(QFileDialog::ExistingFile);
	uiImportDialog->setDirectory( QDir::current() );
	
	// Initialize internal UI properties
	_mode = SelectMode;
	_playback = 0;
	_animatedScroll = true;
	_lockScrollPlayback = false;
	
	// Create plugins menus and toolbars in this main window
	CAPluginManager::enablePlugins(this);
	
	setDocument( 0 );
}

CAMainWin::~CAMainWin()  {
	CACanorus::removeMainWin(this);
	if (!CACanorus::mainWinCount(document()))
		delete document();
	
	delete _playback;
	
	// clear UI
	delete uiImportDialog; delete uiExportDialog;
	delete uiInsertToolBar; delete uiInsertGroup; delete uiContextType; delete uiClefType; delete uiBarlineType;
	delete uiVoiceToolBar; delete uiVoiceNum; delete uiVoiceName; delete uiRemoveVoice; delete uiVoiceStemDirection; delete uiVoiceProperties;
	delete uiPlayableToolBar; delete uiPlayableLength; delete uiPlayableDotted; delete uiNoteAccs; delete uiNoteStemDirection; delete uiHiddenRest;
	delete uiKeySigToolBar; delete uiKeySigNumberOfAccs;
	delete uiTimeSigToolBar; delete uiTimeSigBeats; delete uiTimeSigSlash; delete uiTimeSigBeat;
	delete uiFMToolBar; delete uiFMType; delete uiFMChordArea; delete uiFMTonicDegree; delete uiFMEllipse;
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
		uiInsertToolBar->addWidget( uiContextType = new CAMenuToolButton( tr("Select Context" ), 2, this ));
			uiContextType->addButton( QIcon(":/menu/images/newstaff.png"), CAContext::Staff );
			uiContextType->addButton( QIcon(":/menu/images/newfmcontext.png"), CAContext::FunctionMarkingContext );
			uiContextType->setCurrentId( CAContext::Staff );
			connect( uiContextType, SIGNAL( toggled(bool, int) ), this, SLOT( on_uiContextType_toggled(bool, int) ) );
			connect( uiNewContext, SIGNAL( triggered() ), uiContextType, SLOT( click() ) );
		uiInsertToolBar->addSeparator();
		uiInsertToolBar->addAction( uiInsertPlayable );
		uiInsertToolBar->addWidget(uiClefType = new CAMenuToolButton( tr("Select Clef"), 3, this ));
			uiClefType->addButton( QIcon(":/menu/images/cleftreble.png"), CAClef::Treble );
			uiClefType->addButton( QIcon(":/menu/images/clefbass.png"), CAClef::Bass );
			uiClefType->addButton( QIcon(":/menu/images/clefalto.png"), CAClef::Alto );
			uiClefType->setCurrentId( CAClef::Treble );
			connect( uiClefType, SIGNAL( toggled(bool, int) ), this, SLOT( on_uiClefType_toggled(bool, int) ) );
			connect( uiInsertClef, SIGNAL( triggered() ), uiClefType, SLOT( click() ) );
		uiInsertToolBar->addAction( uiInsertKeySig );
		uiInsertToolBar->addWidget(uiTimeSigType = new CAMenuToolButton( tr("Select Time Signature" ), 3, this ));
			uiTimeSigType->addButton( QIcon(":/menu/images/tsc.png"), TS_44 );
			uiTimeSigType->addButton( QIcon(":/menu/images/tsab.png"), TS_22 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts34.png"), TS_34 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts24.png"), TS_24 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts38.png"), TS_38 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts68.png"), TS_68 );
			uiTimeSigType->addButton( QIcon(":/menu/images/tscustom.png"), TS_CUSTOM );			
			uiTimeSigType->setCurrentId( TS_44 );
			connect( uiTimeSigType, SIGNAL(toggled(bool, int)), this, SLOT(on_uiTimeSigType_toggled(bool, int)) );
			connect( uiInsertTimeSig, SIGNAL(triggered()), uiTimeSigType, SLOT(click()));
		uiInsertToolBar->addWidget( uiBarlineType = new CAMenuToolButton( tr("Select Barline" ), 3, this ));
			uiBarlineType->addButton( QIcon(":/menu/images/barlinesingle.png"), CABarline::Single );
			uiBarlineType->addButton( QIcon(":/menu/images/barlinedouble.png"), CABarline::Double );
			uiBarlineType->addButton( QIcon(":/menu/images/barlineend.png"), CABarline::End );
			uiBarlineType->addButton( QIcon(":/menu/images/barlinerepeatopen.png"), CABarline::RepeatOpen );
			uiBarlineType->addButton( QIcon(":/menu/images/barlinerepeatclose.png"), CABarline::RepeatClose );
			uiBarlineType->addButton( QIcon(":/menu/images/barlinedotted.png"), CABarline::Dotted );
			uiBarlineType->setCurrentId( CABarline::Single );
			connect( uiBarlineType, SIGNAL(toggled(bool, int)), this, SLOT(on_uiBarlineType_toggled(bool, int)) );
			connect( uiInsertBarline, SIGNAL( triggered() ), uiBarlineType, SLOT( click() ) );
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
			uiVoiceStemDirection->addButton( QIcon(":/menu/images/notestemneutral.png"), CANote::StemNeutral );
			uiVoiceStemDirection->addButton( QIcon(":/menu/images/notestemup.png"), CANote::StemUp );
			uiVoiceStemDirection->addButton( QIcon(":/menu/images/notestemdown.png"), CANote::StemDown );
			uiVoiceStemDirection->defaultAction()->setCheckable(false);
		uiVoiceToolBar->addAction( uiVoiceProperties );
		addToolBar(Qt::TopToolBarArea, uiVoiceToolBar);
		
	uiPlayableToolBar = new QToolBar( tr("Playable ToolBar"), this );
		uiPlayableToolBar->addWidget(uiPlayableLength = new CAMenuToolButton( tr("Select Length" ), 4, this ));
			connect( uiPlayableLength, SIGNAL(toggled(bool, int)), this, SLOT(on_uiPlayableLength_toggled(bool, int)) );
			uiPlayableLength->setToolTip(tr("Playable length"));
			uiPlayableLength->addButton( QIcon(":/menu/images/n0.png"), CANote::Breve );
			uiPlayableLength->addButton( QIcon(":/menu/images/n1.png"), CANote::Whole );
			uiPlayableLength->addButton( QIcon(":/menu/images/n2.png"), CANote::Half );
			uiPlayableLength->addButton( QIcon(":/menu/images/n4.png"), CANote::Quarter );
			uiPlayableLength->addButton( QIcon(":/menu/images/n8.png"), CANote::Eighth );
			uiPlayableLength->addButton( QIcon(":/menu/images/n16.png"), CANote::Sixteenth );
			uiPlayableLength->addButton( QIcon(":/menu/images/n32.png"), CANote::ThirtySecond );
			uiPlayableLength->addButton( QIcon(":/menu/images/n64.png"), CANote::SixtyFourth );
			uiPlayableLength->defaultAction()->setCheckable(false);
			uiPlayableLength->setCurrentId( CANote::Quarter );
		uiPlayableToolBar->addWidget( uiPlayableDotted = new QLabel( " .. ", this ) );
			uiPlayableDotted->setToolTip(tr("Number of dots"));
		uiPlayableToolBar->addAction( uiAccsVisible );
		uiPlayableToolBar->addWidget(uiNoteStemDirection = new CAMenuToolButton( tr("Select Note Stem Direction" ), 4, this ));
			connect( uiNoteStemDirection, SIGNAL(toggled(bool, int)), this, SLOT(on_uiNoteStemDirection_toggled(bool, int)) );
			uiVoiceStemDirection->setToolTip(tr("Note stem direction"));		
			uiNoteStemDirection->addButton( QIcon(":/menu/images/notestemneutral.png"), CANote::StemNeutral );
			uiNoteStemDirection->addButton( QIcon(":/menu/images/notestemup.png"), CANote::StemUp );
			uiNoteStemDirection->addButton( QIcon(":/menu/images/notestemdown.png"), CANote::StemDown );
			uiNoteStemDirection->addButton( QIcon(":/menu/images/notestemvoice.png"), CANote::StemPrefered );
			uiNoteStemDirection->defaultAction()->setCheckable(false);
			uiNoteStemDirection->setCurrentId( CANote::StemPrefered );
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
		uiTimeSigBeats->setValue( 4 );
		uiTimeSigBeats->setToolTip( tr("Number of beats") );
		connect( uiTimeSigBeats, SIGNAL(valueChanged(int)), this, SLOT(on_uiTimeSigBeats_valChanged(int)) );
		uiTimeSigToolBar->addWidget( uiTimeSigSlash = new QLabel( "/", this ) );
		uiTimeSigToolBar->addWidget( uiTimeSigBeat = new QSpinBox(this) );
		uiTimeSigBeat->setValue( 4 );
		uiTimeSigBeat->setToolTip( tr("Beat") );
		connect( uiTimeSigBeat, SIGNAL(valueChanged(int)), this, SLOT(on_uiTimeSigBeat_valChanged(int)) );
		addToolBar(Qt::TopToolBarArea, uiTimeSigToolBar);
	
	// Mutual exclusive groups
	uiInsertGroup = new QActionGroup( this );
	uiInsertGroup->addAction( uiSelectMode );
	uiInsertGroup->addAction( uiNewContext );
	uiInsertGroup->addAction( uiContextType->defaultAction() );
	uiInsertGroup->addAction( uiInsertPlayable );
	uiInsertGroup->addAction( uiInsertClef );
	uiInsertGroup->addAction( uiClefType->defaultAction() );
	uiInsertGroup->addAction( uiInsertTimeSig );
	uiInsertGroup->addAction( uiTimeSigType->defaultAction() );
	uiInsertGroup->addAction( uiInsertKeySig );
	uiInsertGroup->addAction( uiInsertBarline );
	uiInsertGroup->addAction( uiBarlineType->defaultAction() );
	uiInsertGroup->addAction( uiInsertFM );
	uiInsertGroup->setExclusive( true );
	
	uiInsertToolBar->hide();
	uiSheetToolBar->hide();
	uiContextToolBar->hide();
	uiPlayableToolBar->hide();
	uiTimeSigToolBar->hide();
	uiKeySigToolBar->hide();
}

void CAMainWin::newDocument() {
	// clear the logical part
	if (CACanorus::mainWinCount(document()))
		delete document();
	
	setDocument(new CADocument());
	
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
	Adds an already created \a sheet to the document.
	This adds a tab to tabWidget and creates a single score viewport of the sheet.
*/
void CAMainWin::addSheet(CASheet *s) {
	CAScoreViewPort *v = new CAScoreViewPort(s, 0);
	initScoreViewPort(v);
	
	_viewPortList << v;
	
	CAViewPortContainer *vpc = new CAViewPortContainer(v, 0);
	_viewPortContainerList << vpc;
	
	uiTabWidget->addTab(vpc, s->name());
	uiTabWidget->setCurrentIndex(uiTabWidget->count()-1);
	
	setCurrentViewPortContainer(vpc);
	setCurrentViewPort(v);
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
	for (int i=0; i<_viewPortList.size(); i++)
		delete _viewPortList[i];
	_viewPortList.clear();
	setCurrentViewPort( 0 );
}

/*!
	Called when the current sheet is switched in the tab widget.
	\warning This method is only called when the index of the selected tab changes. If you remove the current tab and the next selected tab gets the same index, this slot isn't called!
*/
void CAMainWin::on_uiTabWidget_currentChanged(int idx) {
	setCurrentViewPortContainer( static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget()) );
	if (currentViewPortContainer())
		setCurrentViewPort( _currentViewPortContainer->lastUsedViewPort() );
	
	updateToolBars();
}

void CAMainWin::on_uiFullscreen_toggled(bool checked) {
	if (checked)
		this->showFullScreen();
	else
		this->showNormal();
}

void CAMainWin::on_uiSplitHorizontally_triggered() {
	CAViewPort *v = static_cast<CAViewPort*>(_currentViewPortContainer->splitHorizontally());
	if(!v)
		return;
	
	if (v->viewPortType() == CAViewPort::ScoreViewPort)
		initScoreViewPort(static_cast<CAScoreViewPort*>(v));
	
	uiUnsplitAll->setEnabled(true);
	uiCloseCurrentView->setEnabled(true);
	_viewPortList << v;
	setMode(_mode);	// updates the new viewport border settings
}

void CAMainWin::on_uiSplitVertically_triggered() {
	CAViewPort *v = static_cast<CAViewPort*>(_currentViewPortContainer->splitVertically());
	if(!v)
		return;
	
	if (v->viewPortType() == CAViewPort::ScoreViewPort)
		initScoreViewPort(static_cast<CAScoreViewPort*>(v));
	
	uiUnsplitAll->setEnabled(true);
	uiCloseCurrentView->setEnabled(true);
	_viewPortList << v;
	setMode(_mode);	// updates the new viewport border settings
}

void CAMainWin::doUnsplit(CAViewPort *v) {
	CAViewPort::CAViewPortType vpt = v?v->viewPortType():_currentViewPortContainer->lastUsedDockedViewPort()->viewPortType();
	v = _currentViewPortContainer->unsplit(v);
	if (!v) return;
	
	_viewPortList.removeAll(v);
	
	if(_currentViewPortContainer->dockedViewPortsList().size() == 1)
	{
		uiCloseCurrentView->setEnabled(false);
		uiUnsplitAll->setEnabled(false);
	}
	_currentViewPort = _currentViewPortContainer->dockedViewPortsList().back();
}

void CAMainWin::on_uiUnsplitAll_triggered() {
	QList<CAViewPort*> dockedViewPorts = _currentViewPortContainer->unsplitAll();
	for(QList<CAViewPort*>::iterator i = dockedViewPorts.begin(); i < dockedViewPorts.end(); i++)
		_viewPortList.removeAll(*i);
	uiCloseCurrentView->setEnabled(false);
	uiUnsplitAll->setEnabled(false);
	_currentViewPort = _currentViewPortContainer->dockedViewPortsList().back();
}

void CAMainWin::on_uiCloseCurrentView_triggered() {
	doUnsplit();
}

void CAMainWin::on_uiCloseDocument_triggered() {
	if ( CACanorus::mainWinCount(document()) == 1 ) {
			delete document();
	}
	setDocument( 0 );
	rebuildUI();
}

/*!
	Shows the current score in CanorusML syntax in a new or the current viewport.
*/
void CAMainWin::on_uiCanorusMLSource_triggered() {
	CASourceViewPort *v = new CASourceViewPort(document(), currentViewPort()->parent());
	currentViewPortContainer()->addViewPort(v);
	
	connect(v, SIGNAL(CACommit(CASourceViewPort*, QString)), this, SLOT(sourceViewPortCommit(CASourceViewPort*, QString)));
	
	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	v->setFocusPolicy(Qt::ClickFocus);
	v->setFocus();
	
	uiUnsplitAll->setEnabled(true);
	uiCloseCurrentView->setEnabled(true);
	_viewPortList << v;
	setMode(_mode);	// updates the new viewport border settings
}

void CAMainWin::on_uiNewViewport_triggered() {
	CAViewPort *v = _currentViewPortContainer->newViewPort(_currentViewPort);

	if (v->viewPortType() == CAViewPort::ScoreViewPort)
		initScoreViewPort(static_cast<CAScoreViewPort*>(v));

	_viewPortList << v;
	setMode(_mode);	// updates the new viewport border settings
}

/*!
	Initializes the newly created score viewport.
	Connects its signals to slots.
	Sets the icon, focus policy and sets the focus.
*/
void CAMainWin::initScoreViewPort(CAScoreViewPort *v) {
	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	
	connect( v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)),
	         this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)) );
	connect( v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)),
	        this, SLOT(viewPortMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)) );
	connect( v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)),
	        this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)) );
	connect( v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAViewPort *)),
	        this, SLOT(viewPortKeyPressEvent(QKeyEvent *, CAViewPort *)) );
	
	v->setFocusPolicy(Qt::ClickFocus);
	v->setFocus();
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

void CAMainWin::on_uiNewSheet_triggered() {
	// add a new empty sheet
	document()->addSheet(tr("Sheet%1").arg(QString::number(document()->sheetCount()+1)));
	CACanorus::rebuildUI(document());
}

/*!
	Adds a new voice to the staff.
*/
void CAMainWin::on_uiNewVoice_triggered() {
	CAStaff *staff = currentStaff();
	if (staff)
		staff->addVoice(new CAVoice(staff, staff->name() + tr("Voice%1").arg( staff->voiceCount()+1 )));
	
	uiVoiceNum->setMax(staff->voiceCount());
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
			voice->clear();
			voice->staff()->removeVoice(voice);
			uiVoiceNum->setMax( voice->staff()->voiceCount() );
			uiVoiceNum->setRealValue( voice->staff()->voiceCount() );
			CACanorus::rebuildUI(document(), currentSheet());
			delete voice;
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
			CASheet *sheet = context->sheet();
			sheet->removeContext(context);
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
					((CAScoreViewPort*)_viewPortList[i])->repaint();
				}
			}
			break;
		}
		case InsertMode: {
			QPen p;
			p.setColor(Qt::blue);
			p.setWidth(3);
			
			if (_musElementFactory->musElementType() == CAMusElement::Note)
				((CAScoreViewPort*)_currentViewPort)->setShadowNoteVisible(true);

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
					((CAScoreViewPort*)_viewPortList[i])->repaint();
				}
			}
		}
	}	//switch(mode)
	updateToolBars();
	currentViewPort()->setFocus();
}

/*!
	Rebuild the GUI from data.
	
	This method is called eg. when multiple viewports share the same data and a change has been made (eg. a
	note pitch has changed). ViewPorts content is repositioned and redrawn (CAEngraver creates CADrawable
	elements for every viewport).
	
	\a sheet argument is a pointer to the data sheet where the change occured. This way only viewports showing
	the given sheet are updated which speeds up the process.
	If no \a sheet argument is passed, the whole UI is rebuilt from the data part. This is called for eg. upon
	Opening file after the data part has been read or when creating a new document.
	
	If \a repaint is True (default) the rebuilt viewports are also repainted. If False, viewports content is
	only created but not yet drawn. This is useful when multiple operations which could potentially change the
	content are to happen and we want to actually draw it only at the end.
*/
void CAMainWin::rebuildUI(CASheet *sheet, bool repaint) {
	if (document()) {
		// save the current state of viewports
		QList<QRect> worldCoordsList;
		for (int i=0; i<_viewPortList.size(); i++)
			if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
				worldCoordsList << static_cast<CAScoreViewPort*>(_viewPortList[i])->worldCoords();
		 
		if (!sheet) {
			clearUI();
			for (int i=0; i<document()->sheetCount(); i++) {
				addSheet(document()->sheetAt(i));
				
				// restore the current state of viewports
				if ( _viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort &&
				     i < worldCoordsList.size() )
					static_cast<CAScoreViewPort*>(_viewPortList[i])->setWorldCoords(worldCoordsList[i]);
			}
		}
		
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
}

/*!
	Processes the mouse press event \a e with coordinates \a coords of the viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mousePressEvent(), viewPortMouseMoveEvent(), viewPortWheelEvent(), viewPortKeyPressEvent()
*/
void CAMainWin::viewPortMousePressEvent(QMouseEvent *e, const QPoint coords, CAViewPort *viewPort) {
	setCurrentViewPort( viewPort );
	_currentViewPortContainer->setLastUsedViewPort( currentViewPort() );
	
	if (viewPort->viewPortType() == CAViewPort::ScoreViewPort) {
		CAScoreViewPort *v = (CAScoreViewPort*)viewPort;
		
		CADrawableContext *currentContext = v->currentContext();
		
		v->selectCElement(coords.x(), coords.y());
		if ( v->selectMElement(coords.x(), coords.y()) ||	// select a music element at the given location - select none, if there's none there
		     v->currentContext() ) {
			// voice number widget
			if (currentContext != v->currentContext()) {	// new context was selected
				if (v->currentContext()->context()->contextType() == CAContext::Staff) {
					uiVoiceNum->setRealValue(0);
					uiVoiceNum->setMax(static_cast<CAStaff*>(v->currentContext()->context())->voiceCount());
				}
			}
			v->repaint();
		} else
		if (currentContext != v->currentContext()) { // no context selected
			if ( mode()==InsertMode ) // If in insert mode, stay in the current context
				v->setCurrentContext( currentContext );
			v->repaint();
		}
		
		if (e->modifiers()==Qt::ControlModifier) {
			CAMusElement *elt;
			if ( elt = v->removeMElement(coords.x(), coords.y()) ) {
				elt->context()->removeMusElement(elt, true);	//free the memory as well!
				CACanorus::rebuildUI(document(), v->sheet());
				return;
			}
		}
		
		switch (_mode) {
			case SelectMode:
			case EditMode: {
				if (v->selection().size()) {
					CAMusElement *elt = v->selection().front()->musElement();
					// debug
					std::cout << "musElement: " << elt << ", timeStart=" << elt->timeStart() << ", timeEnd=" << elt->timeEnd() << ", context=" << elt->context();
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
					CAContext* newContext;
					switch(uiContextType->currentId()) {
					case CAContext::Staff:
						v->sheet()->addContext(newContext = new CAStaff(v->sheet(), tr("Staff%1").arg(v->sheet()->staffCount()+1)));
						static_cast<CAStaff*>(newContext)->addVoice(new CAVoice(static_cast<CAStaff*>(newContext), newContext->name() + tr("Voice%1").arg(1)));
						break;
					case CAContext::FunctionMarkingContext:
						v->sheet()->addContext(newContext = new CAFunctionMarkingContext(v->sheet(), tr("Function marking context %1").arg(v->sheet()->contextCount()+1)));
						break;
					}
					CACanorus::rebuildUI(document(), v->sheet());
					
					v->selectContext(newContext);
					if (newContext->contextType()==CAContext::Staff) {
						uiVoiceNum->setMax( 1 );
						uiVoiceNum->setRealValue( 0 );
					}
					uiSelectMode->toggle();
					v->repaint();
					break;
				}
				
				// Insert music element
				if (uiInsertPlayable->isChecked()) {
					// Add Note/Rest
					if (e->button()==Qt::RightButton && _musElementFactory->musElementType()==CAMusElement::Note)
						// place a rest when using right mouse button and note insertion is selected
						_musElementFactory->setMusElementType( CAMusElement::Rest );
				}
				
				insertMusElementAt( coords, v, *_musElementFactory->getMusElement() );
				
				if (_musElementFactory->musElementType()==CAMusElement::Rest)
					_musElementFactory->setMusElementType( CAMusElement::Note );
				
				break;
			}
		}
		CAPluginManager::action("onScoreViewPortClick", document(), 0, 0, this);
	}
	updateToolBars();
}

/*!
	Processes the mouse move event \a e with coordinates \a coords of the viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mouseMoveEvent(), viewPortMousePressEvent(), viewPortWheelEvent(), viewPortKeyPressEvent()
*/
void CAMainWin::viewPortMouseMoveEvent(QMouseEvent *e, QPoint coords, CAViewPort *v) {
	if ((mode() == InsertMode) &&
	    (_musElementFactory->musElementType() == CAMusElement::Note) &&
	    (v->viewPortType()==CAViewPort::ScoreViewPort)
	   ) {
		CAScoreViewPort *c = (CAScoreViewPort*)v;
		CADrawableStaff *s;
		if (c->currentContext()?(c->currentContext()->drawableContextType() == CADrawableContext::DrawableStaff):0)
			s = (CADrawableStaff*)c->currentContext(); 
		else
			return;

		if (_musElementFactory->musElementType() == CAMusElement::Note || 
                    _musElementFactory->musElementType() == CAMusElement::Rest)
			c->setShadowNoteVisible(true);
		
		//calculate the logical pitch out of absolute world coordinates and the current clef
		int pitch = s->calculatePitch(coords.x(), coords.y());
		
		//write into the main window's status bar the note pitch name
		int iNoteAccs = s->getAccs(coords.x(), pitch)+_musElementFactory->noteExtraAccs();
		_musElementFactory->setNoteAccs( iNoteAccs );
		statusBar()->showMessage(CANote::generateNoteName(pitch, iNoteAccs));
		((CAScoreViewPort*)v)->setShadowNoteAccs(iNoteAccs);
	}
}

/*!
	Processes the mouse wheel event \a e with coordinates \a coords of the viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::wheelEvent(), viewPortMousePressEvent(), viewPortMouseMoveEvent(), viewPortKeyPressEvent()
*/
void CAMainWin::viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *v) {
	_currentViewPort = v;
	if (v->viewPortType()==CAViewPort::ScoreViewPort) {
		CAScoreViewPort *sv = static_cast<CAScoreViewPort*>(v);
		
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
		
		v->repaint();
	}
}

/*!
	Processes the key press event \a e of the viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::keyPressEvent(), viewPortMousePressEvent(), viewPortMouseMoveEvent(), viewPortWheelEvent()
*/
void CAMainWin::viewPortKeyPressEvent(QKeyEvent *e, CAViewPort *v) {
	_currentViewPort = v;
	
	if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort) {
		switch (e->key()) {
			//Music editing keys
			case Qt::Key_Right: {
				//select next music element
				((CAScoreViewPort*)_currentViewPort)->selectNextMusElement();
				_currentViewPort->repaint();
				break;
			}
			
			case Qt::Key_Left: {
				//select previous music element
				((CAScoreViewPort*)_currentViewPort)->selectPrevMusElement();
				_currentViewPort->repaint();
				break;
			}
			
			case Qt::Key_B: {
				//place a barline
				CADrawableContext *drawableContext;
				drawableContext = ((CAScoreViewPort*)v)->currentContext();
				
				if ( (!drawableContext) || (drawableContext->context()->contextType() != CAContext::Staff) )
					return;
			
				CAStaff *staff = (CAStaff*)drawableContext->context();
				CAMusElement *left = 0;
				if (!((CAScoreViewPort*)v)->selection().isEmpty())
					left = ((CAScoreViewPort*)v)->selection().back()->musElement();
					
				CABarline *bar = new CABarline(
					CABarline::Single,
					staff,
					(left?left->timeEnd():staff->lastTimeEnd())
				);
				staff->insertSignAfter(bar, left, true);	//insert the barline in all the voices
				
				CACanorus::rebuildUI(document(), ((CAScoreViewPort*)v)->sheet());
				((CAScoreViewPort*)v)->selectMElement(bar);
				v->repaint();
				break;
			}
			
			case Qt::Key_Up: {
				if (mode() == SelectMode) {	//select the upper music element
					((CAScoreViewPort*)_currentViewPort)->selectUpMusElement();
					_currentViewPort->repaint();
				} else if ((mode() == InsertMode) || (mode() == EditMode)) {
					if (!((CAScoreViewPort*)_currentViewPort)->selection().isEmpty()) {
						CADrawableMusElement *elt =
							((CAScoreViewPort*)_currentViewPort)->selection().back();
						
						//pitch note for one step higher
						if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
							CANote *note = (CANote*)elt->musElement();
							note->setPitch(note->pitch()+1);
							CACanorus::rebuildUI(document(), note->voice()->staff()->sheet());
						}
					}
				}
				break;
			}
			
			case Qt::Key_Down: {
				if (mode() == SelectMode) {	//select the upper music element
					((CAScoreViewPort*)_currentViewPort)->selectUpMusElement();
					_currentViewPort->repaint();
				} else if ((mode() == InsertMode) || (mode() == EditMode)) {
					if (!((CAScoreViewPort*)_currentViewPort)->selection().isEmpty()) {
						CADrawableMusElement *elt =
							((CAScoreViewPort*)_currentViewPort)->selection().back();
						
						//pitch note for one step higher
						if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
							CANote *note = (CANote*)elt->musElement();
							note->setPitch(note->pitch()-1);
							CACanorus::rebuildUI(document(), note->voice()->staff()->sheet());
						}
					}
				}
				break;
			}
			case Qt::Key_Plus: {
				if (v->viewPortType()==CAViewPort::ScoreViewPort) {
					if (mode()==InsertMode) {
						_musElementFactory->addNoteExtraAccs(1); _musElementFactory->addNoteAccs(1);
						((CAScoreViewPort*)v)->setDrawShadowNoteAccs(_musElementFactory->noteExtraAccs()!=0);
						((CAScoreViewPort*)v)->setShadowNoteAccs(_musElementFactory->noteAccs());
						v->repaint();
					} else if (mode()==EditMode) {
						if (!((CAScoreViewPort*)v)->selection().isEmpty()) {
							CAMusElement *elt = ((CAScoreViewPort*)v)->selection().front()->musElement();
							if (elt->musElementType()==CAMusElement::Note) {
								((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()+1);
								CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
							}
						}
					}
				}
				break;
			}
			
			case Qt::Key_Minus: {
				if (v->viewPortType()==CAViewPort::ScoreViewPort) {
					if (mode()==InsertMode) {
						_musElementFactory->subNoteExtraAccs(1); _musElementFactory->subNoteAccs(1);
						((CAScoreViewPort*)v)->setDrawShadowNoteAccs(_musElementFactory->noteExtraAccs()!=0);
						((CAScoreViewPort*)v)->setShadowNoteAccs(_musElementFactory->noteAccs());
						v->repaint();
					} else if (mode()==EditMode) {
						if (!((CAScoreViewPort*)v)->selection().isEmpty()) {
							CAMusElement *elt = ((CAScoreViewPort*)v)->selection().front()->musElement();
							if (elt->musElementType()==CAMusElement::Note) {
								((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()-1);
								CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
							}
						}
					}
				}
				break;
			}
			
			case Qt::Key_Period: {
				if (v->viewPortType()==CAViewPort::ScoreViewPort) {
					if (mode()==InsertMode) {
						_musElementFactory->addPlayableDotted( 1 );
						((CAScoreViewPort*)v)->setShadowNoteDotted(_musElementFactory->playableDotted());
						v->repaint();
					} else if (mode()==EditMode) {
						if (!((CAScoreViewPort*)v)->selection().isEmpty()) {
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
								CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
							}
						}
					}
				}
				break;
			}
			
			case Qt::Key_Delete:
				if (!((CAScoreViewPort*)_currentViewPort)->selection().isEmpty()) {
					CAMusElement *elt = ((CAScoreViewPort*)_currentViewPort)->selection().back()->musElement();
					if (elt->context()->contextType() == CAContext::Staff) {
						((CAStaff*)elt->context())->removeMusElement(elt);
						CACanorus::rebuildUI(document(), ((CAScoreViewPort*)_currentViewPort)->sheet());
					}
				}
				
				break;
			
			//Mode keys
			case Qt::Key_Escape:
				if ((mode()==SelectMode) && (_currentViewPort) && (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)) {
					((CAScoreViewPort*)_currentViewPort)->clearMSelection();
					((CAScoreViewPort*)_currentViewPort)->clearCSelection();
				}
				uiSelectMode->toggle();
				uiVoiceNum->setRealValue(0);
				//if (uiKeySigPSP)
				//	uiKeySigPSP->hide();
				break;
			case Qt::Key_I:
				_musElementFactory->setMusElementType( CAMusElement::Note );
				setMode(InsertMode);
				break;
			case Qt::Key_E:
				setMode(EditMode);
				break;
		}
	}
	updateToolBars();
}

/*!
	This method places the already created music element (directly or via CAMusElementFactory) to the staff or
	voice, dependent on the music element type and the viewport coordinates.
*/
void CAMainWin::insertMusElementAt(const QPoint coords, CAScoreViewPort *v, CAMusElement &roMusElement ) {
	CADrawableContext *context = v->currentContext();
	
	CAStaff *staff=0;
	CADrawableStaff *drawableStaff=0;
	bool success;
	int iPlayableDotted = 0;
	
	if (!context)
		return;
	
	switch ( roMusElement.musElementType() ) {
		case CAMusElement::Clef: {
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y());
			success = _musElementFactory->configureClef( context, left );
			break;
		}
		case CAMusElement::KeySignature: {
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y());
			success = _musElementFactory->configureKeySignature( context, left );
			break;
		}
		case CAMusElement::TimeSignature: {
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y());
			success = _musElementFactory->configureTimeSignature( context, left );
			break;
		}
		case CAMusElement::Barline: {
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y());
			success = _musElementFactory->configureBarline( context, left );
			break;
		}
		case CAMusElement::Note: { // Do we really need to do all that here??
			int iVoiceNum = uiVoiceNum->getRealValue()-1<0?0:uiVoiceNum->getRealValue()-1;
			drawableStaff = (CADrawableStaff*)context;
			staff = drawableStaff->staff();
			CAVoice *voice = staff->voiceAt( iVoiceNum );
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y(), voice);
			success = _musElementFactory->configureNote( voice, coords, context, left );
			if (success) { 
				_musElementFactory->setNoteExtraAccs( 0 );
				v->setDrawShadowNoteAccs(false); 
				iPlayableDotted = ((CAPlayable *)(_musElementFactory->getMusElement()))->dotted();
			}
			break;
		}
		case CAMusElement::Rest: {
			int iVoiceNum = uiVoiceNum->getRealValue()-1<0?0:uiVoiceNum->getRealValue()-1;
			drawableStaff = (CADrawableStaff*)context;
			staff = drawableStaff->staff();
			CAVoice *voice = staff->voiceAt( iVoiceNum );
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y(), voice);
			success = _musElementFactory->configureRest( voice, coords, context, left );
			if (success)
				iPlayableDotted = ((CAPlayable *)(_musElementFactory->getMusElement()))->dotted();
			break;
		}
	}
	
	if (success) {
		CACanorus::rebuildUI(document(), v->sheet());
		v->selectMElement( _musElementFactory->getMusElement() );
		v->setShadowNoteDotted(iPlayableDotted);
		v->repaint();
	} else
	  {
		_musElementFactory->removeMusElem( true );
		_musElementFactory->createMusElem(); // Factory always must have a valid element
	  }
}

/*!
	Main window's key press event.
	
	\sa viewPortKeyPressEvent()
*/
void CAMainWin::keyPressEvent(QKeyEvent *e) {
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
	
	setMode(_mode);
}

/*!
	Connected with the play button which starts the playback.
*/
void CAMainWin::on_uiPlayFromSelection_toggled(bool checked) {
	if (checked && (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)) {
		CACanorus::midiDevice()->openOutputPort(CACanorus::midiOutPort());
		_repaintTimer = new QTimer();
		_repaintTimer->setInterval(100);
		_repaintTimer->start();
		//connect(_repaintTimer, SIGNAL(timeout()), this, SLOT(on_repaintTimer_timeout())); //TODO: timeout is connected directly to repaint() directly. This should be optimized in the future -Matevz
		connect(_repaintTimer, SIGNAL(timeout()), _currentViewPort, SLOT(repaint()));
		_playbackViewPort = _currentViewPort;
		
		_playback = new CAPlayback((CAScoreViewPort*)_currentViewPort, CACanorus::midiDevice());
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
	if (document() && (s=document()->fileName()).isEmpty())
		on_uiSaveDocumentAs_triggered();
		
	saveDocument(s);
}

void CAMainWin::on_uiSaveDocumentAs_triggered() {
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

/*!
	Opens a document with the given absolute fileName.
	Previous document will be lost.
*/
bool CAMainWin::openDocument(QString fileName) {
	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		if (CACanorus::mainWinCount(document())==1)
			delete document();
		
		QXmlInputSource input(&file);
		CADocument *openedDoc = CACanorusML::openDocument(&input, this);
		if (openedDoc)
			setDocument(openedDoc);
		
		openedDoc->setFileName(fileName);
		file.close();
		rebuildUI(); // local rebuild only
		uiTabWidget->setCurrentIndex(0);
		
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
		CACanorusML::saveDocument(document(), out);
		document()->setFileName(fileName);
		file.close();
		
		return true;
	} else
		return false;
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
			// TODO: call appropriate built-in export function here
			// eg. CALilyExport::exportDocument(out, &_document);
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
			// TODO: call appropriate built-in import function here
			// eg. CALilyExport::importDocument(in, &_document);
			file.close();
		}
	}              
}

/*!
	Called when a user changes the current voice number.
*/
void CAMainWin::on_uiVoiceNum_valChanged(int voiceNr) {
	updateVoiceToolBar();
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
			CAKeySignature *keySig = dynamic_cast<CAKeySignature*>(v->selection().at(0)->musElement());
			if ( keySig ) {
				keySig->setKeySignatureType( CAKeySignature::MajorMinor, accs, CAKeySignature::Major );
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
	if (voice)
		voice->setName(uiVoiceName->text());
}

void CAMainWin::on_uiInsertPlayable_toggled(bool checked) {
	if (checked) {
		setMode(InsertMode);
		if (!uiVoiceNum->getRealValue())
			uiVoiceNum->setRealValue( 1 ); // select the first voice if none selected
		
		_musElementFactory->setMusElementType( CAMusElement::Note );
	}
}

void CAMainWin::on_uiPlayableLength_toggled(bool checked, int buttonId) {
	// Read currently selected entry from tool button menu
	enum CAPlayable::CAPlayableLength length =
		static_cast<CAPlayable::CAPlayableLength>(buttonId);
		
	// New note length type
	_musElementFactory->setPlayableLength( length );
}

void CAMainWin::on_uiClefType_toggled(bool checked, int buttonId) {
	// Read currently selected entry from tool button menu
	enum CAClef::CAClefType clefType =
		static_cast<CAClef::CAClefType>(buttonId);
		
	_musElementFactory->setMusElementType( CAMusElement::Clef );
	
	// New clef type
	_musElementFactory->setClef( clefType );
	
	if ( checked )
		setMode( InsertMode );
}

void CAMainWin::on_uiTimeSigBeats_valChanged(int beats) {
	if (mode()==InsertMode) {
		_musElementFactory->setTimeSigBeats( beats );
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
		setMode(InsertMode);
		
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
		setMode(InsertMode);
		_musElementFactory->setMusElementType( CAMusElement::KeySignature );
	}
}

void CAMainWin::on_uiBarlineType_toggled(bool checked, int buttonId) {
	if (checked) {
		setMode(InsertMode);
		_musElementFactory->setMusElementType( CAMusElement::Barline );
		_musElementFactory->setBarlineType( static_cast<CABarline::CABarlineType>(buttonId) );
	}
}

void CAMainWin::sourceViewPortCommit(CASourceViewPort *v, QString inputString) {
	if (v->document()) {
		delete document();
		
		QXmlInputSource input;
		input.setData(inputString);
		setDocument(CACanorusML::openDocument(&input, this));
	} else
	if (v->voice()) {
		v->voice()->clear();
		
		CALilyPondImport(inputString, v->voice());
	}
	
	CACanorus::rebuildUI(document());
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
	if ( context &&
	     context->contextType()==CAContext::Staff ) {
		CAStaff *staff = static_cast<CAStaff*>(static_cast<CAScoreViewPort*>(currentViewPort())->currentContext()->context());
		int voiceNum = uiVoiceNum->getRealValue()-1<0?0:uiVoiceNum->getRealValue()-1;
		CAVoice *voice = staff->voiceAt( voiceNum );
		CASourceViewPort *v = new CASourceViewPort(voice, currentViewPort()->parent());
		currentViewPortContainer()->addViewPort(v);
		
		connect(v, SIGNAL(CACommit(CASourceViewPort*, QString)), this, SLOT(sourceViewPortCommit(CASourceViewPort*, QString)));
		
		v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
		v->setFocusPolicy(Qt::ClickFocus);
		v->setFocus();
		
		uiUnsplitAll->setEnabled(true);
		uiCloseCurrentView->setEnabled(true);
		_viewPortList << v;
		setMode(mode());	// updates the new viewport border settings
	}
}

/*!
	Removes the sheet, all its contents and rebuilds the GUI.
*/
void CAMainWin::on_uiRemoveSheet_triggered() {
	CASheet *sheet = currentSheet();
	if (sheet) {
		document()->removeSheet(currentSheet());
		CACanorus::rebuildUI(document());
		delete sheet;
	}
}

void CAMainWin::on_uiSheetName_returnPressed() {
	CASheet *sheet = currentSheet();
	if (sheet) {
		sheet->setName( uiSheetName->text() );
		uiTabWidget->setTabText(uiTabWidget->currentIndex(), sheet->name());
	}
}

/*!
	Sets the current context name.
*/
void CAMainWin::on_uiContextName_returnPressed() {
	CAContext *context = currentContext();
	if (context)
		context->setName(uiContextName->text());
}

/*!
	Sets the number of lines in the staff.
*/
void CAMainWin::on_uiStaffNumberOfLines_valueChanged(int lines) {
	CAStaff *staff = currentStaff();
	if (staff) {
		staff->setNumberOfLines(lines);
		CACanorus::rebuildUI(document(), currentSheet());
	}
}

void CAMainWin::on_uiVoiceStemDirection_toggled(bool checked, int direction) {
	CAVoice *voice = currentVoice();
	if (voice) {
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
		CAScoreViewPort *v = currentScoreViewPort();
		if ( v && v->selection().size() ) {
			CANote *note = dynamic_cast<CANote*>(v->selection().at(0)->musElement());
			if ( note ) {
				note->setStemDirection( direction );
				CACanorus::rebuildUI(document(), currentSheet());
			}
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
			int voiceNr = uiVoiceNum->getRealValue();
			if (voiceNr) {
				CAVoice *curVoice = staff->voiceAt(voiceNr-1);
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
			case CAContext::Staff:
				uiStaffNumberOfLines->setValue(static_cast<CAStaff*>(context)->numberOfLines());
				uiStaffNumberOfLines->setVisible(true);
				break;
			case CAContext::FunctionMarkingContext:
				uiStaffNumberOfLines->setVisible(false);
				break;
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
						uiInsertClef->setVisible(true); // menu
						uiInsertBarline->setVisible(true); // menu
						uiClefType->setVisible(true);
						uiTimeSigType->setVisible(true);
						uiInsertKeySig->setVisible(true);
						uiInsertTimeSig->setVisible(true);
						uiBarlineType->setVisible(true);
						uiInsertFM->setVisible(false);
						break;
					case CAContext::FunctionMarkingContext:
						// function marking context selected
						uiInsertPlayable->setVisible(false);
						uiInsertClef->setVisible(false); // menu
						uiInsertBarline->setVisible(false); // menu
						uiClefType->setVisible(false);
						uiTimeSigType->setVisible(false);
						uiInsertKeySig->setVisible(false);
						uiInsertTimeSig->setVisible(false);
						uiBarlineType->setVisible(false);
						uiInsertFM->setVisible(true);
						break;
				}
			} else {
				// no contexts selected
				uiInsertPlayable->setVisible(false);
				uiInsertClef->setVisible(false); // menu
				uiInsertBarline->setVisible(false); // menu
				uiClefType->setVisible(false);
				uiTimeSigType->setVisible(false);
				uiInsertKeySig->setVisible(false);
				uiInsertTimeSig->setVisible(false);
				uiBarlineType->setVisible(false);
				uiInsertFM->setVisible(false);
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
	if (uiInsertPlayable->isChecked() && mode()==InsertMode) {
		uiPlayableLength->defaultAction()->setEnabled(true);
		uiPlayableLength->setCurrentId( _musElementFactory->playableLength() );
		uiPlayableDotted->setText( "0" ); //QString(_musElementFactory->playableDotted()) );
		uiNoteStemDirection->setCurrentId( _musElementFactory->noteStemDirection() );
		uiHiddenRest->setChecked(_musElementFactory->restType()==CARest::Hidden);
		uiPlayableToolBar->show();
	} else if (mode()==EditMode) {
		CAScoreViewPort *v = currentScoreViewPort();
		if (v && v->selection().size()) {
			CAPlayable *playable = dynamic_cast<CAPlayable*>(v->selection().at(0)->musElement());
			if (playable) {
				uiPlayableLength->defaultAction()->setEnabled(false);
				uiPlayableLength->setCurrentId( playable->playableLength() );
				uiPlayableDotted->setText( "0" ); //QString(playable->dotted()) );
				if (playable->musElementType()==CAMusElement::Note) {
					CANote *note = static_cast<CANote*>(playable);
					uiNoteStemDirection->setCurrentId( note->stemDirection() );
				} else if (playable->musElementType()==CAMusElement::Rest) {
					CARest *rest = static_cast<CARest*>(playable);
					uiHiddenRest->setChecked(rest->restType()==CARest::Hidden);
				}
				uiPlayableToolBar->show();
			} else
				uiPlayableToolBar->hide();
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
