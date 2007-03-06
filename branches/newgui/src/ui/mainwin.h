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

class CAPluginManager;
class CAPlayback;
class CAToolBar;
class CAButtonMenu;
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

#define _currentViewPortContainer static_cast<CAViewPortContainer*>(tabWidget->currentWidget())

class CAMainWin : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	CAMainWin(QMainWindow *oParent = 0);
	~CAMainWin();

	
	void newDocument();
	
	void addSheet(CASheet *s);

	void insertMusElementAt(const QPoint coords, CAScoreViewPort *v, CAMusElement &roMusElement );

	void clearUI();
	
	CAViewPortContainer* currentViewPortContainer() { return _currentViewPortContainer; }
	
	void rebuildUI(CASheet *sheet=0, bool repaint=true);

	/**
	 * Activate the key signature perspective and show/hide it.
	 * 
	 * @param bVisible 'true': Show key signature perspective
	 */
	void setKeySigPSPVisible( bool bVisible );

	/**
	 * Activate the time signature perspective and show/hide it.
	 * 
	 * @param bVisible 'true': Show time signature perspective
	 */
	void setTimeSigPSPVisible( bool bVisible );
	
	bool openDocument(QString fileName);
	bool saveDocument(QString fileName);
	
	inline QFileDialog *exportDialog() { return _exportDialog; }
	inline QFileDialog *importDialog() { return _importDialog; }
	
	inline CADocument *document() { return _document; }
	inline void setDocument(CADocument *document) { _document = document; }
	
private slots:
	//////////////////////
	// Menu bar actions //
	//////////////////////
	void closeEvent(QCloseEvent *event);
	// File menu
	void on_actionNew_triggered();
	void on_actionNew_sheet_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSave_as_triggered();
	void on_actionExport_triggered();
	void on_actionImport_triggered();
	
	// Edit menu
	void on_actionMIDI_Setup_triggered();
	
	// Insert menu
	void on_actionNew_staff_triggered();
	void on_action_Clef_triggered();
	void on_action_Key_signature_triggered();
	void on_action_Time_signature_triggered();

	//View menu
	void on_action_Fullscreen_toggled(bool);
	void on_actionAnimated_scroll_toggled(bool);
	void on_actionLock_scroll_playback_toggled(bool);
	void on_actionZoom_to_selection_triggered();
	void on_actionZoom_to_fit_triggered();
	void on_actionZoom_to_width_triggered();
	void on_actionZoom_to_height_triggered();
	void on_actionVoice_in_LilyPond_source_toggled(bool);
	
	// Playback menu
	void on_actionPlay_toggled(bool);
	
	// Perspective menu
	void on_actionSource_view_perspective_toggled(bool);
	
	// Window menu
	void on_actionSplit_horizontally_triggered();
	void on_actionSplit_vertically_triggered();
	void on_actionUnsplit_all_triggered();
	void on_actionClose_current_view_triggered();
	void on_actionNew_viewport_triggered();
	void on_actionNew_window_triggered();
	
	// Help menu
	void on_actionAbout_Qt_triggered();
	void on_actionAbout_Canorus_triggered();
	
	/////////////////////
	// Toolbar actions //
	/////////////////////
	// Toolbar 
	void sl_mpoVoiceNum_valChanged(int iVoice);
	void sl_mpoTimeSig_valChanged(int iBeats, int iBeat);
	void on_actionNoteSelect_toggled(bool);
	void on_actionClefSelect_toggled(bool);
	void on_actionTimeSigSelect_toggled(bool);

	//////////////////////////////
	// Process ViewPort signals //
	//////////////////////////////
	void viewPortMousePressEvent(QMouseEvent *e, const QPoint coords, CAViewPort *v);
	void viewPortMouseMoveEvent(QMouseEvent *e, const QPoint coords, CAViewPort *v);
	void viewPortWheelEvent(QWheelEvent *e, const QPoint coords, CAViewPort *v);
	void viewPortKeyPressEvent(QKeyEvent *e, CAViewPort *v);
	void sourceViewPortCommit(CASourceViewPort*, QString inputString);
	
	void on_tabWidget_currentChanged(int);

	void keyPressEvent(QKeyEvent *);

private slots:
	void playbackFinished();
	//void on_repaintTimer_timeout();	///Used for repaint events

private:	
	////////////////////////////////////
	// General properties and methods //
	////////////////////////////////////
	CADocument *_document;
	CAMode _mode;
	
	void setMode(CAMode mode);
	inline CAMode mode() { return _mode; }
	
	void doUnsplit(CAViewPort *v = 0);
	
	QList<CAViewPort *> _viewPortList;
	CAViewPort *_activeViewPort;
	bool _animatedScroll;
	bool _lockScrollPlayback;
	CAViewPort *_playbackViewPort;
	QTimer *_repaintTimer;
	
	CAPlayback *_playback;
	
	/////////////////////////
	// Pure user interface //
	/////////////////////////
	void setupCustomUi();
	void initToolBars();
	
	// Toolbars
	CAToolBar    *mpoMEToolBar;			/// Toolbar that contains clef/note/timesig buttons
	CAButtonMenu *mpoClefMenu;          /// Menu for selection of a clef
	CAButtonMenu *mpoNoteMenu;          /// Menu for the selection of a note length
	CAButtonMenu *mpoTimeSigMenu;       /// Menu for selection of a key signature
	
	// Action groups, Actions
	QActionGroup *mpoMEGroup;           /// Group for mutual exclusive selection of music elements
	QAction      *mpoVoiceNumAction;    /// Voice number action
	QAction      *actionNoteSelect;     /// Action for having a note length selected
	QAction      *actionClefSelect;     /// Action for having a clef selected
	QAction      *actionTimeSigSelect;  /// Action for having a clef selected
	
	CAKeySigPSP  *mpoKeySigPSP;	        /// Key signature perspective
	CATimeSigPSP *mpoTimeSigPSP;        /// Time signature perspective
	
	// Widgets
	CAMusElementFactory *mpoMEFactory;  /// Factory for creating/configuring music elements
	CALCDNumber *mpoVoiceNum;           /// LCD placed in Toolbar for showing current voice
	
	// Dialogs, Windows
	QFileDialog *_exportDialog;
	QFileDialog *_importDialog;
};
#endif /* CANORUS_H_*/
