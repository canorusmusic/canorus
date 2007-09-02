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

class CASheet;

class CAMidiDevice : public QObject {
public:
	enum CAMidiDeviceType {
		RtMidiDevice
	};
	
	static const QStringList GM_INSTRUMENTS;
	static unsigned char freeMidiChannel( CASheet* );
	
	virtual ~CAMidiDevice() {};
	
	inline CAMidiDeviceType midiDeviceType() { return _midiDeviceType; };
	
	virtual QMap<int, QString> getOutputPorts() = 0;
	virtual QMap<int, QString> getInputPorts() = 0;
	
	virtual bool openOutputPort(int port) = 0;	// return true on success, false otherwise
	virtual bool openInputPort(int port) = 0;	// return true on success, false otherwise	
	virtual void closeOutputPort() = 0;
	virtual void closeInputPort() = 0;
	virtual void send(QVector<unsigned char> message) = 0;
	
/*signals:
	void midiInEvent( QVector<unsigned char> message );
*/
protected:
	inline void setMidiDeviceType( CAMidiDeviceType t ) { _midiDeviceType = t; }
	CAMidiDeviceType _midiDeviceType;
};

#endif /*MIDIDEVICE_H_*/
