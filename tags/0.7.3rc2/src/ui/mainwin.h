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

#include "control/mainwinprogressctl.h"

#include "core/notechecker.h"

#include "score/document.h"
#include "score/muselement.h"
#include "score/note.h"
#include "score/clef.h"

#include "interface/playback.h"
#include "interface/pyconsoleinterface.h"

#include "widgets/viewcontainer.h"
#include "widgets/scoreview.h"
#include "widgets/resourceview.h"

class QKeyEvent;
class QSlider;
class QSpinBox;
class QToolBar;
class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QAction;

class CAKeySignatureUI;

class CAMainWinProgressCtl;
class CAHelpBrowser;
class CAMenuToolButton;
class CAUndoToolButton;
class CALCDNumber;
class CASheet;
class CAScoreView;
class CASourceView;
class CAMusElementFactory;
class CAPrintPreviewCtl;
class CAPrintCtl;
class CAPreviewCtl;
class CAPyConsole;
class CATransposeView;
class CAMidiRecorderView;
class CAKeybdInput;
class CAExport;
class CAActionStorage;

class CAMainWin : public QMainWindow, private Ui::uiMainWindow
{
	Q_OBJECT

	friend class CAMainWinProgressCtl;
    friend class CAActionDelegate;
    friend class CAActionStorage;

public:
	enum CAMode {
		NoDocumentMode,
		ProgressMode,
		InsertMode,
		EditMode,
		ReadOnlyMode
	};

	CAMainWin(QMainWindow *oParent = 0);
	~CAMainWin();

	void clearUI();
	void rebuildUI(CASheet *sheet, bool repaint=true);
	void rebuildUI(bool repaint=true);
	inline bool rebuildUILock() { return _rebuildUILock; }
	void updateWindowTitle();

	void newDocument();
	void addSheet(CASheet *s);
	void removeSheet(CASheet *s);
	bool insertMusElementAt( const QPoint coords, CAScoreView *v );
	void restartTimeEditedTime() { _timeEditedTime = 0; };
	void deleteSelection( CAScoreView *v, bool deleteSyllable, bool deleteNotes, bool undo );
	void copySelection( CAScoreView *v );
	void pasteAt( const QPoint coords, CAScoreView *v );

	CADocument *openDocument( const QString& fileName );
	CADocument *openDocument( CADocument* doc );
	bool saveDocument( QString fileName );

	void setMode(CAMode mode, const QString &oModeHash);
	inline CAMode mode() { return _mode; }
	inline QFileDialog *exportDialog() { return uiExportDialog; }
	inline QFileDialog *importDialog() { return uiImportDialog; }
	inline CAResourceView *resourceView() { return _resourceView; }
	inline QAction        *resourceViewAction() { return uiResourceView; }
	inline CAMidiRecorderView *midiRecorderView() { return _midiRecorderView; }
	inline void setMidiRecorderView( CAMidiRecorderView *v ) { _midiRecorderView = v; }
	inline CAView *currentView() { return _currentView; }
	inline void removeView(CAView* v) { _viewList.removeAll(v); }
	inline const QList<CAView*>& viewList() const { return _viewList; }

	inline CAScoreView *currentScoreView() {
		if (currentView()) return dynamic_cast<CAScoreView*>(currentView());
		else return 0;
	}

	CASheet *currentSheet();

	inline CAStaff *currentStaff() {
		CAContext *context = currentContext();
		if (context && context->contextType()==CAContext::Staff) return static_cast<CAStaff*>(context);
		else return 0;
	}

	CAContext *currentContext();
	void       setCurrentVoice( CAVoice* );
	CAVoice   *currentVoice();
	inline CAViewContainer *currentViewContainer() { return _currentViewContainer; }
	inline CADocument *document() { return _document; }

	inline void setDocument(CADocument *document) { _document = document; _resourceView->setDocument( document ); }
	inline bool isInsertKeySigChecked() { return uiInsertKeySig->isChecked(); }

	// Dialogs, Windows
	static QFileDialog *uiSaveDialog;
	static QFileDialog *uiOpenDialog;
	static QFileDialog *uiExportDialog;
	static QFileDialog *uiImportDialog;

	// Python Console
	CAPyConsole *pyConsole;
	CAPyConsoleInterface* pyConsoleIface;

	QDockWidget *helpDock() { return uiHelpDock; }
	CAHelpBrowser *helpWidget() { return uiHelpWidget; }

private slots:
	///////////////////////////
	// ToolBar/Menus actions //
	///////////////////////////
	void closeEvent(QCloseEvent *event);
	// File
	void on_uiNewDocument_triggered();
	void on_uiOpenDocument_triggered();
	bool on_uiSaveDocument_triggered();
	bool on_uiSaveDocumentAs_triggered();
	void on_uiCloseDocument_triggered();
	void on_uiExportDocument_triggered();
	void on_uiImportDocument_triggered();
	void on_uiExportToPdf_triggered();
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
	void on_uiEditMode_toggled(bool);
	void on_uiNewSheet_triggered();
	void on_uiNewVoice_triggered();
	void on_uiContextType_toggled(bool, int);
	void on_uiClefType_toggled(bool, int);
	void on_uiTimeSigType_toggled(bool, int);
	void on_uiBarlineType_toggled(bool, int);
	void on_uiInsertPlayable_toggled(bool);
	void on_uiSlurType_toggled(bool, int);
	void on_uiMarkType_toggled(bool, int);
	void on_uiArticulationType_toggled(bool, int);
	void on_uiInsertSyllable_toggled(bool);
	void on_uiInsertFBM_toggled(bool);
	void on_uiInsertFM_toggled(bool);

	// View
	void on_uiFullscreen_toggled(bool);
	void on_uiLockScrollPlayback_toggled(bool);
	void on_uiZoomToSelection_triggered();
	void on_uiZoomToFit_triggered();
	void on_uiZoomToWidth_triggered();
	void on_uiZoomToHeight_triggered();
	void on_uiScoreView_triggered();
	void on_uiLilyPondSource_triggered();
	void on_uiCanorusMLSource_triggered();
	void on_uiResourceView_toggled(bool);
	void on_uiShowRuler_toggled(bool);

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
	void on_uiTupletType_toggled(bool, int);
	void on_uiTupletNumber_valueChanged(int);
	void on_uiTupletActualNumber_valueChanged(int);
	void on_uiNoteStemDirection_toggled(bool, int);
	void on_uiHiddenRest_toggled( bool checked );
	void onMidiInEvent( QVector<unsigned char> message );

	// Time Signature
	void on_uiTimeSigBeats_valueChanged(int);
	void on_uiTimeSigBeat_valueChanged(int);

	// Clef
	void on_uiClefOffset_valueChanged(int);

	// Lyrics
	void onTextEditKeyPressEvent(QKeyEvent *);
	void confirmTextEdit(CAScoreView *v, CATextEdit *textEdit, CAMusElement *elt);

	// Function marks
	void on_uiFMFunction_toggled(bool, int);
	void on_uiFMChordArea_toggled(bool, int);
	void on_uiFMTonicDegree_toggled(bool, int);
	void on_uiFMEllipse_toggled(bool);

	// Figured bass marks
	void on_uiFBMNumber_toggled(bool, int);
	void on_uiFBMAccs_toggled(bool, int);

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
	void on_uiTranspose_triggered();
	void on_uiMidiRecorder_triggered();

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
	void on_uiNewView_triggered();
	void on_uiNewWindow_triggered();

	// Help
public slots:
	void on_uiUsersGuide_triggered();
private slots:
	void on_uiAboutCanorus_triggered();
	void on_uiAboutQt_triggered();

	//////////////////////////////////
	// Handle other widgets signals //
	//////////////////////////////////
	void keyPressEvent(QKeyEvent *);
	void on_uiTabWidget_currentChanged(int);
	void on_uiTabWidget_CANewTab();
	void on_uiTabWidget_CAMoveTab(int from, int to);

	void viewClicked();

	void scoreViewMousePress(QMouseEvent *e, const QPoint coords);
	void scoreViewMouseMove(QMouseEvent *e, const QPoint coords);
	void scoreViewMouseRelease(QMouseEvent *e, const QPoint coords);
	void scoreViewDoubleClick(QMouseEvent *e, const QPoint coords);
	void scoreViewTripleClick(QMouseEvent *e, const QPoint coords);
	void scoreViewWheel(QWheelEvent *e, const QPoint coords);
	void scoreViewKeyPress(QKeyEvent *e);
	void sourceViewCommit(QString inputString);
	void floatViewClosed(CAView*);

	void onTimeEditedTimerTimeout();

	void playbackFinished();
	void onScoreViewSelectionChanged();
	void onRepaintTimerTimeout();

	////////////////////////////////
	// Handle progress bar events //
	////////////////////////////////
	void onImportDone( int status );
	void onExportDone( int status );

private:
	void playImmediately( QList<CAMusElement*> elements );

	////////////////////////
	// General properties //
	////////////////////////
	CADocument *_document;
	CAMode _mode;

	CAPreviewCtl *_poPrintPreviewCtl;
	CAPrintCtl   *_poPrintCtl;
	CAExport *_poExp; // abstract export instance
	CAResourceView *_resourceView;
	CATransposeView *_transposeView;
	CAMidiRecorderView *_midiRecorderView;

	QStatusBar *_permanentStatusBar;
	CAMainWinProgressCtl _mainWinProgressCtl;

	void setMode(CAMode mode);
	QString createModeHash();
	inline void setCurrentView( CAView *view ) { _currentView = view; }
	inline void setCurrentViewContainer( CAViewContainer *vpc )
		{ _currentViewContainer = vpc; }

	CAViewContainer *_currentViewContainer;
	QList<CAViewContainer *>_viewContainerList;

	QList<CAView *> _viewList;
	QHash<CAViewContainer*, CASheet*> _sheetMap;
	QHash<QString, int> _modeHash;
	int _iNumAllowed;
	CAView *_currentView;
	CAView *_playbackView;
	QList<CADrawableMusElement*> _prePlaybackSelection;
	QTimer *_repaintTimer;
	bool _rebuildUILock;
	inline void setRebuildUILock(bool l) { _rebuildUILock = l; }

	CAPlayback *_playback;
	QTimer _timeEditedTimer;
	unsigned int  _timeEditedTime;
	CAMusElementFactory *_musElementFactory;
	CANoteChecker _noteChecker;
public:
	inline CAMusElementFactory *musElementFactory() { return _musElementFactory; }
private:
	inline bool stopPlayback() {
			if(_playback && _playback->isRunning())
				_playback->stopNow();
			return true;
		}

	bool handleUnsavedChanges();

	CAKeybdInput *_keybdInput;
	///////////////////////////////////////////////////////////////////////////
	// Pure user interface - widgets that weren't created by Qt Designer yet //
	///////////////////////////////////////////////////////////////////////////
	void createCustomActions();
	void setupCustomUi();
	void initView(CAView*);
	void updateUndoRedoButtons();
	void updateToolBars();
	void updateSheetToolBar();
	void updateContextToolBar();
	void updateVoiceToolBar();
	void updateInsertToolBar();
	void updatePlayableToolBar();
	void updateTimeSigToolBar();
	void updateClefToolBar();
	void updateFBMToolBar();
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
			// QAction       *uiInsertFBM;  // made by Qt Designer
			// QAction       *uiInsertFM;  // made by Qt Designer

		QToolBar *uiSheetToolBar;
			// QAction        *uiNewSheet; // made by Qt Designer
			QLineEdit         *uiSheetName;
			// QAction        *uiRemoveSheet; // made by Qt Designer
            // QAction        *uiSheetProperties; // made by Qt Designer

		QToolBar *uiContextToolBar;
			// CAContext
            QLineEdit         *uiContextName;
            //QAction         *uiRemoveContext; // made by Qt Designer
            //QAction         *uiContextProperties; // made by Qt Designer
			// CAStaff
			// CALyricsContext
            QSpinBox          *uiStanzaNumber;
            QAction           *uiStanzaNumberAction;
            QComboBox         *uiAssociatedVoice;
            QAction           *uiAssociatedVoiceAction;
			// CAFunctionMarkContext

		QToolBar *uiVoiceToolBar;
            // QAction        *uiNewVoice;  // made by Qt Designer
            CALCDNumber       *uiVoiceNum;
            QLineEdit         *uiVoiceName;
            QComboBox         *uiVoiceInstrument;
            // QAction        *uiRemoveVoice; // made by Qt Designer
            CAMenuToolButton  *uiVoiceStemDirection;
            // QAction        *uiVoiceProperties; // made by Qt Designer

		QToolBar *uiPlayableToolBar; // note and rest properties are merged for the time being
			// Note properties
            CAMenuToolButton  *uiPlayableLength;
            CAMenuToolButton  *uiNoteAccs;
            CAMenuToolButton  *uiSlurType;
public:		// Because CAKeyboardInput (input with midi keyboard) needs to operate these widgets to
			// provide gui feedback, these, probably even more, should become public.
			// Some clean interface would be appropriate.
            CAMenuToolButton  *uiTupletType;
            QSpinBox          *uiTupletNumber;
            QSpinBox          *uiTupletActualNumber;
private:
            QAction           *uiTupletNumberAction;
            QLabel            *uiTupletInsteadOf;
            QAction           *uiTupletInsteadOfAction;
            QAction           *uiTupletActualNumberAction;
            // QAction        *uiNoteAccsVisible; // made by Qt Designer
            CAMenuToolButton  *uiNoteStemDirection;
            CAActionStorage   *actionStorage;
			// Rest properties
			// CAMenuToolButton *uiPlayableLength; // same as note properties
            // QLabel           *uiPlayableDotted; // same as note properties
			// QAction          *uiHiddenRest; // made by Qt Designer

		CAKeySignatureUI *_poKeySignatureUI; // Key signature UI parts

		QToolBar *uiClefToolBar;
			QSpinBox *uiClefOffset;
			int oldUiClefOffsetValue;

		QToolBar *uiTimeSigToolBar;
			QSpinBox         *uiTimeSigBeats;
			QLabel           *uiTimeSigSlash;
			QSpinBox         *uiTimeSigBeat;
			// CAMenuToolButton *uiTimeSigStyle; /// \todo Implement it. -Matevz

		QToolBar *uiFBMToolBar;                      // figured bass tool bar
			CAMenuToolButton *uiFBMNumber;
			CAMenuToolButton *uiFBMAccs;

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

		// Python console
		QDockWidget *uiPyConsoleDock;

		// Help widget
		QDockWidget *uiHelpDock;
		CAHelpBrowser   *uiHelpWidget;
};
#endif /* MAINWIN_H_ */
