/*! 
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef MIDIDEVICE_H_
#define MIDIDEVICE_H_

#include <QStringList>
#include <QVector>
#include <QMap>
#include <QString>
#include <QObject>

#include "core/diatonicpitch.h"

class CASheet;
class CADiatonicKey;

class CAMidiDevice : public QObject {
#ifndef SWIG
	Q_OBJECT
#endif
	
	friend void rtMidiInCallback( double deltatime, std::vector< unsigned char > *message, void *userData );
public:
	enum CAMidiDeviceType {
		RtMidiDevice,
		MidiExportDevice
	};
	
	CAMidiDevice();

	enum midiCommands {
		Meta_Text        = 0x01,
		Meta_Timesig     = 0x58,
		Meta_Keysig      = 0x59,
		Meta_Tempo       = 0x51,

		Midi_Ctl_Reverb  = 0x5b,
		Midi_Ctl_Chorus  = 0x5d,
		Midi_Ctl_Pan     = 0x0a,
		Midi_Ctl_Volume  = 0x07,
		Midi_Ctl_Sustain = 0x40,

		Midi_Note_Off    = 0x80,
		Midi_Note_On     = 0x90
	};

	
	QStringList GM_INSTRUMENTS;
	static unsigned char freeMidiChannel( CASheet* );
	
	virtual ~CAMidiDevice() {};
	
	inline CAMidiDeviceType midiDeviceType() { return _midiDeviceType; };
	
	bool isRealTime() { return _realTime; }
	virtual QMap<int, QString> getOutputPorts() = 0;
	virtual QMap<int, QString> getInputPorts() = 0;
	
	virtual bool openOutputPort(int port) = 0;	// return true on success, false otherwise
	virtual bool openInputPort(int port) = 0;	// return true on success, false otherwise	
	virtual void closeOutputPort() = 0;
	virtual void closeInputPort() = 0;
	virtual void send(QVector<unsigned char> message, int mSeconds=0) = 0; // message and absolute time of the message in miliseconds
	
#ifndef SWIG
signals:
	void midiInEvent( QVector< unsigned char > message );	
#endif
	
public:
	static int diatonicPitchToMidiPitch( CADiatonicPitch );
	static CADiatonicPitch midiPitchToDiatonicPitch( int );
	static CADiatonicPitch midiPitchToDiatonicPitch( int, CADiatonicKey k );
	
protected:
	void setRealTime( bool r ) { _realTime = r; }
	inline void setMidiDeviceType( CAMidiDeviceType t ) { _midiDeviceType = t; }
	CAMidiDeviceType _midiDeviceType;
	bool _realTime;  // is the device
};

#endif /* MIDIDEVICE_H_ */
