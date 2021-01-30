/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CHORDNAMECONTEXT_H_
#define CHORDNAMECONTEXT_H_

#include "score/context.h"
#include <QList>

class CAChordName;

class CAChordNameContext : public CAContext {
public:
    CAChordNameContext(QString name, CASheet* sheet);
    ~CAChordNameContext();

    std::shared_ptr<CAContext> cloneRealContext(CASheet*);
    std::shared_ptr<CAChordNameContext> cloneChordNameContext(CASheet*);
    void clear();
    CAMusElement* next(CAMusElement* elt);
    CAMusElement* previous(CAMusElement* elt);
    bool remove(CAMusElement* elt);

    QList<CAChordName*>& chordNameList() { return _chordNameList; }
    CAChordName* chordNameAtTimeStart(int timeStart);

    void repositChordNames();
    void addChordName(CAChordName*, bool replace = true);
    void addEmptyChordName(int timeStart, int timeLength);

private:
    QList<CAChordName*> _chordNameList;
};

#endif /* CHORDNAMECONTEXT_H_ */
