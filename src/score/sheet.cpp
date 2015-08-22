/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QHash>   // used for mapping when cloning the sheet to a new sheet
#include <QObject> // QObject::tr

#include "score/document.h"
#include "score/context.h"
#include "score/staff.h"
#include "score/sheet.h"
#include "score/voice.h"
#include "score/lyricscontext.h"
#include "score/tempo.h"
#include "score/notecheckererror.h"

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

CASheet::~CASheet() {

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
	for (int i=0; i<contextList().size(); i++) {
		CAContext *newContext = contextList()[i]->clone( newSheet );
		if ( newContext->contextType()==CAContext::Staff ) {
			for (int j=0; j<static_cast<CAStaff*>(contextList()[i])->voiceList().size(); j++) {
				voiceMap[ static_cast<CAStaff*>(contextList()[i])->voiceList()[j] ] = static_cast<CAStaff*>(newContext)->voiceList()[j];
			}
		}
		contextMap[contextList()[i]] = newContext;
		newSheet->addContext( newContext );
	}

	// assign contexts between each other (like associated voice of lyrics context etc.)
	for (int i=0; i<contextList().size(); i++) {
		if ( newSheet->contextList()[i]->contextType()==CAContext::LyricsContext ) {
			CAVoice *voice = voiceMap[ static_cast<CALyricsContext*>(newSheet->contextList()[i])->associatedVoice() ];
			static_cast<CALyricsContext*>(newSheet->contextList()[i])->setAssociatedVoice(voice);
			if (voice)
				voice->removeLyricsContext(static_cast<CALyricsContext*>(contextList()[i]));
		}
	}

	return newSheet;
}

/*!
	Appends a new staff to the sheet with one empty voice.
 */
CAStaff *CASheet::addStaff() {
	CAStaff *s = new CAStaff( QObject::tr("Staff%1").arg(staffList().size()+1), this );
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

/*!
	Returns the first context with the given \a name.
 */
CAContext *CASheet::findContext(const QString name) {
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
	QList<CAStaff*> staffs = staffList();
	for (int i=staffs.size()-1; i>=0; i--) {
		chordList << staffs[i]->getChord(time);
	}

	return chordList;
}

/*!
	Returns the Tempo element active at the given time.
 */
CATempo *CASheet::getTempo( int time ) {
	CATempo *tempo = 0;
	for (int i=0; i<staffList().size(); i++) {
		CATempo *t = staffList()[i]->getTempo(time);
		if ( t && (!tempo || t->timeStart() > tempo->timeStart()) ) {
			tempo = t;
		}
	}

	return tempo;
}

/*!
	Returns the list of all the voices in the sheets staffs.
*/
QList<CAVoice*> CASheet::voiceList() {
	QList<CAVoice*> list;
	QList<CAStaff*> staffs = staffList();
	for (int i=0; i<staffs.size(); i++)
		list << staffs[i]->voiceList();

	return list;
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
	Inserts the given context \a c after the context \a after.
 */
void CASheet::insertContextAfter( CAContext *after, CAContext *c ) {
	int idx = _contextList.indexOf(after);
	if (idx==-1) {
		_contextList.prepend(c);
	} else {
		_contextList.insert(idx+1, c);
	}
}

/*!
 * Removes any note checker errors in the current sheet.
 * This function is usually called when changing the score and before re-running
 * the note checker.
 */
void CASheet::clearNoteCheckerErrors() {
	for (int i=0; i<_noteCheckerErrorList.size(); i++) {
		delete _noteCheckerErrorList[i]; // delete also remove an instance from _noteCheckerErrorList
		i--;
	}
}
