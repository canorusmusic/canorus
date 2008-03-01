/*!
        Copyright (c) 2007, Reinhard Katzmann, Canorus development team
        
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _CAEDITORCTL_H_
#define _CAEDITORCTL_H_

class CASheet;

class CAEditorCtl
{
public:
	enum CAMode {
		NoDocumentMode,
		InsertMode,
		SelectMode,
		EditMode,
		ReadOnlyMode
	};

	CAEditorCtl();
	~CAEditorCtl();

	void clearUI();
	bool rebuildUI(CASheet *sheet, bool repaint=true);
	bool rebuildUI( bool repaint, int curIndex );
    	
	CAContext *currentContext();
	CAVoice   *currentVoice();
    	
	void setMode(CAMode mode);
    	
	void scoreViewPortMousePress(QMouseEvent *e, 
		                         const QPoint coords,
		                         CAScoreViewPort *v,
		                         bool bSyllable,
		                         bool bPlayable);
	QString scoreViewPortMouseMove(QMouseEvent *e, 
			                       QPoint coords,
			                       CAScoreViewPort *c);
	void scoreViewPortMouseRelease(QMouseEvent *e,
			                       QPoint coords,
			                       CAScoreViewPort *c);
	void scoreViewPortWheel(QWheelEvent *e,
			                QPoint coords,
			                CAScoreViewPort *sv);
	void scoreViewPortKeyPress(QKeyEvent *e, CAScoreViewPort *v);
	void newDocument();
	void insertMusElementAt(const QPoint coords, CAScoreViewPort *v);
	int  newVoice_triggered();
	int  removeVoice_triggered();
	void removeContext_triggered();
	void hiddenRest_toggled( bool checked );
	void playFromSelection_toggled(bool checked);
	void playbackFinished();
	void initViewPort(CAViewPort*);
	CADocument *openDocument(const QString& fileName);
	bool saveDocument( QString fileName );
	void voiceNum_valChanged(int voiceNr);
	void keySig_activated( int row );
	void clefOffset_valueChanged( int newOffset );
	void voiceName_returnPressed();
	void insertPlayable_toggled(bool checked);
	void FMFunction_toggled( bool checked, int buttonId );
	void FMChordArea_toggled(bool checked, int buttonId);
	void FMTonicDegree_toggled(bool checked, int buttonId);
	void FMEllipse_toggled( bool checked );
	void slurType_toggled( bool checked, int buttonId );
	void clefType_toggled(bool checked, int buttonId);
	void timeSigBeats_valueChanged(int beats);
	void sourceViewPortCommit(QString inputString, CASourceViewPort *v);
	void LilyPondSource_triggered();
	bool scoreView_triggered();
	void removeSheet_triggered();
	void addSheet(CASheet *s);
	CAViewPortContainer *removeSheet( CASheet *sheet );
	CASheet *sheetName_returnPressed();
	void contextName_returnPressed();
	void stanzaNumber_valueChanged(int stanzaNumber);
	void associatedVoice_activated(int idx);
	void voiceStemDirection_toggled(bool checked, int direction);
	void noteStemDirection_toggled(bool, int);
	void cut_triggered();
	void copySelection( CAScoreViewPort *v );
	void deleteSelection( CAScoreViewPort *v, bool deleteSyllable, bool doUndo );
	void pasteAt( const QPoint coords, CAScoreViewPort *v );
		
	void onRepaintTimerTimeout();
	void onSyllableEditKeyPressEvent(QKeyEvent *e, CASyllableEdit *syllableEdit);
	
	// Inline setters
	inline void setCurrentViewPort( CAViewPort *viewPort ) { _currentViewPort = viewPort; }
	inline void setCurrentViewPortContainer( CAViewPortContainer *vpc )
		{ _currentViewPortContainer = vpc; }
	inline void setRebuildUILock(bool l) { _rebuildUILock = l; }
	inline void setDocument(CADocument *document) { _document = document; }
	inline void setAnimatedScroll( bool bVal ) { _animatedScroll = bVal; }
	inline void setsetLockScrollPlayback( bool bVal ) { _lockScrollPlayback = bVal; };
	inline void restartTimeEditedTime() { _timeEditedTime = 0; };
	
	inline void triggerZoomToSelection() {
		if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
			((CAScoreViewPort*)_currentViewPort)->zoomToSelection(_animatedScroll);
	}
	
	inline void triggerZoomToFit() {
		if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
			((CAScoreViewPort*)_currentViewPort)->zoomToFit();
	}
	
	inline void triggerZoomToWidth() {
		if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
			((CAScoreViewPort*)_currentViewPort)->zoomToWidth();
	}
	
	inline void triggerZoomToHeight() {
		if (_currentViewPort->viewPortType() == CAViewPort::ScoreViewPort)
			((CAScoreViewPort*)_currentViewPort)->zoomToHeight();
	}
	
	// Inline getters
	inline CAMode mode() { return _mode; }
	inline bool rebuildUILock() { return _rebuildUILock; }
	inline CAViewPort *currentViewPort() { return _currentViewPort; }
	inline int voiceNum() { return _iVoiceNum; }
	inline int maxVoiceNum() { return _iMaxVoiceNum; }
	inline bool selectMode() { return _bSelectMode; }
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
	
	inline CAViewPortContainer *currentViewPortContainer() { return _currentViewPortContainer; }
	inline CADocument *document() { return _document; }
	
	inline void removeViewPort(CAViewPort* v) { _viewPortList.removeAll(v); }
	
private:
	////////////////////////////////////
	// General properties and methods //
	////////////////////////////////////
	CADocument *_document;
	CAMode _mode;
		
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
	
	CAPlayback *_playback;
	QTimer _timeEditedTimer;
	unsigned int  _timeEditedTime;
	CAMusElementFactory *_musElementFactory;
	inline CAMusElementFactory *musElementFactory() { return _musElementFactory; }
	int _iVoiceNum;
	int _iMaxVoiceNum;
	bool _bSelectMode;
};

#endif /* _CAEDITORCTL_H_ */
