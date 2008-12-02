/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TUPLET_H_
#define TUPLET_H_

#include "core/muselement.h"

class CAContext;
class CAVoice;

class CATuplet : public CAMusElement {
public:
	CATuplet( int number, int actualNumber, QList<CAPlayable*> noteList );
	CATuplet( int number, int actualNumber );
	virtual ~CATuplet();

	CATuplet* clone(CAContext* context=0);
	CATuplet* clone(QList<CAPlayable*> newList);
	int compare(CAMusElement*);

	inline int number() { return _number; }
	inline void setNumber( int n ) { _number = n; }

	inline int actualNumber() { return _actualNumber; }
	inline void setActualNumber( int n ) { _actualNumber = n; }

	inline QList<CAPlayable*>& noteList() { return _noteList; }
	void addNote( CAPlayable* p );
	inline void addNotes( QList<CAPlayable*> l ) { noteList() << l; }
	inline void removeNote( CAPlayable* p ) { noteList().removeAll(p); }
	inline CAPlayable *firstNote() { return noteList().front(); }
	inline CAPlayable *lastNote() { return noteList().back(); }
	inline bool containsNote( CAPlayable *p ) { return noteList().contains(p); }
	CAPlayable *nextTimed( CAPlayable *p );

	int timeLength();
	int timeStart();

	void assignTimes();

private:
	void resetTimes();

	int _number;
	int _actualNumber;

	QList<CAPlayable*> _noteList;
};

#endif /* TUPLET_H_ */
