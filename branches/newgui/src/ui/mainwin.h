/** \file ui/mainwin.h
 * 
 * Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef MAINWIN_H_
#define MAINWIN_H_

#include <QObject>
#include <QFileDialog>

#include "ui_mainwin.h"

#include "core/document.h"
#include "core/muselement.h"
#include "core/note.h"
#include "core/clef.h"

#include "widgets/viewportcontainer.h"

class QKeyEvent;
class QSlider;
class QSpinBox;
class QToolBar;
class QLabel;
class QLineEdit;
class QComboBox;

class CAPlayback;
class CAMenuToolButton;
class CALCDNumber;
class CASheet;
class CAKeySigPSP;
class CATimeSigPSP;
class CAViewPort;
class CAScoreViewPort;
class CASourceViewPort;
class CAMusElementFactory;

enum CAMode {
	NoDocumentMode,
	InsertMode,
	SelectMode,
	EditMode,
	ReadOnlyMode
};

class CAMainWin : public QMainWindow, private Ui::uiMainWindow
{
	Q_OBJECT

public:
	CAMainWin(QMainWindow *oParent = 0);
	~CAMainWin();

	void clearUI();
	void rebuildUI(CASheet *sheet=0, bool repaint=true);
	
	void newDocument();	
	void addSheet(CASheet *s);
	void insertMusElementAt(const QPoint coords, CAScoreViewPort *v, CAMusElement &roMusElement );

	bool openDocument(QString fileName);
	bool saveDocument(QString fileName);
	
	inline CAMode mode() { return _mode; }
	inline QFileDialog *exportDialog() { return uiExportDialog; }
	inline QFileDialog *importDialog() { return uiImportDialog; }	
	inline CAViewPort *currentViewPort() { return _currentViewPort; }
	inline CAViewPortContainer *currentViewPortContainer() { return _currentViewPortContainer; }
	inline CADocument *document() { return _document; }
	
	inline void setDocument(CADocument *document) { _document = document; }
	
private slots:
	///////////////////////////
	// ToolBar/Menus actions //
	///////////////////////////
	void closeEvent(QCloseEvent *event);
	// File
	void on_uiNewDocument_triggered();
	void on_uiOpenDocument_triggered();
	void on_uiSaveDocument_triggered();
	void on_uiSaveDocumentAs_triggered();
	void on_uiExportDocument_triggered();
	void on_uiImportDocument_triggered();
	
	// Edit
	void on_uiMIDISetup_triggered();
	
	// Insert
	void on_uiSelectMode_toggled(bool);
	void on_uiNewSheet_triggered();
	void on_uiNewContext_toggled(bool);      // menu
	void on_uiContextType_toggled(bool, int);
	void on_uiNewVoice_triggered();
	void on_uiInsertClef_toggled(bool);      // menu
	void on_uiClefType_toggled(bool, int);
	void on_uiInsertKeySignature_toggled(bool); // menu
	void on_uiInsertTimeSignature_toggled(bool); // menu
	void on_uiTimeSigType_toggled(bool, int);
	void on_uiBarlineType_toggled(bool, int);
	void on_uiInsertPlayable_toggled(bool);  // menu
	
	// View
	void on_uiFullscreen_toggled(bool);
	void on_uiAnimatedScroll_toggled(bool);
	void on_uiLockScrollPlayback_toggled(bool);
	void on_uiZoomToSelection_triggered();
	void on_uiZoomToFit_triggered();
	void on_uiZoomToWidth_triggered();
	void on_uiZoomToHeight_triggered();
	void on_uiViewLilyPondSource_triggered();
	void on_uiViewCanorusMLSource_triggered();
	
	// Playback
	void on_uiPlayFromSelection_toggled(bool);
	
	// Playable
	void on_uiPlayableLength_toggled(bool, int);
	
	// Voice
	void on_uiVoiceNum_valChanged(int);
	
	// Window
	void on_uiSplitHorizontally_triggered();
	void on_uiSplitVertically_triggered();
	void on_uiUnsplitAll_triggered();
	void on_uiCloseCurrentView_triggered();
	void on_uiNewViewport_triggered();
	void on_uiNewWindow_triggered();
	
	// Help
	void on_uiAboutCanorus_triggered();
	void on_uiAboutQt_triggered();
	
	//////////////////////////////////
	// Handle other widgets signals //
	//////////////////////////////////
	void keyPressEvent(QKeyEvent *);
	void on_uiTabWidget_currentChanged(int);
	
	void viewPortMousePressEvent(QMouseEvent *e, const QPoint coords, CAViewPort *v);
	void viewPortMouseMoveEvent(QMouseEvent *e, const QPoint coords, CAViewPort *v);
	void viewPortWheelEvent(QWheelEvent *e, const QPoint coords, CAViewPort *v);
	void viewPortKeyPressEvent(QKeyEvent *e, CAViewPort *v);
	void sourceViewPortCommit(CASourceViewPort*, QString inputString);
	
	void playbackFinished();
	//void on_repaintTimer_timeout();	///Used for repaint events

private:	
	////////////////////////////////////
	// General properties and methods //
	////////////////////////////////////
	CADocument *_document;
	CAMode _mode;
	
	void doUnsplit(CAViewPort *v = 0);
	
	void setMode(CAMode mode);
	inline void setCurrentViewPort( CAViewPort *viewPort ) { _currentViewPort = viewPort; }
	inline void setCurrentViewPortContainer( CAViewPortContainer *vpc )
		{ _currentViewPortContainer = vpc; }
	
	CAViewPortContainer *_currentViewPortContainer;
	QList<CAViewPortContainer *>_viewPortContainerList;
	
	QList<CAViewPort *> _viewPortList;
	CAViewPort *_currentViewPort;
	bool _animatedScroll;
	bool _lockScrollPlayback;
	CAViewPort *_playbackViewPort;
	QTimer *_repaintTimer;
	
	CAPlayback *_playback;
	CAMusElementFactory *_musElementFactory;
	
	///////////////////////////////////////////////////////////////////////////
	// Pure user interface - widgets that weren't created by Qt Designer yet //
	///////////////////////////////////////////////////////////////////////////
	void setupCustomUi();
	void initScoreViewPort(CAScoreViewPort*);
	void updateVoiceToolBar();
	void updateContextToolBar();
	
	// Dialogs, Windows
	QFileDialog *uiExportDialog;
	QFileDialog *uiImportDialog;
	
		/////////////////////
		// Toolbar section //
		/////////////////////
		// Insert toolbar
		QToolBar     *uiInsertToolBar;
			QActionGroup *uiInsertGroup;           // Group for mutual exclusive selection of music elements
			// QAction       *uiSelectMode; // made by Qt Designer
			// QAction       *uiInsertNewSheet; // made by Qt Designer
			CAMenuToolButton *uiContextType;
			
			// QAction       *uiInsertPlayable;  // made by Qt Designer
			CAMenuToolButton *uiClefType;
			// QAction       *uiInsertKeySig;  // made by Qt Designer
			CAMenuToolButton *uiTimeSigType;  // made by Qt Designer
			CAMenuToolButton *uiBarlineType;
			// QAction       *uiInsertFM;  // made by Qt Designer
		
		QToolBar *uiContextToolBar;
			QLineEdit        *uiContextName;
			QSpinBox         *uiStaffNumberOfLines;
			//QAction          *uiRemoveContext; // made by Qt Designer
			//QAction          *uiContextProperties; // made by Qt Designer
		
		QToolBar *uiVoiceToolBar;
			// QAction       *uiNewVoice;  // made by Qt Designer
			CALCDNumber      *uiVoiceNum;
			QLineEdit        *uiVoiceName;
			// QAction       *uiRemoveVoice; // made by Qt Designer
			CAMenuToolButton *uiVoiceStemDirection;
			// QAction       *uiVoiceProperties; // made by Qt Designer
			
		QToolBar *uiPlayableToolBar; // note and rest properties are merged for the time being
			// Note properties
			CAMenuToolButton *uiPlayableLength;
			QLabel           *uiPlayableDotted;
			CAMenuToolButton *uiNoteAccs;
			// QAction       *uiNoteAccsVisible;
			CAMenuToolButton *uiNoteStemDirection;
			// Rest properties
			// CAMenuToolButton *uiPlayableLength; // same as note properties
			// QLabel        *uiPlayableDotted; // same as note properties
			// QAction          *uiHiddenRest; // made by Qt Designer
		
		QToolBar *uiKeySigToolBar;
			CAKeySigPSP  *uiKeySigPSP;	            // Key signature perspective
			QSpinBox     *uiKeySigNumberOfAccs;
			QComboBox    *uiKeySigGender;
		
		QToolBar *uiTimeSigToolBar;
			QSpinBox         *uiTimeSigBeats;
			QLabel           *uiTimeSigSlash;
			QSpinBox         *uiTimeSigBeat;
			CAMenuToolButton *uiTimeSigStyle;
		
		QToolBar *uiFMToolBar; // function marking tool bar
			CAMenuToolButton  *uiFMType;
			CAMenuToolButton  *uiFMChordArea;
			CAMenuToolButton  *uiFMTonicDegree;
			//QSpinBox        *uiKeySigNumberOfAccs; // defined in uiKeySigToolBar
			//QComboBox       *uiKeySigGender; // defined in uiKeySigToolBar
			QAction           *uiFMEllipse;
};
#endif /* CANORUS_H_ */
