/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QHash>

#include "core/document.h"
#include "core/context.h"
#include "core/staff.h"
#include "core/sheet.h"
#include "core/voice.h"
#include "core/lyricscontext.h"

/*!
	\class CASheet
	\brief Represents a single sheet of paper in the document

	CASheet represents a sheet of paper for the composer. The idea was taken out from
	spreadsheet applications. Each sheet is usually in its own tab.

	CASheet parent is CADocument and CASheet includes various contexts CAContext, let it
	be staffs, lyrics, function marks etc.

	\sa CADocument, CAContext
*/

/*!
	Creats a new sheet named \a name with parent document \a doc.
*/
CASheet::CASheet(const QString name, CADocument *doc) {
	_name = name;
	_document = doc;
}

/*!
	Clones the current sheet with all its content.
	If a new parent document \a doc is given, it also sets the document.
*/
CASheet *CASheet::clone( CADocument *doc ) {
	CASheet *newSheet = new CASheet( name(), doc );

	QHash< CAContext*, CAContext* > contextMap; // map between oldContexts<->cloned contexts
	QHash< CAVoice*, CAVoice* > voiceMap; // map between oldVoices<->cloned voices

	// create clones of contexts
	for (int i=0; i<contextCount(); i++) {
		CAContext *newContext = contextAt(i)->clone( newSheet );
		if ( newContext->contextType()==CAContext::Staff ) {
			for (int j=0; j<static_cast<CAStaff*>(contextAt(i))->voiceCount(); j++) {
				voiceMap[ static_cast<CAStaff*>(contextAt(i))->voiceAt(j) ] = static_cast<CAStaff*>(newContext)->voiceAt(j);
			}
		}
		contextMap[contextAt(i)] = newContext;
		newSheet->addContext( newContext );
	}

	// assign contexts between each other (like associated voice of lyrics context etc.)
	for (int i=0; i<contextCount(); i++) {
		if ( newSheet->contextAt(i)->contextType()==CAContext::LyricsContext ) {
			CAVoice *voice = voiceMap[ static_cast<CALyricsContext*>(newSheet->contextAt(i))->associatedVoice() ];
			static_cast<CALyricsContext*>(newSheet->contextAt(i))->setAssociatedVoice(voice);
			if (voice)
				voice->removeLyricsContext(static_cast<CALyricsContext*>(contextAt(i)));
		}
	}

	return newSheet;
}

void CASheet::addContext(CAContext *c) {
	_contextList.append(c);
	c->setSheet(this);
}

void CASheet::insertContextAfter(CAContext *after, CAContext *c) {
	int idx = _contextList.indexOf(after);
	_contextList.insert(idx+1, c);
}

/*!
	Appends a new staff to the sheet with one empty voice.
 */
CAStaff *CASheet::addStaff() {
	CAStaff *s = new CAStaff( QObject::tr("Staff%1").arg(staffCount()+1), this );
	s->addVoice();

	_contextList.append(s);

	return s;
}

void CASheet::clear() {
	for (int i=0; i<_contextList.size(); i++) {
		_contextList[i]->clear();
		delete _contextList[i];
	}

	_contextList.clear();
}

CAContext *CASheet::context(const QString name) {
	for (int i=0; i<_contextList.size(); i++)
		if (_contextList[i]->name() == name)
			return _contextList[i];

	return 0;
}

/*!
	Returns a list of notes and rests (chord) for all the voices in all the staffs
	in the given time slice \a time.

	This is useful for determination of the harmony at certain point in time.

	\sa CAStaff:getChord(), CAVoice::getChord()
*/
QList<CAPlayable*> CASheet::getChord(int time) {
	QList<CAPlayable*> chordList;
	for (int i=staffCount()-1; i>=0; i--) {
		chordList << staffAt(i)->getChord(time);
	}

	return chordList;
}

/*!
	Returns the list of all the voices in the sheets staffs.
*/
QList<CAVoice*> CASheet::voiceList() {
	QList<CAVoice*> list;
	for (int i=0; i<staffCount(); i++)
		list << staffAt(i)->voiceList();

	return list;
}

int CASheet::staffCount() {
	int sum=0;
	for (int i=0; i<_contextList.size(); i++) {
		if ( _contextList[i]->contextType()==CAContext::Staff )
			sum++;
	}
	return sum;
}

CAStaff *CASheet::staffAt(int idx) {
	int sum=0;
	for (int i=0; i<_contextList.size(); i++) {
		if ( _contextList[i]->contextType()==CAContext::Staff ) {
			if(sum == idx)
				return static_cast<CAStaff*>(_contextList[i]);
			sum++;
		}
	}

	return 0;
}

QList<CAStaff*> CASheet::staffList() {
	QList<CAStaff*> staffList;

	for (int i=0; i<_contextList.size(); i++) {
		if ( _contextList[i]->contextType()==CAContext::Staff ) {
			staffList << static_cast<CAStaff*>(_contextList[i]);
		}
	}

	return staffList;
}

/*!
	\fn CASheet::context(const QString name)
	Looks up for the context with the given name.

	\sa contextAt(), _contextList
*/

/*!
	\fn CASheet::contextAt(int i)
	Return the context with index \a i counting from 0.

	\sa context(), _contextList
*/

/*!
	\var CASheet::_contextList
	List of all the contexts in the sheet (lyrics, staffs, tablatures, general-bas
	marks etc.).

	\sa context(), contextAt(), contextCount(), _staffList
*/
