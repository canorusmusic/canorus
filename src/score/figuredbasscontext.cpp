/*!
	Copyright (c) 2009-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/figuredbasscontext.h"
#include "score/figuredbassmark.h"
#include "score/playable.h"
#include "score/playablelength.h"
#include "score/sheet.h"

/*!
	\class CAFiguredBassContext
	\brief Context for keeping the figured bass marks

	This class represents a container for the figured bass (or general bass) marks.
	It is somehow similar to CALyricsContext in terms of use.
	The class keeps all figured bass marks in a single list. No specific times are stored.
	This is because exactly one figured bass mark is assigned to every chord.
 */

CAFiguredBassContext::CAFiguredBassContext(QString name, CASheet* sheet)
    : CAContext(name, sheet)
{
    setContextType(FiguredBassContext);
    repositFiguredBassMarks();
}

CAFiguredBassContext::~CAFiguredBassContext()
{
    clear();
}

/*!
	Inserts the given figured bass mark \a m according to its timeStart.
	Replaces any existing figured bass marks at that time, if \a replace is True (default).
 */
void CAFiguredBassContext::addFiguredBassMark(CAFiguredBassMark* m, bool replace)
{
    int i;
    for (i = 0; i < _figuredBassMarkList.size() && _figuredBassMarkList[i]->timeStart() < m->timeStart(); i++)
        ;
    //int s = _figuredBassMarkList.size();
    if (i < _figuredBassMarkList.size() && replace) {
        _figuredBassMarkList.takeAt(i);
    }
    _figuredBassMarkList.insert(i, m);
    for (i++; i < _figuredBassMarkList.size(); i++)
        _figuredBassMarkList[i]->setTimeStart(_figuredBassMarkList[i]->timeStart() + m->timeLength());
}

/*!
	Inserts an empty figured bass mark and shifts the marks after.
	This function is usually called when initializing the context.
*/
void CAFiguredBassContext::addEmptyFiguredBassMark(int timeStart, int timeLength)
{
    int i;
    for (i = 0; i < _figuredBassMarkList.size() && _figuredBassMarkList[i]->timeStart() < timeStart; i++)
        ;
    auto newFBM = std::make_shared<CAFiguredBassMark>(this, timeStart, timeLength);
    _figuredBassMarkList.insert(i, newFBM.get());
    _figuredBassMarkListShared.insert(i, newFBM);
    for (i++; i < _figuredBassMarkList.size(); i++)
        _figuredBassMarkList[i]->setTimeStart(_figuredBassMarkList[i]->timeStart() + timeLength);
}

/*!
	Updates timeStarts and timeLength of all figured bass marks according to the chords they belong.
	Adds new empty figured bass marks at the end, if needed.

 	\sa CALyricsContext::repositSyllables(), CAFunctionMarkContext::repositFunctions(), CAChordNameContext::repositChordNames()
 */
void CAFiguredBassContext::repositFiguredBassMarks()
{
    if (!sheet()) {
        return;
    }

    QList<CAPlayable*> chord = sheet()->getChord(0);
    int fbmIdx = 0;
    while (chord.size()) {
        int maxTimeStart = chord[0]->timeStart();
        int minTimeEnd = chord[0]->timeEnd();
        bool notes = false; // are notes present in the chord or only rests?
        for (int i = 1; i < chord.size(); i++) {
            if (chord[i]->musElementType() == CAMusElement::Note) {
                notes = true;
            }

            if (chord[i]->timeStart() > maxTimeStart) {
                maxTimeStart = chord[i]->timeStart();
            }
            if (chord[i]->timeEnd() < minTimeEnd) {
                minTimeEnd = chord[i]->timeEnd();
            }
        }

        // only assign figured bass marks under the notes
        if (notes) {
            // add new empty figured bass, if none exist
            if (fbmIdx == _figuredBassMarkList.size()) {
                addEmptyFiguredBassMark(maxTimeStart, minTimeEnd - maxTimeStart);
            }

            CAFiguredBassMark* mark = _figuredBassMarkList[fbmIdx];
            mark->setTimeStart(maxTimeStart);
            mark->setTimeLength(minTimeEnd - maxTimeStart);
            fbmIdx++;
        }

        chord = sheet()->getChord(minTimeEnd);
    }

    // updated times for the figured bass marks at the end (after the score)
    for (; fbmIdx < _figuredBassMarkList.size(); fbmIdx++) {
        _figuredBassMarkList[fbmIdx]->setTimeStart(((fbmIdx > 0) ? _figuredBassMarkList[fbmIdx - 1] : _figuredBassMarkList[0])->timeEnd());
        _figuredBassMarkList[fbmIdx]->setTimeLength(CAPlayableLength::Quarter);
    }
}

/*!
	Returns figured bass mark at the given \a time.
 */
CAFiguredBassMark* CAFiguredBassContext::figuredBassMarkAtTimeStart(int time)
{
    int i;
    for (i = 0; i < _figuredBassMarkList.size() && _figuredBassMarkList[i]->timeStart() <= time; i++)
        ;
    if (i > 0 && _figuredBassMarkList[--i]->timeEnd() > time) {
        return _figuredBassMarkList[i];
    } else {
        return nullptr;
    }
}

std::shared_ptr<CAContext> CAFiguredBassContext::cloneRealContext(CASheet* s)
{
    return cloneFBC(s);
}

std::shared_ptr<CAFiguredBassContext> CAFiguredBassContext::cloneFBC(CASheet *s)
{
    std::shared_ptr<CAFiguredBassContext> newFbc = std::make_shared<CAFiguredBassContext>(name(), s);

    for (int i = 0; i < _figuredBassMarkList.size(); i++) {
        auto newFbm = _figuredBassMarkList[i]->cloneFBM(newFbc.get());
        newFbc->addFiguredBassMark(newFbm.get());
    }
    return newFbc;
}

void CAFiguredBassContext::clear()
{
    while (!_figuredBassMarkList.isEmpty()) {
        _figuredBassMarkList.takeFirst();
        _figuredBassMarkListShared.takeFirst();
    }
}

CAMusElement* CAFiguredBassContext::next(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::FiguredBassMark)
        return nullptr;

    int i = _figuredBassMarkList.indexOf(static_cast<CAFiguredBassMark*>(elt));
    if (i != -1 && ++i < _figuredBassMarkList.size())
        return _figuredBassMarkList[i];
    else
        return nullptr;
}

CAMusElement* CAFiguredBassContext::previous(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::FiguredBassMark)
        return nullptr;

    int i = _figuredBassMarkList.indexOf(static_cast<CAFiguredBassMark*>(elt));
    if (i != -1 && --i > -1)
        return _figuredBassMarkList[i];
    else
        return nullptr;
}

bool CAFiguredBassContext::remove(CAMusElement* elt)
{
    if (!elt || elt->musElementType() != CAMusElement::FiguredBassMark)
        return false;

    bool success = false;
    for (int i=0; i<_figuredBassMarkListShared.size(); i++) {
        if (_figuredBassMarkListShared[i].get() == elt) {
            _figuredBassMarkListShared.removeAt(i);
            i--;
        }
    }
    success = _figuredBassMarkList.removeAll(static_cast<CAFiguredBassMark*>(elt));


    return success;
}
