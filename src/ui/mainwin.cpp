/*****************************************************************************/
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify it   */
/* under the terms of the GNU General Public License as published by the     */ 
/* Free Software Foundation; version 2 of the License.	                     */
/*                                                                           */
/* This program is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY WARRANTY; without even the implied warranty of                */ 
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  */
/* Public License for more details.                                          */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with this program; (See "LICENSE.GPL"). If not, write to the Free         */
/* Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA        */
/* 02111-1307, USA.                                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*      Reinhard Katzmann, GERMANY                                           */
/*      reinhard@suamor.de                                                   */
/*                                                                           */
/*      Matevž Jekovec, SLOVENIA                                             */
/*      matevz.jekovec@gmail.com                                             */
/*                                                                           */
/*****************************************************************************/

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
#include <QCoreApplication>
#include <QSettings>
#include <iostream>

using namespace std;

#include "ui/mainwin.h"
#include "ui/keysigpsp.h"
#include "ui/timesigpsp.h"
#include "ui/midisetupdialog.h"

#include "widgets/toolbar.h"
#include "widgets/lcdnumber.h"

#include "interface/rtmididevice.h"
#include "interface/playback.h"
#include "interface/engraver.h"
#include "interface/pluginmanager.h"

#include "widgets/scrollwidget.h"
#include "widgets/viewport.h"
#include "widgets/scoreviewport.h"
#include "widgets/sourceviewport.h"

#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"
#include "drawable/drawablemuselement.h"
#include "drawable/drawablenote.h"
#include "drawable/drawableaccidental.h"	//DEBUG, this isn't needed though

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

QString locateResource(const QString fileName) {
	//! \todo Config file implementation
	//! \todo Application path argument
	QString curPath;
	
	//Try current working directory
	curPath = QDir::currentPath() + "/" + fileName;
	if (QFile(curPath).exists())
		return curPath;
	
	//Try application exe directory
	curPath = QCoreApplication::applicationDirPath() + "/" + fileName;
	if (QFile(curPath).exists())
		return curPath;
	
#ifdef DEFAULT_DATA_DIR
	//Try compiler defined DEFAULT_DATA_DIR constant (useful for Linux OSes). DEFAULT_DATA_DIR already includes leading slash!
	curPath = QString(DEFAULT_DATA_DIR) + "/" + fileName;
	if (QFile(curPath).exists())
		return curPath;
	
#endif
	//Else, if file not found, return empty string
	return QString("");
}

QString locateResourceDirectory(const QString fileName) {
	QString path = locateResource(fileName);
	return path.left(path.lastIndexOf("/"));
}

// Constructor
CAMainWin::CAMainWin(QMainWindow *oParent)
	: QMainWindow( oParent )
{
	// Init main application properties
	QCoreApplication::setOrganizationName("Canorus");
	QCoreApplication::setOrganizationDomain("canorus.org");
	QCoreApplication::setApplicationName("Canorus");
	
	// Open canorus config file in order to load needed settings
	// Open file is always an INI file in user's home directory.
	// No native formats are used (Windows registry etc.) - this is provided for easier transition of settings between the platforms. 
#ifdef Q_WS_WIN	// M$ is of course an exception
	_settings = new QSettings(QDir::homePath()+"/Application Data/Canorus/canorus.ini", QSettings::IniFormat);
#else	// POSIX systems use the same config file path
	_settings = new QSettings(QDir::homePath()+"/.config/Canorus/canorus.ini", QSettings::IniFormat);
#endif
		
	// Add main music insertion toolbar
	mpoMEToolBar = new CAToolBar( this );
	mpoMEToolBar->setOrientation(Qt::Vertical);
	initToolBar();
	addToolBar(static_cast<Qt::ToolBarArea>(2), mpoMEToolBar);
	
	// Initialize widgets
	moMainWin.setupUi( this );
	// Add a signal so if the menu action is used, the toolbar
	// is notified to change it's state
	mpoMEToolBar->setAction( actionClefSelect->objectName(), 
	                         moMainWin.action_Clef );
	mpoVoiceNum = new CALCDNumber( 0, 20, 0, "Voice number" );
	mpoVoiceNumAction = moMainWin.mpoToolBar->addWidget( mpoVoiceNum );
	// Connect manually as the action cannot be created earlier
	connect( mpoVoiceNum, SIGNAL( valChanged( int ) ), this,
	         SLOT( sl_mpoVoiceNum_valChanged( int ) ) );
	mpoVoiceNum->setEnabled(false);	//current voice number gets enabled when staff is selected and gets disabled when staff is unselected. By default, it's disabled.
	
	moMainWin.actionAnimated_scroll->setChecked(true);
	moMainWin.actionLock_scroll_playback->setChecked(false);
	moMainWin.actionUnsplit_all->setEnabled(false);
	moMainWin.actionClose_current_view->setEnabled(false);
	
	mpoKeySigPSP  = 0;
	mpoTimeSigPSP = 0;
	mpoMEFactory = new CAMusElementFactory();
	
	// Initialize import/export dialogs
	_exportDialog = new QFileDialog(this);
	_exportDialog->setFileMode(QFileDialog::AnyFile);
	_exportDialog->setDirectory( QDir::current() );
	_exportDialog->setAcceptMode( QFileDialog::AcceptSave );
	_exportDialog->setFilter(LILYPOND_FILTER); // add LilyPond filter defined in mainwin.h
	
	_importDialog = new QFileDialog(this);
	_importDialog->setFileMode(QFileDialog::ExistingFile);
	_importDialog->setDirectory( QDir::current() );
	_importDialog->setAcceptMode( QFileDialog::AcceptOpen );
	
	// Initialize internal UI properties
	_currentMode = SelectMode;
	_playback = 0;
	_animatedScroll = true;
	_lockScrollPlayback = false;
	
	// Initialize playback
	_defaultRtMidiOutPort = -1;
	_defaultRtMidiInPort = -1;
	initMidi();
	
	// Initialize scripting and plugins subsystem
#ifdef USE_RUBY	
	CASwigRuby::init();
#endif
#ifdef USE_PYTHON
	CASwigPython::init();
#endif
	_pluginManager = new CAPluginManager(this);
	_pluginManager->readPlugins();
	_pluginManager->enablePlugins();
	
	newDocument();
}

CAMainWin::~CAMainWin() 
{
	delete mpoClefMenu;
	delete mpoNoteMenu;
	delete mpoTimeSigMenu;
	delete _midi;
	delete mpoMEToolBar;
	delete _settings;
}

void CAMainWin::initToolBar()
{
	// Test Code for Toolbar
	mpoClefMenu    = new CAButtonMenu( tr("Select Clef" ) );
	mpoNoteMenu    = new CAButtonMenu( tr("Select Note" ) );
	mpoTimeSigMenu = new CAButtonMenu( tr("Select Time Signature" ) ); 
	mpoTimeSigMenu->setNumIconsPerRow( 3 );
	
	QIcon oClefTrebleIcon( QString::fromUtf8(":/menu/images/cleftreble.png") );
	QIcon oClefBassIcon(   QString::fromUtf8(":/menu/images/clefbass.png") );
	QIcon oClefAltoIcon(   QString::fromUtf8(":/menu/images/clefalto.png") );
	
	QIcon oN0Icon(  QString::fromUtf8(":/menu/images/n0.png") );
	QIcon oN1Icon(  QString::fromUtf8(":/menu/images/n1.png") );
	QIcon oN2Icon(  QString::fromUtf8(":/menu/images/n2.png") );
	QIcon oN4Icon(  QString::fromUtf8(":/menu/images/n4.png") );
	QIcon oN8Icon(  QString::fromUtf8(":/menu/images/n8.png") );
	QIcon oN16Icon(  QString::fromUtf8(":/menu/images/n16.png") );
	QIcon oN32Icon(  QString::fromUtf8(":/menu/images/n32.png") );
	QIcon oN64Icon(  QString::fromUtf8(":/menu/images/n64.png") );

	QIcon oTCIcon( QString::fromUtf8(":/menu/images/tsc.png") );
	QIcon oTABIcon( QString::fromUtf8(":/menu/images/tsab.png") );
	QIcon oT34Icon( QString::fromUtf8(":/menu/images/ts34.png") );
	QIcon oT24Icon( QString::fromUtf8(":/menu/images/ts24.png") );
	QIcon oT38Icon( QString::fromUtf8(":/menu/images/ts38.png") );
	QIcon oT68Icon( QString::fromUtf8(":/menu/images/ts68.png") );
	
	actionClefSelect = mpoMEToolBar->addToolMenu( "Add Clef", "actionClefSelect",
	                                              mpoClefMenu, 
	                                              &oClefTrebleIcon, true );
	actionNoteSelect = mpoMEToolBar->addToolMenu( "Change Note length",
	                                              "actionNoteSelect", mpoNoteMenu,
	                                              &oN4Icon, true );
	actionTimeSigSelect = mpoMEToolBar->addToolMenu( "Add Time Signature",
	                                                "actionTimeSigSelect", mpoTimeSigMenu,
	                                                &oTCIcon, true );
	                                                
	mpoMEGroup = new QActionGroup( this );
	mpoMEGroup->addAction( actionClefSelect );
	mpoMEGroup->addAction( actionNoteSelect );
	mpoMEGroup->addAction( actionTimeSigSelect );
	mpoMEGroup->setExclusive( true );
	
	// Add all the menu entries, either as text or icons
	mpoClefMenu->addButton( oClefTrebleIcon, CAClef::Treble );
	mpoClefMenu->addButton( oClefBassIcon, CAClef::Bass );
	mpoClefMenu->addButton( oClefAltoIcon, CAClef::Alto );
	mpoNoteMenu->addButton( oN0Icon, CANote::Breve );
	mpoNoteMenu->addButton( oN1Icon, CANote::Whole );
	mpoNoteMenu->addButton( oN2Icon, CANote::Half );
	mpoNoteMenu->addButton( oN4Icon, CANote::Quarter );
	mpoNoteMenu->addButton( oN8Icon, CANote::Eighth );
	mpoNoteMenu->addButton( oN16Icon, CANote::Sixteenth );
	mpoNoteMenu->addButton( oN32Icon, CANote::ThirtySecond );
	mpoNoteMenu->addButton( oN64Icon, CANote::SixtyFourth );
	mpoTimeSigMenu->addButton( oTCIcon, TS_44 );
	mpoTimeSigMenu->addButton( oTABIcon, TS_22 );
	mpoTimeSigMenu->addButton( oT34Icon, TS_34 );
	mpoTimeSigMenu->addButton( oT24Icon, TS_24 );
	mpoTimeSigMenu->addButton( oT38Icon, TS_38 );
	mpoTimeSigMenu->addButton( oT68Icon, TS_68 );
}

void CAMainWin::newDocument() {
	_document.clear();	//clear the logical part
	clearUI();			//clear the UI part

#ifdef USE_PYTHON
	QList<PyObject*> argsPython;
	argsPython << CASwigPython::toPythonObject(&_document, CASwigPython::Document);
	CASwigPython::callFunction(locateResource("scripts/newdocument.py"), "newDefaultDocument", argsPython);
#endif
	rebuildUI();
}

void CAMainWin::addSheet(CASheet *s) {
	CAScoreViewPort *v = new CAScoreViewPort(s, 0);
	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	connect(v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAViewPort *)), this, SLOT(viewPortKeyPressEvent(QKeyEvent *, CAViewPort *)));
	v->setFocusPolicy(Qt::ClickFocus);
	v->setFocus();

	_viewPortList.append(v);
	
	moMainWin.tabWidget->addTab(new CAScrollWidget(v, 0), s->name());
	moMainWin.tabWidget->setCurrentIndex(moMainWin.tabWidget->count()-1);
	
	_activeViewPort = v;
}

void CAMainWin::clearUI() {
	if(_viewPortList.size() > 1)
		on_actionUnsplit_all_triggered();
	for (int i=0; i<_viewPortList.size(); i++)
		delete _viewPortList[i];
	
	_viewPortList.clear();
	
	while (moMainWin.tabWidget->count()) {
		delete _currentScrollWidget;
		moMainWin.tabWidget->removeTab(moMainWin.tabWidget->currentIndex());
	}
	
	_fileName = "";
	
	mpoVoiceNum->setEnabled(false);
}

void CAMainWin::on_tabWidget_currentChanged(int idx) {
	_activeViewPort = _currentScrollWidget->lastUsedViewPort();
}

void CAMainWin::on_action_Fullscreen_toggled(bool checked) {
	if (checked)
		this->showFullScreen();
	else
		this->showNormal();
}

void CAMainWin::on_actionSplit_horizontally_triggered() {
	CAViewPort *v = (CAViewPort *)_currentScrollWidget->splitHorizontally();
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
	
	moMainWin.actionUnsplit_all->setEnabled(true);
	moMainWin.actionClose_current_view->setEnabled(true);
	_viewPortList.append(v);
	setMode(_currentMode);	//updates the new viewport border settings
}

void CAMainWin::on_actionSplit_vertically_triggered() {
	CAViewPort *v = (CAViewPort *)_currentScrollWidget->splitVertically();
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
	
	moMainWin.actionUnsplit_all->setEnabled(true);
	moMainWin.actionClose_current_view->setEnabled(true);
	_viewPortList.append(v);
	setMode(_currentMode);	//updates the new viewport border settings
}

void CAMainWin::doUnsplit(CAViewPort *v) {
	CAViewPort::CAViewPortType vpt = v?v->viewPortType():_currentScrollWidget->lastUsedDockedViewPort()->viewPortType();
	v = _currentScrollWidget->unsplit(v);
	if (!v) return;
	
	_viewPortList.removeAll(v);
	
	if(_currentScrollWidget->dockedViewPortsList().size() == 1)
	{
		moMainWin.actionClose_current_view->setEnabled(false);
		moMainWin.actionUnsplit_all->setEnabled(false);
	}
	if(vpt == CAViewPort::SourceViewPort)
		moMainWin.actionSource_view_perspective->setChecked(false);
	_activeViewPort = _currentScrollWidget->dockedViewPortsList().back();
}

void CAMainWin::on_actionUnsplit_all_triggered() {
	QList<CAViewPort*> dockedViewPorts = _currentScrollWidget->unsplitAll();
	for(QList<CAViewPort*>::iterator i = dockedViewPorts.begin(); i < dockedViewPorts.end(); i++)
		_viewPortList.removeAll(*i);
	moMainWin.actionClose_current_view->setEnabled(false);
	moMainWin.actionUnsplit_all->setEnabled(false);
	moMainWin.actionSource_view_perspective->setChecked(false);
	_activeViewPort = _currentScrollWidget->dockedViewPortsList().back();
}

void CAMainWin::on_actionClose_current_view_triggered() {
	doUnsplit();
}

void CAMainWin::on_actionSource_view_perspective_toggled(bool status) {
	if(!status)
	{
		for(QList<CAViewPort*>::iterator i = _viewPortList.begin(); i < _viewPortList.end(); i++)
		{
			if((*i)->viewPortType() == CAViewPort::SourceViewPort)
				doUnsplit(*i);
		}
		return;
	} else {
		CASourceViewPort *v = new CASourceViewPort(&_document, _activeViewPort->parent());
		_currentScrollWidget->addViewPort(v);
		
		connect(v, SIGNAL(CACommit(CASourceViewPort*, QString)), this, SLOT(sourceViewPortCommit(CASourceViewPort*, QString)));
		
		v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
		v->setFocusPolicy(Qt::ClickFocus);
		v->setFocus();
		
		moMainWin.actionUnsplit_all->setEnabled(true);
		moMainWin.actionClose_current_view->setEnabled(true);
		_viewPortList.append(v);
		setMode(_currentMode);	//updates the new viewport border settings
	}
}

void CAMainWin::on_actionNew_viewport_triggered() {
	CAViewPort *v = _currentScrollWidget->newViewPort(_activeViewPort);

	v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	if (v->viewPortType() == CAViewPort::ScoreViewPort) {
		connect((CAScoreViewPort*)v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMouseMoveEvent(QMouseEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
		connect((CAScoreViewPort*)v, SIGNAL(CAKeyPressEvent(QKeyEvent *, CAViewPort *)), this, SLOT(viewPortKeyPressEvent(QKeyEvent *, CAViewPort *)));
		v->setFocusPolicy(Qt::ClickFocus);
		v->setFocus();
	}

	_viewPortList.append(v);
	setMode(_currentMode);	//updates the new viewport border settings
}

void CAMainWin::on_actionNew_triggered() {
	newDocument();
}

void CAMainWin::on_actionNew_sheet_triggered() {
	addSheet(_document.addSheet(tr("Sheet %1").arg(QString::number(_document.sheetCount()+1))));			//add a new empty sheet
}

void CAMainWin::on_actionNew_staff_triggered() {
	if (_activeViewPort->viewPortType() != CAViewPort::ScoreViewPort)
		return;
	
	CASheet *sheet = ((CAScoreViewPort*)_activeViewPort)->sheet();
	CAStaff *staff = sheet->addStaff();
	staff->addVoice(new CAVoice( staff, tr("Voice %1").arg(QString::number(1)) ));
	
	rebuildUI(sheet);
	
	((CAScoreViewPort*)_activeViewPort)->selectContext(staff);
	((CAScoreViewPort*)_activeViewPort)->repaint();
}

void CAMainWin::setMode(CAMode mode) {
	_currentMode = mode;
	
	switch (mode) {
		case SelectMode: {
			for (int i=0; i<_viewPortList.size(); i++) {
				if (_viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort) {
					if (!((CAScoreViewPort*)_viewPortList[i])->playing())
						((CAScoreViewPort*)_viewPortList[i])->unsetBorder();
					((CAScoreViewPort*)_viewPortList[i])->setShadowNoteVisible(false);
					statusBar()->showMessage("");
					mpoMEFactory->setMusElementType( CAMusElement::Undefined );
					((CAScoreViewPort*)_viewPortList[i])->repaint();
				}
			}
			break;
		}
		case InsertMode: {
			QPen p;
			p.setColor(Qt::blue);
			p.setWidth(3);
			
			if (mpoMEFactory->musElementType() == CAMusElement::Note)
				((CAScoreViewPort*)_activeViewPort)->setShadowNoteVisible(true);

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

void CAMainWin::on_action_Clef_triggered() {
	setMode(InsertMode);
	mpoMEFactory->setMusElementType( CAMusElement::Clef );
}

void CAMainWin::rebuildUI(CASheet *sheet, bool repaint) {
	if (!sheet) {
		clearUI();
		for (int i=0; i<_document.sheetCount(); i++)
			addSheet(_document.sheetAt(i));
	}

	for (int i=0; i<_viewPortList.size(); i++) {
		_viewPortList[i]->rebuild();
		
		if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
			((CAScoreViewPort*)(_viewPortList[i]))->checkScrollBars();
			
		if (repaint)
			_viewPortList[i]->repaint();
	}
}

void CAMainWin::viewPortMousePressEvent(QMouseEvent *e, const QPoint coords, CAViewPort *viewPort) {
	_activeViewPort = viewPort;
	_currentScrollWidget->setLastUsedViewPort(_activeViewPort);
	
	if (viewPort->viewPortType() == CAViewPort::ScoreViewPort) {
		CAScoreViewPort *v = (CAScoreViewPort*)viewPort;
		
		CADrawableContext *currentContext = v->currentContext();
		
		v->selectCElement(coords.x(), coords.y());
		if ( v->selectMElement(coords.x(), coords.y()) ||	//select a music element at the given location - select none, if there's none there
		     v->selectCElement(coords.x(), coords.y()) ) {
			//voice number widget
			if (currentContext != v->currentContext()) {	//new context was selected
				if (v->currentContext()->context()->contextType() == CAContext::Staff) {
					mpoVoiceNum->setEnabled(true);
					mpoVoiceNum->setRealValue(0);
					mpoVoiceNum->setMax(((CAStaff*)v->currentContext()->context())->voiceCount());
				} else
					mpoVoiceNum->setEnabled(false);
			}
			v->repaint();
		}

		if (e->modifiers()==Qt::ControlModifier) {
			CAMusElement *elt;
			if ( elt = v->removeMElement(coords.x(), coords.y()) ) {
				elt->context()->removeMusElement(elt, true);	//free the memory as well!
				rebuildUI(v->sheet());
				return;
			}
		}
			
		switch (_currentMode) {
			case SelectMode:
			case EditMode: {
				if (!v->selection()->isEmpty()) {
					CAMusElement *elt = v->selection()->front()->musElement();
					std::cout << "musElement: " << elt << ", timeStart=" << elt->timeStart() << ", timeEnd=" << elt->timeEnd();
					if (elt->isPlayable())
						std::cout << ", voice=" << ((CAPlayable*)elt)->voice() << ", voiceNr=" << ((CAPlayable*)elt)->voice()->voiceNumber() << ", idxInVoice=" << ((CAPlayable*)elt)->voice()->indexOf(elt);
					std::cout << std::endl;
				}
				break;
			}
			case InsertMode: {
				if (e->button()==Qt::RightButton && mpoMEFactory->musElementType()==CAMusElement::Note)
					//place a rest when using right mouse button and note insertion is selected
					mpoMEFactory->setMusElementType( CAMusElement::Rest );
				insertMusElementAt( coords, v, *mpoMEFactory->getMusElement() );
				if (mpoMEFactory->musElementType()==CAMusElement::Rest)
					mpoMEFactory->setMusElementType( CAMusElement::Note );
				break;
			}
		}
		_pluginManager->action("onScoreViewPortClick", &_document, 0, 0);
	}
}

void CAMainWin::insertMusElementAt(const QPoint coords, CAScoreViewPort *v, CAMusElement &roMusElement )
{
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
			success = mpoMEFactory->configureClef( context, left );
			break;
		}
		case CAMusElement::KeySignature: {
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y());
			success = mpoMEFactory->configureKeySignature( mpoKeySigPSP->getKeySignature()-7,
			                                               context, left );
			break;
		}
		case CAMusElement::TimeSignature: { // ToDo: Move TimeSigPSP code
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y());
			if( mpoTimeSigPSP ) // Change via perspective as well
			{
				int iTimeSigBeats, iTimeSigBeat;
				mpoTimeSigPSP->getTimeSignature( iTimeSigBeats, iTimeSigBeat );
				mpoMEFactory->setTimeSigBeats( iTimeSigBeats );
				mpoMEFactory->setTimeSigBeat( iTimeSigBeat );
			}
			success = mpoMEFactory->configureTimeSignature( context, left );
			break;
		}
		case CAMusElement::Note: { // Do we really need to do all that here??
			int iVoiceNum = mpoVoiceNum->getRealValue()-1<0?0:mpoVoiceNum->getRealValue()-1;
			drawableStaff = (CADrawableStaff*)context;
			staff = drawableStaff->staff();
			CAVoice *voice = staff->voiceAt( iVoiceNum );
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y(), voice);
			success = mpoMEFactory->configureNote( voice, coords, context, left );
			if (success) { 
				mpoMEFactory->setNoteExtraAccs( 0 );
				v->setDrawShadowNoteAccs(false); 
				iPlayableDotted = ((CAPlayable *)(mpoMEFactory->getMusElement()))->dotted();
			}
			break;
		}
		case CAMusElement::Rest: {
			int iVoiceNum = mpoVoiceNum->getRealValue()-1<0?0:mpoVoiceNum->getRealValue()-1;
			drawableStaff = (CADrawableStaff*)context;
			staff = drawableStaff->staff();
			CAVoice *voice = staff->voiceAt( iVoiceNum );
			CADrawableMusElement *left = v->nearestLeftElement(coords.x(), coords.y(), voice);
			success = mpoMEFactory->configureRest( voice, coords, context, left );
			if (success)
				iPlayableDotted = ((CAPlayable *)(mpoMEFactory->getMusElement()))->dotted();
			break;
		}
	}
	
	if (success) {
		rebuildUI(v->sheet(), true);
		v->selectMElement( mpoMEFactory->getMusElement() );
		v->setShadowNoteDotted(iPlayableDotted);
		v->repaint();
	} else
	  {
		mpoMEFactory->removeMusElem( true );
		mpoMEFactory->createMusElem(); // Factory always must have a valid element
	  }
}

void CAMainWin::viewPortMouseMoveEvent(QMouseEvent *e, QPoint coords, CAViewPort *v) {
	if ((_currentMode == InsertMode) &&
	    (mpoMEFactory->musElementType() == CAMusElement::Note) &&
	    (v->viewPortType()==CAViewPort::ScoreViewPort)
	   ) {
		CAScoreViewPort *c = (CAScoreViewPort*)v;
		CADrawableStaff *s;
		if (c->currentContext()?(c->currentContext()->drawableContextType() == CADrawableContext::DrawableStaff):0)
			s = (CADrawableStaff*)c->currentContext(); 
		else
			return;

		if (mpoMEFactory->musElementType() == CAMusElement::Note || 
                    mpoMEFactory->musElementType() == CAMusElement::Rest)
			c->setShadowNoteVisible(true);
		
		//calculate the logical pitch out of absolute world coordinates and the current clef
		int pitch = s->calculatePitch(coords.x(), coords.y());
		
		//write into the main window's status bar the note pitch name
		int iNoteAccs = s->getAccs(coords.x(), pitch)+mpoMEFactory->noteExtraAccs();
		mpoMEFactory->setNoteAccs( iNoteAccs );
		statusBar()->showMessage(CANote::generateNoteName(pitch, iNoteAccs));
		((CAScoreViewPort*)v)->setShadowNoteAccs(iNoteAccs);
	}
}

void CAMainWin::viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *c) {
	_activeViewPort = c;
	CAScoreViewPort *v = (CAScoreViewPort*) c;

	int val;
	switch (e->modifiers()) {
		case Qt::NoModifier:			//scroll horizontally
			v->setWorldX( v->worldX() - (int)((0.5*e->delta()) / v->zoom()), _animatedScroll );
			break;
		case Qt::AltModifier:			//scroll horizontally, fast
			v->setWorldX( v->worldX() - (int)(e->delta() / v->zoom()), _animatedScroll );
			break;
		case Qt::ShiftModifier:			//scroll vertically
			v->setWorldY( v->worldY() - (int)((0.5*e->delta()) / v->zoom()), _animatedScroll );
			break;
		case 0x0A000000://SHIFT+ALT		//scroll vertically, fast
			v->setWorldY( v->worldY() - (int)(e->delta() / v->zoom()), _animatedScroll );
			break;
		case Qt::ControlModifier:		//zoom
			if (e->delta() > 0)
				v->setZoom( v->zoom()*1.1, coords.x(), coords.y(), _animatedScroll );
			else
				v->setZoom( v->zoom()/1.1, coords.x(), coords.y(), _animatedScroll );
			
			break;
	}

	v->repaint();
}

void CAMainWin::viewPortKeyPressEvent(QKeyEvent *e, CAViewPort *v) {
	_activeViewPort = v;
	
	if (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort) {
		switch (e->key()) {
			//Music editing keys
			case Qt::Key_Right: {
				//select next music element
				((CAScoreViewPort*)_activeViewPort)->selectNextMusElement();
				_activeViewPort->repaint();
				break;
			}
			
			case Qt::Key_Left: {
				//select previous music element
				((CAScoreViewPort*)_activeViewPort)->selectPrevMusElement();
				_activeViewPort->repaint();
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
				
				rebuildUI(((CAScoreViewPort*)v)->sheet(), true);
				((CAScoreViewPort*)v)->selectMElement(bar);
				v->repaint();
				break;
			}
			
			case Qt::Key_Up: {
				if (_currentMode == SelectMode) {	//select the upper music element
					((CAScoreViewPort*)_activeViewPort)->selectUpMusElement();
					_activeViewPort->repaint();
				} else if ((_currentMode == InsertMode) || (_currentMode == EditMode)) {
					if (!((CAScoreViewPort*)_activeViewPort)->selection()->isEmpty()) {
						CADrawableMusElement *elt =
							((CAScoreViewPort*)_activeViewPort)->selection()->back();
						
						//pitch note for one step higher
						if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
							CANote *note = (CANote*)elt->musElement();
							note->setPitch(note->pitch()+1);
							rebuildUI(note->voice()->staff()->sheet());
						}
					}
				}
				break;
			}
			
			case Qt::Key_Down: {
				if (_currentMode == SelectMode) {	//select the upper music element
					((CAScoreViewPort*)_activeViewPort)->selectUpMusElement();
					_activeViewPort->repaint();
				} else if ((_currentMode == InsertMode) || (_currentMode == EditMode)) {
					if (!((CAScoreViewPort*)_activeViewPort)->selection()->isEmpty()) {
						CADrawableMusElement *elt =
							((CAScoreViewPort*)_activeViewPort)->selection()->back();
						
						//pitch note for one step higher
						if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
							CANote *note = (CANote*)elt->musElement();
							note->setPitch(note->pitch()-1);
							rebuildUI(note->voice()->staff()->sheet());
						}
					}
				}
				break;
			}
			case Qt::Key_Plus: {
				if (v->viewPortType()==CAViewPort::ScoreViewPort) {
					if (currentMode()==InsertMode) {
						mpoMEFactory->addNoteExtraAccs(1); mpoMEFactory->addNoteAccs(1);
						((CAScoreViewPort*)v)->setDrawShadowNoteAccs(mpoMEFactory->noteExtraAccs()!=0);
						((CAScoreViewPort*)v)->setShadowNoteAccs(mpoMEFactory->noteAccs());
						v->repaint();
					} else if (currentMode()==EditMode) {
						if (!((CAScoreViewPort*)v)->selection()->isEmpty()) {
							CAMusElement *elt = ((CAScoreViewPort*)v)->selection()->front()->musElement();
							if (elt->musElementType()==CAMusElement::Note) {
								((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()+1);
								rebuildUI(((CANote*)elt)->voice()->staff()->sheet());
							}
						}
					}
				}
				break;
			}
			
			case Qt::Key_Minus: {
				if (v->viewPortType()==CAViewPort::ScoreViewPort) {
					if (currentMode()==InsertMode) {
						mpoMEFactory->subNoteExtraAccs(1); mpoMEFactory->subNoteAccs(1);
						((CAScoreViewPort*)v)->setDrawShadowNoteAccs(mpoMEFactory->noteExtraAccs()!=0);
						((CAScoreViewPort*)v)->setShadowNoteAccs(mpoMEFactory->noteAccs());
						v->repaint();
					} else if (currentMode()==EditMode) {
						if (!((CAScoreViewPort*)v)->selection()->isEmpty()) {
							CAMusElement *elt = ((CAScoreViewPort*)v)->selection()->front()->musElement();
							if (elt->musElementType()==CAMusElement::Note) {
								((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()-1);
								rebuildUI(((CANote*)elt)->voice()->staff()->sheet());
							}
						}
					}
				}
				break;
			}
			
			case Qt::Key_Period: {
				if (v->viewPortType()==CAViewPort::ScoreViewPort) {
					if (currentMode()==InsertMode) {
						mpoMEFactory->addPlayableDotted( 1 );
						((CAScoreViewPort*)v)->setShadowNoteDotted(mpoMEFactory->playableDotted());
						v->repaint();
					} else if (currentMode()==EditMode) {
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
								rebuildUI(((CANote*)elt)->voice()->staff()->sheet());
							}
						}
					}
				}
				break;
			}
			
			case Qt::Key_Delete:
				if (!((CAScoreViewPort*)_activeViewPort)->selection()->isEmpty()) {
					CAMusElement *elt = ((CAScoreViewPort*)_activeViewPort)->selection()->back()->musElement();
					if (elt->context()->contextType() == CAContext::Staff) {
						((CAStaff*)elt->context())->removeMusElement(elt);
						rebuildUI(((CAScoreViewPort*)_activeViewPort)->sheet());
					}
				}
				
				break;
			
			//Mode keys
			case Qt::Key_Escape:
				if ((currentMode()==SelectMode) && (_activeViewPort) && (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort)) {
					((CAScoreViewPort*)_activeViewPort)->clearMSelection();
					((CAScoreViewPort*)_activeViewPort)->clearCSelection();
				}
				setMode(SelectMode);
				mpoVoiceNum->setRealValue(0);
				if (mpoKeySigPSP)
					mpoKeySigPSP->hide();
				break;
			case Qt::Key_I:
				mpoMEFactory->setMusElementType( CAMusElement::Note );
				setMode(InsertMode);
				break;
			case Qt::Key_E:
				setMode(EditMode);
				break;
		}
	}
}

void CAMainWin::keyPressEvent(QKeyEvent *e) {
}

void CAMainWin::initMidi() {
	_midi = new CARtMidiDevice();
	
	if (_settings->contains("rtmidi/defaultoutputport") && _settings->contains("rtmidi/defaultinputport")) {
		_defaultRtMidiInPort = _settings->value("rtmidi/defaultinputport").toInt();
		if (_defaultRtMidiInPort >= _midi->getInputPorts().count())
			_defaultRtMidiInPort = -1;

		_defaultRtMidiOutPort = _settings->value("rtmidi/defaultoutputport").toInt();
		if (_defaultRtMidiOutPort >= _midi->getOutputPorts().count())
			_defaultRtMidiOutPort = -1;
			
	} else {
		on_actionMIDI_Setup_triggered();
	}
}

void CAMainWin::setKeySigPSPVisible( bool bVisible )
{
	if (!mpoKeySigPSP) {
		mpoKeySigPSP = new CAKeySigPSP(tr("Edit key signature"), this);
		addDockWidget( Qt::LeftDockWidgetArea, mpoKeySigPSP );
	}
	mpoKeySigPSP->setVisible( bVisible );
}	

void CAMainWin::setTimeSigPSPVisible( bool bVisible )
{
	if (!mpoTimeSigPSP) {
		mpoTimeSigPSP = new CATimeSigPSP("Edit time signature", this);
		addDockWidget( Qt::LeftDockWidgetArea, mpoTimeSigPSP );
		connect( mpoTimeSigPSP, SIGNAL( timeSigChanged( int, int ) ),
                         this, SLOT( sl_mpoTimeSig_valChanged( int, int ) ) );
	}
	mpoTimeSigPSP->setVisible( bVisible );
}	

void CAMainWin::playbackFinished() {
	_playback->disconnect();
	//delete _playback;	//TODO: crashes on application close, if deleted! Is this ok? -Matevz
	moMainWin.actionPlay->setChecked(false);
	
	_repaintTimer->stop();
	_repaintTimer->disconnect();	//TODO: crashes, if disconnected sometimes. -Matevz
	delete _repaintTimer;			//TODO: crashes, if deleted. -Matevz
	_midi->closeOutputPort();
	
	setMode(_currentMode);
}

void CAMainWin::on_actionPlay_toggled(bool checked) {
	if (checked && (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort)) {
		_midi->openOutputPort(_defaultRtMidiOutPort);
		_repaintTimer = new QTimer();
		_repaintTimer->setInterval(100);
		_repaintTimer->start();
		//connect(_repaintTimer, SIGNAL(timeout()), this, SLOT(on_repaintTimer_timeout())); //TODO: timeout is connected directly to repaint() directly. This should be optimized in the future -Matevz
		connect(_repaintTimer, SIGNAL(timeout()), _activeViewPort, SLOT(repaint()));
		_playbackViewPort = _activeViewPort;
		
		_playback = new CAPlayback((CAScoreViewPort*)_activeViewPort, _midi);
		connect(_playback, SIGNAL(finished()), this, SLOT(playbackFinished()));
		_playback->start();
	} else {
		_playback->stop();
	}
}

/*void CAMainWin::on_repaintTimer_timeout() { //TODO: timeout is connected directly to repaint() currently
	if (_lockScrollPlayback && (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort))
		((CAScoreViewPort*)_activeViewPort)->zoomToSelection(_animatedScroll);
	_activeViewPort->repaint();
}*/

void CAMainWin::on_actionAnimated_scroll_toggled(bool val) {
	_animatedScroll = val;
}

void CAMainWin::on_actionLock_scroll_playback_toggled(bool val) {
	_lockScrollPlayback = val;
}

void CAMainWin::on_actionZoom_to_selection_triggered() {
	if (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_activeViewPort)->zoomToSelection(_animatedScroll);
}

void CAMainWin::on_actionZoom_to_fit_triggered() {
	if (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_activeViewPort)->zoomToFit();
}

void CAMainWin::on_actionZoom_to_width_triggered() {
	if (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_activeViewPort)->zoomToWidth();
}

void CAMainWin::on_actionZoom_to_height_triggered() {
	if (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort)
		((CAScoreViewPort*)_activeViewPort)->zoomToHeight();
}

void CAMainWin::closeEvent(QCloseEvent *event) {	//TODO: Make the main window the main window of the application somehow - when it's closed, the destructor is also called. This way, this function will not be needed anymore. -Matevz
	clearUI();
	delete _pluginManager;
}

void CAMainWin::on_actionOpen_triggered() {
	QString s = QFileDialog::getOpenFileName(
	                this,
	                tr("Choose a file to open"),
	                "",
	                tr("Canorus document (*.xml)"));

	if (s.isEmpty())
		return;

	QFile file(s);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		_fileName = s;
		_document.clear();
		clearUI();
		
		QXmlInputSource input(&file);
		CADocument *openedDoc = CACanorusML::openDocument(&input, this);
		if (openedDoc)
			_document = *openedDoc;
		
		file.close();
		rebuildUI();
		moMainWin.tabWidget->setCurrentIndex(0);
	}               
}

void CAMainWin::on_actionSave_triggered() {
	QString s;
	if (_fileName.isEmpty()) { 
		s = QFileDialog::getSaveFileName(
		                this,
		                tr("Choose a file to save"),
		                "",
		                tr("Canorus document (*.xml)"));
	}

	if (s.isEmpty())
		return;

	//append the extension, if the last 4 characters don't already contain the dot
	int i;
	for (i=0; (i<4) && ((s.length()-i-1) > 0); i++) if (s[s.length()-i-1] == '.') break;
	if (i==4) s.append(".xml");
		
	QFile file(s);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		_fileName = s;
		QTextStream out(&file);
		CACanorusML::saveDocument(&_document, out);
		file.close();
	}               
}

void CAMainWin::on_actionSave_as_triggered() {
	QString s = QFileDialog::getSaveFileName(
	                this,
	                tr("Choose a file to save"),
	                "",
	                tr("Canorus document (*.xml)"));
	
	if (s.isEmpty())
		return;
	
	//append the extension, if the last 4 characters don't already contain the dot
	int i;
	for (i=0; (i<4) && ((s.length()-i-1) > 0); i++) if (s[s.length()-i-1] == '.') break;
	if (i==4) s.append(".xml");
	
	QFile file(s);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		_fileName = s;
		QTextStream out(&file);
		CACanorusML::saveDocument(&_document, out);
		file.close();
	}
}

void CAMainWin::on_actionExport_triggered() {
	QStringList fileNames;
	int ffound = _exportDialog->exec();
	if (ffound)
		fileNames = _exportDialog->selectedFiles();
	
	if (!ffound)
		return;
	
	QString s = fileNames[0];
	
	if (_pluginManager->exportActionExists(_exportDialog->selectedFilter()))
		_pluginManager->exportAction(_exportDialog->selectedFilter(), &_document, 0, 0, s);
	else {
		QFile file(s);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&file);
			
			// LilyPond
			if (_exportDialog->selectedFilter()==LILYPOND_FILTER) {
				CALilyPondExport(&_document, &out);
			}
			
			file.close();
		}
	}              
}

void CAMainWin::on_actionImport_triggered() {
	QStringList fileNames;
	int ffound = _importDialog->exec();
	if (ffound)
		fileNames = _importDialog->selectedFiles();
	
	if (!ffound)
		return;
	
	QString s = fileNames[0];
	
	if (_pluginManager->importActionExists(_importDialog->selectedFilter()))
		_pluginManager->importAction(_importDialog->selectedFilter(), &_document, 0, 0, fileNames[0]);
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

void CAMainWin::sl_mpoVoiceNum_valChanged(int iVoice)
{
	printf("New voice number: %d\n",iVoice);
	fflush( stdout );
}

void CAMainWin::sl_mpoTimeSig_valChanged(int iBeats, int iBeat)
{
	mpoMEFactory->setMusElementType( CAMusElement::TimeSignature );
	mpoMEFactory->setTimeSigBeats( iBeats );
	mpoMEFactory->setTimeSigBeat( iBeat );
	CAFixedTimeSig eElem = mpoTimeSigPSP->getTimeSignatureFixed();
	if( eElem != TS_UNKNOWN )
		mpoMEToolBar->changeMenuIcon( mpoTimeSigMenu->getButton( (int)eElem ) );
	printf("Time signature new values: %d, %d\n",iBeats, iBeat);
	fflush( stdout );
}

void CAMainWin::on_actionNoteSelect_toggled(bool bOn)
{
	// Read currently selected entry from tool button menu
	enum CAPlayable::CAPlayableLength eElem = (CAPlayable::CAPlayableLength)
	  mpoMEToolBar->toolElemValue( mpoNoteMenu->objectName() ).toInt();
	mpoMEFactory->setMusElementType( CAMusElement::Note );
	// New note length type
	mpoMEFactory->setPlayableLength( eElem );
	printf("Note Input switched: On %d Note %d\n", bOn, eElem);
	fflush( stdout );
	if( bOn )
		setMode(InsertMode);
}

void CAMainWin::on_actionClefSelect_toggled(bool bOn)
{
	// Read currently selected entry from tool button menu
	enum CAClef::CAClefType eElem = (CAClef::CAClefType)
	  mpoMEToolBar->toolElemValue( mpoClefMenu->objectName() ).toInt();
	mpoMEFactory->setMusElementType( CAMusElement::Clef );
	// New clef type
	mpoMEFactory->setClef( eElem );
	printf("Note Clef switched: On %d Clef %d\n", bOn, eElem);
	fflush( stdout );
	if( bOn )
		on_action_Clef_triggered();
}

void CAMainWin::on_actionTimeSigSelect_toggled(bool bOn)
{
	int iTimeSigBeats = 4;
	int iTimeSigBeat  = 4;
	// Read currently selected entry from tool button menu
        enum CATimeSignature::CATimeSignatureType eBaseElem = CATimeSignature::Classical;
        enum CAFixedTimeSig eElem = (CAFixedTimeSig)
	  mpoMEToolBar->toolElemValue( mpoTimeSigMenu->objectName() ).toInt();

	mpoMEFactory->setMusElementType( CAMusElement::TimeSignature );
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
	mpoMEFactory->setTimeSigBeats( iTimeSigBeats );
	mpoMEFactory->setTimeSigBeat( iTimeSigBeat );
	printf("Note TimeSig switched: On %d TimeSig %d\n", bOn, eElem);
	fflush( stdout );
	if( bOn )
		setMode(InsertMode);
}
	
void CAMainWin::on_action_Key_signature_triggered() {
	setKeySigPSPVisible( true );
	mpoKeySigPSP->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	mpoKeySigPSP->setFocusPolicy(Qt::ClickFocus);
	mpoKeySigPSP->setFocus();
	
	moMainWin.actionSplit_horizontally->setEnabled(false);
	moMainWin.actionSplit_vertically->setEnabled(false);
	moMainWin.actionUnsplit_all->setEnabled(true);
	moMainWin.actionClose_current_view->setEnabled(true);
	
	setMode(InsertMode);
	mpoMEFactory->setMusElementType( CAMusElement::KeySignature );
	actionNoteSelect->setChecked( false );
	actionTimeSigSelect->setChecked( false );	
}

void CAMainWin::on_action_Time_signature_triggered() {
	setTimeSigPSPVisible( true );
	
	mpoTimeSigPSP->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
	mpoTimeSigPSP->setFocusPolicy(Qt::ClickFocus);
	mpoTimeSigPSP->setFocus();
	
	moMainWin.actionSplit_horizontally->setEnabled(false);
	moMainWin.actionSplit_vertically->setEnabled(false);
	moMainWin.actionUnsplit_all->setEnabled(true);
	moMainWin.actionClose_current_view->setEnabled(true);
	
	setMode(InsertMode);
	mpoMEFactory->setMusElementType( CAMusElement::TimeSignature );
	// Update toolbar if possible
	CAFixedTimeSig eElem = mpoTimeSigPSP->getTimeSignatureFixed();
	if( eElem != TS_UNKNOWN )
		mpoMEToolBar->changeMenuIcon( mpoTimeSigMenu->getButton( (int)eElem ) );
	actionNoteSelect->setChecked( false );
	actionClefSelect->setChecked( false );
}

void CAMainWin::sourceViewPortCommit(CASourceViewPort *v, QString inputString) {
	if (v->document()) {
		clearUI();
		_document.clear();
		
		QXmlInputSource input;
		input.setData(inputString);
		_document = *CACanorusML::openDocument(&input, this);
		
		on_actionSource_view_perspective_toggled(false);
	} else
	if (v->voice()) {
		v->voice()->clear();
		
		CALilyPondImport(inputString, v->voice());
		
		on_actionVoice_in_LilyPond_source_toggled(false);
	}
	
	rebuildUI();
}

void CAMainWin::on_actionAbout_Qt_triggered()
{
	QMessageBox::aboutQt( this, tr("About Qt") );
}

void CAMainWin::on_actionAbout_Canorus_triggered()
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

void CAMainWin::on_actionMIDI_Setup_triggered() {
	CAMidiSetupDialog(this, _midi->getInputPorts(), _midi->getOutputPorts(), &_defaultRtMidiInPort, &_defaultRtMidiOutPort);
	_settings->setValue("rtmidi/defaultoutputport", _defaultRtMidiOutPort);
	_settings->setValue("rtmidi/defaultinputport", _defaultRtMidiInPort);
}

void CAMainWin::on_actionVoice_in_LilyPond_source_toggled(bool checked) {
	if(!checked)
	{
		for(QList<CAViewPort*>::iterator i = _viewPortList.begin(); i < _viewPortList.end(); i++)
		{
			if((*i)->viewPortType() == CAViewPort::SourceViewPort)
				doUnsplit(*i);
		}
		return;
	} else
	if ( (_activeViewPort->viewPortType() == CAViewPort::ScoreViewPort) &&
	     (static_cast<CAScoreViewPort*>(_activeViewPort)->currentContext()) &&
	     (static_cast<CAScoreViewPort*>(_activeViewPort)->currentContext()->context()->contextType()==CAContext::Staff) ) {
		CAStaff *staff = static_cast<CAStaff*>(static_cast<CAScoreViewPort*>(_activeViewPort)->currentContext()->context());
		int voiceNum = mpoVoiceNum->getRealValue()-1<0?0:mpoVoiceNum->getRealValue()-1;
		CAVoice *voice = staff->voiceAt( voiceNum );
		CASourceViewPort *v = new CASourceViewPort(voice, _activeViewPort->parent());
		_currentScrollWidget->addViewPort(v);
		
		connect(v, SIGNAL(CACommit(CASourceViewPort*, QString)), this, SLOT(sourceViewPortCommit(CASourceViewPort*, QString)));
		
		v->setWindowIcon(QIcon(QString::fromUtf8(":/menu/images/clogosm.png")));
		v->setFocusPolicy(Qt::ClickFocus);
		v->setFocus();
		
		moMainWin.actionUnsplit_all->setEnabled(true);
		moMainWin.actionClose_current_view->setEnabled(true);
		_viewPortList.append(v);
		setMode(_currentMode);	//updates the new viewport border settings
	}
}
