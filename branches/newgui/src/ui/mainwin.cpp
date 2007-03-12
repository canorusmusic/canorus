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
	
	setDocument(NULL);
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
	delete uiPlayableToolBar; delete uiPlayableLength; delete uiPlayableDotted; delete uiNoteAccs; delete uiNoteAccsVisible; delete uiNoteStemDirection; delete uiHiddenRest; delete uiRestWholeBar;
	delete uiKeySigToolBar; delete uiKeySigPSP; delete uiKeySigNumberOfAccs; delete uiKeySigGender;
	delete uiTimeSigToolBar; delete uiTimeSigBeats; delete uiTimeSigSlash; delete uiTimeSigBeat; delete uiTimeSigStyle;
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
		uiInsertToolBar->addSeparator();
		uiInsertToolBar->addAction( uiInsertPlayable );
		uiInsertToolBar->addWidget(uiClefType = new CAMenuToolButton( tr("Select Clef"), 3, this ));
			uiClefType->addButton( QIcon(":/menu/images/cleftreble.png"), CAClef::Treble );
			uiClefType->addButton( QIcon(":/menu/images/clefbass.png"), CAClef::Bass );
			uiClefType->addButton( QIcon(":/menu/images/clefalto.png"), CAClef::Alto );
			uiClefType->setCurrentId( CAClef::Treble );
			connect( uiClefType, SIGNAL( toggled(bool, int) ), this, SLOT( on_uiClefType_toggled(bool, int) ) );
		uiInsertToolBar->addWidget(uiTimeSigType = new CAMenuToolButton( tr("Select Time Signature" ), 3, this ));
			uiTimeSigType->addButton( QIcon(":/menu/images/tsc.png"), TS_44 );
			uiTimeSigType->addButton( QIcon(":/menu/images/tsab.png"), TS_22 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts34.png"), TS_34 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts24.png"), TS_24 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts38.png"), TS_38 );
			uiTimeSigType->addButton( QIcon(":/menu/images/ts68.png"), TS_68 );
			uiTimeSigType->setCurrentId( TS_44 );
		uiInsertToolBar->addWidget( uiBarlineType = new CAMenuToolButton( tr("Select Barline" ), 5, this ));
			uiBarlineType->addButton( QIcon(":/menu/images/delete.png"), 0 );
			uiBarlineType->setCurrentId( 0 );
		uiInsertToolBar->addAction( uiInsertFM );
		addToolBar(Qt::TopToolBarArea, uiInsertToolBar);
	
	uiVoiceToolBar = new QToolBar( tr("Voice ToolBar"), this );
		uiVoiceToolBar->addWidget( uiVoiceNum = new CALCDNumber( 0, 20, 0, "Voice number" ) );
		connect( uiVoiceNum, SIGNAL( valChanged( int ) ), this,
		         SLOT(on_uiVoiceNum_valChanged( int ) ) );
		uiVoiceNum->setEnabled(false);	//current voice number gets enabled when staff is selected and gets disabled when staff is unselected. By default, it's disabled.
		addToolBar(Qt::TopToolBarArea, uiVoiceToolBar);
		
	uiPlayableToolBar = new QToolBar( tr("Playable ToolBar"), this );
		uiPlayableToolBar->addWidget(uiPlayableLength = new CAMenuToolButton( tr("Select Length" ), 4, this ));
			uiPlayableLength->addButton( QIcon(":/menu/images/n0.png"), CANote::Breve );
			uiPlayableLength->addButton( QIcon(":/menu/images/n1.png"), CANote::Whole );
			uiPlayableLength->addButton( QIcon(":/menu/images/n2.png"), CANote::Half );
			uiPlayableLength->addButton( QIcon(":/menu/images/n4.png"), CANote::Quarter );
			uiPlayableLength->addButton( QIcon(":/menu/images/n8.png"), CANote::Eighth );
			uiPlayableLength->addButton( QIcon(":/menu/images/n16.png"), CANote::Sixteenth );
			uiPlayableLength->addButton( QIcon(":/menu/images/n32.png"), CANote::ThirtySecond );
			uiPlayableLength->addButton( QIcon(":/menu/images/n64.png"), CANote::SixtyFourth );
			uiPlayableLength->setCurrentId( CANote::Quarter );
		addToolBar(Qt::TopToolBarArea, uiPlayableToolBar);
	
	uiKeySigPSP  = 0;
	
	// Mutual exclusive groups
	uiInsertGroup = new QActionGroup( this );
	uiInsertGroup->addAction( uiSelectMode );
	uiInsertGroup->addAction( uiNewContext );
	//uiInsertGroup->addAction( uiContextType->defaultAction() );
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
	
	uiInsertToolBar->show();
	uiPlayableToolBar->hide();
}

void CAMainWin::newDocument() {
	// clear the logical part
	if (CACanorus::mainWinCount(document()))
		delete document();
	
	// clear the UI part
	clearUI();

	setDocument(new CADocument());
	
#ifdef USE_PYTHON
	QList<PyObject*> argsPython;
	argsPython << CASwigPython::toPythonObject(document(), CASwigPython::Document);
	CASwigPython::callFunction(CACanorus::locateResource("scripts/newdocument.py").at(0), "newDefaultDocument", argsPython);
#endif
	
	// call local rebuild only because no other main windows share the new document
	rebuildUI();
}

/*!
	Adds an already created \a sheet to the document.
	This adds a tab to tabWidget and creates a single score viewport of the sheet.
*/
void CAMainWin::addSheet(CASheet *s) {
	CAScoreViewPort *v = new CAScoreViewPort(s, 0);
	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	connect(v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAViewPort *)), this, SLOT(viewPortKeyPressEvent(QKeyEvent *, CAViewPort *)));
	v->setFocusPolicy(Qt::ClickFocus);
	v->setFocus();
	
	_viewPortList << v;
	
	CAViewPortContainer *vpc = new CAViewPortContainer(v, 0);
	_viewPortContainerList << vpc;
	
	uiTabWidget->addTab(vpc, s->name());
	uiTabWidget->setCurrentIndex(uiTabWidget->count()-1);
	
	_currentViewPortContainer = vpc;
	_currentViewPort = v;
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
	for (int i=0; i<_viewPortList.size(); i++)
		delete _viewPortList[i];
	
	_viewPortList.clear();
	
	while (uiTabWidget->count()) {
		delete _currentViewPortContainer;
		uiTabWidget->removeTab(uiTabWidget->currentIndex());
	}
	
	uiVoiceNum->setEnabled(false);
}

/*!
	Called when the current sheet is switched in the tab widget.
*/
void CAMainWin::on_uiTabWidget_currentChanged(int idx) {
	_currentViewPortContainer = static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget());
	_currentViewPort = _currentViewPortContainer->lastUsedViewPort();
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
	
	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	if (v->viewPortType() == CAViewPort::ScoreViewPort) {
		connect((CAScoreViewPort*)v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAViewPort *)), this, SLOT(viewPortKeyPressEvent(QKeyEvent *, CAViewPort *)));
		v->setFocusPolicy(Qt::ClickFocus);
		v->setFocus();
	}
	
	uiUnsplitAll->setEnabled(true);
	uiCloseCurrentView->setEnabled(true);
	_viewPortList << v;
	setMode(_mode);	// updates the new viewport border settings
}

void CAMainWin::on_uiSplitVertically_triggered() {
	CAViewPort *v = static_cast<CAViewPort*>(_currentViewPortContainer->splitVertically());
	if(!v)
		return;
	
	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	if (v->viewPortType() == CAViewPort::ScoreViewPort) {
		connect((CAScoreViewPort*)v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAViewPort *)), this, SLOT(viewPortKeyPressEvent(QKeyEvent *, CAViewPort *)));
		v->setFocusPolicy(Qt::ClickFocus);
		v->setFocus();
	}
	
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

void CAMainWin::on_uiViewCanorusMLSource_triggered() {
	CASourceViewPort *v = new CASourceViewPort(document(), _currentViewPort->parent());
	_currentViewPortContainer->addViewPort(v);
	
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

	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	if (v->viewPortType() == CAViewPort::ScoreViewPort) {
		connect((CAScoreViewPort*)v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAViewPort *)), this, SLOT(viewPortKeyPressEvent(QKeyEvent *, CAViewPort *)));
		v->setFocusPolicy(Qt::ClickFocus);
		v->setFocus();
	}

	_viewPortList << v;
	setMode(_mode);	// updates the new viewport border settings
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
	addSheet(document()->addSheet(tr("Sheet %1").arg(QString::number(document()->sheetCount()+1))));
}

void CAMainWin::on_uiNewVoice_triggered() {
}

void CAMainWin::on_uiContextType_toggled(bool checked, int buttonId) {
	std::cout << "uiContextType_toggled" << std::endl;
/*	if (_currentViewPort->viewPortType() != CAViewPort::ScoreViewPort)
		return;
	
	CASheet *sheet = ((CAScoreViewPort*)_currentViewPort)->sheet();
	CAStaff *staff = sheet->addStaff();
	staff->addVoice(new CAVoice( staff, tr("Voice %1").arg(QString::number(1)) ));
	
	CACanorus::rebuildUI(document(), sheet);
	
	((CAScoreViewPort*)_currentViewPort)->selectContext(staff);
	((CAScoreViewPort*)_currentViewPort)->repaint(); */
}

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
}

void CAMainWin::on_uiInsertClef_toggled(bool) {
	std::cout << "uiInsertClef_triggered" << std::endl;
	setMode(InsertMode);
	_musElementFactory->setMusElementType( CAMusElement::Clef );
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
	if (!sheet) {
		clearUI();
		for (int i=0; i<document()->sheetCount(); i++)
			addSheet(document()->sheetAt(i));
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
}

/*!
	Processes the mouse press event \a e with coordinates \a coords of the viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mousePressEvent(), viewPortMouseMoveEvent(), viewPortWheelEvent(), viewPortKeyPressEvent()
*/
void CAMainWin::viewPortMousePressEvent(QMouseEvent *e, const QPoint coords, CAViewPort *viewPort) {
	_currentViewPort = viewPort;
	_currentViewPortContainer->setLastUsedViewPort(_currentViewPort);
	
	if (viewPort->viewPortType() == CAViewPort::ScoreViewPort) {
		CAScoreViewPort *v = (CAScoreViewPort*)viewPort;
		
		CADrawableContext *currentContext = v->currentContext();
		
		v->selectCElement(coords.x(), coords.y());
		if ( v->selectMElement(coords.x(), coords.y()) ||	// select a music element at the given location - select none, if there's none there
		     v->selectCElement(coords.x(), coords.y()) ) {
			// voice number widget
			if (currentContext != v->currentContext()) {	// new context was selected
				if (v->currentContext()->context()->contextType() == CAContext::Staff) {
					uiVoiceNum->setEnabled(true);
					uiVoiceNum->setRealValue(0);
					uiVoiceNum->setMax(((CAStaff*)v->currentContext()->context())->voiceCount());
				} else
					uiVoiceNum->setEnabled(false);
			}
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
				if (!v->selection()->isEmpty()) {
					CAMusElement *elt = v->selection()->front()->musElement();
					// debug
					std::cout << "musElement: " << elt << ", timeStart=" << elt->timeStart() << ", timeEnd=" << elt->timeEnd();
					if (elt->isPlayable())
						std::cout << ", voice=" << ((CAPlayable*)elt)->voice() << ", voiceNr=" << ((CAPlayable*)elt)->voice()->voiceNumber() << ", idxInVoice=" << ((CAPlayable*)elt)->voice()->indexOf(elt);
					std::cout << std::endl;
				}
				break;
			}
			case InsertMode: {
				if (e->button()==Qt::RightButton && _musElementFactory->musElementType()==CAMusElement::Note)
					//place a rest when using right mouse button and note insertion is selected
					_musElementFactory->setMusElementType( CAMusElement::Rest );
				insertMusElementAt( coords, v, *_musElementFactory->getMusElement() );
				if (_musElementFactory->musElementType()==CAMusElement::Rest)
					_musElementFactory->setMusElementType( CAMusElement::Note );
				break;
			}
		}
		CAPluginManager::action("onScoreViewPortClick", document(), 0, 0, this);
	}
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
				if (!((CAScoreViewPort*)v)->selection()->isEmpty())
					left = ((CAScoreViewPort*)v)->selection()->back()->musElement();
					
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
					if (!((CAScoreViewPort*)_currentViewPort)->selection()->isEmpty()) {
						CADrawableMusElement *elt =
							((CAScoreViewPort*)_currentViewPort)->selection()->back();
						
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
					if (!((CAScoreViewPort*)_currentViewPort)->selection()->isEmpty()) {
						CADrawableMusElement *elt =
							((CAScoreViewPort*)_currentViewPort)->selection()->back();
						
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
						if (!((CAScoreViewPort*)v)->selection()->isEmpty()) {
							CAMusElement *elt = ((CAScoreViewPort*)v)->selection()->front()->musElement();
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
						if (!((CAScoreViewPort*)v)->selection()->isEmpty()) {
							CAMusElement *elt = ((CAScoreViewPort*)v)->selection()->front()->musElement();
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
						if (!((CAScoreViewPort*)v)->selection()->isEmpty()) {
							CAMusElement *elt = ((CAScoreViewPort*)v)->selection()->front()->musElement();
							
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
				if (!((CAScoreViewPort*)_currentViewPort)->selection()->isEmpty()) {
					CAMusElement *elt = ((CAScoreViewPort*)_currentViewPort)->selection()->back()->musElement();
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
				setMode(SelectMode);
				uiVoiceNum->setRealValue(0);
				if (uiKeySigPSP)
					uiKeySigPSP->hide();
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
}

/*!
	This method places the already created music element (directly or via CAMusElementFactory) to the staff or
	voice, dependent on the music element type and the viewport coordinates.
*/
void CAMainWin::insertMusElementAt(const QPoint coords, CAScoreViewPort *v, CAMusElement &roMusElement ) {
	CADrawableContext *context = v->selectCElement(coords.x(), coords.y());
	
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
			success = _musElementFactory->configureKeySignature(uiKeySigPSP->getKeySignature()-7,
			                                               context, left );
			break;
		}
/*		case CAMusElement::TimeSignature: { /// \todo Move TimeSigPSP code
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y());
			if( mpoTimeSigPSP ) // Change via perspective as well
			{
				int iTimeSigBeats, iTimeSigBeat;
				_uiTimeSigPSP->getTimeSignature( iTimeSigBeats, iTimeSigBeat );
				_musElementFactory->setTimeSigBeats( iTimeSigBeats );
				_musElementFactory->setTimeSigBeat( iTimeSigBeat );
			}
			success = _musElementFactory->configureTimeSignature( context, left );
			break;
		} */
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

/*void CAMainWin::setKeySigPSPVisible( bool bVisible ) {
	if (!uiKeySigPSP) {
		uiKeySigPSP = new CAKeySigPSP(tr("Edit key signature"), this);
		addDockWidget( Qt::LeftDockWidgetArea, mpoKeySigPSP );
	}
	uiKeySigPSP->setVisible( bVisible );
}*/	

/*void CAMainWin::setTimeSigPSPVisible( bool bVisible ) {
	if (!mpoTimeSigPSP) {
		mpoTimeSigPSP = new CATimeSigPSP("Edit time signature", this);
		addDockWidget( Qt::LeftDockWidgetArea, mpoTimeSigPSP );
		connect( mpoTimeSigPSP, SIGNAL( timeSigChanged( int, int ) ),
                         this, SLOT( sl_mpoTimeSig_valChanged( int, int ) ) );
	}
	mpoTimeSigPSP->setVisible( bVisible );
}	*/

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
		clearUI();
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

void CAMainWin::on_uiVoiceNum_valChanged(int voiceNr) {
	std::cout << "New voice number:" << voiceNr << std::endl;
}

/*void CAMainWin::sl_mpoTimeSig_valChanged(int iBeats, int iBeat)
{
	_musElementFactory->setMusElementType( CAMusElement::TimeSignature );
	_musElementFactory->setTimeSigBeats( iBeats );
	_musElementFactory->setTimeSigBeat( iBeat );
	CAFixedTimeSig eElem = mpoTimeSigPSP->getTimeSignatureFixed();
	if( eElem != TS_UNKNOWN )
		mpoMEToolBar->changeMenuIcon( mpoTimeSigMenu->getButton( (int)eElem ) );
	printf("Time signature new values: %d, %d\n",iBeats, iBeat);
	fflush( stdout );
}*/

void CAMainWin::on_uiInsertPlayable_toggled(bool checked) {
	if (checked)
		uiPlayableToolBar->show();
	else
		uiPlayableToolBar->hide();
}

void CAMainWin::on_uiPlayableLength_toggled(bool checked, int buttonId) {
	// Read currently selected entry from tool button menu
	enum CAPlayable::CAPlayableLength length =
		static_cast<CAPlayable::CAPlayableLength>(buttonId);
		
	_musElementFactory->setMusElementType( CAMusElement::Note );
	
	// New note length type
	_musElementFactory->setPlayableLength( length );
	std::cout << "Note Input switched: On " << checked << " Note " << buttonId << std::endl;
	if( checked )
		setMode( InsertMode );
}

void CAMainWin::on_uiClefType_toggled(bool checked, int buttonId) {
	// Read currently selected entry from tool button menu
	enum CAClef::CAClefType clefType =
		static_cast<CAClef::CAClefType>(buttonId);
		
	_musElementFactory->setMusElementType( CAMusElement::Clef );
	
	// New clef type
	_musElementFactory->setClef( clefType );
	std::cout << "Clef Input switched: On " << checked << " Clef " << buttonId << std::endl;
	if ( checked )
		setMode( InsertMode );
}

void CAMainWin::on_uiTimeSigType_toggled(bool checked, int buttonId) {
/*	int iTimeSigBeats = 4;
	int iTimeSigBeat  = 4;
	// Read currently selected entry from tool button menu
        enum CATimeSignature::CATimeSignatureType eBaseElem = CATimeSignature::Classical;
        enum CAFixedTimeSig eElem = (CAFixedTimeSig)
	  mpoMEToolBar->toolElemValue( mpoTimeSigMenu->objectName() ).toInt();

	_musElementFactory->setMusElementType( CAMusElement::TimeSignature );
	// Make sure perspective is active
	setTimeSigPSPVisible( true );
	// New (fixed) time signature
	switch( eElem )
	{
		default:
		case TS_44:
		  iTimeSigBeats = 4;
		  iTimeSigBeat = 4;
		  break;
		case TS_22:
		  iTimeSigBeats = 2;
		  iTimeSigBeat = 2;
		  break;
		case TS_34:
		  iTimeSigBeats = 3;
		  iTimeSigBeat = 4;
		  break;
		case TS_24:
		  iTimeSigBeats = 2;
		  iTimeSigBeat = 4;
		  break;
		case TS_38:
		  iTimeSigBeats = 3;
		  iTimeSigBeat = 8;
		  break;
		case TS_68:
		  iTimeSigBeats = 6;
		  iTimeSigBeat = 8;
		  break;
		case TS_UNKNOWN:
		  break;
	}
	// Update Time signature perspective
	mpoTimeSigPSP->setTimeSignature( iTimeSigBeats, iTimeSigBeat );
	_musElementFactory->setTimeSigBeats( iTimeSigBeats );
	_musElementFactory->setTimeSigBeat( iTimeSigBeat );
	printf("Note TimeSig switched: On %d TimeSig %d\n", bOn, eElem);
	fflush( stdout );
	if( bOn )
		setMode(InsertMode); */
	std::cout << "uiInsertTimeSig_triggered()" << std::endl;
	_musElementFactory->setMusElementType( CAMusElement::KeySignature );
}

void CAMainWin::on_uiInsertTimeSignature_toggled(bool) {
	std::cout << "uiInsertTimeSig_triggered()" << std::endl;
	_musElementFactory->setMusElementType( CAMusElement::TimeSignature );
}

void CAMainWin::on_uiInsertKeySignature_toggled(bool) {
}

void CAMainWin::on_uiBarlineType_toggled(bool checked, int buttonId) {
}

void CAMainWin::on_uiNewContext_toggled(bool) {
}

void CAMainWin::sourceViewPortCommit(CASourceViewPort *v, QString inputString) {
	if (v->document()) {
		clearUI();
		delete document();
		
		QXmlInputSource input;
		input.setData(inputString);
		setDocument(CACanorusML::openDocument(&input, this));
	} else
	if (v->voice()) {
		v->voice()->clear();
		
		CALilyPondImport(inputString, v->voice());
		
		on_uiViewLilyPondSource_triggered();
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

void CAMainWin::on_uiMIDISetup_triggered() {
	CAMidiSetupDialog(this);
}

void CAMainWin::on_uiViewLilyPondSource_triggered() {
	if ( (currentViewPort()->viewPortType() == CAViewPort::ScoreViewPort) &&
	     (static_cast<CAScoreViewPort*>(currentViewPort())->currentContext()) &&
	     (static_cast<CAScoreViewPort*>(currentViewPort())->currentContext()->context()->contextType()==CAContext::Staff) ) {
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
