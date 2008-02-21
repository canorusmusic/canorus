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

#include "interface/playback.h"

#include "widgets/viewportcontainer.h"
#include "widgets/scoreviewport.h"

class QKeyEvent;
class QSlider;
class QSpinBox;
class QToolBar;
class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QAction;

class CAMenuToolButton;
class CAUndoToolButton;
class CALCDNumber;
class CASheet;
class CAScoreViewPort;
class CASourceViewPort;
class CAMusElementFactory;

class CAMainWin : public QMainWindow, private Ui::uiMainWindow
{
	Q_OBJECT

public:
	enum CAMode {
		NoDocumentMode,
		InsertMode,
		SelectMode,
		EditMode,
		ReadOnlyMode
	};

	CAMainWin(QMainWindow *oParent = 0);
	~CAMainWin();

	void clearUI();
	void rebuildUI(CASheet *sheet, bool repaint=true);
	void rebuildUI(bool repaint=true);
	inline bool rebuildUILock() { return _rebuildUILock; }
	
	void newDocument();	
	void addSheet(CASheet *s);
	void removeSheet(CASheet *s);
	void insertMusElementAt( const QPoint coords, CAScoreViewPort *v );
	void restartTimeEditedTime() { _timeEditedTime = 0; };
	void deleteSelection( CAScoreViewPort *v, bool deleteSyllable, bool deleteNotes, bool undo );
	void copySelection( CAScoreViewPort *v );
	void pasteAt( const QPoint coords, CAScoreViewPort *v );
	
	CADocument *openDocument( const QString& fileName );
	CADocument *openDocument( CADocument* doc );
	bool saveDocument( QString fileName );
	
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
	
	// Dialogs, Windows
	static QFileDialog *uiSaveDialog;
	static QFileDialog *uiOpenDialog;
	static QFileDialog *uiExportDialog;
	static QFileDialog *uiImportDialog;
	
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
	void on_uiOpenRecent_aboutToShow();
	void onUiOpenRecentDocumentTriggered();
	
	// Edit
	void on_uiUndo_toggled(bool, int);
	void on_uiRedo_toggled(bool, int);
	void on_uiCopy_triggered();
	void on_uiCut_triggered();
	void on_uiPaste_triggered();
	void on_uiSelectAll_triggered();
	void on_uiInvertSelection_triggered();
	void on_uiDocumentProperties_triggered();
	
	// Insert
	void on_uiSelectMode_toggled(bool);
	void on_uiEditMode_toggled(bool);
	void on_uiNewSheet_triggered();
	void on_uiNewVoice_triggered();
	void on_uiContextType_toggled(bool, int);
	void on_uiClefType_toggled(bool, int);
	void on_uiInsertKeySig_toggled(bool);
	void on_uiTimeSigType_toggled(bool, int);
	void on_uiBarlineType_toggled(bool, int);
	void on_uiInsertPlayable_toggled(bool);
	void on_uiSlurType_toggled(bool, int);
	void on_uiMarkType_toggled(bool, int);
	void on_uiArticulationType_toggled(bool, int);
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
	void on_uiSheetProperties_triggered();
	
	// Context
	void on_uiContextName_returnPressed();
	void on_uiRemoveContext_triggered();
	void on_uiStanzaNumber_valueChanged(int);
	void on_uiAssociatedVoice_activated(int);
	void on_uiContextProperties_triggered();
	
	// Playback
	void on_uiPlayFromSelection_toggled(bool);
	
	// Playable
	void on_uiPlayableLength_toggled(bool, int);
	void on_uiNoteStemDirection_toggled(bool, int);
	void on_uiHiddenRest_toggled( bool checked );
	void onMidiInEvent( QVector<unsigned char> message );
	
	// Key Signature
	void on_uiKeySig_activated( int );
	
	// Time Signature
	void on_uiTimeSigBeats_valueChanged(int);
	void on_uiTimeSigBeat_valueChanged(int);
	
	// Clef
	void on_uiClefOffset_valueChanged(int);
	
	// Lyrics
	void onTextEditKeyPressEvent(QKeyEvent *);
	
	// Function marks
	void on_uiFMFunction_toggled(bool, int);
	void on_uiFMChordArea_toggled(bool, int);
	void on_uiFMTonicDegree_toggled(bool, int);
	void on_uiFMEllipse_toggled(bool);
	
	// Dynamic marks
	void on_uiDynamicText_toggled(bool, int);
	void on_uiDynamicVolume_valueChanged(int);
	void on_uiDynamicCustomText_returnPressed();
	
	// Instrument change
	void on_uiInstrumentChange_activated( int );
	
	// Fermata
	void on_uiFermataType_toggled( bool, int );
	
	// Repeat Mark
	void on_uiRepeatMarkType_toggled( bool, int );
	
	// Fingering
	void on_uiFinger_toggled( bool checked, int t );
	void on_uiFingeringOriginal_toggled( bool checked );
	
	// Tempo
	void on_uiTempoBeat_toggled( bool, int );
	void on_uiTempoBpm_returnPressed();
	
	// Tools
	void on_uiSettings_triggered();
	
	// Voice
	void on_uiVoiceNum_valChanged(int);
	void on_uiVoiceName_returnPressed();
	void on_uiVoiceInstrument_activated(int);
	void on_uiRemoveVoice_triggered();
	void on_uiVoiceStemDirection_toggled(bool, int);
	void on_uiVoiceProperties_triggered();
	
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
	
	void onTimeEditedTimerTimeout();
	
	void playbackFinished();
	void onScoreViewPortSelectionChanged();
	void onRepaintTimerTimeout();
	
private:	
	////////////////////////////////////
	// General properties and methods //
	////////////////////////////////////
	CADocument *_document;
	CAMode _mode;
	
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
	QList<CADrawableMusElement*> _prePlaybackSelection;
	QTimer *_repaintTimer;
	bool _rebuildUILock;
	inline void setRebuildUILock(bool l) { _rebuildUILock = l; }
	
	CAPlayback *_playback;
	QTimer _timeEditedTimer;
	unsigned int  _timeEditedTime;
	CAMusElementFactory *_musElementFactory;
	inline CAMusElementFactory *musElementFactory() { return _musElementFactory; }
	inline bool stopPlayback() { 
			if(_playback && _playback->isRunning())
				_playback->stopNow();
		}
	
	///////////////////////////////////////////////////////////////////////////
	// Pure user interface - widgets that weren't created by Qt Designer yet //
	///////////////////////////////////////////////////////////////////////////
	void createCustomActions();
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
	void updateClefToolBar();
	void updateFMToolBar();
	void updateDynamicToolBar();
	void updateInstrumentToolBar();
	void updateTempoToolBar();
	void updateFermataToolBar();
	void updateRepeatMarkToolBar();
	void updateFingeringToolBar();
	
		/////////////////////
		// Toolbar section //
		/////////////////////
		// Standard toolbar
		//QToolBar	*uiStandardToolBar;
			CAUndoToolButton	*uiUndo;
			CAUndoToolButton	*uiRedo;
		
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
			CAMenuToolButton *uiMarkType;
			CAMenuToolButton *uiArticulationType;
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
			// CALyricsContext
			QSpinBox         *uiStanzaNumber;
			QAction          *uiStanzaNumberAction;
			QComboBox        *uiAssociatedVoice;
			QAction          *uiAssociatedVoiceAction;
			// CAFunctionMarkContext
		
		QToolBar *uiVoiceToolBar;
			// QAction       *uiNewVoice;  // made by Qt Designer
			CALCDNumber      *uiVoiceNum;
			QLineEdit        *uiVoiceName;
			QComboBox        *uiVoiceInstrument;
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
			QComboBox *uiKeySig;
			// QComboBox    *uiKeySigGender;
		
		QToolBar *uiClefToolBar;
			QSpinBox *uiClefOffset;
			int oldUiClefOffsetValue;
		
		QToolBar *uiTimeSigToolBar;
			QSpinBox         *uiTimeSigBeats;
			QLabel           *uiTimeSigSlash;
			QSpinBox         *uiTimeSigBeat;
			// CAMenuToolButton *uiTimeSigStyle; /// \todo Implement it. -Matevz
		
		QToolBar *uiFMToolBar;                       // function mark tool bar
			CAMenuToolButton  *uiFMFunction;
			CAMenuToolButton  *uiFMChordArea;
			CAMenuToolButton  *uiFMTonicDegree;
			QComboBox         *uiFMKeySig;
			//QSpinBox        *uiKeySigNumberOfAccs; // defined in uiKeySigToolBar
			//QComboBox       *uiKeySigGender;       // defined in uiKeySigToolBar
			//QAction         *uiFMEllipse;          // made by Qt Designer
		
		// Marks tool bars:
		QToolBar *uiDynamicToolBar;
			CAMenuToolButton *uiDynamicText;
			QSpinBox         *uiDynamicVolume;
			QLineEdit        *uiDynamicCustomText;
		QToolBar *uiInstrumentToolBar;
			QComboBox        *uiInstrumentChange;
		QToolBar *uiTempoToolBar;
			CAMenuToolButton *uiTempoBeat;
			QLabel           *uiTempoEquals;
			QLineEdit        *uiTempoBpm;
		QToolBar *uiFermataToolBar;
			CAMenuToolButton *uiFermataType;
		QToolBar *uiRepeatMarkToolBar;
			CAMenuToolButton *uiRepeatMarkType;
		QToolBar *uiFingeringToolBar;
			CAMenuToolButton *uiFinger;
			QCheckBox        *uiFingeringOriginal;
};
#endif /* MAINWIN_H_ */
