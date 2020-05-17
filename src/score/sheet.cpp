/*!
	Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QHash> // used for mapping when cloning the sheet to a new sheet
#include <QObject> // QObject::tr

#include "score/context.h"
#include "score/document.h"
#include "score/lyricscontext.h"
#include "score/notecheckererror.h"
#include "score/sheet.h"
#include "score/staff.h"
#include "score/tempo.h"
#include "score/voice.h"

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
CASheet::CASheet(const QString name, CADocument* doc)
{
    _name = name;
    _document = doc;
}

CASheet::~CASheet()
{
}

/*!
	Clones the current sheet with all its content.
	If a new parent document \a doc is given, it also sets the document.
*/
CASheet* CASheet::cloneSheet(CADocument* doc)
{
    auto newSheet = std::make_shared<CASheet>(name(), doc);

    QHash<CAContext*, CAContext*> contextMap; // map between oldContexts<->cloned contexts
    QHash<CAVoice*, CAVoice*> voiceMap; // map between oldVoices<->cloned voices

    // create clones of contexts
    for (int i = 0; i < contextList().size(); i++) {
        std::shared_ptr<CAContext> newContext = contextList()[i]->cloneRealContext(newSheet.get());
        if (newContext->contextType() == CAContext::Staff) {
            CAStaff *staff = static_cast<CAStaff*>(contextList()[i]),
                    *clonedStaff = static_cast<CAStaff*>(newContext.get());
            for (int j = 0; j < staff->voiceList().size(); j++) {
                auto clonedVoice = clonedStaff->voiceList()[j];
                auto voicePosition = staff->voiceList()[j];
                voiceMap.insert(voicePosition, clonedVoice);
            }
        }
        contextMap[contextList()[i]] = newContext.get();
        newSheet->addContext(newContext.get());
    }

    // assign contexts between each other (like associated voice of lyrics context etc.)
    for (int i = 0; i < contextList().size(); i++) {
        if (newSheet->contextList()[i]->contextType() == CAContext::LyricsContext) {
            CAVoice *voice = voiceMap[static_cast<CALyricsContext*>(newSheet->contextList()[i])->associatedVoice()];
            static_cast<CALyricsContext*>(newSheet->contextList()[i])->setAssociatedVoice(voice);
            if (voice)
                voice->removeLyricsContext(static_cast<CALyricsContext*>(contextList()[i]));
        }
    }

    return newSheet.get();
}

/*!
	Appends a new staff to the sheet with one empty voice.
 */
CAStaff* CASheet::addStaff()
{
    auto s = std::make_shared<CAStaff>(QObject::tr("Staff%1").arg(staffList().size() + 1), this);
    s->addVoice();

    _contextList.append(s.get());
    _contextListShared.append(s);

    return s.get();
}

void CASheet::clear()
{
    for (int i = 0; i < _contextList.size(); i++) {
        _contextList[i]->clear();
    }

    _contextList.clear();
}

/*!
	Returns the first context with the given \a name.
 */
CAContext* CASheet::findContext(const QString name)
{
    for (int i = 0; i < _contextList.size(); i++)
        if (_contextList[i]->name() == name)
            return _contextList[i];

    return nullptr;
}

/*!
 * Finds unique context name given the mask and starting with 1.
 *
 * This is used when inserting a new context and we want to assign it a unique name.
 *
 * \param mask Context name with %1 placeholder for unique number
 * \return Unique context name
 */
QString CASheet::findUniqueContextName(const QString mask)
{
    int i = 1;
    while (findContext(mask.arg(i))) {
        i++;
    }

    return mask.arg(i);
}

/*!
	Returns a list of notes and rests (chord) for all the voices in all the staffs
	in the given time slice \a time.

	This is useful for determination of the harmony at certain point in time.

	\sa CAStaff:getChord(), CAVoice::getChord()
*/
QList<CAPlayable*> CASheet::getChord(int time)
{
    QList<CAPlayable*> chordList;
    QList<CAStaff*> staffs = staffList();
    for (int i = staffs.size() - 1; i >= 0; i--) {
        chordList << staffs[i]->getChord(time);
    }

    return chordList;
}

/*!
	Returns the Tempo element active at the given time.
 */
CATempo* CASheet::getTempo(int time)
{
    CATempo* tempo = nullptr;
    for (int i = 0; i < staffList().size(); i++) {
        CATempo* t = staffList()[i]->getTempo(time);
        if (t && (!tempo || t->timeStart() > tempo->timeStart())) {
            tempo = t;
        }
    }

    return tempo;
}

/*!
	Returns the list of all the voices in the sheets staffs.
*/
QList<CAVoice*> CASheet::voiceList()
{
    QList<CAVoice*> list;
    QList<CAStaff*> staffs = staffList();
    for (int i = 0; i < staffs.size(); i++)
        list << staffs[i]->voiceList();

     return list;
}

QList<CAStaff*> CASheet::staffList()
{
    QList<CAStaff*> staffList;

    for (int i = 0; i < _contextList.size(); i++) {
        if (_contextList[i]->contextType() == CAContext::Staff) {
            staffList << static_cast<CAStaff*>(_contextList[i]);
        }
    }

    return staffList;
}

/*!
	Inserts the given context \a c after the context \a after.
 */
void CASheet::insertContextAfter(CAContext* after, CAContext* c)
{
    int idx = _contextList.indexOf(after);
    if (idx == -1) {
        _contextList.prepend(c);
    } else {
        _contextList.insert(idx + 1, c);
    }
}

/*!
 * Removes any note checker errors in the current sheet.
 * This function is usually called when changing the score and before re-running
 * the note checker.
 */
void CASheet::clearNoteCheckerErrors()
{
    for (int i = 0; i < _noteCheckerErrorList.size(); i++) {
        _noteCheckerErrorList.takeAt(i); // also removes an instance from _noteCheckerErrorList
        i--;
    }
}
