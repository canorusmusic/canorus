/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICNESE.GPL for details.
*/

#ifndef VOICE_H_
#define VOICE_H_

#include <QList> // music elements container

#include "score/muselement.h"
#include "score/note.h"

class CAKeySignature;
class CATimeSignature;
class CAClef;
class CALyricsContext;
class CARest;
class CATempo;

class CAVoice {
	friend class CAStaff; // used for insertion of music elements and updateTimes() when inserting elements and synchronizing voices

public:
	CAVoice( const QString name, CAStaff *staff, CANote::CAStemDirection stemDirection=CANote::StemNeutral );
	~CAVoice();
	inline CAStaff *staff() { return _staff; }
	inline void setStaff(CAStaff *staff) { _staff = staff; }
	void clear();
	CAVoice *clone( CAStaff *newStaff = 0 );
	void cloneVoiceProperties( CAVoice* v );

	/////////////////////////////////////////
	// Notes, rests and signs manipulation //
	/////////////////////////////////////////
	void append( CAMusElement *elt, bool addToChord=false );
	bool insert( CAMusElement *eltAfter, CAMusElement *elt, bool addToChord=false );
	bool remove( CAMusElement *elt, bool updateSignsTimes=true );
	CAPlayable* insertInTupletAndVoiceAt( CAPlayable *p, CAPlayable *n );
	bool synchronizeMusElements();

	//////////////////////////////
	// Voice analysis and query //
	//////////////////////////////
	inline const QList<CAMusElement*>& musElementList() { return _musElementList; }

	QList<CAMusElement*> getSignList();
	QList<CANote*> getNoteList();
	bool containsPitch( int noteName , int timeStart );
	bool containsPitch( CADiatonicPitch p, int timeStart );
	CAMusElement *next(CAMusElement *elt);
	CAMusElement *previous(CAMusElement *elt);
	CAMusElement *nextByType(CAMusElement::CAMusElementType type, CAMusElement *elt);
	CAMusElement *previousByType(CAMusElement::CAMusElementType type, CAMusElement *elt);

	CANote *nextNote(int timeStart);
	CANote *previousNote(int timeStart);
	CARest *nextRest(int timeStart);
	CARest *previousRest(int timeStart);
	CAPlayable *nextPlayable(int timeStart);
	CAPlayable *previousPlayable(int timeStart);

	bool binarySearch_startTime(int time, int& position);

	CAMusElement *getOneEltByType(CAMusElement::CAMusElementType type, int startTime);
	QList<CAMusElement*> getEltByType(CAMusElement::CAMusElementType type, int startTime);
	CAMusElement *getOnePreviousByType(CAMusElement::CAMusElementType type, int startTime);
	QList<CAMusElement*> getPreviousByType(CAMusElement::CAMusElementType type, int startTime);

	inline int lastTimeEnd() { return (musElementList().size()?musElementList().back()->timeEnd():0); }
	inline int lastTimeStart() { return (musElementList().size()?musElementList().back()->timeStart():0); }
	inline CAMusElement *lastMusElement() { return musElementList().size()?musElementList().back():0; }
	CADiatonicPitch lastNotePitch(bool inChord=false);
	CAPlayable* lastPlayableElt();
	CANote*     lastNote();
	CATimeSignature*   getTimeSig(CAMusElement *elt);
	CAKeySignature*    getKeySig(CAMusElement *elt);
	CAClef*            getClef(CAMusElement *elt);
	QList<CAPlayable*>   getChord( int time );
	QList<CAMusElement*> getBar( int time );
	CATempo             *getTempo( int time );

	QList<CAMusElement*> getKeySignature(int startTime);
	QList<CAMusElement*> getTimeSignature(int startTime);
	QList<CAMusElement*> getClef(int startTime);
	QList<CAMusElement*> getPreviousKeySignature(int startTime);
	QList<CAMusElement*> getPreviousTimeSignature(int startTime);
	QList<CAMusElement*> getPreviousClef(int startTime);

	////////////////
	// Properties //
	////////////////
	inline int voiceNumber() { return (staff()?(staff()->voiceList().indexOf(this)+1):1); }
	inline bool isFirstVoice() { return (voiceNumber()==1); }

	inline CANote::CAStemDirection stemDirection() { return _stemDirection; }
	inline void setStemDirection( CANote::CAStemDirection direction ) { _stemDirection = direction; }

	inline const QString name() { return _name; }
	inline void setName(const QString name) { _name = name; }

	inline unsigned char midiChannel() { return _midiChannel; }
	inline void setMidiChannel(const unsigned char ch) { _midiChannel = ch; }

	inline unsigned char midiProgram() { return _midiProgram; }
	inline void setMidiProgram(const unsigned char program) { _midiProgram = program; }

	inline const QList<CALyricsContext*>& lyricsContextList() { return _lyricsContextList; }
	inline void addLyricsContext( CALyricsContext *lc ) { _lyricsContextList << lc; }
	inline void setLyricsContexts( QList<CALyricsContext*> list ) { _lyricsContextList = list; }
	inline void addLyricsContexts( QList<CALyricsContext*> list ) { _lyricsContextList += list; }
	inline bool removeLyricsContext( CALyricsContext *lc ) { return _lyricsContextList.removeAll(lc); }

private:
	bool addNoteToChord(CANote *note, CANote *referenceNote);
	bool insertMusElement( CAMusElement *before, CAMusElement *elt );
	bool updateTimes( int idx, int length, bool signsToo=false );

	// list of all the music elements
	QList<CAMusElement *> _musElementList;
	CAStaff *_staff; // parent staff
	
	CANote::CAStemDirection _stemDirection;
	QList<CALyricsContext*> _lyricsContextList;

	QString _name;

	/////////////////////
	// MIDI properties //
	/////////////////////
	unsigned char _midiChannel;
	unsigned char _midiProgram;
};
#endif /* VOICE_H_ */
