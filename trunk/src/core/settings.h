/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QSettings>
#include <QColor>
#include "ui/settingsdialog.h"
#include "ui/mainwin.h"

class CASettings : public QSettings {
public:
	CASettings( const QString & fileName, Format format, QObject * parent = 0 );
	virtual ~CASettings();
	
	CASettingsDialog::CASettingsPage readSettings();
	void writeSettings();
	
	inline bool finaleLyricsBehaviour() { return _finaleLyricsBehaviour; }
	inline QDir documentsDirectory() { return _documentsDirectory; }
	inline CAFileFormats::CAFileFormatType defaultSaveFormat() { return _defaultSaveFormat; }
	inline int autoRecoveryInterval() { return _autoRecoveryInterval; }
	inline QColor backgroundColor() { return _backgroundColor; }
	inline QColor foregroundColor() { return _foregroundColor; }
	inline QColor selectionColor() { return _selectionColor; }
	inline QColor selectionAreaColor() { return _selectionAreaColor; }
	inline QColor selectedContextColor() { return _selectedContextColor; }
	inline QColor hiddenElementsColor() { return _hiddenElementsColor; }
	inline QColor disabledElementsColor() { return _disabledElementsColor; }
	inline int midiInPort() { return _midiInPort; }
	inline int midiOutPort() { return _midiOutPort; }
	
	inline void setFinaleLyricsBehaviour( bool b ) { _finaleLyricsBehaviour = b; }
	inline void setDocumentsDirectory( QDir d ) { _documentsDirectory = d; }
	inline void setDefaultSaveFormat( CAFileFormats::CAFileFormatType t ) { _defaultSaveFormat = t; }
	inline void setAutoRecoveryInterval( int interval ) { _autoRecoveryInterval = interval; };
	inline void setBackgroundColor( QColor backgroundColor ) { _backgroundColor = backgroundColor; }
	inline void setForegroundColor( QColor foregroundColor ) { _foregroundColor = foregroundColor; }
	inline void setSelectionColor( QColor selectionColor ) { _selectionColor = selectionColor; }
	inline void setSelectionAreaColor( QColor selectionAreaColor ) { _selectionAreaColor = selectionAreaColor; }
	inline void setSelectedContextColor( QColor selectedContextColor ) { _selectedContextColor = selectedContextColor; }
	inline void setDisabledElementsColor( QColor disabledElementsColor ) { _disabledElementsColor = disabledElementsColor; }
	inline void setHiddenElementsColor( QColor hiddenElementsColor ) { _hiddenElementsColor = hiddenElementsColor; }
	inline void setMidiInPort( int in ) { _midiInPort = in; }
	inline void setMidiOutPort( int out ) { _midiOutPort = out; }
	
	static const bool DEFAULT_FINALE_LYRICS_BEHAVIOUR;
	static const QDir DEFAULT_DOCUMENTS_DIRECTORY;
	static const CAFileFormats::CAFileFormatType DEFAULT_SAVE_FORMAT;
	static const int DEFAULT_AUTO_RECOVERY_INTERVAL;
	static const QColor DEFAULT_BACKGROUND_COLOR;
	static const QColor DEFAULT_FOREGROUND_COLOR;
	static const QColor DEFAULT_SELECTION_COLOR;
	static const QColor DEFAULT_SELECTION_AREA_COLOR;
	static const QColor DEFAULT_SELECTED_CONTEXT_COLOR;
	static const QColor DEFAULT_HIDDEN_ELEMENTS_COLOR;
	static const QColor DEFAULT_DISABLED_ELEMENTS_COLOR;
	static const int DEFAULT_MIDI_IN_PORT;
	static const int DEFAULT_MIDI_OUT_PORT;
	
private:
	/////////////////////
	// Editor settings //
	/////////////////////
	bool _finaleLyricsBehaviour;
	
	/////////////////////////////
	// Loading/Saving settings //
	/////////////////////////////
	QDir _documentsDirectory; // location of directory automatically opened when File->Open is selected
	CAFileFormats::CAFileFormatType _defaultSaveFormat;
	int _autoRecoveryInterval; // auto recovery interval in minutes
	
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
