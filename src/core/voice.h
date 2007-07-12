/*
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICNESE.GPL for details.
 */

#ifndef VOICE_H_
#define VOICE_H_

#include <QList>

#include "core/muselement.h"
#include "core/note.h"

class CAStaff;
class CAClef;
class CALyricsContext;

class CAVoice {
public:
	CAVoice(CAStaff *staff, const QString name, int voiceNumber, CANote::CAStemDirection stemDirection);
	~CAVoice();
	inline CAStaff *staff() { return _staff; }
	inline void setStaff(CAStaff *staff) { _staff = staff; }
	void clear();
	
	void insertMusElement(CAMusElement *elt, bool updateTimes=true);
	
	bool insertMusElementBefore(CAMusElement *elt, CAMusElement *eltAfter, bool updateTimes = true, bool force=false);
	bool insertMusElementAfter(CAMusElement *elt, CAMusElement *eltBefore, bool updateTimes = true, bool force=false);
	
	bool appendMusElement(CAMusElement *elt);
	bool prependMusElement(CAMusElement *elt);
	
	bool addNoteToChord(CANote *note, CANote *referenceNote);
	bool removeElement(CAMusElement *elt);
	
	int voiceNumber() { return _voiceNumber; }
	bool isFirstVoice() { return !_voiceNumber; }
	void setVoiceNumber(int idx) { _voiceNumber = idx; }
	
	int musElementCount() { return _musElementList.count(); }
	CAMusElement *musElementAt(int i) { return _musElementList[i]; }
	int indexOf(CAMusElement *elt) { return _musElementList.indexOf(elt); }
	bool contains(CAMusElement *elt) { return _musElementList.contains(elt); }
	
	QList<CANote*> noteList();
	CANote *findNextNote(int timeStart);
	CANote *findPrevNote(int timeStart);
	
	QList<CAMusElement*> getEltByType(CAMusElement::CAMusElementType type, int startTime);
	
	bool containsPitch(int pitch, int startTime);
	
	QList<CAMusElement*> musElementList() { return _musElementList; }
	int lastTimeEnd() { return (_musElementList.size()?_musElementList.back()->timeEnd():0); }
	int lastTimeStart() { return (_musElementList.size()?_musElementList.back()->timeStart():0); }
	CAMusElement *lastMusElement() { return _musElementList.back(); }
	CAMusElement *eltBefore(CAMusElement *elt);
	CAMusElement *eltAfter(CAMusElement *elt);
	int lastNotePitch(bool inChord=false);
	CAPlayable* lastPlayableElt();
	CANote*     lastNote();
	CAClef *getClef(CAMusElement *elt);
	QList<CAPlayable*> getChord(int time);
	
	CANote::CAStemDirection stemDirection() { return _stemDirection; }
	void setStemDirection(CANote::CAStemDirection direction);
	
	const QString name() { return _name; }
	void setName(const QString name) { _name = name; }
	
	unsigned char midiChannel() { return _midiChannel; }
	void setMidiChannel(const unsigned char ch) { _midiChannel = ch; }
	
	unsigned char midiProgram() { return _midiProgram; }
	void setMidiProgram(const unsigned char program) { _midiProgram = program; }
	
	inline QList<CALyricsContext*> lyricsContextList() { return _lyricsContextList; }
	inline void addLyricsContext( CALyricsContext *lc ) { _lyricsContextList << lc; }
	inline bool removeLyricsContext( CALyricsContext *lc ) { return _lyricsContextList.removeAll(lc); }
	
	bool updateTimes(CAMusElement *elt, int length=0);
	bool updateTimesAfter(CAMusElement *eltBefore, int length);
	
private:
	void updateTimes(int idx, int length=0);
	QList<CAMusElement *> _musElementList;
	CAStaff *_staff; // parent staff
	int _voiceNumber;
	CANote::CAStemDirection _stemDirection;
	QList<CALyricsContext*> _lyricsContextList;
	
	QString _name;
	
	/////////////////////
	// MIDI properties //
	/////////////////////
	unsigned char _midiChannel;
	unsigned char _midiProgram;
};
#endif /*VOICE_H_*/
