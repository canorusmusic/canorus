/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QSettings>

class CASettings : public QSettings {
public:
	CASettings( const QString & fileName, Format format, QObject * parent = 0 );
	virtual ~CASettings();
	
	bool readSettings();
	void writeSettings();
	
	inline int midiInPort() { return _midiInPort; }
	inline int midiOutPort() { return _midiOutPort; }
	
	inline void setMidiInPort( int in ) { _midiInPort = in; }
	inline void setMidiOutPort( int out ) { _midiOutPort = out; }
	
private:
	int _midiOutPort; // -1 disabled, 0+ port number
	static const int DEFAULT_MIDI_IN_PORT;
	
	int _midiInPort;  // -1 disabled, 0+ port number
	static const int DEFAULT_MIDI_OUT_PORT;
};

#endif /* SETTINGS_H_ */
