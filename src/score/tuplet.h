/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TUPLET_H_
#define TUPLET_H_

#include "score/muselement.h"

class CAContext;
class CAVoice;
class CASlur;

class CATuplet : public CAMusElement {
public:
    CATuplet(int number, int actualNumber, QList<CAPlayable*> noteList);
    CATuplet(int number, int actualNumber);
    virtual ~CATuplet();

    std::shared_ptr<CAMusElement> cloneRealElement(CAContext* context = nullptr);
    std::shared_ptr<CATuplet> cloneTuplet(QList<CAPlayable*> newList);
    int compare(CAMusElement*);

    inline int number() { return _number; }
    inline void setNumber(int n) { _number = n; }

    inline int actualNumber() { return _actualNumber; }
    inline void setActualNumber(int n) { _actualNumber = n; }

    inline const QList<CAPlayable*>& noteList() const { return _noteList; }
    void addNote(CAPlayable* p);
    inline void addNotes(QList<CAPlayable*> l) { _noteList << l; }
    inline void removeNote(CAPlayable* p) { _noteList.removeAll(p); }
    CAPlayable* firstNote();
    CAPlayable* lastNote();
    inline bool containsNote(CAPlayable* p) { return noteList().contains(p); }
    CAPlayable* nextTimed(CAPlayable* p);

    int timeLength() const;
    int timeStart() const;

    void assignTimes();

private:
    void resetTimes();
    QList<QList<CASlur*>> getNoteSlurs();
    void assignNoteSlurs(QList<QList<CASlur*>>);

    int _number;
    int _actualNumber;

    QList<CAPlayable*> _noteList;
};

#endif /* TUPLET_H_ */
