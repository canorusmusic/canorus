/*!
	Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef MAINWIN_H_
#define MAINWIN_H_

#include <QObject>
#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <QHash>

#include "ui_mainwin.h"

#include "core/document.h"
#include "core/muselement.h"
#include "core/note.h"
#include "core/clef.h"

#include "widgets/viewportcontainer.h"
#include "widgets/scoreviewport.h"

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
	void rebuildUI(CASheet *sheet, bool repaint=true);
	void rebuildUI(bool repaint=true);
	inline bool rebuildUILock() { return _rebuildUILock; }
	
	void newDocument();	
	void addSheet(CASheet *s);
	void removeSheet(CASheet *s);
	void insertMusElementAt(const QPoint coords, CAScoreViewPort *v);
	void restartTimeEditedTime() { _timeEditedTime.restart(); };
	
	bool openDocument(QString fileName);
	bool saveDocument(QString fileName);
	
	inline CAMode mode() { return _mode; }
	inline QFileDialog *exportDialog() { return uiExportDialog; }
	inline QFileDialog *importDialog() { return uiImportDialog; }	
	inline CAViewPort *currentViewPort() { return _currentViewPort; }
	inline void removeViewPort(CAViewPort* v) { _viewPortList.removeAll(v); }
	inline QList<CAViewPort*> viewPortList() { return _viewPortList; }
	
	inline CAScoreViewPort *currentScoreViewPort() {
		if (currentViewPort()) return dynamic_cast<CAScoreViewPort*>(currentViewPort());
		else return 0;
	}
	
	inline CASheet *currentSheet() {
		CAScoreViewPort *v = currentScoreViewPort();
		if (v) return v->sheet();
		else return 0;
	}
	
	inline CAStaff *currentStaff() {
		CAContext *context = currentContext();
		if (context && context->contextType()==CAContext::Staff) return static_cast<CAStaff*>(context);
		else return 0;
	}
	
	CAContext *currentContext();
	CAVoice   *currentVoice();
	inline CAViewPortContainer *currentViewPortContainer() { return _currentViewPortContainer; }
	inline CADocument *document() { return _document; }
	
	inline void setDocument(CADocument *document) { _document = document; }
	static const QString LILYPOND_FILTER;
	
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
	void on_uiCloseDocument_triggered();
	void on_uiExportDocument_triggered();
	void on_uiImportDocument_triggered();
	
	// Edit
	void on_uiUndo_triggered();
	void on_uiRedo_triggered();
	
	// Insert
	void on_uiSelectMode_toggled(bool);
	void on_uiNewSheet_triggered();
	void on_uiNewVoice_triggered();
	void on_uiContextType_toggled(bool, int);
	void on_uiClefType_toggled(bool, int);
	void on_uiInsertKeySig_toggled(bool);
	void on_uiTimeSigType_toggled(bool, int);
	void on_uiBarlineType_toggled(bool, int);
	void on_uiInsertPlayable_toggled(bool);
	void on_uiSlurType_toggled(bool, int);
	void on_uiInsertSyllable_toggled(bool);
	void on_uiInsertFM_toggled(bool);
	
	// View
	void on_uiFullscreen_toggled(bool);
	void on_uiAnimatedScroll_toggled(bool);
	void on_uiLockScrollPlayback_toggled(bool);
	void on_uiZoomToSelection_triggered();
	void on_uiZoomToFit_triggered();
	void on_uiZoomToWidth_triggered();
	void on_uiZoomToHeight_triggered();
	void on_uiScoreView_triggered();
	void on_uiLilyPondSource_triggered();
	void on_uiCanorusMLSource_triggered();
	
	// Sheet
	void on_uiRemoveSheet_triggered();
	void on_uiSheetName_returnPressed();
	
	// Context
	void on_uiContextName_returnPressed();
	void on_uiRemoveContext_triggered();
	void on_uiStaffNumberOfLines_valueChanged(int);
	void on_uiStanzaNumber_valueChanged(int);
	void on_uiAssociatedVoice_currentIndexChanged(int);
	
	// Playback
	void on_uiPlayFromSelection_toggled(bool);
	
	// Playable
	void on_uiPlayableLength_toggled(bool, int);
	void on_uiNoteStemDirection_toggled(bool, int);
	void on_uiHiddenRest_toggled( bool checked );
	void on_midiInEvent( QVector<unsigned char> message );
	
	// Key Signature
	void on_uiKeySigNumberOfAccs_valChanged(int);
	
	// Time Signature
	void on_uiTimeSigBeats_valChanged(int);
	void on_uiTimeSigBeat_valChanged(int);
	
	// Lyrics
	void on_syllableEdit_keyPressEvent(QKeyEvent *, CASyllableEdit*);
	
	// Function marking
	void on_uiFMFunction_toggled(bool, int);
	void on_uiFMChordArea_toggled(bool, int);
	void on_uiFMTonicDegree_toggled(bool, int);
	void on_uiFMEllipse_toggled(bool);
	
	// Tools
	void on_uiSettings_triggered();
	
	// Voice
	void on_uiVoiceNum_valChanged(int);
	void on_uiVoiceName_returnPressed();
	void on_uiRemoveVoice_triggered();
	void on_uiVoiceStemDirection_toggled(bool, int);
	
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
	
	void scoreViewPortMousePress(QMouseEvent *e, const QPoint coords, CAScoreViewPort *v);
	void scoreViewPortMouseMove(QMouseEvent *e, const QPoint coords, CAScoreViewPort *v);
	void scoreViewPortMouseRelease(QMouseEvent *e, const QPoint coords, CAScoreViewPort *v);
	void scoreViewPortWheel(QWheelEvent *e, const QPoint coords, CAScoreViewPort *v);
	void scoreViewPortKeyPress(QKeyEvent *e, CAScoreViewPort *v);
	void sourceViewPortCommit(QString inputString, CASourceViewPort*);
	
	void on_timeEditedTimer_timeout();
	
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
	QHash<CAViewPortContainer*, CASheet*> _sheetMap;
	CAViewPort *_currentViewPort;
	bool _animatedScroll;
	bool _lockScrollPlayback;
	CAViewPort *_playbackViewPort;
	QTimer *_repaintTimer;
	bool _rebuildUILock;
	inline void setRebuildUILock(bool l) { _rebuildUILock = l; }
	
	CAPlayback *_playback;
	QTimer _timeEditedTimer;
	QTime  _timeEditedTime;
	CAMusElementFactory *_musElementFactory;
	inline CAMusElementFactory *musElementFactory() { return _musElementFactory; }
	
	///////////////////////////////////////////////////////////////////////////
	// Pure user interface - widgets that weren't created by Qt Designer yet //
	///////////////////////////////////////////////////////////////////////////
	void setupCustomUi();
	void initViewPort(CAViewPort*);
	void updateUndoRedoButtons();
	void updateToolBars();
	void updateSheetToolBar();
	void updateContextToolBar();
	void updateVoiceToolBar();
	void updateInsertToolBar();
	void updatePlayableToolBar();
	void updateKeySigToolBar();
	void updateTimeSigToolBar();
	void updateFMToolBar();
	
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
			// QAction       *uiNewSheet; // made by Qt Designer
			CAMenuToolButton *uiContextType;
			
			// QAction       *uiInsertPlayable;  // made by Qt Designer
			CAMenuToolButton *uiClefType;
			// QAction       *uiInsertKeySig;  // made by Qt Designer
			CAMenuToolButton *uiTimeSigType;  // made by Qt Designer
			CAMenuToolButton *uiBarlineType;
			// QAction       *uiInsertSyllable;  // made by Qt Designer
			// QAction       *uiInsertFM;  // made by Qt Designer
		
		QToolBar *uiSheetToolBar;
			// QAction        *uiNewSheet; // made by Qt Designer
			QLineEdit         *uiSheetName;
			// QAction        *uiRemoveSheet; // made by Qt Designer
			// QAction        *uiSheetProperties; // made by Qt Designer
			
		QToolBar *uiContextToolBar;
			// CAContext
			QLineEdit        *uiContextName;
			//QAction          *uiRemoveContext; // made by Qt Designer
			//QAction          *uiContextProperties; // made by Qt Designer
			// CAStaff
			QSpinBox         *uiStaffNumberOfLines;
			// CALyricsContext
			QSpinBox         *uiStanzaNumber;
			QComboBox        *uiAssociatedVoice;
			// CAFunctionMarkingContext
		
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
			CAMenuToolButton *uiNoteAccs;
			CAMenuToolButton *uiSlurType;
			// QAction       *uiNoteAccsVisible; // made by Qt Designer
			CAMenuToolButton *uiNoteStemDirection;
			// Rest properties
			// CAMenuToolButton *uiPlayableLength; // same as note properties
			// QLabel        *uiPlayableDotted; // same as note properties
			// QAction          *uiHiddenRest; // made by Qt Designer
		
		QToolBar *uiKeySigToolBar;
			// CAKeySigPSP  *uiKeySigPSP;	            /// Key signature perspective. \todo Reimplement it.
			QSpinBox     *uiKeySigNumberOfAccs;
			// QComboBox    *uiKeySigGender;
		
		QToolBar *uiTimeSigToolBar;
			QSpinBox         *uiTimeSigBeats;
			QLabel           *uiTimeSigSlash;
			QSpinBox         *uiTimeSigBeat;
			// CAMenuToolButton *uiTimeSigStyle; /// \todo Implement it. -Matevz
		
		QToolBar *uiFMToolBar; // function marking tool bar
			CAMenuToolButton  *uiFMFunction;
			CAMenuToolButton  *uiFMChordArea;
			CAMenuToolButton  *uiFMTonicDegree;
			//QSpinBox        *uiKeySigNumberOfAccs; // defined in uiKeySigToolBar
			//QComboBox       *uiKeySigGender; // defined in uiKeySigToolBar
			//QAction         *uiFMEllipse; // made by Qt Designer
};
#endif /* MAINWIN_H_ */
