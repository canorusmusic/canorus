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
class CAScrollWidget;
class CAViewPort;
class CAScoreViewPort;
class CASourceViewPort;
class CAMusElementFactory;

enum CAMode {
	InsertMode,
	SelectMode,
	EditMode,
	ReadOnlyMode
};

#define _currentScrollWidget ((CAScrollWidget*)(moMainWin.tabWidget->currentWidget()))


class CAMainWin : public QMainWindow
{
	Q_OBJECT

public:
	CAMainWin(QMainWindow *oParent = 0);
	~CAMainWin();

	void initToolBar();
	void newDocument();
	
	/**
	* Adds a sheet to the document.
	*/
	void addSheet(CASheet *s);

	void insertMusElementAt(const QPoint coords, CAScoreViewPort *v, CAMusElement &roMusElement );

	/**
	 * Delete all viewports and its contents.
	 * Delete all signals.
	 * Release all buttons and modes.
	 * 
	 * WARNING! This function delets the UI only (drawable elements). All the data classes should stay intact. Use _document.clear() in order to clear the data part as well.
	 */
	void clearUI();
	
	CAScrollWidget* currentScrollWidget() { return _currentScrollWidget; }
	
	/**
	 * Rebuild the UI from the data part.
	 * 
	 * This method is called eg. when multiple viewports share the same logical source and a change has been made in the logical musElement list.
	 * This way, sheet argument is a pointer to the data sheet where the change occured and viewports showing the given sheet are updated (CAEngraver create CADrawable elements for every viewport).
	 * If no sheet argument is passed, the whole UI is rebuilt from the data part. This is called for eg. on Open file after the data part has been read.
	 * 
	 * @param sheet Pointer to the common CASheet viewports use.
	 * @param repaint Should the viewports be repainted as well (sometimes we want just engraver to generate the drawable notes and wait for other events and then repaint once).
	 */
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
	////////////////////////////////////////////////////
	//Menu bar actions
	////////////////////////////////////////////////////
	void closeEvent(QCloseEvent *event);
	//File menu
	void on_actionNew_triggered();
	void on_actionNew_sheet_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSave_as_triggered();
	void on_actionExport_triggered();
	void on_actionImport_triggered();
	
	//Edit menu
	void on_actionMIDI_Setup_triggered();
	
	//Insert menu
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
	
	//Playback menu
	void on_actionPlay_toggled(bool);
	
	//Perspective menu
	void on_actionSource_view_perspective_toggled(bool);
	
	//Window menu
	void on_actionSplit_horizontally_triggered();
	void on_actionSplit_vertically_triggered();
	void on_actionUnsplit_all_triggered();
	void on_actionClose_current_view_triggered();
	void on_actionNew_viewport_triggered();
	
	//Help menu
	void on_actionAbout_Qt_triggered();
	void on_actionAbout_Canorus_triggered();

	// Toolbar 
	void sl_mpoVoiceNum_valChanged(int iVoice);
	void sl_mpoTimeSig_valChanged(int iBeats, int iBeat);
	void on_actionNoteSelect_toggled(bool);
	void on_actionClefSelect_toggled(bool);
	void on_actionTimeSigSelect_toggled(bool);

	////////////////////////////////////////////////////
	//Process ViewPort signals
	////////////////////////////////////////////////////
	/**
	 * Process the mouse press events of the children viewports.
	 * 
	 * @param e Mouse event which gets processed.
	 * @param coords Absolute world coordinates where the mouse cursor was at time of the event.
	 * @param v Pointer to viewport where the event happened.
	 */
	void viewPortMousePressEvent(QMouseEvent *e, const QPoint coords, CAViewPort *v);

	/**
	 * Process the mouse move events of the children viewports.
	 * 
	 * @param e Mouse event which gets processed.
	 * @param coords Absolute world coordinates where the mouse cursor was at time of the event.
	 * @param v Pointer to viewport where the event happened.
	 */
	void viewPortMouseMoveEvent(QMouseEvent *e, const QPoint coords, CAViewPort *v);
	
	/**
	 * Process the wheel events of the children viewports.
	 * 
	 * @param e Wheel event which gets processed.
	 * @param coords Absolute world coordinates where the mouse cursor was at time of the event.
	 * @param v Pointer to viewport where the event happened.
	 */
	void viewPortWheelEvent(QWheelEvent *e, const QPoint coords, CAViewPort *v);
	
	/**
	 * Process the key events of the children viewports.
	 * 
	 * @param e Key event which gets processed.
	 * @param v Pointer to the viewport where the event happened.
	 */
	void viewPortKeyPressEvent(QKeyEvent *e, CAViewPort *v);
	
	void sourceViewPortCommit(CASourceViewPort*, QString inputString);
	
	/**
	 * Called when the tab is switched.
	 */
	void on_tabWidget_currentChanged(int);

	void keyPressEvent(QKeyEvent *);

private slots:
	void playbackFinished();	///Temporarily as we don't find better solution.
	//void on_repaintTimer_timeout();	///Used for repaint events

private:	
	////////////////////////////////////////////////////
	//General properties
	////////////////////////////////////////////////////
	CADocument *_document;	///Pointer to the main window's document.
	CAMode _currentMode;	///Every main window has its own current mode (view, insert, edit etc.). See enum CAMode.
	
	void setMode(CAMode mode);
	inline CAMode currentMode() { return _currentMode; }
	
	////////////////////////////////////////////////////
	//Playback
	////////////////////////////////////////////////////
	CAPlayback *_playback;
	
	////////////////////////////////////////////////////
	//User interface, toolbar
	////////////////////////////////////////////////////
	CAToolBar    *mpoMEToolBar;			/// Toolbar that contains clef/note/timesig buttons
	CAButtonMenu *mpoClefMenu;          /// Menu for selection of a clef
	CAButtonMenu *mpoNoteMenu;          /// Menu for the selection of a note length
	CAButtonMenu *mpoTimeSigMenu;       /// Menu for selection of a key signature
	QActionGroup *mpoMEGroup;           /// Group for mutual exclusive selection of music elements
	QAction      *actionNoteSelect;     /// Action for having a note length selected
	QAction      *actionClefSelect;     /// Action for having a clef selected
	QAction      *actionTimeSigSelect;  /// Action for having a clef selected
	CAKeySigPSP  *mpoKeySigPSP;	        /// Key signature perspective
	CATimeSigPSP *mpoTimeSigPSP;        /// Time signature perspective
	
	////////////////////////////////////////////////////
	//User interface, widgets
	////////////////////////////////////////////////////
	Ui::MainWindow moMainWin;	///Main window widget representative

	CAMusElementFactory *mpoMEFactory;  /// Factory for creating/configuring music elements
	QList<CAViewPort *> _viewPortList;	/// List of all available viewports for any
	                                    /// sheet for this document
	CAViewPort *_activeViewPort;	    /// Current active viewport
	bool _animatedScroll;		        /// animate scroll/zoom
	bool _lockScrollPlayback;	        /// Lock the scroll UI while playback
	CAViewPort *_playbackViewPort;	    /// Viewport needed to be updated when playback
	                                    /// is active	
	QTimer *_repaintTimer;	            /// Used when playback is active
	CALCDNumber *mpoVoiceNum;           /// LCD placed in Toolbar for showing current voice
	
	////////////////////////////////////////////////////
	//User interface, dialogs, windows
	////////////////////////////////////////////////////
	QFileDialog *_exportDialog;
	QFileDialog *_importDialog;
	
	////////////////////////////////////////////////////
	//User interface, action objects from toolbars
	////////////////////////////////////////////////////
	QAction *mpoVoiceNumAction;  ///Voice number action
	
	void doUnsplit(CAViewPort *v = 0);
};
#endif /* CANORUS_H_*/
