/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QSettings>
#include <QColor>
#include <QDir>
#include "core/fileformats.h"

class CASettings : public QSettings {
public:
	CASettings( QObject * parent = 0 );
	CASettings( const QString & fileName, QObject * parent = 0 );
	virtual ~CASettings();

	int readSettings();
	void writeSettings();

	static QString defaultSettingsPath();

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

	///////////////////////
	// Playback settings //
	///////////////////////
	inline int midiInPort() { return _midiInPort; }
	void setMidiInPort( int in );
	static const int DEFAULT_MIDI_IN_PORT;
	inline int midiOutPort() { return _midiOutPort; }
	inline void setMidiOutPort( int out ) { _midiOutPort = out; }
	static const int DEFAULT_MIDI_OUT_PORT;

private:
	void writeRecentDocuments();
	void readRecentDocuments();

	/////////////////////
	// Editor settings //
	/////////////////////
	bool _finaleLyricsBehaviour;
	bool _shadowNotesInOtherStaffs;
	bool _playInsertedNotes;

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
	QColor _backgroundColor;
	QColor _foregroundColor;
	QColor _selectionColor;
	QColor _selectionAreaColor;
	QColor _selectedContextColor;
	QColor _hiddenElementsColor;
	QColor _disabledElementsColor;

	///////////////////////
	// Playback settings //
	///////////////////////
	int _midiOutPort; // -1 disabled, 0+ port number
	int _midiInPort;  // -1 disabled, 0+ port number
};

#endif /* SETTINGS_H_ */
