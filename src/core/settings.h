/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QSettings>
#ifndef SWIG
#include <QColor>
//#include "ui/singleaction.h"
#include <QAction>
#endif
#include <QDir>
#include "core/fileformats.h"
#include "core/typesetter.h"

class CASettings : public QSettings {
public:
	CASettings( QObject * parent = 0 );
	CASettings( const QString & fileName, QObject * parent = 0 );
	virtual ~CASettings();

	int readSettings();
	void writeSettings();

	static const QString defaultSettingsPath();

	/////////////////////
	// Editor settings //
	/////////////////////
	inline bool finaleLyricsBehaviour() { return _finaleLyricsBehaviour; }
	inline void setFinaleLyricsBehaviour( bool b ) { _finaleLyricsBehaviour = b; }
	static const bool DEFAULT_FINALE_LYRICS_BEHAVIOUR;
	inline bool shadowNotesInOtherStaffs() { return _shadowNotesInOtherStaffs; }
	inline void setShadowNotesInOtherStaffs( bool b ) { _shadowNotesInOtherStaffs = b; }
	static const bool DEFAULT_SHADOW_NOTES_IN_OTHER_STAFFS;
	inline bool playInsertedNotes() { return _playInsertedNotes; }
	inline void setPlayInsertedNotes( bool b ) { _playInsertedNotes = b; }
	static const bool DEFAULT_PLAY_INSERTED_NOTES;
	inline bool autoBar() { return _autoBar; }
	inline void setAutoBar( bool b ) { _autoBar = b; }
	static const bool DEFAULT_AUTO_BAR;

	/////////////////////////////
	// Loading/Saving settings //
	/////////////////////////////
	inline QDir documentsDirectory() { return _documentsDirectory; }
	inline void setDocumentsDirectory( QDir d ) { _documentsDirectory = d; }
	static const QDir DEFAULT_DOCUMENTS_DIRECTORY;
	inline CAFileFormats::CAFileFormatType defaultSaveFormat() { return _defaultSaveFormat; }
	inline void setDefaultSaveFormat( CAFileFormats::CAFileFormatType t ) { _defaultSaveFormat = t; }
	static const CAFileFormats::CAFileFormatType DEFAULT_SAVE_FORMAT;
	inline int autoRecoveryInterval() { return _autoRecoveryInterval; }
	inline void setAutoRecoveryInterval( int interval ) { _autoRecoveryInterval = interval; };
	static const int DEFAULT_AUTO_RECOVERY_INTERVAL;
	inline int maxRecentDocuments() { return _maxRecentDocuments; }
	inline void setMaxRecentDocuments( int r ) { _maxRecentDocuments = r; }
	static const int DEFAULT_MAX_RECENT_DOCUMENTS;

	/////////////////////////
	// Appearance settings //
	/////////////////////////
#ifndef SWIG
	inline bool lockScrollPlayback() { return _lockScrollPlayback; }
	inline void setLockScrollPlayback( bool l ) { _lockScrollPlayback = l; }
	static const bool DEFAULT_LOCK_SCROLL_PLAYBACK;
	inline bool animatedScroll() { return _animatedScroll; }
	inline void setAnimatedScroll( bool a ) { _animatedScroll = a; }
	static const bool DEFAULT_ANIMATED_SCROLL;
	inline bool antiAliasing() { return _antiAliasing; }
	inline void setAntiAliasing( bool a ) { _antiAliasing = a; }
	static const bool DEFAULT_ANTIALIASING;
	inline QColor backgroundColor() { return _backgroundColor; }
	inline void setBackgroundColor( QColor backgroundColor ) { _backgroundColor = backgroundColor; }
	static const QColor DEFAULT_BACKGROUND_COLOR;
	inline QColor foregroundColor() { return _foregroundColor; }
	inline void setForegroundColor( QColor foregroundColor ) { _foregroundColor = foregroundColor; }
	static const QColor DEFAULT_FOREGROUND_COLOR;
	inline QColor selectionColor() { return _selectionColor; }
	inline void setSelectionColor( QColor selectionColor ) { _selectionColor = selectionColor; }
	static const QColor DEFAULT_SELECTION_COLOR;
	inline QColor selectionAreaColor() { return _selectionAreaColor; }
	inline void setSelectionAreaColor( QColor selectionAreaColor ) { _selectionAreaColor = selectionAreaColor; }
	static const QColor DEFAULT_SELECTION_AREA_COLOR;
	inline QColor selectedContextColor() { return _selectedContextColor; }
	inline void setSelectedContextColor( QColor selectedContextColor ) { _selectedContextColor = selectedContextColor; }
	static const QColor DEFAULT_SELECTED_CONTEXT_COLOR;
	inline QColor hiddenElementsColor() { return _hiddenElementsColor; }
	inline void setDisabledElementsColor( QColor disabledElementsColor ) { _disabledElementsColor = disabledElementsColor; }
	static const QColor DEFAULT_HIDDEN_ELEMENTS_COLOR;
	inline QColor disabledElementsColor() { return _disabledElementsColor; }
	inline void setHiddenElementsColor( QColor hiddenElementsColor ) { _hiddenElementsColor = hiddenElementsColor; }
	static const QColor DEFAULT_DISABLED_ELEMENTS_COLOR;
#endif

	///////////////////////
	// Playback settings //
	///////////////////////
	inline int midiInPort() { return _midiInPort; }
	void setMidiInPort( int in );
	static const int DEFAULT_MIDI_IN_PORT;
	inline int midiOutPort() { return _midiOutPort; }
	inline void setMidiOutPort( int out ) { _midiOutPort = out; }
	static const int DEFAULT_MIDI_OUT_PORT;

	///////////////////////
	// Printing settings //
	///////////////////////
	inline CATypesetter::CATypesetterType typesetter() { return _typesetter; }
	void setTypesetter( CATypesetter::CATypesetterType t ) { _typesetter = t; }
	static const CATypesetter::CATypesetterType DEFAULT_TYPESETTER;
	inline QString typesetterLocation() { return _typesetterLocation; }
	void setTypesetterLocation( QString tl ) { _typesetterLocation = tl; }
	static const QString DEFAULT_TYPESETTER_LOCATION;
	inline bool useSystemDefaultTypesetter() { return _useSystemDefaultTypesetter; }
	void setUseSystemDefaultTypesetter( bool s ) { _useSystemDefaultTypesetter = s; }
	static const bool DEFAULT_USE_SYSTEM_TYPESETTER;
	inline QString pdfViewerLocation() { return _pdfViewerLocation; }
	void setPdfViewerLocation( QString pl ) { _pdfViewerLocation = pl; }
	static const QString DEFAULT_PDF_VIEWER_LOCATION;
	inline bool useSystemDefaultPdfViewer() { return _useSystemDefaultPdfViewer; }
	void setUseSystemDefaultPdfViewer( bool s ) { _useSystemDefaultPdfViewer= s; }
	static const bool DEFAULT_USE_SYSTEM_PDF_VIEWER;

	///////////////////////////////
	// Action / Command settings //
	///////////////////////////////
	inline QDir latestShortcutsDirectory() { return _latestShortcutsDirectory; }
	inline void setLatestShortcutsDirectory( QDir d ) { _latestShortcutsDirectory = d; }
#ifndef SWIG
	QAction &getSingleAction(QString oCommand);
	void setSingleAction(QAction oSingleAction);
	inline const QList<QAction*>& getActionList() { return _actionList; }
	void setActionList(QList<QAction *> &oActionList);
	void addSingleAction(QAction oSingleAction);
	void deleteSingleAction(QString oCommand);
#endif

private:
#ifndef SWIG
	void writeRecentDocuments();
	void readRecentDocuments();
#endif

	/////////////////////
	// Editor settings //
	/////////////////////
	bool _finaleLyricsBehaviour;
	bool _shadowNotesInOtherStaffs;
	bool _playInsertedNotes;
	bool _autoBar;

	/////////////////////////////
	// Loading/Saving settings //
	/////////////////////////////
	QDir _documentsDirectory;  // location of directory automatically opened when File->Open is selected
	CAFileFormats::CAFileFormatType _defaultSaveFormat;
	int _autoRecoveryInterval; // auto recovery interval in minutes
	int _maxRecentDocuments;   // number of stored recently opened files

	/////////////////////////
	// Appearance settings //
	/////////////////////////
#ifndef SWIG
	bool   _lockScrollPlayback;
	bool   _animatedScroll;
	bool   _antiAliasing;
	QColor _backgroundColor;
	QColor _foregroundColor;
	QColor _selectionColor;
	QColor _selectionAreaColor;
	QColor _selectedContextColor;
	QColor _hiddenElementsColor;
	QColor _disabledElementsColor;
#endif
	///////////////////////
	// Playback settings //
	///////////////////////
	int _midiOutPort; // -1 disabled, 0+ port number
	int _midiInPort;  // -1 disabled, 0+ port number

	///////////////////////
	// Printing settings //
	///////////////////////
	CATypesetter::CATypesetterType _typesetter;
	QString                        _typesetterLocation;
	bool                           _useSystemDefaultTypesetter;
	QString                        _pdfViewerLocation;
	bool                           _useSystemDefaultPdfViewer;

/*
% To adjust the size of notes and fonts in points, it can be done like this:
% #(set-global-staff-size 16.0)

% Some examples to adjust the page size:
% \paper { #(set-paper-size "a3") } letter legal
% \paper { #(set-paper-size "a4" 'landscape) }
% For special size, like the screen, adjustments can be done like this:
% \paper{
%   paper-width = 16\cm
%   line-width = 12\cm
%   left-margin = 2\cm
%   top-margin = 3\cm
%   bottom-margin = 3\cm
%   ragged-last-bottom = ##t
% }
*/

	/////////////////////////////
	// Action/Command settings //
	/////////////////////////////
	QDir _latestShortcutsDirectory; // save location of shortcuts/midi commands
	// @ToDo: QAction can be exported to SWIG ? Abstract interface but requires QObject
#ifndef SWIG
    QList<QAction *> _actionList;
#endif
};

#endif /* SETTINGS_H_ */
